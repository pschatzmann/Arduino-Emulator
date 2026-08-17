/*
	SignalHandler.h
	Copyright (c) 2025 Phil Schatzmann. All right reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#pragma once
#include <signal.h>
#include <csignal>
#include <functional>
#include <vector>
#include <map>
#include <algorithm>
#include <mutex>
#include <thread>
#include <unistd.h>

#undef INADDR_NONE

// Generic signal handler utility
//
// dispatch() used to run the registered handlers (std::map lookup,
// std::function calls) and call exit(0) directly from inside the real OS
// signal handler. None of that is async-signal-safe: exit() in particular
// is explicitly documented as unsafe to call from a signal handler, since
// it runs atexit()/static-destructor cleanup - including destroying the
// very std::map dispatch() itself was reading. In a multi-threaded process
// (any sketch using audio/video/network libraries alongside this) a second
// signal delivered to another thread while the first is mid-exit() re-enters
// dispatch() and reads/frees that map concurrently - a genuine
// heap-use-after-free, reproducible under AddressSanitizer.
//
// Fixed with the standard self-pipe trick: the real signal handler only
// does the one thing that's actually async-signal-safe here - write() one
// byte to a pipe - and a dedicated background thread, running in ordinary
// (non-signal) context, blocks reading that pipe and does the unsafe work
// (map lookup, invoking handlers, exit()) with no signal-safety
// restrictions and no reentrancy risk.
//
// That move alone isn't enough, though: any sketch with other threads
// still running at signal time (e.g. an audio callback thread, a decoder
// thread) still races exit()'s normal C++ shutdown, which runs every
// static/global object's destructor on the reaper thread while those
// other threads may still be mid-call on the very same objects - observed
// as "pure virtual method called" (a virtual call landing on an object
// whose vtable was already torn down mid-destruction). The registered
// HandlerFunc callbacks above are the sketch's actual cleanup and still
// run first, in full; what follows uses _exit(), which ends the process
// immediately without invoking any other static destructor or atexit
// handler, so it can't race with them.
class SignalHandler {
 public:
  using HandlerFunc = std::function<void(int)>;

  static void registerHandler(int signum, HandlerFunc handler) {
    auto& vec = getHandlers()[signum];
    vec.push_back(handler);
    std::signal(signum, SignalHandler::dispatch);
    ensureReaperThread();
  }

 private:
  static std::map<int, std::vector<HandlerFunc>>& getHandlers() {
    static std::map<int, std::vector<HandlerFunc>> handlers;
    return handlers;
  }

  // Async-signal-safe: writes one byte and returns. No map access, no
  // std::function calls, no exit() - all of that is deferred to the
  // reaper thread, well outside signal-handler context.
  static void dispatch(int signum) {
    char sig = (char)signum;
    ssize_t n = write(pipeWriteFd(), &sig, 1);
    (void)n;  // nothing safe to do with a failed write() from a handler
  }

  static int& pipeWriteFd() {
    static int fd = -1;
    return fd;
  }
  static int& pipeReadFd() {
    static int fd = -1;
    return fd;
  }

  // Starts the reaper thread at most once, the first time any signal is
  // registered - safe to call from registerHandler() every time.
  static void ensureReaperThread() {
    static std::once_flag started;
    std::call_once(started, [] {
      int fds[2];
      pipe(fds);
      pipeReadFd() = fds[0];
      pipeWriteFd() = fds[1];
      std::thread([] {
        char sig;
        while (read(pipeReadFd(), &sig, 1) == 1) {
          int signum = (int)(unsigned char)sig;
          auto& handlers = getHandlers();
          auto it = handlers.find(signum);
          if (it != handlers.end()) {
            for (auto& func : it->second) {
              func(signum);
            }
          }
          // not exit(): see the class comment - avoids racing other still
          // -running threads against this thread's C++ static destructors.
          _exit(0);
        }
      }).detach();
    });
  }
};

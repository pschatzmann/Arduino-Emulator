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

#undef INADDR_NONE

// Generic signal handler utility
class SignalHandler {
 public:
  using HandlerFunc = std::function<void(int)>;

  static void registerHandler(int signum, HandlerFunc handler) {
    auto& vec = getHandlers()[signum];
    vec.push_back(handler);
    std::signal(signum, SignalHandler::dispatch);
  }

 private:
  static std::map<int, std::vector<HandlerFunc>>& getHandlers() {
    static std::map<int, std::vector<HandlerFunc>> handlers;
    return handlers;
  }
  static void dispatch(int signum) {
    auto& handlers = getHandlers();
    auto it = handlers.find(signum);
    if (it != handlers.end()) {
      for (auto& func : it->second) {
        func(signum);
      }
    }
    exit(0);
  }
};

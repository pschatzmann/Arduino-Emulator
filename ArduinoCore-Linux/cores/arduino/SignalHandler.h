#pragma once
#include <signal.h>
#include <csignal>
#include <functional>
#include <vector>
#include <map>
#include <algorithm>

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

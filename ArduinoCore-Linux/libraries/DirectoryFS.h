/*
  DirectoryFS.h
  Copyright (c) 2026 Phil Schatzmann. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/
#pragma once
#include <stdlib.h>
#include <string.h>

#include <cstdio>
#include <string>

#include "SD.h"

/**
 * @brief File system (ESP32 FS API) which is a directory of the computer:
 * open("/test.txt") opens <root>/test.txt. The root is the value of an
 * environment variable or a default.
 */
class DirectoryFS {
 public:
  DirectoryFS(const char* envName, const char* defaultRoot)
      : env_name(envName), root(defaultRoot), default_root(defaultRoot) {}

  /// Determines the root directory: the environment variable or the default
  bool beginRoot() {
    const char* env = getenv(env_name);
    root = env != nullptr && env[0] != 0 ? env : default_root;
    struct stat info;
    return stat(root.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
  }

  void end() {}

  /// Changes the root directory
  void setRoot(const char* dir) { root = dir; }
  const char* getRoot() { return root.c_str(); }

  /// Opens a file: mode "r", "w", "a", "r+", "w+" or "a+"
  File open(const char* path, const char* mode = "r", bool create = false) {
    File file;
    file.open(fullPath(path).c_str(), toFlags(mode));
    return file;
  }

  bool exists(const char* path) {
    struct stat info;
    return stat(fullPath(path).c_str(), &info) == 0;
  }

  bool remove(const char* path) { return std::remove(fullPath(path).c_str()) == 0; }

  bool rename(const char* from, const char* to) {
    return std::rename(fullPath(from).c_str(), fullPath(to).c_str()) == 0;
  }

  bool mkdir(const char* path) {
#if defined(_WIN32)
    return ::_mkdir(fullPath(path).c_str()) == 0;
#else
    return ::mkdir(fullPath(path).c_str(), 0777) == 0;
#endif
  }

  bool rmdir(const char* path) { return ::rmdir(fullPath(path).c_str()) == 0; }

  bool format() { return false; }

  size_t totalBytes() { return SD.totalBytes(); }
  size_t usedBytes() { return SD.usedBytes(); }

 protected:
  const char* env_name;
  std::string root;
  const char* default_root;

  std::string fullPath(const char* path) {
    std::string result = root;
    if (path == nullptr) return result;
    if (path[0] != '/') result += "/";
    result += path;
    return result;
  }

  static int toFlags(const char* mode) {
    if (mode == nullptr || strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0) return O_RDONLY;
    if (mode[0] == 'r') return ios::in | ios::out;               // r+
    if (mode[0] == 'w') return mode[1] == '+' ? ios::in | ios::out | ios::trunc : ios::out | ios::trunc;
    if (mode[0] == 'a') return mode[1] == '+' ? ios::in | ios::out | ios::app : ios::out | ios::app;
    return O_RDONLY;
  }
};


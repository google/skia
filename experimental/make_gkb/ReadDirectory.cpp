/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ReadDirectory.h"

#include <cassert>
#include <climits>
#include <cstring>

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

// Implementation uses POSIX.  Add non-POSIX implementations when needed.

#include <dirent.h>
#include <sys/stat.h>

static bool ends_with(const char* str, const std::string& ending) {
    assert(strlen(str) <= INT_MAX);
    assert(ending.size() <= INT_MAX);
    int offset = (int)strlen(str) - (int)ending.size();
    return offset >= 0 && 0 == strcmp(str + offset, ending.c_str());
}

std::vector<std::string> ReadDirectory(const char* dir, const char* ending) {
    std::vector<std::string> ret;
    DIR* dp = opendir(dir);
    if (!dp) {
        perror(dir);
        exit(1);
    }
    std::string ending_str(ending ? ending : "");
    while (struct dirent* ep = readdir(dp)) {
        const char* name = ep->d_name;
        if (0 != strcmp(name, ".") && 0 != strcmp(name, "..") &&
            (!ending || ends_with(name, ending_str))) {
            ret.push_back(std::string(name));
        }
    }
    (void)closedir(dp);
    return ret;
}

bool MakeDirectory(const char* path) {
    struct stat status;
    bool exists = 0 == stat(path, &status);
    return (exists && (status.st_mode & S_IFDIR) != 0) ||
           (!exists && 0 != mkdir(path, 0777));
}

#else
#error "Unsupported OS"
#endif

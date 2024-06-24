/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkGetExecutablePath.h"

#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstddef>

// Note that /proc/self/exe is Linux-specific; this won't work on other UNIX systems.

std::string SkGetExecutablePath() {
    std::string result(PATH_MAX, '\0');
    ssize_t len = readlink("/proc/self/exe", result.data(), result.size() - 1);
    if (len < 0 || static_cast<size_t>(len) >= PATH_MAX - 1) {
        result.clear();
    } else {
        result.resize(len);
    }
    return result;
}

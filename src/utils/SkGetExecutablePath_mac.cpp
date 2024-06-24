/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkGetExecutablePath.h"

#include <mach-o/dyld.h>

std::string SkGetExecutablePath() {
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    std::string result(size, '\0');
    if (_NSGetExecutablePath(result.data(), &size) != 0) {
        result.clear();
    }
    return result;
}

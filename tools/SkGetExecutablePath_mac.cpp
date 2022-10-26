/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkGetExecutablePath.h"
#include <mach-o/dyld.h>

SkString SkGetExecutablePath() {
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    SkString result(/*text=*/nullptr, size);
    if (_NSGetExecutablePath(result.writable_str(), &size) != 0) {
        result.reset();
    }
    return result;
}

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBitmapKey_DEFINED
#define SkBitmapKey_DEFINED

#include "include/core/SkRect.h"

#include <cstdint>

struct SkBitmapKey {
    SkIRect fSubset;
    uint32_t fID;
    bool operator==(const SkBitmapKey& rhs) const {
        return fID == rhs.fID && fSubset == rhs.fSubset;
    }
    bool operator!=(const SkBitmapKey& rhs) const { return !(*this == rhs); }
};


#endif  // SkBitmapKey_DEFINED

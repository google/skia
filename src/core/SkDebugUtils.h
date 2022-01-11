/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDebugUtils_DEFINED
#define SkDebugUtils_DEFINED

#include "include/core/SkTileMode.h"

static constexpr const char* SkTileModeToStr(SkTileMode tm) {
    switch (tm) {
        case SkTileMode::kClamp:  return "Clamp";
        case SkTileMode::kRepeat: return "Repeat";
        case SkTileMode::kMirror: return "Mirror";
        case SkTileMode::kDecal:  return "Decal";
    }
    SkUNREACHABLE;
}

#endif // SkDebugUtils_DEFINED

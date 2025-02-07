/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDebugUtils_DEFINED
#define SkDebugUtils_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTPin.h"

#include <cstdint>
#include <string>

static constexpr const char* SkTileModeToStr(SkTileMode tm) {
    switch (tm) {
        case SkTileMode::kClamp:  return "Clamp";
        case SkTileMode::kRepeat: return "Repeat";
        case SkTileMode::kMirror: return "Mirror";
        case SkTileMode::kDecal:  return "Decal";
    }
    SkUNREACHABLE;
}

#if defined(SK_DEBUG)
inline void SkDumpBuffer(uint8_t const* const buffer, int w, int h, int rowBytes) {
    SkASSERT(buffer);

    const std::string shades = " .:-=+*%#@";

    for (int y = 0; y < h; ++y) {
        SkString line;
        for (int x = 0; x < w; ++x) {
            uint8_t pixelValue = buffer[y * rowBytes + x];
            int idx = static_cast<int>(pixelValue * shades.length() / 256);
            idx = SkTPin(idx, 0, (int)shades.length() - 1);
            line += shades[idx];
        }
        SkDebugf("%s\n", line.c_str());
    }
}
#else
inline void SkDumpBuffer(uint8_t*, int, int, int) {}
#endif


#endif // SkDebugUtils_DEFINED

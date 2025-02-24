/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDebugUtils_DEFINED
#define SkDebugUtils_DEFINED

#include "include/core/SkTileMode.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"

#include <array>
#include <cstdint>

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
inline void SkDumpBuffer(uint8_t const* const buffer, int w, int h, int rowBytes,
                         bool dumpActualValues = false) {
    SkASSERT(buffer);

    static constexpr char shades[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint8_t pixelValue = buffer[y * rowBytes + x];
            if (dumpActualValues) {
                SkDebugf("%u\t", pixelValue);
            } else {
                int idx = static_cast<int>(pixelValue * std::size(shades) / 256);
                SkASSERT(idx >= 0 && idx < (int)std::size(shades));
                SkDebugf("%c", shades[idx]);
            }
        }
        SkDebugf("\n");
    }
}
#else
inline void SkDumpBuffer(uint8_t*, int, int, int) {}
#endif


#endif // SkDebugUtils_DEFINED

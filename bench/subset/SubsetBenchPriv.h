/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SubsetBenchPriv_DEFINED
#define SubsetBenchPriv_DEFINED

#include "SkCodec.h"
#include "SkData.h"
#include "SkImageGenerator.h"

/*
 * If we plan to decode to kIndex8, we must create a color table and pass it to the
 * bitmap when we allocate pixels.  Otherwise, we simply allocate pixels using the
 * decode info.
 */
static inline void alloc_pixels(SkBitmap* bitmap, const SkImageInfo& info, SkPMColor* colors,
        int colorCount) {
    if (kIndex_8_SkColorType == info.colorType()) {
        SkAutoTUnref<SkColorTable> colorTable(new SkColorTable(colors, colorCount));
        bitmap->allocPixels(info, nullptr, colorTable);
    } else {
        bitmap->allocPixels(info);
    }
}

#endif // SubsetBenchPriv_DEFINED

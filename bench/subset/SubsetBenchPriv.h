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
 * Convert the color type to a string
 */
static const char* get_color_name(SkColorType colorType) {
    switch(colorType) {
        case kN32_SkColorType:
            return "N32";
        case kRGB_565_SkColorType:
            return "565";
        case kGray_8_SkColorType:
            return "Gray8";
        case kIndex_8_SkColorType:
            return "Index8";
        case kAlpha_8_SkColorType:
            return "Alpha8";
        default:
            return "Unknown";
    }
}

#endif // SubsetBenchPriv_DEFINED

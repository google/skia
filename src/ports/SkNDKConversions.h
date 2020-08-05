/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNDKConversions_DEFINED
#define SkNDKConversions_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"

#include <android/bitmap.h>
#include <android/data_space.h>

namespace SkNDKConversions {
    // Supports a small subset of SkColorType. Others are treated as
    // ANDROID_BITMAP_FORMAT_NONE.
    AndroidBitmapFormat toAndroidBitmapFormat(SkColorType);

    SkColorType toColorType(AndroidBitmapFormat);

    // Treats null as ADATASPACE_SRGB.
    ADataSpace toDataSpace(SkColorSpace*);

    // Treats ADATASPACE_UNKNOWN as nullptr.
    sk_sp<SkColorSpace> toColorSpace(ADataSpace);
}

#endif // SkNDKConversions_DEFINED

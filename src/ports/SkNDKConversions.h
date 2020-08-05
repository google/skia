/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"

#include <android/bitmap.h>
#include <android/data_space.h>

namespace SkNDKConversions {
    AndroidBitmapFormat toAndroidBitmapFormat(SkColorType);
    SkColorType toColorType(AndroidBitmapFormat);

    ADataSpace toDataSpace(SkColorSpace*);
    sk_sp<SkColorSpace> toColorSpace(ADataSpace);
}

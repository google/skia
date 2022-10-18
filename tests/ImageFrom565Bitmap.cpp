/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h" // IWYU pragma: keep
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "tests/Test.h"

DEF_TEST(ImageFrom565Bitmap, r) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(
        5, 7, kRGB_565_SkColorType, kOpaque_SkAlphaType));
    bm.eraseColor(SK_ColorBLACK);
    REPORTER_ASSERT(r, bm.asImage() != nullptr);
}

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkImageInfo.h"
#include "SkImage.h"
#include "Test.h"

DEF_TEST(ImageFrom565Bitmap, r) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(
        5, 7, kRGB_565_SkColorType, kOpaque_SkAlphaType));
    bm.eraseColor(SK_ColorBLACK);
    REPORTER_ASSERT(r, SkImage::MakeFromBitmap(bm) != nullptr);
}

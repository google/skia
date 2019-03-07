// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d5547cd2b302822aa85b7b0ae3f48458
REG_FIDDLE(ImageInfo_033, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int height = 2;
    const int width = 2;
    SkImageInfo imageInfo = SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
    SkISize dimensions = imageInfo.dimensions();
    SkIRect bounds = imageInfo.bounds();
    SkIRect dimensionsAsBounds = SkIRect::MakeSize(dimensions);
    SkDebugf("dimensionsAsBounds %c= bounds\n", dimensionsAsBounds == bounds ? '=' : '!');
}
}  // END FIDDLE

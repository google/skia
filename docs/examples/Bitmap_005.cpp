// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=35ea3fed27d8db22dc00f48670de64de
REG_FIDDLE(Bitmap_move_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap original;
    if (original.tryAllocPixels(
            SkImageInfo::Make(25, 35, kRGBA_8888_SkColorType, kOpaque_SkAlphaType))) {
        SkDebugf("original has pixels before move: %s\n", original.getPixels() ? "true" : "false");
        SkBitmap copy = std::move(original);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        SkDebugf("original has pixels after move: %s\n", original.getPixels() ? "true" : "false");
        SkDebugf("copy has pixels: %s\n", copy.getPixels() ? "true" : "false");
    }
}
}  // END FIDDLE

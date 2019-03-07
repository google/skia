// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Bitmap_003, 256, 256, true, 0) {
// HASH=40afd4f1fa69e02d69d92b38252088ef
void draw(SkCanvas* canvas) {
    SkBitmap original;
    if (original.tryAllocPixels(
            SkImageInfo::Make(25, 35, kRGBA_8888_SkColorType, kOpaque_SkAlphaType))) {
        SkDebugf("original has pixels before move: %s\n", original.getPixels() ? "true" : "false");
        SkBitmap copy(std::move(original));
        #if defined(__clang__)
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "bugprone-use-after-move"
        #endif
        SkDebugf("original has pixels after move: %s\n", original.getPixels() ? "true" : "false");
        #if defined(__clang__)
        #pragma clang diagnostic pop
        #endif
        SkDebugf("copy has pixels: %s\n", copy.getPixels() ? "true" : "false");
    }
}
}

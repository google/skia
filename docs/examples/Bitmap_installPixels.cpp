// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8c4f7bf73fffa1a812ee8e88e44e639c
REG_FIDDLE(Bitmap_installPixels, 256, 256, true, 0) {
static void releaseProc(void* addr, void* ) {
    SkDebugf("releaseProc called\n");
    delete[] (uint32_t*) addr;
}

void draw(SkCanvas* canvas) {
   SkBitmap bitmap;
   void* pixels = new uint32_t[8 * 8];
   SkImageInfo info = SkImageInfo::MakeN32(8, 8, kOpaque_SkAlphaType);
   SkDebugf("before installPixels\n");
   bool installed = bitmap.installPixels(info, pixels, 16, releaseProc, nullptr);
   SkDebugf("install " "%s" "successful\n", installed ? "" : "not ");
}
}  // END FIDDLE

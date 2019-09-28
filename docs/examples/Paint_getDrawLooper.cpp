// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=af4c5acc7a91e7f23c2af48018903ad4
REG_FIDDLE(Paint_getDrawLooper, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
   SkPaint paint;
   SkDebugf("nullptr %c= draw looper\n", paint.getDrawLooper() ? '!' : '=');
   SkLayerDrawLooper::Builder looperBuilder;
   paint.setDrawLooper(looperBuilder.detach());
   SkDebugf("nullptr %c= draw looper\n", paint.getDrawLooper() ? '!' : '=');
#endif
}
}  // END FIDDLE

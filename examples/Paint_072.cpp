// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Paint_072, 256, 256, true, 0);
// HASH=a1e059c8f6740fa2044cc64152b39dda
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("kSrcOver %c= getBlendMode\n",
            SkBlendMode::kSrcOver == paint.getBlendMode() ? '=' : '!');
   paint.setBlendMode(SkBlendMode::kSrc);
   SkDebugf("kSrcOver %c= getBlendMode\n",
            SkBlendMode::kSrcOver == paint.getBlendMode() ? '=' : '!');
}

}

#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8b5aa7e555a0dc31be69db7cadf471a1
REG_FIDDLE(Paint_refTypeface, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint1, paint2;
   paint1.setTypeface(SkTypeface::MakeFromName("monospace",
            SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
            SkFontStyle::kItalic_Slant)));
   SkDebugf("typeface1 %c= typeface2\n",
            paint1.getTypeface() == paint2.getTypeface() ? '=' : '!');
   paint2.setTypeface(paint1.refTypeface());
   SkDebugf("typeface1 %c= typeface2\n",
            paint1.getTypeface() == paint2.getTypeface() ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.

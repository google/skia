// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c4b2186d85c142a481298f7144295ffd
REG_FIDDLE(Paint_empty_constructor, 256, 1, false, 0) {
void draw(SkCanvas* canvas) {
    #ifndef SkUserConfig_DEFINED
    #define SkUserConfig_DEFINED
    #define SkPaintDefaults_Flags      0x01   // always enable antialiasing
    #define SkPaintDefaults_TextSize   24.f   // double default font size
    #define SkPaintDefaults_Hinting    3      // use full hinting
    #define SkPaintDefaults_MiterLimit 10.f   // use HTML Canvas miter limit setting
    #endif
}
}  // END FIDDLE

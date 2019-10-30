#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=75f13a78b28b08c72baf32b7d868de1c
REG_FIDDLE(ImageInfo_MakeUnknown, 384, 32, false, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info;  // default constructor
    SkString string;
    string.printf("SkImageInfo() %c= SkImageInfo::MakeUnknown(0, 0)",
                  info == SkImageInfo::MakeUnknown(0, 0) ? '=' : '!');
    SkPaint paint;
    canvas->drawString(string, 0, 12, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.

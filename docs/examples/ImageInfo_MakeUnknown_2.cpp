#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a1af7696ae0cdd6f379546dd1f211b7a
REG_FIDDLE(ImageInfo_MakeUnknown_2, 384, 32, false, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info;  // default constructor
    SkString string;
    string.printf("SkImageInfo() %c= SkImageInfo::MakeUnknown()",
                  info == SkImageInfo::MakeUnknown() ? '=' : '!');
    SkPaint paint;
    canvas->drawString(string, 0, 12, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.

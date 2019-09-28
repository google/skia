// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ab7e73786805c936de386b6c1ebe1f13
REG_FIDDLE(ImageInfo_reset, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(16, 8);
    SkImageInfo copy = info;
    SkDebugf("info %c= copy\n", info == copy ? '=' : '!');
    copy.reset();
    SkDebugf("info %c= reset copy\n", info == copy ? '=' : '!');
    SkDebugf("SkImageInfo() %c= reset copy\n", SkImageInfo() == copy ? '=' : '!');
}
}  // END FIDDLE

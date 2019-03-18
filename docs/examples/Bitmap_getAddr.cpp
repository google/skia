// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ffcefb2344cd38c3b99f69cfe6d64a17
REG_FIDDLE(Bitmap_getAddr, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    char* row0 = (char* ) source.getAddr(0, 0);
    char* row1 = (char* ) source.getAddr(0, 1);
    SkDebugf("addr interval %c= rowBytes\n",
             (size_t) (row1 - row0) == source.rowBytes() ? '=' : '!');
}
}  // END FIDDLE

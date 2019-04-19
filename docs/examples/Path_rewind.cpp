// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f1fedbb89da9c2a33a91805175663012
REG_FIDDLE(Path_rewind, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path1, path2;
    path1.setFillType(SkPath::kInverseWinding_FillType);
    path1.addRect({10, 20, 30, 40});
    SkDebugf("path1 %c= path2\n", path1 == path2 ? '=' : '!');
    path1.rewind();
    SkDebugf("path1 %c= path2\n", path1 == path2 ? '=' : '!');
}
}  // END FIDDLE

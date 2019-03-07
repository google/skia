// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Path_024, 256, 256, true, 0);
// HASH=f1fedbb89da9c2a33a91805175663012
void draw(SkCanvas* canvas) {
    SkPath path1, path2;
    path1.setFillType(SkPath::kInverseWinding_FillType);
    path1.addRect({10, 20, 30, 40});
    SkDebugf("path1 %c= path2\n", path1 == path2 ? '=' : '!');
    path1.rewind();
    SkDebugf("path1 %c= path2\n", path1 == path2 ? '=' : '!');
}
}

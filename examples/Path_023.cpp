// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Path_023, 256, 256, true, 0);
// HASH=8cdca35d2964bbbecb93d79a13f71c65
void draw(SkCanvas* canvas) {
    SkPath path1, path2;
    path1.setFillType(SkPath::kInverseWinding_FillType);
    path1.addRect({10, 20, 30, 40});
    SkDebugf("path1 %c= path2\n", path1 == path2 ? '=' : '!');
    path1.reset();
    SkDebugf("path1 %c= path2\n", path1 == path2 ? '=' : '!');
}
}

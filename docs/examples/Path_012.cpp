// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=019af90e778914e8a109d6305ede4fc4
REG_FIDDLE(Path_012, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkDebugf("default path fill type is %s\n",
            path.getFillType() == SkPath::kWinding_FillType ? "kWinding_FillType" :
            path.getFillType() == SkPath::kEvenOdd_FillType ? "kEvenOdd_FillType" :
            path.getFillType() == SkPath::kInverseWinding_FillType ? "kInverseWinding_FillType" :
                                                                     "kInverseEvenOdd_FillType");
}
}  // END FIDDLE

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=019af90e778914e8a109d6305ede4fc4
REG_FIDDLE(Path_getFillType, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkDebugf("default path fill type is %s\n",
            path.getFillType() == SkPathFillType::kWinding ? "kWinding" :
            path.getFillType() == SkPathFillType::kEvenOdd ? "kEvenOdd" :
            path.getFillType() == SkPathFillType::kInverseWinding ? "kInverseWinding" :
                                                                    "kInverseEvenOdd");
}
}  // END FIDDLE

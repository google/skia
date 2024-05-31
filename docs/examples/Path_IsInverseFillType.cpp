// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_IsInverseFillType, 256, 256, true, 0) {
#define nameValue(fill) { SkPathFillType::fill, #fill }

void draw(SkCanvas* canvas) {
    struct {
        SkPathFillType fill;
        const char* name;
    } fills[] = {
        nameValue(kWinding),
        nameValue(kEvenOdd),
        nameValue(kInverseWinding),
        nameValue(kInverseEvenOdd),
    };
    for (auto fill: fills ) {
        SkDebugf("IsInverseFillType(%s) == %s\n", fill.name, SkPathFillType_IsInverse(fill.fill) ?
                 "true" : "false");
    }
}
}  // END FIDDLE

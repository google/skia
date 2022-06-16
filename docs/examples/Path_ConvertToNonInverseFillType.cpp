// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=319f6b124458dcc0f9ce4d7bbde65810
REG_FIDDLE(Path_ConvertToNonInverseFillType, 256, 256, true, 0) {
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
    for (unsigned i = 0; i < std::size(fills); ++i) {
        if (fills[i].fill != (SkPathFillType) i) {
            SkDebugf("fills array order does not match FillType enum order");
            break;
        }
        SkDebugf("ConvertToNonInverseFillType(%s) == %s\n", fills[i].name,
                fills[(int) SkPathFillType_ConvertToNonInverse(fills[i].fill)].name);
    }
}
}  // END FIDDLE

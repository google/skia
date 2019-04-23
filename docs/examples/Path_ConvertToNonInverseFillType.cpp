// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=319f6b124458dcc0f9ce4d7bbde65810
REG_FIDDLE(Path_ConvertToNonInverseFillType, 256, 256, true, 0) {
#define nameValue(fill) { SkPath::fill, #fill }

void draw(SkCanvas* canvas) {
    struct {
        SkPath::FillType fill;
        const char* name;
    } fills[] = {
        nameValue(kWinding_FillType),
        nameValue(kEvenOdd_FillType),
        nameValue(kInverseWinding_FillType),
        nameValue(kInverseEvenOdd_FillType),
    };
    for (unsigned i = 0; i < SK_ARRAY_COUNT(fills); ++i) {
        if (fills[i].fill != (SkPath::FillType) i) {
            SkDebugf("fills array order does not match FillType enum order");
            break;
        }
        SkDebugf("ConvertToNonInverseFillType(%s) == %s\n", fills[i].name,
                fills[(int) SkPath::ConvertToNonInverseFillType(fills[i].fill)].name);
    }
}
}  // END FIDDLE

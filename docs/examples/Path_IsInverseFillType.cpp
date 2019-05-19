// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1453856a9d0c73e8192bf298c4143563
REG_FIDDLE(Path_IsInverseFillType, 256, 256, true, 0) {
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
    for (auto fill: fills ) {
        SkDebugf("IsInverseFillType(%s) == %s\n", fill.name, SkPath::IsInverseFillType(fill.fill) ?
                 "true" : "false");
    }
}
}  // END FIDDLE

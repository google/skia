#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=026dd8b180fe8e43f477fce43e9217b3
REG_FIDDLE(Region_Op, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkRegion operand({35, 35, 85, 85});
    const char* labels[] = {"difference", "intersect", "union", "xor", "reverse diff", "replace"};
    int index = 0;
    SkPaint paint;
    for (auto op : { SkRegion::kDifference_Op, SkRegion::kIntersect_Op, SkRegion::kUnion_Op,
                     SkRegion::kXOR_Op, SkRegion::kReverseDifference_Op, SkRegion::kReplace_Op } ) {
        SkRegion target({10, 10, 60, 60});
        target.op(operand, op);
        canvas->drawRegion(target, paint);
        canvas->drawString(labels[index++], 40, 100, paint);
        canvas->translate(80, 0);
        if (SkRegion::kUnion_Op == op) {
            canvas->translate(-240, 120);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.

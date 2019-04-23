// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=97a031f9186ade586928563840ce9116
REG_FIDDLE(Path_IsLineDegenerate, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint points[] = { {100, 100}, {100.000001f, 100.000001f}, {100.0001f, 100.0001f} };
    for (size_t i = 0; i < SK_ARRAY_COUNT(points) - 1; ++i) {
        for (bool exact : { false, true } ) {
            SkDebugf("line from (%1.8g,%1.8g) to (%1.8g,%1.8g) is %s" "degenerate, %s\n",
                    points[i].fX, points[i].fY, points[i + 1].fX, points[i + 1].fY,
                    SkPath::IsLineDegenerate(points[i], points[i + 1], exact)
                    ? "" : "not ", exact ? "exactly" : "nearly");
        }
    }
}
}  // END FIDDLE

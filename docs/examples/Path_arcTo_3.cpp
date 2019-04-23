// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0c056264a361579c18e5d02d3172d4d4
REG_FIDDLE(Path_arcTo_3, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo({156, 20});
    path.arcTo({200, 20}, {170, 20}, 50);
    SkPath::Iter iter(path, false);
    SkPoint p[4];
    SkPath::Verb verb;
    while (SkPath::kDone_Verb != (verb = iter.next(p))) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkDebugf("move to (%g,%g)\n", p[0].fX, p[0].fY);
                break;
            case SkPath::kLine_Verb:
                SkDebugf("line (%g,%g),(%g,%g)\n", p[0].fX, p[0].fY, p[1].fX, p[1].fY);
                break;
            case SkPath::kConic_Verb:
                SkDebugf("conic (%g,%g),(%g,%g),(%g,%g) weight %g\n",
                          p[0].fX, p[0].fY, p[1].fX, p[1].fY, p[2].fX, p[2].fY, iter.conicWeight());
                break;
            default:
                SkDebugf("unexpected verb\n");
        }
    }
}
}  // END FIDDLE

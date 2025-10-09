// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_arcTo_2_c, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPathBuilder()
                  .moveTo({156, 20})
                  .arcTo({200, 20}, {170, 50}, 50)
                  .detach();
    SkPath::Iter iter(path, false);
    while (auto rec = iter.next()) {
        SkSpan<const SkPoint> p = rec->fPoints;
        switch (rec->fVerb) {
            case SkPathVerb::kMove:
                SkDebugf("move to (%g,%g)\n", p[0].fX, p[0].fY);
                break;
            case SkPathVerb::kLine:
                SkDebugf("line (%g,%g),(%g,%g)\n", p[0].fX, p[0].fY, p[1].fX, p[1].fY);
                break;
            case SkPathVerb::kConic:
                SkDebugf("conic (%g,%g),(%g,%g),(%g,%g) weight %g\n",
                         p[0].fX, p[0].fY, p[1].fX, p[1].fY, p[2].fX, p[2].fY, rec->conicWeight());
                break;
            default:
                SkDebugf("unexpected verb\n");
        }
    }
}
}  // END FIDDLE

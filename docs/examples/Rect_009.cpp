// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c7065a83b220a96f903dbbb65906fe7b
REG_FIDDLE(Rect_009, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect tests[] = {{20, 40, 10, 50}, {20, 40, 20, 50}};
    for (auto rect : tests) {
        SkDebugf("rect: {%g, %g, %g, %g} is" "%s sorted\n", rect.left(), rect.top(), rect.right(),
                 rect.bottom(), rect.isSorted() ? "" : " not");
        rect.sort();
        SkDebugf("sorted: {%g, %g, %g, %g} is" "%s sorted\n", rect.left(), rect.top(), rect.right(),
                 rect.bottom(), rect.isSorted() ? "" : " not");
    }
}
}  // END FIDDLE

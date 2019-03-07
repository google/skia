// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1d7b924d6ca2a6aef09684a8a632439c
REG_FIDDLE(Rect_008, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect tests[] = {{20, 40, 10, 50}, {20, 40, 20, 50}};
    for (auto rect : tests) {
        SkDebugf("rect: {%g, %g, %g, %g} is" "%s empty\n", rect.left(), rect.top(), rect.right(),
                 rect.bottom(), rect.isEmpty() ? "" : " not");
        rect.sort();
        SkDebugf("sorted: {%g, %g, %g, %g} is" "%s empty\n", rect.left(), rect.top(), rect.right(),
                 rect.bottom(), rect.isEmpty() ? "" : " not");
    }
}
}  // END FIDDLE

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=edaad064b6de249b7a7c768dfa000adc
REG_FIDDLE(IRect_016, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect tests[] = {{20, 40, 10, 50}, {20, 40, 20, 50}};
    for (auto rect : tests) {
        SkDebugf("rect: {%d, %d, %d, %d} is" "%s empty\n", rect.left(), rect.top(), rect.right(),
                 rect.bottom(), rect.isEmpty() ? "" : " not");
        rect.sort();
        SkDebugf("sorted: {%d, %d, %d, %d} is" "%s empty\n", rect.left(), rect.top(), rect.right(),
                 rect.bottom(), rect.isEmpty() ? "" : " not");
    }
}
}  // END FIDDLE

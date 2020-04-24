// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=eb905faa1084ccab3ad0605df4c27ea4
REG_FIDDLE(IRect_isEmpty64, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect tests[] = {{20, 40, 10, 50}, {20, 40, 20, 50}};
    for (auto rect : tests) {
        SkDebugf("rect: {%d, %d, %d, %d} is" "%s empty\n", rect.left(), rect.top(), rect.right(),
                rect.bottom(), rect.isEmpty64() ? "" : " not");
        rect.sort();
        SkDebugf("sorted: {%d, %d, %d, %d} is" "%s empty\n", rect.left(), rect.top(), rect.right(),
                rect.bottom(), rect.isEmpty64() ? "" : " not");
    }
}
}  // END FIDDLE

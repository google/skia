// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=af0c66aea3ef81b709664c7007f48aae
REG_FIDDLE(Path_037, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkDebugf("empty verb count: %d\n", path.countVerbs());
    path.addRoundRect({10, 20, 30, 40}, 5, 5);
    SkDebugf("round rect verb count: %d\n", path.countVerbs());
}
}  // END FIDDLE

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_dump, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.quadTo(20, 30, 40, 50);
    for (bool dumpAsHex : { false, true } ) {
        path.dump(nullptr, dumpAsHex);
        SkDebugf("\n");
    }
}
}  // END FIDDLE

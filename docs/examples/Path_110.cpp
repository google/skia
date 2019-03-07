// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8036d764452a62f9953af50846f0f3c0
REG_FIDDLE(Path_110, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.quadTo(20, 30, 40, 50);
    for (bool forceClose : { false, true } ) {
        for (bool dumpAsHex : { false, true } ) {
            path.dump(nullptr, forceClose, dumpAsHex);
            SkDebugf("\n");
        }
    }
}
}  // END FIDDLE

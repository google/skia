// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9f459b218ec079c1ada23f4412968f9a
REG_FIDDLE(AutoCanvasRestore_restore, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    for (bool callRestore : { false, true } ) {
        for (bool saveCanvas : {false, true} ) {
            SkAutoCanvasRestore autoRestore(canvas, saveCanvas);
            if (!saveCanvas) {
                canvas->save();
            }
            SkDebugf("saveCanvas: %s  before restore: %d\n",
                   saveCanvas ? "true" : "false", canvas->getSaveCount());
            if (callRestore) autoRestore.restore();
            SkDebugf("saveCanvas: %s  after restore: %d\n",
                   saveCanvas ? "true" : "false", canvas->getSaveCount());
        }
    }
    SkDebugf("final count: %d\n", canvas->getSaveCount());
}
}  // END FIDDLE

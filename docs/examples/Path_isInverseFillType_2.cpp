// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_isInverseFillType_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkDebugf("default path fill type is inverse: %s\n",
            path.isInverseFillType() ? "true" : "false");
}
}  // END FIDDLE

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c722ebe8ac991d77757799ce29e509e1
REG_FIDDLE(Path_isVolatile, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkDebugf("volatile by default is %s\n", path.isVolatile() ? "true" : "false");
}
}  // END FIDDLE

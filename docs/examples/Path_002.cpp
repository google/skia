// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0a0026fca638d1cd75c0ab884e3ee1c6
REG_FIDDLE(Path_empty_constructor, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkDebugf("path is " "%s" "empty", path.isEmpty() ? "" : "not ");
}
}  // END FIDDLE

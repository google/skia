// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_Iter_Iter, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath::Iter iter;
    SkDebugf("iter is " "%s" "done\n", !iter.next() ? "" : "not ");
    SkPath path;
    iter.setPath(path, false);
    SkDebugf("iter is " "%s" "done\n", !iter.next() ? "" : "not ");
}
}  // END FIDDLE

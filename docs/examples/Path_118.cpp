// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=01648775cb9b354b2f1836dad82a25ab
REG_FIDDLE(Path_Iter_Iter, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath::Iter iter;
    SkPoint points[4];
    SkDebugf("iter is " "%s" "done\n", SkPath::kDone_Verb == iter.next(points) ? "" : "not ");
    SkPath path;
    iter.setPath(path, false);
    SkDebugf("iter is " "%s" "done\n", SkPath::kDone_Verb == iter.next(points) ? "" : "not ");
}
}  // END FIDDLE

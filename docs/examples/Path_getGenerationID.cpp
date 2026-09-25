// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_getGenerationID, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkDebugf("empty genID = %" PRIu64 "\n", path.getGenerationID());
    path = SkPathBuilder().lineTo(1, 2).detach();
    SkDebugf("1st lineTo genID = %" PRIu64 "\n", path.getGenerationID());
    path.reset();
    SkDebugf("empty genID = %" PRIu64 "\n", path.getGenerationID());
    path = SkPathBuilder().lineTo(1, 2).detach();
    SkDebugf("2nd lineTo genID = %" PRIu64 "\n", path.getGenerationID());
}
}  // END FIDDLE

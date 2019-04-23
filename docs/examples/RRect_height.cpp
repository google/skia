// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5a3eb1755164a7becec33cec6e6eca31
REG_FIDDLE(RRect_height, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRRect unsorted = SkRRect::MakeRect({ 15, 25, 10, 20 });
    SkDebugf("unsorted height: %g\n", unsorted.height());
    SkRRect large = SkRRect::MakeRect({ 1, -FLT_MAX, 2, FLT_MAX });
    SkDebugf("large height: %.0f\n", large.height());
}
}  // END FIDDLE

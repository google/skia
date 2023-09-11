// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(image_subsets_get_different_uids, 128, 128, true, 5) {
// image_subsets_get_different_uids
void draw(SkCanvas*) {
    SkIRect r{0, 0, 10, 10};
    auto s1 = image->makeSubset(nullptr, r);
    auto s2 = image->makeSubset(nullptr, r);
    SkDebugf("%u\n%u\n", s1->uniqueID(), s2->uniqueID());
}
}  // END FIDDLE

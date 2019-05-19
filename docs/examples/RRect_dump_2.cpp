// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=f850423c7c0c4f803d479ecd92221059
REG_FIDDLE(RRect_dump_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRRect rrect = SkRRect::MakeRect({6.f / 7, 2.f / 3, 6.f / 7, 2.f / 3});
    rrect.dump();
    SkRect bounds = SkRect::MakeLTRB(0.857143f, 0.666667f, 0.857143f, 0.666667f);
    const SkPoint corners[] = {
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
    };
    SkRRect copy;
    copy.setRectRadii(bounds, corners);
    SkDebugf("rrect is " "%s" "equal to copy\n", rrect == copy ? "" : "not ");
}
}  // END FIDDLE

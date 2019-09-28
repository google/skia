// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=265b8d23288dc8026ff788e809360af7
REG_FIDDLE(RRect_dump, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRRect rrect = SkRRect::MakeRect({6.f / 7, 2.f / 3, 6.f / 7, 2.f / 3});
    for (bool dumpAsHex : { false, true } ) {
        rrect.dump(dumpAsHex);
    }
}
}  // END FIDDLE

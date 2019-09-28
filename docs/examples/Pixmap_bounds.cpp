// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=79750fb1d898a4e5c8c828b7bc9acec5
REG_FIDDLE(Pixmap_bounds, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    for (int width : { 0, 2 } ) {
        for (int height : { 0, 2 } ) {
             SkPixmap pixmap(SkImageInfo::MakeA8(width, height), nullptr, width);
             SkDebugf("width: %d height: %d empty: %s\n", width, height,
                      pixmap.bounds().isEmpty() ? "true" : "false");
        }
    }
}
}  // END FIDDLE

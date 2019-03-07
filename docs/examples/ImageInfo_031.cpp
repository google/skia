// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(ImageInfo_031, 256, 256, true, 0) {
// HASH=b8757200da5be0b43763cf79feb681a7
void draw(SkCanvas* canvas) {
    for (int width : { 0, 2 } ) {
        for (int height : { 0, 2 } ) {
             SkImageInfo imageInfo= SkImageInfo::MakeA8(width, height);
             SkDebugf("width: %d height: %d empty: %s\n", width, height,
                      imageInfo.isEmpty() ? "true" : "false");
        }
    }
}
}

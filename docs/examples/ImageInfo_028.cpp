// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5c1d2499a4056b6cff38c1cf924158a1
REG_FIDDLE(ImageInfo_028, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* alphas[] = {"Unknown", "Opaque", "Premul", "Unpremul"};
    SkImageInfo info = SkImageInfo::MakeA8(16, 32);
    SkDebugf("alpha type: k" "%s" "_SkAlphaType\n", alphas[info.alphaType()]);
}
}  // END FIDDLE

// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c6b0f6a3f493cb08d9abcdefe12de245
REG_FIDDLE(ImageInfo_050, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(16, 8);
    for (size_t rowBytes = 60; rowBytes < 72; rowBytes += sizeof(SkPMColor)) {
        SkDebugf("validRowBytes(%llu): %s\n", rowBytes, info.validRowBytes(rowBytes) ?
                 "true" : "false");
    }
}
}  // END FIDDLE

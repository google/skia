#if 0  // disabled
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6a63dfdd62ab77ff57783af8c33d7b78
REG_FIDDLE(ImageInfo_ByteSizeOverflowed, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    if (sizeof(size_t) == 8) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(2, 1000000000);
        for (size_t rowBytes = 100000000; rowBytes < 10000000000000LL; rowBytes *= 10) {
            const size_t size = info.computeByteSize(rowBytes);
            SkDebugf("rowBytes:%llu size:%llu overflowed:%s\n", rowBytes, size,
                     SkImageInfo::ByteSizeOverflowed(size) ? "true" : "false");
        }
    }
}
}  // END FIDDLE
#endif  // disabled

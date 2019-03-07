// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(ImageInfo_049, 256, 256, true, 0);
// HASH=6a63dfdd62ab77ff57783af8c33d7b78
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(2, 1000000000);
    for (size_t rowBytes = 100000000; rowBytes < 10000000000000LL; rowBytes *= 10) {
        const size_t size = info.computeByteSize(rowBytes);
        SkDebugf("rowBytes:%llu size:%llu overflowed:%s\n", rowBytes, size,
                 SkImageInfo::ByteSizeOverflowed(size) ? "true" : "false");
    }
}
}

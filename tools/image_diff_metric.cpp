// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// Image Diff:   Provides a metric for measuring the difference between two encoded images.   Prints
// out a single floating-point number between 0.0 and 1.0; 0 means that the images are identical; 1
// means that each pixel is maximally different in each channel.  A non-zero return value indicates
// that something went wrong.

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"

#include <cmath>
#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 3) {
        const char usage[] = "\nUsage:\n  %s {FILE1}.png {FILE2}.png\n\n";
        fprintf(stderr, usage, argv[0]);
        return 1;
    }
    SkBitmap bm[2];
    for (int i = 0; i < 2; ++i) {
        const char* path = argv[i + 1];
        if (std::unique_ptr<SkCodec> codec =
                SkCodec::MakeFromData(SkData::MakeFromFileName(path))) {
            bm[i].allocN32Pixels(codec->dimensions().fWidth, codec->dimensions().fHeight);
            if (SkCodec::kSuccess == codec->getPixels(bm[i].pixmap())) {
                continue;
            }
        }
        fprintf(stderr, "\nBad file: '%s'.\n\n", path);
        return 2;
    }
    SkISize dim = bm[0].dimensions();
    if (dim != bm[1].dimensions()) {
        fprintf(stderr, "\nImages must be same size: (%d,%d) != (%d,%d)\n\n",
                dim.fWidth, dim.fHeight, bm[1].dimensions().fWidth, bm[1].dimensions().fHeight);
        return 3;
    }
    int64_t totalDiffs = 0; // Manhattan distance in ARGB color-space.
    for (int y = 0; y < dim.fHeight; ++y) {
        const uint8_t* row1 = reinterpret_cast<const uint8_t*>(bm[0].pixmap().addr32(0, y));
        const uint8_t* row2 = reinterpret_cast<const uint8_t*>(bm[1].pixmap().addr32(0, y));
        for (size_t i = 0; i < (size_t)dim.fWidth * (size_t)4; ++i) {
            totalDiffs += std::abs((int)row1[i] - (int)row2[i]);
        }
    }
    printf("%g\n", (double)totalDiffs /
                   ((uint64_t)255 * 4 * (uint64_t)dim.fWidth * (uint64_t)dim.fHeight));
    return 0;
}

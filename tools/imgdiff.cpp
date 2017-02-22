/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Provides a metric for measureing the difference between two images.
// Prints out a single floating-point number between 0.0 and 1.0.
// 0.0 means that the images are identical.
// 1.0 means that each pixel is maximally different in each channel.
// A non-zero return value indicates that something went wrong.
//
// This program is intentionally non-verbose and non-helpful, as it is intended
// for use in scripts to sort pairs of images by sameness.

#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "SkImage.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        return 1;
    }
    auto img1 = SkImage::MakeFromEncoded(SkData::MakeFromFileName(argv[1]));
    auto img2 = SkImage::MakeFromEncoded(SkData::MakeFromFileName(argv[2]));
    if (!img1 || !img2) {
        return 2;
    }
    const int w = SkTMax(img1->width(), img2->width());
    const int h = SkTMax(img1->height(), img2->height());
    const size_t N = SkToSizeT(w) * SkToSizeT(h);
    std::vector<SkPMColor> b1(N), b2(N);
    SkPixmap p1(SkImageInfo::MakeN32Premul(w, h), b1.data(), sizeof(SkPMColor) * w),
             p2(SkImageInfo::MakeN32Premul(w, h), b2.data(), sizeof(SkPMColor) * w);
    if (!img1->readPixels(p1, 0, 0) || !img2->readPixels(p2, 0, 0)) {
        return 3;
    }
    int64_t totalDiffs = 0;
    for (unsigned i = 0; i < N; ++i) {
        if (b1[i] != b2[i]) {
            SkColor c1 = SkUnPreMultiply::PMColorToColor(b1[i]),
                    c2 = SkUnPreMultiply::PMColorToColor(b2[i]);
            // Manhattan distance in ARGB color-space.
            totalDiffs += abs((int)SkColorGetA(c1) - (int)SkColorGetA(c2)) +
                          abs((int)SkColorGetR(c1) - (int)SkColorGetR(c2)) +
                          abs((int)SkColorGetG(c1) - (int)SkColorGetG(c2)) +
                          abs((int)SkColorGetB(c1) - (int)SkColorGetG(c2));
        }
    }
    printf("%g\n", (double)totalDiffs / (4 * 255 * N));
    return 0;
}

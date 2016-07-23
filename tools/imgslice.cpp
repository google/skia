/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"

DEFINE_bool(header, false, "Print an extra row of the min-max values");
DEFINE_string2(label, l, "label", "Label printed as the first value");

DEFINE_string2(image, i, "", "Input image");
DEFINE_int32_2(row, r, -1, "Row to extract");
DEFINE_int32_2(column, c, -1, "Column to extract");

DEFINE_int32_2(min, n, 0, "Minimum row/column to extract - inclusive");
DEFINE_int32_2(max, m, 100, "Maximum row/column to extract - inclusive");

DEFINE_int32(rgb, 0, "Color channel to print (0->b, 1->g, 2->r, 3->a)");

DEFINE_bool2(quiet, q, false, "Quiet");
DEFINE_bool2(reverse, v, false, "Iterate from max to min");


// This tool just loads a single image and prints out a comma-separated row or column
// Return codes:
static const int kSuccess = 0;
static const int kError = 1;

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Print out a row or column of an image.");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_rgb > 3 || FLAGS_rgb < 0) {
        if (!FLAGS_quiet) {
            SkDebugf("Channel (--rgb) must be between 0 and 3 (inclusive) - value is %d.\n",
                     FLAGS_rgb);
        }
        return kError;
    }

    if (FLAGS_row >= 0 && FLAGS_column >= 0) {
        if (!FLAGS_quiet) {
            SkDebugf("Only one of '-c' or '-r' can be specified at at time.\n");
        }
        return kError;
    }

    if (FLAGS_row < 0 && FLAGS_column < 0) {
        if (!FLAGS_quiet) {
            SkDebugf("At least one of '-c' or '-r' need to be specified.\n");
        }
        return kError;
    }

    sk_sp<SkData> data(SkData::MakeFromFileName(FLAGS_image[0]));
    if (nullptr == data) {
        if (!FLAGS_quiet) {
            SkDebugf("Couldn't open file: %s\n", FLAGS_image[0]);
        }
        return kError;
    }

    sk_sp<SkImage> image(SkImage::MakeFromEncoded(data));
    if (!image) {
        if (!FLAGS_quiet) {
            SkDebugf("Couldn't create image for: %s.\n", FLAGS_image[0]);
        }
        return kError;
    }

    SkBitmap bitmap;
    if (!image->asLegacyBitmap(&bitmap, SkImage::kRW_LegacyBitmapMode)) {
        if (!FLAGS_quiet) {
            SkDebugf("Couldn't create bitmap for: %s.\n", FLAGS_image[0]);
        }
        return kError;
    }

    int top, bottom, left, right;

    if (-1 != FLAGS_row) {
        SkASSERT(-1 == FLAGS_column);

        top = bottom = SkTPin(FLAGS_row, 0, bitmap.height()-1);
        FLAGS_min = left = SkTPin(FLAGS_min, 0, bitmap.width()-1);
        FLAGS_max = right = SkTPin(FLAGS_max, left, bitmap.width()-1);
    } else {
        SkASSERT(-1 != FLAGS_column);
        left = right = SkTPin(FLAGS_column, 0, bitmap.width()-1);
        FLAGS_min = top = SkTPin(FLAGS_min, 0, bitmap.height()-1);
        FLAGS_max = bottom = SkTPin(FLAGS_max, top, bitmap.height()-1);
    }

    if (FLAGS_header) {
        SkDebugf("header");
        if (FLAGS_reverse) {
            for (int i = FLAGS_max; i >= FLAGS_min; --i) {
                SkDebugf(", %d", i);
            }
        } else {
            for (int i = FLAGS_min; i <= FLAGS_max; ++i) {
                SkDebugf(", %d", i);
            }
        }
        SkDebugf("\n");
    }

    SkDebugf("%s", FLAGS_label[0]);
    if (FLAGS_reverse) {
        for (int y = bottom; y >= top; --y) {
            for (int x = right; x >= left; --x) {
                SkColor c = bitmap.getColor(x, y);

                SkDebugf(", %d", ((c) >> (FLAGS_rgb*8)) & 0xFF);
            }
        }
    } else {
        for (int y = top; y <= bottom; ++y) {
            for (int x = left; x <= right; ++x) {
                SkColor c = bitmap.getColor(x, y);

                SkDebugf(", %d", ((c) >> (FLAGS_rgb*8)) & 0xFF);
            }
        }
    }

    SkDebugf("\n");

    return kSuccess;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLinearBitmapPipeline.h"
#include "SkColor.h"
#include "SkPM4f.h"
#include "Test.h"

using Pixel = float[4];
DEF_TEST(SkBitmapFP, reporter) {

    int width = 10;
    int height = 10;
    uint32_t* bitmap = new uint32_t[width * height];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bitmap[y * width + x] = (y << 8) + x + (128<<24);
        }
    }

    SkPM4f* FPbuffer = new SkPM4f[width * height];

    SkMatrix m = SkMatrix::I();
    //m.setRotate(30.0f, 1.0f, 1.0f);
    SkMatrix invert;
    bool trash = m.invert(&invert);
    sk_ignore_unused_variable(trash);

    const SkImageInfo info =
        SkImageInfo::MakeN32Premul(width, height, kLinear_SkColorProfileType);

    SkPixmap srcPixmap{info, bitmap, static_cast<size_t>(4 * width)};

    SkLinearBitmapPipeline pipeline{invert, kNone_SkFilterQuality, SkShader::kClamp_TileMode,
                                    SkShader::kClamp_TileMode, srcPixmap};

    int count = 10;

    pipeline.shadeSpan4f(3, 6, FPbuffer, count);
#if 0
    Pixel* pixelBuffer = (Pixel*)FPbuffer;
    for (int i = 0; i < count; i++) {
        printf("i: %d - (%g, %g, %g, %g)\n", i,
               pixelBuffer[i][0] * 255.0f,
               pixelBuffer[i][1] * 255.0f,
               pixelBuffer[i][2] * 255.0f,
               pixelBuffer[i][3] * 255.0f);
    }
#endif

    delete [] bitmap;
    delete [] FPbuffer;
}


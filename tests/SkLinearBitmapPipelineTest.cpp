/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkLinearBitmapPipeline.h"

#include "SkColor.h"

#include "Test.h"

struct SinkBilerpProcessor final : public PointProcessorInterface {
    void pointListFew(int n, Sk4fArg xs, Sk4fArg ys) override { fXs = xs; fYs = ys; }
    void pointList4(Sk4fArg Xs, Sk4fArg Ys) override { fXs = Xs; fYs = Ys; }

    Sk4f fXs;
    Sk4f fYs;
};

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

    SkLinearBitmapPipeline pipeline{invert, SkShader::kClamp_TileMode,
                                    SkShader::kClamp_TileMode, info, bitmap};

    int count = 10;

    pipeline.shadeSpan4f(3, 6, FPbuffer, count);

    Pixel* pixelBuffer = (Pixel*)FPbuffer;
    for (int i = 0; i < count; i++) {
        printf("i: %d - (%g, %g, %g, %g)\n", i,
               pixelBuffer[i][0] * 255.0f,
               pixelBuffer[i][1] * 255.0f,
               pixelBuffer[i][2] * 255.0f,
               pixelBuffer[i][3] * 255.0f);
    }

    delete [] bitmap;
    delete [] FPbuffer;
}


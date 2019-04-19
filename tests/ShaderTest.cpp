/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkPerlinNoiseShader.h"
#include "SkRRect.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "SkData.h"

static void check_isaimage(skiatest::Reporter* reporter, SkShader* shader,
                           int expectedW, int expectedH,
                           SkTileMode expectedX, SkTileMode expectedY,
                           const SkMatrix& expectedM) {
    SkTileMode tileModes[2];
    SkMatrix localM;

    // wack these so we don't get a false positive
    localM.setScale(9999, -9999);
    tileModes[0] = tileModes[1] = (SkTileMode)99;

    SkImage* image = shader->isAImage(&localM, tileModes);
    REPORTER_ASSERT(reporter, image);
    REPORTER_ASSERT(reporter, image->width() == expectedW);
    REPORTER_ASSERT(reporter, image->height() == expectedH);
    REPORTER_ASSERT(reporter, localM == expectedM);
    REPORTER_ASSERT(reporter, tileModes[0] == expectedX);
    REPORTER_ASSERT(reporter, tileModes[1] == expectedY);
}

DEF_TEST(Shader_isAImage, reporter) {
    const int W = 100;
    const int H = 100;
    SkBitmap bm;
    bm.allocN32Pixels(W, H);
    auto img = SkImage::MakeFromBitmap(bm);
    const SkMatrix localM = SkMatrix::MakeScale(2, 3);
    const SkTileMode tmx = SkTileMode::kRepeat;
    const SkTileMode tmy = SkTileMode::kMirror;

    auto shader0 = bm.makeShader(tmx, tmy, &localM);
    auto shader1 = SkImage::MakeFromBitmap(bm)->makeShader(tmx, tmy, &localM);

    check_isaimage(reporter, shader0.get(), W, H, tmx, tmy, localM);
    check_isaimage(reporter, shader1.get(), W, H, tmx, tmy, localM);
}

// Make sure things are ok with just a single leg.
DEF_TEST(ComposeShaderSingle, reporter) {
    SkBitmap srcBitmap;
    srcBitmap.allocN32Pixels(10, 10);
    srcBitmap.eraseColor(SK_ColorRED);
    SkCanvas canvas(srcBitmap);
    SkPaint p;
    p.setShader(
        SkShaders::Blend(SkBlendMode::kClear,
        SkShaders::Empty(),
        SkPerlinNoiseShader::MakeFractalNoise(1.0f, 1.0f, 2, 0.0f)));
    SkRRect rr;
    SkVector rd[] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    rr.setRectRadii({0, 0, 0, 0}, rd);
    canvas.drawRRect(rr, p);
}

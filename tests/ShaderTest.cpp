/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkBitmap.h"
#include "SkImage.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "SkData.h"

static void check_isabitmap(skiatest::Reporter* reporter, SkShader* shader,
                            int expectedW, int expectedH,
                            SkShader::TileMode expectedX, SkShader::TileMode expectedY,
                            const SkMatrix& expectedM) {
    SkShader::TileMode tileModes[2];
    SkMatrix localM;

#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
    SkBitmap bm;
    REPORTER_ASSERT(reporter, shader->isABitmap(&bm, &localM, tileModes));
    REPORTER_ASSERT(reporter, bm.width() == expectedW);
    REPORTER_ASSERT(reporter, bm.height() == expectedH);
    REPORTER_ASSERT(reporter, localM == expectedM);
    REPORTER_ASSERT(reporter, tileModes[0] == expectedX);
    REPORTER_ASSERT(reporter, tileModes[1] == expectedY);
#endif

    // wack these so we don't get a false positive
    localM.setScale(9999, -9999);
    tileModes[0] = tileModes[1] = (SkShader::TileMode)99;

    SkImage* image = shader->isAImage(&localM, tileModes);
    REPORTER_ASSERT(reporter, image);
    REPORTER_ASSERT(reporter, image->width() == expectedW);
    REPORTER_ASSERT(reporter, image->height() == expectedH);
    REPORTER_ASSERT(reporter, localM == expectedM);
    REPORTER_ASSERT(reporter, tileModes[0] == expectedX);
    REPORTER_ASSERT(reporter, tileModes[1] == expectedY);
}

DEF_TEST(Shader_isABitmap, reporter) {
    const int W = 100;
    const int H = 100;
    SkBitmap bm;
    bm.allocN32Pixels(W, H);
    auto img = SkImage::MakeFromBitmap(bm);
    const SkMatrix localM = SkMatrix::MakeScale(2, 3);
    const SkShader::TileMode tmx = SkShader::kRepeat_TileMode;
    const SkShader::TileMode tmy = SkShader::kMirror_TileMode;

    auto shader0 = SkShader::MakeBitmapShader(bm, tmx, tmy, &localM);
    auto shader1 = SkImage::MakeFromBitmap(bm)->makeShader(tmx, tmy, &localM);

    check_isabitmap(reporter, shader0.get(), W, H, tmx, tmy, localM);
    check_isabitmap(reporter, shader1.get(), W, H, tmx, tmy, localM);
}

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static sk_sp<SkImage> make_image(SkCanvas* rootCanvas, SkColor color) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    auto        surface(ToolUtils::makeSurface(rootCanvas, info));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    surface->getCanvas()->drawIRect(SkIRect::MakeXYWH(25, 25, 50, 50), paint);
    return surface->makeImageSnapshot();
}

DEF_SIMPLE_GM(localmatriximageshader, canvas, 250, 250) {
    sk_sp<SkImage> redImage = make_image(canvas, SK_ColorRED);
    SkMatrix translate = SkMatrix::Translate(100.0f, 0.0f);
    SkMatrix rotate;
    rotate.setRotate(45.0f);
    sk_sp<SkShader> redImageShader = redImage->makeShader(SkSamplingOptions(), &rotate);
    sk_sp<SkShader> redLocalMatrixShader = redImageShader->makeWithLocalMatrix(translate);

    // Rotate about the origin will happen first.
    SkPaint paint;
    paint.setShader(redLocalMatrixShader);
    canvas->drawIRect(SkIRect::MakeWH(250, 250), paint);

    sk_sp<SkImage> blueImage = make_image(canvas, SK_ColorBLUE);
    sk_sp<SkShader> blueImageShader = blueImage->makeShader(SkSamplingOptions(), &translate);
    sk_sp<SkShader> blueLocalMatrixShader = blueImageShader->makeWithLocalMatrix(rotate);

    // Translate will happen first.
    paint.setShader(blueLocalMatrixShader);
    canvas->drawIRect(SkIRect::MakeWH(250, 250), paint);

    canvas->translate(100.0f, 0.0f);

    // Use isAImage() and confirm that the shaders will draw exactly the same (to the right by 100).
    SkTileMode mode[2];
    SkMatrix matrix;
    SkImage* image = redLocalMatrixShader->isAImage(&matrix, mode);
    paint.setShader(image->makeShader(mode[0], mode[1], SkSamplingOptions(), &matrix));
    canvas->drawIRect(SkIRect::MakeWH(250, 250), paint);
    image = blueLocalMatrixShader->isAImage(&matrix, mode);
    paint.setShader(image->makeShader(mode[0], mode[1], SkSamplingOptions(), &matrix));
    canvas->drawIRect(SkIRect::MakeWH(250, 250), paint);
}

DEF_SIMPLE_GM(localmatriximageshader_filtering, canvas, 256, 256) {
    // Test that filtering decisions (eg bicubic for upscale) are made correctly when the scale
    // comes from a local matrix shader.
    auto image = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
    SkPaint p;
    SkMatrix m = SkMatrix::Scale(2, 2);
    p.setShader(image->makeShader(SkSamplingOptions(SkCubicResampler::Mitchell()))
                ->makeWithLocalMatrix(m));

    canvas->drawRect(SkRect::MakeXYWH(0, 0, 256, 256), p);
}

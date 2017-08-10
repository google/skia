/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkSurface.h"

static sk_sp<SkImage> make_image(SkCanvas* rootCanvas, SkColor color) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    auto surface(rootCanvas->makeSurface(info));
    if (!surface) {
        surface = SkSurface::MakeRaster(info);
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);
    surface->getCanvas()->drawIRect(SkIRect::MakeXYWH(25, 25, 50, 50), paint);
    return surface->makeImageSnapshot();
}

DEF_SIMPLE_GM(localmatriximageshader, canvas, 250, 250) {
    sk_sp<SkImage> redImage = make_image(canvas, SK_ColorRED);
    SkMatrix translate = SkMatrix::MakeTrans(100.0f, 0.0f);
    SkMatrix rotate;
    rotate.setRotate(45.0f);
    sk_sp<SkShader> redImageShader = redImage->makeShader(&translate);
    sk_sp<SkShader> redLocalMatrixShader = redImageShader->makeWithLocalMatrix(rotate);

    // Rotate about the origin will happen first.
    SkPaint paint;
    paint.setShader(redLocalMatrixShader);
    canvas->drawIRect(SkIRect::MakeWH(250, 250), paint);

    sk_sp<SkImage> blueImage = make_image(canvas, SK_ColorBLUE);
    sk_sp<SkShader> blueImageShader = blueImage->makeShader(&rotate);
    sk_sp<SkShader> blueLocalMatrixShader = blueImageShader->makeWithLocalMatrix(translate);

    // Translate will happen first.
    paint.setShader(blueLocalMatrixShader);
    canvas->drawIRect(SkIRect::MakeWH(250, 250), paint);

    canvas->translate(100.0f, 0.0f);

    // Use isAImage() and confirm that the shaders will draw exactly the same (to the right by 100).
    SkShader::TileMode mode[2];
    SkMatrix matrix;
    SkImage* image = redLocalMatrixShader->isAImage(&matrix, mode);
    paint.setShader(image->makeShader(mode[0], mode[1], &matrix));
    canvas->drawIRect(SkIRect::MakeWH(250, 250), paint);
    image = blueLocalMatrixShader->isAImage(&matrix, mode);
    paint.setShader(image->makeShader(mode[0], mode[1], &matrix));
    canvas->drawIRect(SkIRect::MakeWH(250, 250), paint);
}

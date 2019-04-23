/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "tools/ToolUtils.h"

#include "include/core/SkColorPriv.h"
#include "include/core/SkPath.h"
#include "include/core/SkShader.h"

DEF_SIMPLE_GM_BG(bigmatrix, canvas, 50, 50, ToolUtils::color_to_565(0xFF66AA99)) {
    SkMatrix m;
    m.reset();
    m.setRotate(33 * SK_Scalar1);
    m.postScale(3000 * SK_Scalar1, 3000 * SK_Scalar1);
    m.postTranslate(6000 * SK_Scalar1, -5000 * SK_Scalar1);
    canvas->concat(m);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);

    bool success = m.invert(&m);
    SkASSERT(success);
    (void)success;  // silence compiler :(

    SkPath path;

    SkPoint  pt    = {10 * SK_Scalar1, 10 * SK_Scalar1};
    SkScalar small = 1 / (500 * SK_Scalar1);

    m.mapPoints(&pt, 1);
    path.addCircle(pt.fX, pt.fY, small);
    canvas->drawPath(path, paint);

    pt.set(30 * SK_Scalar1, 10 * SK_Scalar1);
    m.mapPoints(&pt, 1);
    SkRect rect = {pt.fX - small, pt.fY - small, pt.fX + small, pt.fY + small};
    canvas->drawRect(rect, paint);

    SkBitmap bmp;
    bmp.allocN32Pixels(2, 2);
    uint32_t* pixels = reinterpret_cast<uint32_t*>(bmp.getPixels());
    pixels[0]        = SkPackARGB32(0xFF, 0xFF, 0x00, 0x00);
    pixels[1]        = SkPackARGB32(0xFF, 0x00, 0xFF, 0x00);
    pixels[2]        = SkPackARGB32(0x80, 0x00, 0x00, 0x00);
    pixels[3]        = SkPackARGB32(0xFF, 0x00, 0x00, 0xFF);
    pt.set(30 * SK_Scalar1, 30 * SK_Scalar1);
    m.mapPoints(&pt, 1);
    SkMatrix s;
    s.reset();
    s.setScale(SK_Scalar1 / 1000, SK_Scalar1 / 1000);
    paint.setShader(bmp.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &s));
    paint.setAntiAlias(false);
    paint.setFilterQuality(kLow_SkFilterQuality);
    rect.setLTRB(pt.fX - small, pt.fY - small, pt.fX + small, pt.fY + small);
    canvas->drawRect(rect, paint);
}

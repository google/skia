/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "SkPictureFlat.h"
#include "SkShader.h"
#include "SkXfermode.h"

static void flattenFlattenableProc(SkOrderedWriteBuffer& buffer,
                                   const void* obj) {
    buffer.writeFlattenable((SkFlattenable*)obj);
}

class Controller : public SkChunkFlatController {
public:
    Controller() : INHERITED(1024) {
        this->INHERITED::setNamedFactorySet(SkNEW(SkNamedFactorySet))->unref();
    }
private:
    typedef SkChunkFlatController INHERITED;
};

/**
 * Verify that two SkFlatData objects that created from the same object are
 * identical when using an SkNamedFactorySet.
 * @param reporter Object to report failures.
 * @param obj Flattenable object to be flattened.
 * @param flattenProc Function that flattens objects with the same type as obj.
 */
static void testCreate(skiatest::Reporter* reporter, const void* obj,
                       void (*flattenProc)(SkOrderedWriteBuffer&, const void*)) {
    Controller controller;
    // No need to delete data because that will be taken care of by the
    // controller.
    SkFlatData* data1 = SkFlatData::Create(&controller, obj, 0, flattenProc);
    SkFlatData* data2 = SkFlatData::Create(&controller, obj, 1, flattenProc);
    REPORTER_ASSERT(reporter, *data1 == *data2);
}

DEF_TEST(FlatData, reporter) {
    // Test flattening SkShader
    SkPoint points[2];
    points[0].set(0, 0);
    points[1].set(SkIntToScalar(20), SkIntToScalar(20));
    SkColor colors[2];
    colors[0] = SK_ColorRED;
    colors[1] = SK_ColorBLUE;
    SkShader* shader = SkGradientShader::CreateLinear(points, colors, NULL,
                                            2, SkShader::kRepeat_TileMode);
    SkAutoUnref aur(shader);
    testCreate(reporter, shader, flattenFlattenableProc);

    // Test SkBitmap
    {
        SkBitmap bm;
        bm.setConfig(SkBitmap::kARGB_8888_Config, 50, 50);
        bm.allocPixels();
        SkCanvas canvas(bm);
        SkPaint paint;
        paint.setShader(shader);
        canvas.drawPaint(paint);
        testCreate(reporter, &bm, &SkFlattenObjectProc<SkBitmap>);
    }

    // Test SkColorFilter
    SkColorFilter* cf = SkColorFilter::CreateLightingFilter(SK_ColorBLUE,
                                                            SK_ColorRED);
    SkAutoUnref aurcf(cf);
    testCreate(reporter, cf, &flattenFlattenableProc);

    // Test SkXfermode
    SkXfermode* xfer = SkXfermode::Create(SkXfermode::kDstOver_Mode);
    SkAutoUnref aurxf(xfer);
    testCreate(reporter, xfer, &flattenFlattenableProc);
}


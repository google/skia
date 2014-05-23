/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "SkPictureFlat.h"
#include "SkShader.h"
#include "SkXfermode.h"
#include "Test.h"

struct SkFlattenableTraits {
    static void Flatten(SkWriteBuffer& buffer, const SkFlattenable& flattenable) {
        buffer.writeFlattenable(&flattenable);
    }
};

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
template <typename Traits, typename T>
static void testCreate(skiatest::Reporter* reporter, const T& obj) {
    Controller controller;
    // No need to delete data because that will be taken care of by the controller.
    SkFlatData* data1 = SkFlatData::Create<Traits>(&controller, obj, 0);
    SkFlatData* data2 = SkFlatData::Create<Traits>(&controller, obj, 1);
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

    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateLinear(points, colors, NULL, 2,
                                                                 SkShader::kRepeat_TileMode));
    testCreate<SkFlattenableTraits>(reporter, *shader);

    // Test SkColorFilter
    SkAutoTUnref<SkColorFilter> cf(SkColorFilter::CreateLightingFilter(SK_ColorBLUE, SK_ColorRED));
    testCreate<SkFlattenableTraits>(reporter, *cf);

    // Test SkXfermode
    SkAutoTUnref<SkXfermode> xfer(SkXfermode::Create(SkXfermode::kDstOver_Mode));
    testCreate<SkFlattenableTraits>(reporter, *xfer);
}

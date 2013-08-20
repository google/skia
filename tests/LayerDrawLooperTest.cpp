/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDraw.h"
#include "SkDevice.h"
#include "SkLayerDrawLooper.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkXfermode.h"

namespace {

class FakeDevice : public SkDevice {
public:
    FakeDevice() : SkDevice(SkBitmap::kARGB_8888_Config, 100, 100) { }

    virtual void drawRect(const SkDraw& draw, const SkRect& r,
                          const SkPaint& paint) SK_OVERRIDE {
        fLastMatrix = *draw.fMatrix;
        SkDevice::drawRect(draw, r, paint);
    }

    SkMatrix fLastMatrix;
};

} // namespace

static void test_frontToBack(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkLayerDrawLooper> looper(SkNEW(SkLayerDrawLooper));
    SkLayerDrawLooper::LayerInfo layerInfo;

    // Add the front layer, with the defaults.
    (void)looper->addLayer(layerInfo);

    // Add the back layer, with some layer info set.
    layerInfo.fOffset.set(SkFloatToScalar(10.0f), SkFloatToScalar(20.0f));
    layerInfo.fPaintBits |= SkLayerDrawLooper::kXfermode_Bit;
    SkPaint* layerPaint = looper->addLayer(layerInfo);
    layerPaint->setXfermodeMode(SkXfermode::kSrc_Mode);

    FakeDevice device;
    SkCanvas canvas(&device);
    SkPaint paint;
    looper->init(&canvas);

    // The back layer should come first.
    REPORTER_ASSERT(reporter, looper->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrc_Mode));
    canvas.drawRect(SkRect::MakeWH(SkFloatToScalar(50.0f), SkFloatToScalar(50.0f)), paint);
    REPORTER_ASSERT(reporter, SkFloatToScalar(10.0f) == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, SkFloatToScalar(20.0f) == device.fLastMatrix.getTranslateY());
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, looper->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode));
    canvas.drawRect(SkRect::MakeWH(SkFloatToScalar(50.0f), SkFloatToScalar(50.0f)), paint);
    REPORTER_ASSERT(reporter, SkFloatToScalar(0.0f) == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, SkFloatToScalar(0.0f) == device.fLastMatrix.getTranslateY());

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !looper->next(&canvas, &paint));
}

static void test_backToFront(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkLayerDrawLooper> looper(SkNEW(SkLayerDrawLooper));
    SkLayerDrawLooper::LayerInfo layerInfo;

    // Add the back layer, with the defaults.
    (void)looper->addLayerOnTop(layerInfo);

    // Add the front layer, with some layer info set.
    layerInfo.fOffset.set(SkFloatToScalar(10.0f), SkFloatToScalar(20.0f));
    layerInfo.fPaintBits |= SkLayerDrawLooper::kXfermode_Bit;
    SkPaint* layerPaint = looper->addLayerOnTop(layerInfo);
    layerPaint->setXfermodeMode(SkXfermode::kSrc_Mode);

    FakeDevice device;
    SkCanvas canvas(&device);
    SkPaint paint;
    looper->init(&canvas);

    // The back layer should come first.
    REPORTER_ASSERT(reporter, looper->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode));
    canvas.drawRect(SkRect::MakeWH(SkFloatToScalar(50.0f), SkFloatToScalar(50.0f)), paint);
    REPORTER_ASSERT(reporter, SkFloatToScalar(0.0f) == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, SkFloatToScalar(0.0f) == device.fLastMatrix.getTranslateY());
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, looper->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrc_Mode));
    canvas.drawRect(SkRect::MakeWH(SkFloatToScalar(50.0f), SkFloatToScalar(50.0f)), paint);
    REPORTER_ASSERT(reporter, SkFloatToScalar(10.0f) == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, SkFloatToScalar(20.0f) == device.fLastMatrix.getTranslateY());

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !looper->next(&canvas, &paint));
}

static void test_mixed(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkLayerDrawLooper> looper(SkNEW(SkLayerDrawLooper));
    SkLayerDrawLooper::LayerInfo layerInfo;

    // Add the back layer, with the defaults.
    (void)looper->addLayer(layerInfo);

    // Add the front layer, with some layer info set.
    layerInfo.fOffset.set(SkFloatToScalar(10.0f), SkFloatToScalar(20.0f));
    layerInfo.fPaintBits |= SkLayerDrawLooper::kXfermode_Bit;
    SkPaint* layerPaint = looper->addLayerOnTop(layerInfo);
    layerPaint->setXfermodeMode(SkXfermode::kSrc_Mode);

    FakeDevice device;
    SkCanvas canvas(&device);
    SkPaint paint;
    looper->init(&canvas);

    // The back layer should come first.
    REPORTER_ASSERT(reporter, looper->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode));
    canvas.drawRect(SkRect::MakeWH(SkFloatToScalar(50.0f), SkFloatToScalar(50.0f)), paint);
    REPORTER_ASSERT(reporter, SkFloatToScalar(0.0f) == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, SkFloatToScalar(0.0f) == device.fLastMatrix.getTranslateY());
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, looper->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrc_Mode));
    canvas.drawRect(SkRect::MakeWH(SkFloatToScalar(50.0f), SkFloatToScalar(50.0f)), paint);
    REPORTER_ASSERT(reporter, SkFloatToScalar(10.0f) == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, SkFloatToScalar(20.0f) == device.fLastMatrix.getTranslateY());

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !looper->next(&canvas, &paint));
}

static void TestLayerDrawLooper(skiatest::Reporter* reporter) {
    test_frontToBack(reporter);
    test_backToFront(reporter);
    test_mixed(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("LayerDrawLooper", TestLayerDrawLooperClass, TestLayerDrawLooper)

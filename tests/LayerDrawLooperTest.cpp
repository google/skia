/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmap.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkDraw.h"
#include "SkLayerDrawLooper.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkXfermode.h"

class FakeDevice : public SkBitmapDevice {
public:
    FakeDevice() : SkBitmapDevice(SkBitmap::kARGB_8888_Config, 100, 100, false) { }

    virtual void drawRect(const SkDraw& draw, const SkRect& r,
                          const SkPaint& paint) SK_OVERRIDE {
        fLastMatrix = *draw.fMatrix;
        INHERITED::drawRect(draw, r, paint);
    }

    SkMatrix fLastMatrix;

private:
    typedef SkBitmapDevice INHERITED;
};

static void test_frontToBack(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkLayerDrawLooper> looper(SkNEW(SkLayerDrawLooper));
    SkLayerDrawLooper::LayerInfo layerInfo;

    // Add the front layer, with the defaults.
    (void)looper->addLayer(layerInfo);

    // Add the back layer, with some layer info set.
    layerInfo.fOffset.set(10.0f, 20.0f);
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
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 10.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 20.0f == device.fLastMatrix.getTranslateY());
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, looper->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode));
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateY());

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !looper->next(&canvas, &paint));
}

static void test_backToFront(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkLayerDrawLooper> looper(SkNEW(SkLayerDrawLooper));
    SkLayerDrawLooper::LayerInfo layerInfo;

    // Add the back layer, with the defaults.
    (void)looper->addLayerOnTop(layerInfo);

    // Add the front layer, with some layer info set.
    layerInfo.fOffset.set(10.0f, 20.0f);
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
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateY());
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, looper->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrc_Mode));
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 10.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 20.0f == device.fLastMatrix.getTranslateY());

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !looper->next(&canvas, &paint));
}

static void test_mixed(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkLayerDrawLooper> looper(SkNEW(SkLayerDrawLooper));
    SkLayerDrawLooper::LayerInfo layerInfo;

    // Add the back layer, with the defaults.
    (void)looper->addLayer(layerInfo);

    // Add the front layer, with some layer info set.
    layerInfo.fOffset.set(10.0f, 20.0f);
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
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateY());
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, looper->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrc_Mode));
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 10.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 20.0f == device.fLastMatrix.getTranslateY());

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !looper->next(&canvas, &paint));
}

DEF_TEST(LayerDrawLooper, reporter) {
    test_frontToBack(reporter);
    test_backToFront(reporter);
    test_mixed(reporter);
}

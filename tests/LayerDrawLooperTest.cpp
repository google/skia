/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
#include "SkSmallAllocator.h"
#include "SkXfermode.h"
#include "Test.h"

static SkBitmap make_bm(int w, int h) {
    SkBitmap bm;
    bm.allocN32Pixels(w, h);
    return bm;
}

// TODO: can this be derived from SkBaseDevice?
class FakeDevice : public SkBitmapDevice {
public:
    FakeDevice() : INHERITED(make_bm(100, 100), SkSurfaceProps(0, kUnknown_SkPixelGeometry)) {
    }

    void drawRect(const SkDraw& draw, const SkRect& r, const SkPaint& paint) override {
        fLastMatrix = *draw.fMatrix;
        this->INHERITED::drawRect(draw, r, paint);
    }

    SkMatrix fLastMatrix;

private:
    typedef SkBitmapDevice INHERITED;
};

static void test_frontToBack(skiatest::Reporter* reporter) {
    SkLayerDrawLooper::Builder looperBuilder;
    SkLayerDrawLooper::LayerInfo layerInfo;

    // Add the front layer, with the defaults.
    (void)looperBuilder.addLayer(layerInfo);

    // Add the back layer, with some layer info set.
    layerInfo.fOffset.set(10.0f, 20.0f);
    layerInfo.fPaintBits |= SkLayerDrawLooper::kXfermode_Bit;
    SkPaint* layerPaint = looperBuilder.addLayer(layerInfo);
    layerPaint->setXfermodeMode(SkXfermode::kSrc_Mode);

    FakeDevice device;
    SkCanvas canvas(&device);
    SkPaint paint;
    SkAutoTUnref<SkLayerDrawLooper> looper(looperBuilder.detachLooper());
    SkSmallAllocator<1, 32> allocator;
    void* buffer = allocator.reserveT<SkDrawLooper::Context>(looper->contextSize());
    SkDrawLooper::Context* context = looper->createContext(&canvas, buffer);

    // The back layer should come first.
    REPORTER_ASSERT(reporter, context->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrc_Mode));
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 10.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 20.0f == device.fLastMatrix.getTranslateY());
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, context->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode));
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateY());

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !context->next(&canvas, &paint));
}

static void test_backToFront(skiatest::Reporter* reporter) {
    SkLayerDrawLooper::Builder looperBuilder;
    SkLayerDrawLooper::LayerInfo layerInfo;

    // Add the back layer, with the defaults.
    (void)looperBuilder.addLayerOnTop(layerInfo);

    // Add the front layer, with some layer info set.
    layerInfo.fOffset.set(10.0f, 20.0f);
    layerInfo.fPaintBits |= SkLayerDrawLooper::kXfermode_Bit;
    SkPaint* layerPaint = looperBuilder.addLayerOnTop(layerInfo);
    layerPaint->setXfermodeMode(SkXfermode::kSrc_Mode);

    FakeDevice device;
    SkCanvas canvas(&device);
    SkPaint paint;
    SkAutoTUnref<SkLayerDrawLooper> looper(looperBuilder.detachLooper());
    SkSmallAllocator<1, 32> allocator;
    void* buffer = allocator.reserveT<SkDrawLooper::Context>(looper->contextSize());
    SkDrawLooper::Context* context = looper->createContext(&canvas, buffer);

    // The back layer should come first.
    REPORTER_ASSERT(reporter, context->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode));
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateY());
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, context->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrc_Mode));
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 10.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 20.0f == device.fLastMatrix.getTranslateY());

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !context->next(&canvas, &paint));
}

static void test_mixed(skiatest::Reporter* reporter) {
    SkLayerDrawLooper::Builder looperBuilder;
    SkLayerDrawLooper::LayerInfo layerInfo;

    // Add the back layer, with the defaults.
    (void)looperBuilder.addLayer(layerInfo);

    // Add the front layer, with some layer info set.
    layerInfo.fOffset.set(10.0f, 20.0f);
    layerInfo.fPaintBits |= SkLayerDrawLooper::kXfermode_Bit;
    SkPaint* layerPaint = looperBuilder.addLayerOnTop(layerInfo);
    layerPaint->setXfermodeMode(SkXfermode::kSrc_Mode);

    FakeDevice device;
    SkCanvas canvas(&device);
    SkPaint paint;
    SkAutoTUnref<SkLayerDrawLooper> looper(looperBuilder.detachLooper());
    SkSmallAllocator<1, 32> allocator;
    void* buffer = allocator.reserveT<SkDrawLooper::Context>(looper->contextSize());
    SkDrawLooper::Context* context = looper->createContext(&canvas, buffer);

    // The back layer should come first.
    REPORTER_ASSERT(reporter, context->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode));
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 0.0f == device.fLastMatrix.getTranslateY());
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, context->next(&canvas, &paint));
    REPORTER_ASSERT(reporter, SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrc_Mode));
    canvas.drawRect(SkRect::MakeWH(50.0f, 50.0f), paint);
    REPORTER_ASSERT(reporter, 10.0f == device.fLastMatrix.getTranslateX());
    REPORTER_ASSERT(reporter, 20.0f == device.fLastMatrix.getTranslateY());

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !context->next(&canvas, &paint));
}

DEF_TEST(LayerDrawLooper, reporter) {
    test_frontToBack(reporter);
    test_backToFront(reporter);
    test_mixed(reporter);
}

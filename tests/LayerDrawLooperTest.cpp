/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkLayerDrawLooper.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkDraw.h"
#include "tests/Test.h"

static SkBitmap make_bm(int w, int h) {
    SkBitmap bm;
    bm.allocN32Pixels(w, h);
    return bm;
}

// TODO: can this be derived from SkBaseDevice?
class FakeDevice : public SkBitmapDevice {
public:
    FakeDevice() : INHERITED(make_bm(100, 100), SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                             nullptr, nullptr) {
    }

    void drawRect(const SkRect& r, const SkPaint& paint) override {
        fLastMatrix = this->ctm();
        this->INHERITED::drawRect(r, paint);
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
    layerPaint->setBlendMode(SkBlendMode::kSrc);

    SkPaint paint;
    auto looper(looperBuilder.detach());
    SkArenaAlloc alloc{48};
    SkDrawLooper::Context* context = looper->makeContext(&alloc);
    SkDrawLooper::Context::Info info;

    // The back layer should come first.
    REPORTER_ASSERT(reporter, context->next(&info, &paint));
    REPORTER_ASSERT(reporter, paint.getBlendMode() == SkBlendMode::kSrc);
    REPORTER_ASSERT(reporter, 10.0f == info.fTranslate.fX);
    REPORTER_ASSERT(reporter, 20.0f == info.fTranslate.fY);
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, context->next(&info, &paint));
    REPORTER_ASSERT(reporter, paint.getBlendMode() == SkBlendMode::kSrcOver);
    REPORTER_ASSERT(reporter, 0.0f == info.fTranslate.fX);
    REPORTER_ASSERT(reporter, 0.0f == info.fTranslate.fY);

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !context->next(&info, &paint));
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
    layerPaint->setBlendMode(SkBlendMode::kSrc);

    SkPaint paint;
    auto looper(looperBuilder.detach());
    SkArenaAlloc alloc{48};
    SkDrawLooper::Context* context = looper->makeContext(&alloc);
    SkDrawLooper::Context::Info info;

    // The back layer should come first.
    REPORTER_ASSERT(reporter, context->next(&info, &paint));
    REPORTER_ASSERT(reporter, paint.getBlendMode() == SkBlendMode::kSrcOver);
    REPORTER_ASSERT(reporter, 0.0f == info.fTranslate.fX);
    REPORTER_ASSERT(reporter, 0.0f == info.fTranslate.fY);
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, context->next(&info, &paint));
    REPORTER_ASSERT(reporter, paint.getBlendMode() == SkBlendMode::kSrc);
    REPORTER_ASSERT(reporter, 10.0f == info.fTranslate.fX);
    REPORTER_ASSERT(reporter, 20.0f == info.fTranslate.fY);

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !context->next(&info, &paint));
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
    layerPaint->setBlendMode(SkBlendMode::kSrc);

    SkPaint paint;
    sk_sp<SkDrawLooper> looper(looperBuilder.detach());
    SkArenaAlloc alloc{48};
    SkDrawLooper::Context* context = looper->makeContext(&alloc);
    SkDrawLooper::Context::Info info;

    // The back layer should come first.
    REPORTER_ASSERT(reporter, context->next(&info, &paint));
    REPORTER_ASSERT(reporter, paint.getBlendMode() == SkBlendMode::kSrcOver);
    REPORTER_ASSERT(reporter, 0.0f == info.fTranslate.fX);
    REPORTER_ASSERT(reporter, 0.0f == info.fTranslate.fY);
    paint.reset();

    // Then the front layer.
    REPORTER_ASSERT(reporter, context->next(&info, &paint));
    REPORTER_ASSERT(reporter, paint.getBlendMode() == SkBlendMode::kSrc);
    REPORTER_ASSERT(reporter, 10.0f == info.fTranslate.fX);
    REPORTER_ASSERT(reporter, 20.0f == info.fTranslate.fY);

    // Only two layers were added, so that should be the end.
    REPORTER_ASSERT(reporter, !context->next(&info, &paint));
}

DEF_TEST(LayerDrawLooper, reporter) {
    test_frontToBack(reporter);
    test_backToFront(reporter);
    test_mixed(reporter);
}

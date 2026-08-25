/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/GpuTools.h"
#include "bench/SKPAnimationBench.h"
#include "include/core/SkSurface.h"
#include "tools/flags/CommandLineFlags.h"


SKPAnimationBench::SKPAnimationBench(const char* name, const SkPicture* pic, const SkIRect& clip,
                                     sk_sp<Animation> animation, bool doLooping)
    : INHERITED(name, pic, clip, 1.0, doLooping)
    , fAnimation(std::move(animation)) {
    fUniqueName.printf("%s_%s", name, fAnimation->getTag());
}

const char* SKPAnimationBench::onGetUniqueName() {
    return fUniqueName.c_str();
}

void SKPAnimationBench::onPerCanvasPreDraw(SkCanvas* canvas) {
    INHERITED::onPerCanvasPreDraw(canvas);
    fDevBounds = canvas->getDeviceClipBounds();
    SkAssertResult(!fDevBounds.isEmpty());
}

void SKPAnimationBench::drawPicture() {
    const skia_private::TArray<TileInfo>& tileInfo = this->tileInfo();

    for (const TileInfo& tile : tileInfo) {
        SkMatrix trans = SkMatrix::Translate(-1.f * tile.tileRect().fLeft,
                                             -1.f * tile.tileRect().fTop);
        fAnimation->preConcatFrameMatrix(fAnimationTime.nextRangeF(0, 1000), fDevBounds, &trans);

        SkCanvas* canvas = tile.surface()->getCanvas();

        SkAutoCanvasRestore acr(canvas, /* doSave= */ false);
        canvas->clear(SK_ColorWHITE);

        canvas->save();
        canvas->clipRect(tile.clipRect());
        canvas->setMatrix(tile.mat());

        canvas->drawPicture(this->picture(), &trans, nullptr);
    }

    for (const TileInfo& tile : tileInfo) {
        skgpu::Flush(tile.surface());
    }
}

class ZoomAnimation : public SKPAnimationBench::Animation {
public:
    ZoomAnimation(SkScalar zoomMax, double zoomPeriodMs)
        : fZoomMax(zoomMax)
        , fZoomPeriodMs(zoomPeriodMs) {
    }

    const char* getTag() override { return "zoom"; }

    void preConcatFrameMatrix(double animationTimeMs, const SkIRect& devBounds,
                              SkMatrix* drawMatrix) override {
        double t = fmod(animationTimeMs / fZoomPeriodMs, 1.0); // t is in [0, 1).
        t = fabs(2 * t - 1); // Make t ping-pong between 0 and 1
        SkScalar zoom = static_cast<SkScalar>(pow(fZoomMax, t));

        SkPoint center = SkPoint::Make((devBounds.fLeft + devBounds.fRight) / 2.0f,
                                       (devBounds.fTop + devBounds.fBottom) / 2.0f);
        drawMatrix->preTranslate(center.fX, center.fY);
        drawMatrix->preScale(zoom, zoom);
        drawMatrix->preTranslate(-center.fX, -center.fY);
    }

private:
    double   fZoomMax;
    double   fZoomPeriodMs;
};

sk_sp<SKPAnimationBench::Animation> SKPAnimationBench::MakeZoomAnimation(SkScalar zoomMax,
                                                                         double zoomPeriodMs) {
    return sk_make_sp<ZoomAnimation>(zoomMax, zoomPeriodMs);
}

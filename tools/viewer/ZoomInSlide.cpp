/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/ZoomInSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"

// An example implementation of a zoomed-in slide.
// Reproduces issues.skia.org/issues/451536363
class ZoomInSlideDemo : public ZoomInSlide {
public:
    ZoomInSlideDemo() : ZoomInSlide(kScale, kMaskWidth, kMaskHeight, SkString("ZoomInDemo")) {
        fPts[0] = {1.5, 1.5};
        fPts[1] = {1.5, 10.5};
        fPts[2] = {10.5, 10.5};
        fPts[3] = {10.5, 1.5};
    }

    SkPath makePath() {
        return SkPathBuilder()
                .moveTo(fPts[0])
                .lineTo(fPts[1])
                .lineTo(fPts[2])
                .lineTo(fPts[3])
                .close()
                .detach();
    }

    void drawUnderGrid(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeCap(SkPaint::kSquare_Cap);
        paint.setStrokeJoin(SkPaint::kMiter_Join);
        paint.setAntiAlias(true);

        this->drawScaledPath(canvas, makePath(), paint);
    }

    void drawOverGrid(SkCanvas* canvas) override {
        SkPaint truthPaint;
        truthPaint.setStyle(SkPaint::Style::kStroke_Style);
        truthPaint.setColor(SK_ColorRED);
        truthPaint.setAntiAlias(true);
        truthPaint.setStrokeWidth(2.f / fScale);

        canvas->drawPath(makePath(), truthPaint);

        SkPaint handlePaint;
        handlePaint.setAntiAlias(true);
        handlePaint.setStyle(SkPaint::Style::kFill_Style);
        handlePaint.setColor(SkColorSetARGB(255, 255, 70, 10));
        for (auto pt : fPts) {
            canvas->drawCircle(pt, kHandleRadius / kScale, handlePaint);
        }
    }

    void handleClick(const ScaledClick* click) override {
        SkPoint pt = click->currScaled();
        int closestPointIdx = 0;
        float minDist = SkPoint::Distance(pt, fPts[0]);

        for (int i = 1; i < kNumPoints; i++) {
            float dist = SkPoint::Distance(pt, fPts[i]);
            if (dist < minDist) {
                minDist = dist;
                closestPointIdx = i;
            }
        }
        fPts[closestPointIdx] = pt;
    }

private:
    static constexpr size_t kScale = 32;
    // Defines the region of the screen that will be zoomed in on.
    // The path under scrutiny will be drawn in this region.
    static constexpr int kMaskWidth = 20;
    static constexpr int kMaskHeight = 30;

    static constexpr float kHandleRadius = 4.f;
    static constexpr int kNumPoints = 4;

    SkPoint fPts[kNumPoints];
};

DEF_SLIDE(return new ZoomInSlideDemo();)

/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/base/SkRandom.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/Slide.h"

// This slide draws a lot of overlapping rectangles which slide left.
// It's adapted from the performance test at https://benchmarks.slaylines.io/
static constexpr int kWidth = 1000;
static constexpr int kHeight = 639;
static constexpr int kNumRects = 32000;

class AnimatedRects : public Slide {
public:
    AnimatedRects() {
        fName = "animated-rects";
    }

protected:
    void load(SkScalar, SkScalar) override {
        for (int i = 0; i < kNumRects; ++i) {
            fRect[i].x = fRand.nextF() * kWidth;
            fRect[i].y = fRand.nextF() * kHeight;
            fRect[i].size = 10.0 + fRand.nextF() * 40.0;
            fRect[i].speed = 1.0 + fRand.nextF();
        }

        fStrokePaint.setAntiAlias(true);
        fStrokePaint.setColor(SK_ColorBLACK);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
        fStrokePaint.setStrokeWidth(2.0);
        fFillPaint.setAntiAlias(true);
        fFillPaint.setStyle(SkPaint::kFill_Style);
        fFillPaint.setColor(SK_ColorWHITE);
    }

    void draw(SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, /*doSave=*/true);
        canvas->clipRect({0, 0, (float)kWidth, (float)kHeight});

        for (int i = 0; i < kNumRects; ++i) {
            const AnimatedRect& r = fRect[i];
            canvas->drawRect(SkRect{r.x, r.y, r.x + r.size, r.y + r.size}, fStrokePaint);
            canvas->drawRect(SkRect{r.x, r.y, r.x + r.size, r.y + r.size}, fFillPaint);
        }
    }

    bool animate(double nanos) override {
        float seconds = 1e-9 * nanos;
        if (0.0f != fLastTime) {
            float scale = (seconds - fLastTime) * 60;

            for (int i = 0; i < kNumRects; ++i) {
                AnimatedRect& r = fRect[i];
                r.x -= r.speed * scale;
                if (r.x + r.size < 0) {
                    r.x = kWidth + r.size;
                }
            }
        }

        fLastTime = seconds;
        return true;
    }

private:
    struct AnimatedRect {
        float x, y, size, speed;
    };

    AnimatedRect fRect[kNumRects];
    SkRandom fRand;
    SkPaint fStrokePaint, fFillPaint;
    float fLastTime = 0.0f;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE(return new AnimatedRects;)

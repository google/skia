/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/utils/SkRandom.h"
#include "tools/timer/AnimTimer.h"

static const SkScalar kBlurMax = 7.0f;
static const int kNumNodes = 30;
static const int kWidth = 512;
static const int kHeight = 512;
static const SkScalar kBlurAnimationDuration = 4.0f; // in secs

// This GM draws a lot of layers with animating BlurImageFilters
class AnimatedImageBlurs : public skiagm::GM {
public:
    AnimatedImageBlurs() : skiagm::GM("animated-image-blurs", {kWidth, kHeight}, 0xFFCCCCCC) {}

protected:
    bool runAsBench() const override { return true; }

    void onOnceBeforeDraw() override {
        for (int i = 0; i < kNumNodes; ++i) {
            fNodes[i].init(&fRand);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);

        for (int i = 0; i < kNumNodes; ++i) {
            SkPaint layerPaint;
            layerPaint.setImageFilter(SkBlurImageFilter::Make(fNodes[i].sigma(),
                                                              fNodes[i].sigma(),
                                                              nullptr));

            canvas->saveLayer(nullptr, &layerPaint);
                // The rect is outset to block the circle case
                SkRect rect = SkRect::MakeLTRB(fNodes[i].pos().fX - fNodes[i].size()-0.5f,
                                               fNodes[i].pos().fY - fNodes[i].size()-0.5f,
                                               fNodes[i].pos().fX + fNodes[i].size()+0.5f,
                                               fNodes[i].pos().fY + fNodes[i].size()+0.5f);
                SkRRect rrect = SkRRect::MakeRectXY(rect, fNodes[i].size(), fNodes[i].size());
                canvas->drawRRect(rrect, paint);
            canvas->restore();
        }
    }

    bool onAnimate(const AnimTimer& timer) override {
        if (0.0f != fLastTime) {
            for (int i = 0; i < kNumNodes; ++i) {
                fNodes[i].update(timer, fLastTime);
            }
        }

        fLastTime = timer.secs();
        return true;
    }

private:
    class Node {
    public:
        void init(SkRandom* rand) {
            fSize = rand->nextRangeF(10.0f, 60.f);
            fPos.fX = rand->nextRangeF(fSize, kWidth - fSize);
            fPos.fY = rand->nextRangeF(fSize, kHeight - fSize);
            fDir.fX = rand->nextRangeF(-1.0f, 1.0f);
            fDir.fY = SkScalarSqrt(1.0f - fDir.fX * fDir.fX);
            if (rand->nextBool()) {
                fDir.fY = -fDir.fY;
            }
            fBlurOffset = rand->nextRangeF(0.0f, kBlurMax);
            fBlur = fBlurOffset;
            fSpeed = rand->nextRangeF(20.0f, 60.0f);
        }

        void update(const AnimTimer& timer, SkScalar lastTime) {
            SkScalar deltaTime = timer.secs() - lastTime;

            fPos.fX += deltaTime * fSpeed * fDir.fX;
            fPos.fY += deltaTime * fSpeed * fDir.fY;
            if (fPos.fX >= kWidth || fPos.fX < 0.0f) {
                fPos.fX = SkTPin<SkScalar>(fPos.fX, 0.0f, kWidth);
                fDir.fX = -fDir.fX;
            }
            if (fPos.fY >= kHeight || fPos.fY < 0.0f) {
                fPos.fY = SkTPin<SkScalar>(fPos.fY, 0.0f, kHeight);
                fDir.fY = -fDir.fY;
            }

            fBlur = timer.pingPong(kBlurAnimationDuration, fBlurOffset, 0.0f, kBlurMax);
        }

        SkScalar sigma() const { return fBlur; }
        const SkPoint& pos() const { return fPos; }
        SkScalar size() const { return fSize; }

    private:
        SkScalar fSize = 0.0f;
        SkPoint  fPos = { 0.0f, 0.0f };
        SkVector fDir = { 1.0f, 0.0f };
        SkScalar fBlurOffset = 0.0f;
        SkScalar fBlur = 0.0f;
        SkScalar fSpeed = 0.0f;
    };

    Node     fNodes[kNumNodes];
    SkRandom fRand;
    SkScalar fLastTime = 0.0f;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AnimatedImageBlurs;)

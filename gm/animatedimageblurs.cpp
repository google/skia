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
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkRandom.h"
#include "tools/timer/TimeUtils.h"

static const SkScalar kBlurMax = 7.0f;
static const int kNumNodes = 30;
static const int kWidth = 512;
static const int kHeight = 512;
static const SkScalar kBlurAnimationDuration = 4.0f; // in secs

// This GM draws a lot of layers with animating BlurImageFilters
class AnimatedImageBlurs : public skiagm::GM {
public:
    AnimatedImageBlurs() : fLastTime(0.0f) {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    bool runAsBench() const override { return true; }

    SkString getName() const override { return SkString("animated-image-blurs"); }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

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
            layerPaint.setImageFilter(SkImageFilters::Blur(fNodes[i].sigma(), fNodes[i].sigma(),
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

    bool onAnimate(double nanos) override {
        if (0.0f != fLastTime) {
            for (int i = 0; i < kNumNodes; ++i) {
                fNodes[i].update(nanos, fLastTime);
            }
        }

        fLastTime = 1e-9 * nanos;
        return true;
    }

private:
    class Node {
    public:
        Node()
            : fSize(0.0f)
            , fPos { 0.0f, 0.0f }
            , fDir { 1.0f, 0.0f }
            , fBlurOffset(0.0f)
            , fBlur(fBlurOffset)
            , fSpeed(0.0f) {
        }

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

        void update(double nanos, SkScalar lastTime) {
            SkScalar deltaTime = 1e-9 * nanos - lastTime;

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

            fBlur = TimeUtils::PingPong(1e-9 * nanos, kBlurAnimationDuration, fBlurOffset, 0.0f, kBlurMax);
        }

        SkScalar sigma() const { return fBlur; }
        const SkPoint& pos() const { return fPos; }
        SkScalar size() const { return fSize; }

    private:
        SkScalar fSize;
        SkPoint  fPos;
        SkVector fDir;
        SkScalar fBlurOffset;
        SkScalar fBlur;
        SkScalar fSpeed;
    };

    Node     fNodes[kNumNodes];
    SkRandom fRand;
    SkScalar fLastTime;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AnimatedImageBlurs;)

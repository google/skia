/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkBlurMask.h"
#include "tools/timer/TimeUtils.h"

/**
 * In GM mode this draws an array of circles with different radii and different blur radii. Below
 * each circle an almost-circle path is drawn with the same blur filter for comparison.
 *
 * In Sample mode this draws a single circle and almost-circle with animating radius and blur
 * radius.
 *
 * Bench mode draws the same as GM mode but without the comparison almost-circle paths. It also
 * slightly perturbs the blur and circle radii to stress caching of blurred profiles in GPU mode.
 */
class BlurCircles2GM : public skiagm::GM {
public:
    BlurCircles2GM() {
        fAnimRadius = TimeUtils::PingPong(
                0, kRadiusPingPoingPeriod, kRadiusPingPoingShift, kMinRadius, kMaxRadius);
        fAnimBlurRadius = TimeUtils::PingPong(0,
                                              kBlurRadiusPingPoingPeriod,
                                              kBlurRadiusPingPoingShift,
                                              kMinBlurRadius,
                                              kMaxBlurRadius);
    }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("blurcircles2"); }

    SkISize onISize() override {
        return SkISize::Make(730, 1350);
    }

    void onDraw(SkCanvas* canvas) override {
        constexpr SkScalar kMaxR = kMaxRadius + kMaxBlurRadius;

        auto almostCircleMaker = [] (SkScalar radius, SkPath* dst) {
            dst->reset();
            dst->addArc(SkRect::MakeXYWH(-radius, -radius, 2 * radius, 2 * radius), 0, 355);
            dst->setIsVolatile(true);
            dst->close();
        };

        auto blurMaker = [] (SkScalar radius) ->sk_sp<SkMaskFilter> {
            return SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                          SkBlurMask::ConvertRadiusToSigma(radius));
        };

        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        if (this->getMode() == kSample_Mode) {
            paint.setMaskFilter(blurMaker(fAnimBlurRadius));
            SkISize size = canvas->getBaseLayerSize();
            SkPath almostCircle;
            almostCircleMaker(fAnimRadius, &almostCircle);
            canvas->save();
                canvas->translate(size.fWidth / 2.f, size.fHeight / 4.f);
                canvas->drawCircle(0, 0, fAnimRadius, paint);
                canvas->translate(0, 2 * kMaxR);
                canvas->drawPath(almostCircle, paint);
            canvas->restore();
        } else {
            bool benchMode = this->getMode() == kBench_Mode;
            canvas->save();
            constexpr SkScalar kPad = 5;
            constexpr SkScalar kRadiusSteps = 5;
            constexpr SkScalar kBlurRadiusSteps = 5;
            canvas->translate(kPad + kMinRadius + kMaxBlurRadius,
                              kPad + kMinRadius + kMaxBlurRadius);
            constexpr SkScalar kDeltaRadius = (kMaxRadius - kMinRadius) / kRadiusSteps;
            constexpr SkScalar kDeltaBlurRadius = (kMaxBlurRadius - kMinBlurRadius) /
                                                         kBlurRadiusSteps;
            SkScalar lineWidth = 0;
            if (!benchMode) {
                for (int r = 0; r < kRadiusSteps - 1; ++r) {
                    const SkScalar radius = r * kDeltaRadius + kMinRadius;
                    lineWidth += 2 * (radius + kMaxBlurRadius) + kPad;
                }
            }
            for (int br = 0; br < kBlurRadiusSteps; ++br) {
                SkScalar blurRadius = br * kDeltaBlurRadius + kMinBlurRadius;
                if (benchMode) {
                    blurRadius += fRandom.nextSScalar1() * kDeltaBlurRadius;
                }
                const SkScalar maxRowR = blurRadius + kMaxRadius;
                paint.setMaskFilter(blurMaker(blurRadius));
                canvas->save();
                for (int r = 0; r < kRadiusSteps; ++r) {
                    SkScalar radius = r * kDeltaRadius + kMinRadius;
                    if (benchMode) {
                        radius += fRandom.nextSScalar1() * kDeltaRadius;
                    }
                    SkPath almostCircle;
                    if (!benchMode) {
                        almostCircleMaker(radius, &almostCircle);
                    }
                    canvas->save();
                        canvas->drawCircle(0, 0, radius, paint);
                        canvas->translate(0, 2 * maxRowR + kPad);
                    if (!benchMode) {
                        canvas->drawPath(almostCircle, paint);
                    }
                    canvas->restore();
                    const SkScalar maxColR = radius + kMaxBlurRadius;
                    canvas->translate(maxColR * 2 + kPad, 0);
                }
                canvas->restore();
                if (!benchMode) {
                    SkPaint blackPaint;
                    blackPaint.setColor(SK_ColorBLACK);
                    const SkScalar lineY = 3 * maxRowR + 1.5f * kPad;
                    if (br != kBlurRadiusSteps - 1) {
                        canvas->drawLine(0, lineY, lineWidth, lineY, blackPaint);
                    }
                }
                canvas->translate(0, maxRowR * 4 + 2 * kPad);
            }
            canvas->restore();
        }
    }

    bool onAnimate(double nanos) override {
        fAnimRadius = TimeUtils::PingPong(1e-9 * nanos, kRadiusPingPoingPeriod, kRadiusPingPoingShift, kMinRadius,
                                     kMaxRadius);
        fAnimBlurRadius = TimeUtils::PingPong(1e-9 * nanos, kBlurRadiusPingPoingPeriod, kBlurRadiusPingPoingShift,
                                         kMinBlurRadius, kMaxBlurRadius);
        return true;
    }

private:
    static constexpr SkScalar kMinRadius = 15;
    static constexpr SkScalar kMaxRadius = 45;
    static constexpr SkScalar kRadiusPingPoingPeriod = 8;
    static constexpr SkScalar kRadiusPingPoingShift = 3;

    static constexpr SkScalar kMinBlurRadius = 5;
    static constexpr SkScalar kMaxBlurRadius = 45;
    static constexpr SkScalar kBlurRadiusPingPoingPeriod = 3;
    static constexpr SkScalar kBlurRadiusPingPoingShift = 1.5;

    SkScalar    fAnimRadius;
    SkScalar    fAnimBlurRadius;

    SkRandom    fRandom;

    typedef skiagm::GM INHERITED;
};

DEF_GM(return new BlurCircles2GM();)

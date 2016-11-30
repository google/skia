/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkAnimTimer.h"
#include "SkBlurImageFilter.h"
#include "SkPath.h"
#include "SkRandom.h"

static const float kBlurMax = 7.0f;
static const int kNumNodes = 30;
static const int kWidth = 512;
static const int kHeight = 512;

static void add_conic(SkPath* path, const SkPoint& p0, const SkPoint& p1) {
    SkPoint mid = p0 + p1;
    mid *= 0.5f;
    SkVector v = p1 - p0;
    v.normalize();
    mid.fX += v.fY;
    mid.fY += -v.fX;
    path->conicTo(mid, p1, 0.1f);
}

// This GM draws a lot of layers with animating BlurImageFilters
class AnimatedImageBlurs : public skiagm::GM {
public:
    AnimatedImageBlurs() : fLastTime(0.0f) {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("animated-image-blurs"); }

    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }

    void onOnceBeforeDraw() override {
        for (int i = 0; i < kNumNodes; ++i) {
            fNodes[i].init(&fRand);
        }

        // make a bulgy star
        {
            SkPoint inner[5], outer[5];
            static const SkScalar kTwoPiOver5 = (2.0f * SK_ScalarPI) / 5.0f;
            static const SkScalar kInnerRad = 1.0f/3.0f;
            static const SkScalar kOuterRad = 1.0f;

            for (int i = 0; i < 5; ++i) {
                inner[i].fY = SkScalarSinCos(i*kTwoPiOver5 - (kTwoPiOver5/2.0f), &inner[i].fX);
                outer[i].fY = SkScalarSinCos(i*kTwoPiOver5, &outer[i].fX);

                inner[i] *= kInnerRad;
                outer[i] *= kOuterRad;
            }

            fPath.setIsVolatile(true);  // don't want any caching going on

            fPath.moveTo(inner[0]);
            for (int i = 0; i < 5; ++i) {
                add_conic(&fPath, inner[i], outer[i]);
                add_conic(&fPath, outer[i], inner[(i+1)%5]);
            }
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
                canvas->translate(fNodes[i].pos().fX, fNodes[i].pos().fY);
                canvas->rotate(fNodes[i].degrees());
                canvas->scale(fNodes[i].size(), fNodes[i].size());

                canvas->drawPath(fPath, paint);
            canvas->restore();
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
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
        Node() 
            : fSize(0.0f)
            , fPos { 0.0f, 0.0f }
            , fDir { 1.0f, 0.0f }
            , fDegreeOffset(0.0f)
            , fDegrees(fDegreeOffset)
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
            fDegreeOffset = rand->nextRangeF(0.0f, 360.0f);
            fDegrees = fDegreeOffset;
            fBlurOffset = rand->nextRangeF(0.0f, kBlurMax);
            fBlur = fBlurOffset;
            fSpeed = rand->nextRangeF(20.0f, 60.0f);
        }

        void update(const SkAnimTimer& timer, SkScalar lastTime) {

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

            static const SkScalar kRotationSpeed = 100.0f;

            fDegrees = SkScalarMod(fDegreeOffset + timer.scaled(kRotationSpeed, 360.0f), 360.0f);

            static const SkScalar kBlurAnimationDuration = 4.0f; // in secs

            fBlur = timer.pingPong(kBlurAnimationDuration, fBlurOffset, 0.0f, kBlurMax);
        }  

        SkScalar sigma() const { return fBlur; }
        const SkPoint& pos() const { return fPos; }
        SkScalar size() const { return fSize; }
        SkScalar degrees() const { return fDegrees; }

    private:
        SkScalar fSize;
        SkPoint  fPos;
        SkVector fDir;
        SkScalar fDegreeOffset;
        SkScalar fDegrees;
        SkScalar fBlurOffset;
        SkScalar fBlur;
        SkScalar fSpeed;
    };

    Node     fNodes[kNumNodes];
    SkRandom fRand;
    SkPath   fPath;
    SkScalar fLastTime;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AnimatedImageBlurs;)

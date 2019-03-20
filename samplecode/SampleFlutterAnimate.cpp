/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "AnimTimer.h"
#include "Sample.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkFont.h"
#include "SkImage.h"
#include "SkRandom.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "Timer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

// Create an animation of a bunch of letters that rotate in place. This is intended to stress
// the glyph atlas and test that we don't see corruption or bad slowdowns.
class FlutterAnimateView : public Sample {
public:
    FlutterAnimateView() : fCurrTime(0), fResetTime(0) {}

protected:
    void onOnceBeforeDraw() override {
        fTypeface = SkTypeface::MakeFromFile("/skimages/samplefont.ttf");
        initChars();
    }

    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "FlutterAnimate");
            return true;
        }

        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkFont font(fTypeface, 50);
        SkPaint paint;
        paint.setFilterQuality(kMedium_SkFilterQuality);

        // rough center of each glyph
        static constexpr auto kMidX = 35;
        static constexpr auto kMidY = 50;

        canvas->clear(SK_ColorWHITE);
        for (int i = 0; i < kNumChars; ++i) {
            canvas->save();
            double rot = SkScalarInterp(fChars[i].fStartRotation, fChars[i].fEndRotation,
                                        fCurrTime/kDuration);
            canvas->translate(fChars[i].fPosition.fX + kMidX, fChars[i].fPosition.fY - kMidY);
            canvas->rotate(SkRadiansToDegrees(rot));
            canvas->translate(-35,+50);
            canvas->drawString(fChars[i].fChar, 0, 0, font, paint);
            canvas->restore();
        }
    }

    bool onAnimate(const AnimTimer& timer) override {
        fCurrTime = timer.secs() - fResetTime;
        if (fCurrTime > kDuration) {
            this->initChars();
            fResetTime = timer.secs();
            fCurrTime = 0;
        }

        return true;
    }

private:
    void initChars() {
        for (int i = 0; i < kNumChars; ++i) {
            char c = fRand.nextULessThan(26) + 65;
            fChars[i].fChar[0] = c;
            fChars[i].fChar[1] = '\0';
            fChars[i].fPosition = SkPoint::Make(fRand.nextF()*748 + 10, fRand.nextF()*1004 + 10);
            fChars[i].fStartRotation = fRand.nextF();
            fChars[i].fEndRotation = fRand.nextF() * 20 - 10;
        }
    }

    static constexpr double kDuration = 5.0;
    double fCurrTime;
    double fResetTime;
    SkRandom fRand;

    struct AnimatedChar {
        char fChar[2];
        SkPoint  fPosition;
        SkScalar fStartRotation;
        SkScalar fEndRotation;
    };
    sk_sp<SkTypeface> fTypeface;
    static constexpr int kNumChars = 40;
    AnimatedChar fChars[kNumChars];

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new FlutterAnimateView(); )

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkTypeface.h"
#include "src/base/SkRandom.h"
#include "src/base/SkTime.h"
#include "tools/timer/Timer.h"
#include "tools/viewer/Slide.h"

// Create an animation of a bunch of letters that rotate in place. This is intended to stress
// the glyph atlas and test that we don't see corruption or bad slowdowns.
class FlutterAnimateView : public Slide {
public:
    FlutterAnimateView() : fCurrTime(0), fResetTime(0) { fName = "FlutterAnimate"; }

public:
    void load(SkScalar w, SkScalar h) override {
        fTypeface = SkTypeface::MakeFromFile("/skimages/samplefont.ttf");
        initChars();
    }

    void draw(SkCanvas* canvas) override {
        SkFont font(fTypeface, 50);
        SkPaint paint;

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

    bool animate(double nanos) override {
        fCurrTime = 1e-9 * nanos - fResetTime;
        if (fCurrTime > kDuration) {
            this->initChars();
            fResetTime = 1e-9 * nanos;
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

    inline static constexpr double kDuration = 5.0;
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
    inline static constexpr int kNumChars = 40;
    AnimatedChar fChars[kNumChars];
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new FlutterAnimateView(); )

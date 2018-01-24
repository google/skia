/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkImage.h"
#include "SkRandom.h"
#include "SkSystemEventTypes.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "Timer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

class FlutterAnimateView : public SampleView {
public:
    FlutterAnimateView() : fCurrTime(0), fResetTime(0) {}

protected:
    void onOnceBeforeDraw() override {
        initChars();
    }

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "FlutterAnimate");
            return true;
        }

        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setTypeface(SkTypeface::MakeFromFile("/skimages/samplefont.ttf"));
        paint.setAntiAlias(true);
        paint.setFilterQuality(kMedium_SkFilterQuality);
        paint.setTextSize(50);

        canvas->clear(SK_ColorWHITE);
        for (int i = 0; i < kNumChars; ++i) {
            canvas->save();
            double rot = fChars[i].fStartRotation + (fChars[i].fEndRotation - fChars[i].fStartRotation)*fCurrTime/kDuration;
            canvas->translate(fChars[i].fPosition.fX + 35,fChars[i].fPosition.fY - 50);
            canvas->rotate(rot*180.0/SK_MScalarPI);
            canvas->translate(-35,+50);
            canvas->drawString(fChars[i].fChar, 0, 0,
                               paint);
            canvas->restore();
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
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
            fChars[i].fChar.set(&c, 1);
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
        SkString fChar;
        SkPoint  fPosition;
        SkScalar fStartRotation;
        SkScalar fEndRotation;
    };
    static constexpr int kNumChars = 40;
    AnimatedChar fChars[kNumChars];

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new FlutterAnimateView; }
static SkViewRegister reg(MyFactory);

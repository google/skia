/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "tools/ToolUtils.h"
#include "tools/timer/TimeUtils.h"

// Reproduces https://code.google.com/p/chromium/issues/detail?id=279014

constexpr int kWidth = 440;
constexpr int kHeight = 440;
constexpr SkScalar kAngle = 0.305f;
constexpr int kMaxNumSteps = 140;

// Renders a string art shape.
// The particular shape rendered can be controlled by adjusting kAngle, from 0 to 1

class StringArtGM : public skiagm::GM {
public:
    StringArtGM() : fNumSteps(kMaxNumSteps) {}

protected:

    SkString onShortName() override {
        return SkString("stringart");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar angle = kAngle*SK_ScalarPI + SkScalarHalf(SK_ScalarPI);
        SkScalar size = SkIntToScalar(SkMin32(kWidth, kHeight));
        SkPoint center = SkPoint::Make(SkScalarHalf(kWidth), SkScalarHalf(kHeight));
        SkScalar length = 5;
        SkScalar step = angle;

        SkPath path;
        path.moveTo(center);

        for (int i = 0; i < fNumSteps && length < (SkScalarHalf(size) - 10.f); ++i) {
            SkPoint rp = SkPoint::Make(length*SkScalarCos(step) + center.fX,
                                       length*SkScalarSin(step) + center.fY);
            path.lineTo(rp);
            length += angle / SkScalarHalf(SK_ScalarPI);
            step += angle;
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(ToolUtils::color_to_565(0xFF007700));

        canvas->drawPath(path, paint);
    }

    bool onAnimate(double nanos) override {
        constexpr SkScalar kDesiredDurationSecs = 3.0f;

        // Make the animation ping-pong back and forth but start in the fully drawn state
        SkScalar fraction = 1.0f - TimeUtils::Scaled(1e-9 * nanos, 2.0f/kDesiredDurationSecs, 2.0f);
        if (fraction <= 0.0f) {
            fraction = -fraction;
        }

        SkASSERT(fraction >= 0.0f && fraction <= 1.0f);

        fNumSteps = (int) (fraction * kMaxNumSteps);
        return true;
    }

private:
    int fNumSteps;

    typedef GM INHERITED;
};

DEF_GM( return new StringArtGM; )

/////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
#include "modules/skottie/include/Skottie.h"

class SkottieGM : public skiagm::GM {
    enum {
        kWidth = 800,
        kHeight = 600,
    };

    enum {
        N = 100,
    };
    skottie::Animation* fAnims[N];
    SkRect              fRects[N];
    SkScalar            fDur;

public:
    SkottieGM() {
        sk_bzero(fAnims, sizeof(fAnims));
    }
    ~SkottieGM() override {
        for (auto anim : fAnims) {
            SkSafeUnref(anim);
        }
    }

protected:

    SkString onShortName() override { return SkString("skottie"); }

    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }

    void init() {
        SkRandom rand;
        auto data = SkData::MakeFromFileName("/Users/reed/Downloads/maps_pinlet.json");
   //     for (;;) skottie::Animation::Make((const char*)data->data(), data->size());
        for (int i = 0; i < N; ++i) {
            fAnims[i] = skottie::Animation::Make((const char*)data->data(), data->size()).release();
            SkScalar x = rand.nextF() * kWidth;
            SkScalar y = rand.nextF() * kHeight;
            fRects[i].setXYWH(x, y, 400, 400);
        }
        fDur = fAnims[0]->duration();
    }

    void onDraw(SkCanvas* canvas) override {
        if (!fAnims[0]) {
            this->init();
        }
        canvas->drawColor(0xFFBBBBBB);
        for (int i = 0; i < N; ++i) {
            fAnims[0]->render(canvas, &fRects[i]);
        }
    }

    bool onAnimate(double nanos) override {
        SkScalar time = (float)(fmod(1e-9 * nanos, fDur) / fDur);
        for (auto anim : fAnims) {
            anim->seek(time);
        }
        return true;
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new SkottieGM; )
#endif


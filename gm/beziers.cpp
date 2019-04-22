/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkPath.h"
#include "include/utils/SkRandom.h"

#define W   400
#define H   400
#define N   10

constexpr SkScalar SH = SkIntToScalar(H);

static void rnd_quad(SkPath* p, SkPaint* paint, SkRandom& rand) {
    auto a = rand.nextRangeScalar(0,W),
         b = rand.nextRangeScalar(0,H);
    p->moveTo(a,b);
    for (int x = 0; x < 2; ++x) {
        auto c = rand.nextRangeScalar(W/4, W),
             d = rand.nextRangeScalar(  0, H),
             e = rand.nextRangeScalar(  0, W),
             f = rand.nextRangeScalar(H/4, H);
        p->quadTo(c,d,e,f);
    }
    paint->setColor(rand.nextU());
    SkScalar width = rand.nextRangeScalar(1, 5);
    width *= width;
    paint->setStrokeWidth(width);
    paint->setAlphaf(1.0f);
}

static void rnd_cubic(SkPath* p, SkPaint* paint, SkRandom& rand) {
    auto a = rand.nextRangeScalar(0,W),
         b = rand.nextRangeScalar(0,H);
    p->moveTo(a,b);
    for (int x = 0; x < 2; ++x) {
        auto c = rand.nextRangeScalar(W/4, W),
             d = rand.nextRangeScalar(  0, H),
             e = rand.nextRangeScalar(  0, W),
             f = rand.nextRangeScalar(H/4, H),
             g = rand.nextRangeScalar(W/4, W),
             h = rand.nextRangeScalar(H/4, H);
        p->cubicTo(c,d,e,f,g,h);
    }
    paint->setColor(rand.nextU());
    SkScalar width = rand.nextRangeScalar(1, 5);
    width *= width;
    paint->setStrokeWidth(width);
    paint->setAlphaf(1.0f);
}

class BeziersGM : public skiagm::GM {
public:
    BeziersGM() {}

protected:

    SkString onShortName() override {
        return SkString("beziers");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H*2);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(9)/2);
        paint.setAntiAlias(true);

        SkRandom rand;
        for (int i = 0; i < N; i++) {
            SkPath p;
            rnd_quad(&p, &paint, rand);
            canvas->drawPath(p, paint);
        }
        canvas->translate(0, SH);
        for (int i = 0; i < N; i++) {
            SkPath p;
            rnd_cubic(&p, &paint, rand);
            canvas->drawPath(p, paint);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new BeziersGM; )

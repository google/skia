/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"
#include "SkRandom.h"

#define W   400
#define H   400
#define N   10

static const SkScalar SH = SkIntToScalar(H);

static void rnd_quad(SkPath* p, SkPaint* paint, SkRandom& rand) {
    p->moveTo(rand.nextRangeScalar(0,  W), rand.nextRangeScalar(0,  H));
    for (int x = 0; x < 2; ++x) {
        p->quadTo(rand.nextRangeScalar(W / 4,  W), rand.nextRangeScalar(0,  H),
                rand.nextRangeScalar(0,  W), rand.nextRangeScalar(H / 4,  H));
    }
    paint->setColor(rand.nextU());
    SkScalar width = rand.nextRangeScalar(1, 5);
    width *= width;
    paint->setStrokeWidth(width);
    paint->setAlpha(0xFF);
}

static void rnd_cubic(SkPath* p, SkPaint* paint, SkRandom& rand) {
    p->moveTo(rand.nextRangeScalar(0,  W), rand.nextRangeScalar(0,  H));
    for (int x = 0; x < 2; ++x) {
        p->cubicTo(rand.nextRangeScalar(W / 4,  W), rand.nextRangeScalar(0,  H),
                rand.nextRangeScalar(0,  W), rand.nextRangeScalar(H / 4,  H),
                rand.nextRangeScalar(W / 4,  W), rand.nextRangeScalar(H / 4,  H));
    }
    paint->setColor(rand.nextU());
    SkScalar width = rand.nextRangeScalar(1, 5);
    width *= width;
    paint->setStrokeWidth(width);
    paint->setAlpha(0xFF);
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

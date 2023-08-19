/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"

#define W   400
#define H   400
#define N   10

constexpr SkScalar SH = SkIntToScalar(H);

static SkPath rnd_quad(SkPaint* paint, SkRandom& rand) {
    auto a = rand.nextRangeScalar(0,W),
         b = rand.nextRangeScalar(0,H);

    SkPathBuilder builder;
    builder.moveTo(a, b);
    for (int x = 0; x < 2; ++x) {
        auto c = rand.nextRangeScalar(W/4, W),
             d = rand.nextRangeScalar(  0, H),
             e = rand.nextRangeScalar(  0, W),
             f = rand.nextRangeScalar(H/4, H);
        builder.quadTo(c,d,e,f);
    }
    paint->setColor(rand.nextU());
    SkScalar width = rand.nextRangeScalar(1, 5);
    width *= width;
    paint->setStrokeWidth(width);
    paint->setAlphaf(1.0f);
    return builder.detach();
}

static SkPath rnd_cubic(SkPaint* paint, SkRandom& rand) {
    auto a = rand.nextRangeScalar(0,W),
         b = rand.nextRangeScalar(0,H);

    SkPathBuilder builder;
    builder.moveTo(a, b);
    for (int x = 0; x < 2; ++x) {
        auto c = rand.nextRangeScalar(W/4, W),
             d = rand.nextRangeScalar(  0, H),
             e = rand.nextRangeScalar(  0, W),
             f = rand.nextRangeScalar(H/4, H),
             g = rand.nextRangeScalar(W/4, W),
             h = rand.nextRangeScalar(H/4, H);
        builder.cubicTo(c,d,e,f,g,h);
    }
    paint->setColor(rand.nextU());
    SkScalar width = rand.nextRangeScalar(1, 5);
    width *= width;
    paint->setStrokeWidth(width);
    paint->setAlphaf(1.0f);
    return builder.detach();
}

class BeziersGM : public skiagm::GM {
public:
    BeziersGM() {}

protected:
    SkString getName() const override { return SkString("beziers"); }

    SkISize getISize() override { return SkISize::Make(W, H * 2); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(9)/2);
        paint.setAntiAlias(true);

        SkRandom rand;
        for (int i = 0; i < N; i++) {
            canvas->drawPath(rnd_quad(&paint, rand), paint);
        }
        canvas->translate(0, SH);
        for (int i = 0; i < N; i++) {
            canvas->drawPath(rnd_cubic(&paint, rand), paint);
        }
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM( return new BeziersGM; )

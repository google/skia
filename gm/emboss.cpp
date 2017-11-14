/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"

#ifdef SK_SUPPORT_LEGACY_EMBOSSMASKFILTER
static SkBitmap make_bm() {
    SkBitmap bm;
    bm.allocN32Pixels(100, 100);

    SkCanvas canvas(bm);
    canvas.clear(0);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawCircle(50, 50, 50, paint);
    return bm;
}

class EmbossGM : public skiagm::GM {
public:
    EmbossGM() {
    }

protected:
    SkString onShortName() override {
        return SkString("emboss");
    }

    SkISize onISize() override {
        return SkISize::Make(600, 120);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        SkBitmap bm = make_bm();
        canvas->drawBitmap(bm, 10, 10, &paint);

        const SkScalar dir[] = { 1, 1, 1 };
        paint.setMaskFilter(SkBlurMaskFilter::MakeEmboss(3, dir, 0.3f, 0.1f));
        canvas->translate(bm.width() + SkIntToScalar(10), 0);
        canvas->drawBitmap(bm, 10, 10, &paint);

        // this combination of emboss+colorfilter used to crash -- so we exercise it to
        // confirm that we have a fix.
        paint.setColorFilter(SkColorFilter::MakeModeFilter(0xFFFF0000, SkBlendMode::kSrcATop));
        canvas->translate(bm.width() + SkIntToScalar(10), 0);
        canvas->drawBitmap(bm, 10, 10, &paint);
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new EmbossGM;)
#endif

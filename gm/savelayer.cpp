/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

static void saveLayer(SkCanvas* canvas, SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
//    uint32_t flag = 1U << 31;
    uint32_t flag = 0;
    SkRect rect = SkRect::MakeLTRB(l, t, r, b);
    canvas->saveLayer({ &rect, nullptr, nullptr, flag });
}

static void do_draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRandom rand;

    SkAutoCanvasRestore acr(canvas, true);
    for (int i = 0; i < 20; ++i) {
        paint.setColor(rand.nextU() | (0xFF << 24));
        canvas->drawRect({ 15, 15, 290, 40 }, paint);
        canvas->translate(0, 30);
    }
}

class UnclippedSaveLayerGM : public skiagm::GM {
public:
    UnclippedSaveLayerGM() { this->setBGColor(SK_ColorWHITE); }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("savelayer_unclipped"); }

    SkISize onISize() override { return SkISize::Make(320, 640); }

    void onDraw(SkCanvas* canvas) override {
        const SkScalar L = 10;
        const SkScalar T = 10;
        const SkScalar R = 310;
        const SkScalar B = 630;

        canvas->clipRect({ L, T, R, B });

        for (int i = 0; i < 100; ++i) {
            SkAutoCanvasRestore acr(canvas, true);
#if 0
            saveLayer(canvas, L, T, R, T + 20);
            saveLayer(canvas, L, B - 20, R, B);
#else
            saveLayer(canvas, 0, 0, 320, 640);
#endif
            do_draw(canvas);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new UnclippedSaveLayerGM;)



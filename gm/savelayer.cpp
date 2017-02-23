/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"


static const uint32_t SkCanvas_kDontClipToLayer_PrivateSaveLayerFlag = 1U << 31;

// This GM tests out the deprecated Android-specific unclipped saveLayer "feature".
// In particular, it attempts to compare the performance of unclipped saveLayers with alternatives.

static void save_layer_unclipped(SkCanvas* canvas,
                                 SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
    SkRect rect = SkRect::MakeLTRB(l, t, r, b);
    canvas->saveLayer({ &rect, nullptr, nullptr, SkCanvas_kDontClipToLayer_PrivateSaveLayerFlag });
}

static void do_draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRandom rand;

    for (int i = 0; i < 20; ++i) {
        paint.setColor(sk_tool_utils::color_to_565(rand.nextU() | (0xFF << 24)));
        canvas->drawRect({ 15, 15, 290, 40 }, paint);
        canvas->translate(0, 30);
    }
}

class UnclippedSaveLayerGM : public skiagm::GM {
public:
    enum class Mode {
        kClipped,
        kUnclipped
    };

    UnclippedSaveLayerGM(Mode mode) : fMode(mode) { this->setBGColor(SK_ColorWHITE); }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override {
        if (Mode::kClipped == fMode) {
            return SkString("savelayer_unclipped");
        } else {
            SkASSERT(Mode::kUnclipped == fMode);
            return SkString("savelayer_clipped");
        }
    }

    SkISize onISize() override { return SkISize::Make(320, 640); }

    void onDraw(SkCanvas* canvas) override {
        const SkScalar L = 10;
        const SkScalar T = 10;
        const SkScalar R = 310;
        const SkScalar B = 630;

        canvas->clipRect({ L, T, R, B });

        for (int i = 0; i < 100; ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            if (Mode::kClipped == fMode) {
                save_layer_unclipped(canvas, L, T, R, T + 20);
                save_layer_unclipped(canvas, L, B - 20, R, B);
            } else {
                SkASSERT(Mode::kUnclipped == fMode);
                canvas->saveLayer({ L, T, R, B }, nullptr);
            }

            do_draw(canvas);
        }
    }

private:
    Mode fMode;

    typedef skiagm::GM INHERITED;
};

DEF_SIMPLE_GM(picture_savelayer, canvas, 320, 640) {
    SkPaint paint1, paint2, paint3;
    paint1.setAlpha(0x7f);
    paint2.setAlpha(0x3f);
    paint3.setColor(0xFFFF0000);
    SkRect rect1{40, 5, 80, 70}, rect2{5, 40, 70, 80}, rect3{10, 10, 70, 70};
    // In the future, we might also test the clipped case by allowing i = 0
    for(int i = 1; i < 2; ++i) {
        canvas->translate(100 * i, 0);
        auto flag = i ? SkCanvas_kDontClipToLayer_PrivateSaveLayerFlag : 0;
        canvas->saveLayer({ &rect1, &paint1, nullptr, flag});
        canvas->saveLayer({ &rect2, &paint2, nullptr, flag});
        canvas->drawRect(rect3, paint3);
        canvas->restore();
        canvas->restore();
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new UnclippedSaveLayerGM(UnclippedSaveLayerGM::Mode::kClipped);)
DEF_GM(return new UnclippedSaveLayerGM(UnclippedSaveLayerGM::Mode::kUnclipped);)


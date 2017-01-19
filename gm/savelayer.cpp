/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

// This GM tests out the deprecated Android-specific unclipped saveLayer "feature".
// In particular, it attempts to compare the performance of unclipped saveLayers with alternatives.

static void save_layer_unclipped(SkCanvas* canvas,
                                 SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
    static const uint32_t kSecretDeprecated_DontClipToLayerFlag = 1U << 31;
    SkRect rect = SkRect::MakeLTRB(l, t, r, b);
    canvas->saveLayer({ &rect, nullptr, nullptr, kSecretDeprecated_DontClipToLayerFlag });
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

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new UnclippedSaveLayerGM(UnclippedSaveLayerGM::Mode::kClipped);)
DEF_GM(return new UnclippedSaveLayerGM(UnclippedSaveLayerGM::Mode::kUnclipped);)



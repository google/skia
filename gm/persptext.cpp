/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "include/utils/SkTextUtils.h"
#include "src/core/SkMatrixPriv.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>

class PerspTextGM : public skiagm::GM {
public:
    PerspTextGM(bool minimal) : fMinimal(minimal) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString getName() const override {
        return SkString(fMinimal ? "persptext_minimal" : "persptext");
    }

    SkISize getISize() override { return SkISize::Make(1024, 768); }

    // #define TEST_PERSP_CHECK

    void onDraw(SkCanvas* canvas) override {

        canvas->clear(0xffffffff);

        SkPaint paint;
        paint.setAntiAlias(true);

        SkFont font(ToolUtils::CreatePortableTypeface("serif", SkFontStyle()));
        font.setSubpixel(true);
        font.setSize(32);
        font.setBaselineSnap(false);

        const char* text = "Hamburgefons";
        const size_t textLen = strlen(text);

        SkScalar textWidth = font.measureText(text, textLen, SkTextEncoding::kUTF8,
                                              nullptr, nullptr);
        SkScalar textHeight = font.getMetrics(nullptr);

        SkScalar x = 10, y = textHeight + 5.f;
        const int kSteps = 8;
        float kMinimalFactor = fMinimal ? 32.f : 1.f;
        for (auto pm : {PerspMode::kX, PerspMode::kY, PerspMode::kXY}) {
            for (int i = 0; i < kSteps; ++i) {
                canvas->save();
#ifdef TEST_PERSP_CHECK
                // draw non-perspective text in the background for comparison
                paint.setColor(SK_ColorRED);
                canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, x, y, font, paint);
#endif

                SkMatrix persp = SkMatrix::I();
                switch (pm) {
                    case PerspMode::kX:
                        if (fMinimal) {
                            persp.setPerspX(i*0.0005f/kSteps/kMinimalFactor);
                        } else {
                            persp.setPerspX(i*0.00025f/kSteps);
                        }
                        break;
                    case PerspMode::kY:
                        persp.setPerspY(i*0.0025f/kSteps/kMinimalFactor);
                        break;
                    case PerspMode::kXY:
                        persp.setPerspX(i*-0.00025f/kSteps/kMinimalFactor);
                        persp.setPerspY(i*-0.00125f/kSteps/kMinimalFactor);
                        break;
                }
                persp = SkMatrix::Concat(persp, SkMatrix::Translate(-x, -y));
                persp = SkMatrix::Concat(SkMatrix::Translate(x, y), persp);
                canvas->concat(persp);

                paint.setColor(SK_ColorBLACK);
#ifdef TEST_PERSP_CHECK
                // Draw text as red if it is nearly affine
                SkRect bounds = SkRect::MakeXYWH(0, -textHeight, textWidth, textHeight);
                bounds.offset(x, y);
                if (SkMatrixPriv::NearlyAffine(persp, bounds, SK_Scalar1/(1 << 4))) {
                    paint.setColor(SK_ColorRED);
                }
#endif
                canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, x, y, font, paint);

                y += textHeight + 5.f;
                canvas->restore();
            }

            x += textWidth + 10.f;
            y = textHeight + 5.f;
        }

    }

private:
    enum class PerspMode { kX, kY, kXY };
    bool fMinimal;
};

DEF_GM(return new PerspTextGM(true);)
DEF_GM(return new PerspTextGM(false);)

DEF_SIMPLE_GM(persptext_shader, canvas, 420, 180) {
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(ToolUtils::create_checkerboard_shader(SK_ColorBLUE, SK_ColorRED, 20));

    SkM44 persp;
    persp.setRC(3, 2, -1.0f / 350.0f);
    canvas->translate(210.0f, 90.0f);
    canvas->concat(persp *
                   SkM44::Rotate({0, 1, 0}, -SK_ScalarPI / 5) *
                   SkM44::Rotate({1, 0, 0}, SK_ScalarPI / 6));

    // background
    paint.setAlphaf(0.18f);
    canvas->drawRect({-180, -60, 180, 60}, paint);

    // text
    SkFont font(ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Bold()), 30.0f);
    paint.setAlphaf(1.0f);
    SkTextUtils::DrawString(
            canvas, "Checkerboard should", 0.0f, -8.0f, font, paint, SkTextUtils::kCenter_Align);
    SkTextUtils::DrawString(
            canvas, "align with background", 0.0f, 32.0f, font, paint, SkTextUtils::kCenter_Align);
}

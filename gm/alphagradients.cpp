/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradient.h"

class AlphaGradientsGM : public skiagm::GM {
public:
    AlphaGradientsGM() {}

protected:
    SkString getName() const override { return SkString("alphagradients"); }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    static void draw_grad(SkCanvas* canvas, const SkRect& r,
                          SkColor4f c0, SkColor4f c1, bool doPreMul) {
        SkColor4f colors[] = { c0, c1 };
        SkPoint pts[] = { { r.fLeft, r.fTop }, { r.fRight, r.fBottom } };
        SkPaint paint;
        auto pm = doPreMul ? SkGradient::Interpolation::InPremul::kYes
                           : SkGradient::Interpolation::InPremul::kNo;
        paint.setShader(SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {pm}}));
        canvas->drawRect(r, paint);

        paint.setShader(nullptr);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(r, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        constexpr struct {
            SkColor4f fColor0;
            SkColor4f fColor1;
        } gRec[] = {
            { SkColors::kWhite, {0, 0, 0, 0} },
            { SkColors::kWhite, {1, 0, 0, 0} },
            { SkColors::kWhite, {1, 1, 0, 0} },
            { SkColors::kWhite, {1, 1, 1, 0} },

            { SkColors::kRed, {0, 0, 0, 0} },
            { SkColors::kRed, {1, 0, 0, 0} },
            { SkColors::kRed, {1, 1, 0, 0} },
            { SkColors::kRed, {1, 1, 1, 0} },

            { SkColors::kBlue, {0, 0, 0, 0} },
            { SkColors::kBlue, {1, 0, 0, 0} },
            { SkColors::kBlue, {1, 1, 0, 0} },
            { SkColors::kBlue, {1, 1, 1, 0} },
        };

        SkRect r = SkRect::MakeWH(300, 30);

        canvas->translate(10, 10);

        for (int doPreMul = 0; doPreMul <= 1; ++doPreMul) {
            canvas->save();
            for (size_t i = 0; i < std::size(gRec); ++i) {
                draw_grad(canvas, r, gRec[i].fColor0, gRec[i].fColor1, SkToBool(doPreMul));
                canvas->translate(0, r.height() + 8);
            }
            canvas->restore();
            canvas->translate(r.width() + 10, 0);
        }
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM(return new AlphaGradientsGM;)

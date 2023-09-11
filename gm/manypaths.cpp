/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"

namespace skiagm {

static SkColor gen_color(SkRandom* rand) {
    SkScalar hsv[3];
    hsv[0] = rand->nextRangeF(0.0f, 360.0f);
    hsv[1] = rand->nextRangeF(0.5f, 1.0f);
    hsv[2] = rand->nextRangeF(0.5f, 1.0f);

    return ToolUtils::color_to_565(SkHSVToColor(hsv));
}

class ManyCirclesGM : public GM {
    // This GM attempts to flood Ganesh with more circles than will fit in a single index buffer
    // Stresses crbug.com/688582.
public:
    ManyCirclesGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    static const int kWidth = 800;
    static const int kHeight = 600;

    SkString getName() const override { return SkString("manycircles"); }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {
        SkRandom rand(1);
        SkPaint paint;
        paint.setAntiAlias(true);
        int total = 10000;
        while (total--) {
            SkScalar x = rand.nextF() * kWidth - 100;
            SkScalar y = rand.nextF() * kHeight - 100;
            SkScalar w = rand.nextF() * 200;
            SkRect circle = SkRect::MakeXYWH(x, y, w, w);
            paint.setColor(gen_color(&rand));
            canvas->drawOval(circle, paint);
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

class ManyRRectsGM : public GM {
    // This GM attempts to flood Ganesh with more rrects than will fit in a single index buffer
    // Stresses crbug.com/684112
public:
    ManyRRectsGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString getName() const override { return SkString("manyrrects"); }

    SkISize getISize() override { return SkISize::Make(800, 300); }

    void onDraw(SkCanvas* canvas) override {
        SkRandom rand(1);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);
        int total = 7000;

        // Rectangle positioning variables
        int x = 0;
        int y = 0;
        const int kXLimit = 700;
        const int kYIncrement = 5;
        const int kXIncrement = 5;

        SkRect rect = SkRect::MakeLTRB(0, 0, 4, 4);
        SkRRect rrect = SkRRect::MakeRectXY(rect, 1, 1);
        while (total--) {
            canvas->save();
            canvas->translate(x, y);
            canvas->drawRRect(rrect, paint);
            x += kXIncrement;
            if (x > kXLimit) {
                x = 0;
                y += kYIncrement;
            }
            canvas->restore();
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ManyCirclesGM; )
DEF_GM( return new ManyRRectsGM; )

}  // namespace skiagm

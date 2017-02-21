/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkTArray.h"
#include "SkRandom.h"
#include "SkMatrix.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkBlurDrawLooper.h"
#include "SkRect.h"
#include "SkRRect.h"

namespace skiagm {

class ManyCirclesGM : public GM {
public:
    ManyCirclesGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:

    SkString onShortName() override {
        return SkString("manycircles");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 600);
    }

    SkColor genColor(SkRandom* rand) {
        SkScalar hsv[3];
        hsv[0] = rand->nextRangeF(0.0f, 360.0f);
        hsv[1] = rand->nextRangeF(0.5f, 1.0f);
        hsv[2] = rand->nextRangeF(0.5f, 1.0f);

        return sk_tool_utils::color_to_565(SkHSVToColor(hsv));
    }

    void onDraw(SkCanvas* canvas) override {
        SkRandom rand(1);
        SkPaint paint;
        paint.setAntiAlias(true);
        int total = 10000;
        while (total--) {
            SkScalar x = rand.nextF() * 800 - 100;
            SkScalar y = rand.nextF() * 600 - 100;
            SkScalar w = rand.nextF() * 200;
            SkRect circle = SkRect::MakeXYWH(x, y, w, w);
            paint.setColor(this->genColor(&rand));
            canvas->drawOval(circle, paint);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* CircleFactory(void*) { return new ManyCirclesGM; }
static GMRegistry circlereg(CircleFactory);

//////////////////////////////////////////////////////////////////////////////

class ManyRRectsGM : public GM {
public:
    ManyRRectsGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:

    SkString onShortName() override {
        return SkString("manyrrects");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 300);
    }

    SkColor genColor(SkRandom* rand) {
        SkScalar hsv[3];
        hsv[0] = rand->nextRangeF(0.0f, 360.0f);
        hsv[1] = rand->nextRangeF(0.5f, 1.0f);
        hsv[2] = rand->nextRangeF(0.5f, 1.0f);

        return sk_tool_utils::color_to_565(SkHSVToColor(hsv));
    }

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
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* RRectFactory(void*) { return new ManyRRectsGM; }
static GMRegistry rrectreg(RRectFactory);

}

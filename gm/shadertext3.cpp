/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "tools/ToolUtils.h"

#include <string.h>

namespace skiagm {

static void makebm(SkBitmap* bm, int w, int h) {
    bm->allocN32Pixels(w, h);
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas    canvas(*bm);
    SkScalar    s = SkIntToScalar(SkMin32(w, h));
    const SkPoint     kPts0[] = { { 0, 0 }, { s, s } };
    const SkPoint     kPts1[] = { { s/2, 0 }, { s/2, s } };
    const SkScalar    kPos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    const SkColor kColors0[] = {0x80F00080, 0xF0F08000, 0x800080F0 };
    const SkColor kColors1[] = {0xF08000F0, 0x8080F000, 0xF000F080 };


    SkPaint     paint;

    paint.setShader(SkGradientShader::MakeLinear(kPts0, kColors0, kPos,
                    SK_ARRAY_COUNT(kColors0), SkTileMode::kClamp));
    canvas.drawPaint(paint);
    paint.setShader(SkGradientShader::MakeLinear(kPts1, kColors1, kPos,
                    SK_ARRAY_COUNT(kColors1), SkTileMode::kClamp));
    canvas.drawPaint(paint);
}

///////////////////////////////////////////////////////////////////////////////

struct LabeledMatrix {
    SkMatrix    fMatrix;
    const char* fLabel;
};

constexpr int kPointSize = 300;

class ShaderText3GM : public GM {
public:
    ShaderText3GM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:

    SkString onShortName() override {
        return SkString("shadertext3");
    }

    SkISize onISize() override{ return SkISize::Make(820, 930); }

    void onOnceBeforeDraw() override {
        makebm(&fBmp, kPointSize / 4, kPointSize / 4);
    }

    void onDraw(SkCanvas* canvas) override {

        SkPaint bmpPaint;
        bmpPaint.setAntiAlias(true);
        bmpPaint.setFilterQuality(kLow_SkFilterQuality);
        bmpPaint.setAlphaf(0.5f);
        canvas->drawBitmap(fBmp, 5.f, 5.f, &bmpPaint);

        SkFont  font(ToolUtils::create_portable_typeface(), SkIntToScalar(kPointSize));
        SkPaint outlinePaint;
        outlinePaint.setStyle(SkPaint::kStroke_Style);
        outlinePaint.setStrokeWidth(0.f);

        canvas->translate(15.f, 15.f);

        // draw glyphs scaled up
        canvas->scale(2.f, 2.f);

        constexpr SkTileMode kTileModes[] = {
            SkTileMode::kRepeat,
            SkTileMode::kMirror,
        };

        // position the baseline of the first run
        canvas->translate(0.f, 0.75f * kPointSize);

        canvas->save();
        int i = 0;
        for (size_t tm0 = 0; tm0 < SK_ARRAY_COUNT(kTileModes); ++tm0) {
            for (size_t tm1 = 0; tm1 < SK_ARRAY_COUNT(kTileModes); ++tm1) {
                SkMatrix localM;
                localM.setTranslate(5.f, 5.f);
                localM.postRotate(20);
                localM.postScale(1.15f, .85f);

                SkPaint fillPaint;
                fillPaint.setAntiAlias(true);
                fillPaint.setFilterQuality(kLow_SkFilterQuality);
                fillPaint.setShader(fBmp.makeShader(kTileModes[tm0], kTileModes[tm1], &localM));

                constexpr char kText[] = "B";
                canvas->drawString(kText, 0, 0, font, fillPaint);
                canvas->drawString(kText, 0, 0, font, outlinePaint);
                SkScalar w = font.measureText(kText, strlen(kText), kUTF8_SkTextEncoding);
                canvas->translate(w + 10.f, 0.f);
                ++i;
                if (!(i % 2)) {
                    canvas->restore();
                    canvas->translate(0, 0.75f * kPointSize);
                    canvas->save();
                }
            }
        }
        canvas->restore();
    }

private:
    SkBitmap fBmp;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ShaderText3GM; )
}

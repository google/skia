/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkRandom.h"

static sk_sp<SkShader> make_shader1(SkScalar w, SkScalar h) {
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorCYAN, SK_ColorGREEN, SK_ColorWHITE,
        SK_ColorMAGENTA, SK_ColorBLUE, SK_ColorYELLOW,
    };
    const SkPoint pts[] = { { w/4, 0 }, { 3*w/4, h } };

    return SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                        SkShader::kMirror_TileMode);
}

static sk_sp<SkShader> make_shader2() {
    return SkShader::MakeColorShader(SK_ColorBLUE);
}

static sk_sp<SkColorFilter> make_color_filter() {
    return SkColorFilter::MakeModeFilter(0xFFAABBCC, SkXfermode::kDarken_Mode);
}

class VerticesGM : public skiagm::GM {
    SkPoint                 fPts[9];
    SkPoint                 fTexs[9];
    SkColor                 fColors[9];
    sk_sp<SkShader>         fShader1;
    sk_sp<SkShader>         fShader2;
    sk_sp<SkColorFilter>    fColorFilter;

public:
    VerticesGM() {}

protected:

    void onOnceBeforeDraw() override {
        const SkScalar X = 30;
        const SkScalar Y = 30;

        fPts[0].set(0, 0);    fPts[1].set(X/2, 3);   fPts[2].set(X, 0);
        fPts[3].set(3, Y/2);  fPts[4].set(X/2, Y/2); fPts[5].set(X-3, Y/2);
        fPts[6].set(0, Y);    fPts[7].set(X/2, Y-3); fPts[8].set(X, Y);

        const SkScalar w = 40;
        const SkScalar h = 40;

        fTexs[0].set(0, 0);     fTexs[1].set(w/2, 0);   fTexs[2].set(w, 0);
        fTexs[3].set(0, h/2);   fTexs[4].set(w/2, h/2); fTexs[5].set(w, h/2);
        fTexs[6].set(0, h);     fTexs[7].set(w/2, h);   fTexs[8].set(w, h);

        fShader1 = make_shader1(w, h);
        fShader2 = make_shader2();
        fColorFilter = make_color_filter();

        SkRandom rand;
        for (size_t i = 0; i < SK_ARRAY_COUNT(fColors); ++i) {
            fColors[i] = rand.nextU() | 0xFF000000;
        }
    }

    SkString onShortName() override {
        SkString name("vertices");
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(600, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        // start with the center of a 3x3 grid
        static const uint16_t fan[] = {
            4,
            0, 1, 2, 5, 8, 7, 6, 3, 0
        };

        const struct {
            const SkColor*              fColors;
            const SkPoint*              fTexs;
            const sk_sp<SkShader>&      fShader;
            const sk_sp<SkColorFilter>& fColorFilter;
            uint8_t                     fAlpha;
        } rec[] = {
            { fColors,  nullptr, fShader1, nullptr     , 0xFF },
            { nullptr,  fTexs  , fShader1, nullptr     , 0xFF },
            { fColors,  fTexs  , fShader1, nullptr     , 0xFF },
            { fColors,  nullptr, fShader2, nullptr     , 0xFF },
            { nullptr,  fTexs  , fShader2, nullptr     , 0xFF },
            { fColors,  fTexs  , fShader2, nullptr     , 0xFF },
            { fColors,  nullptr, fShader1, fColorFilter, 0xFF },
            { nullptr,  fTexs  , fShader1, fColorFilter, 0xFF },
            { fColors,  fTexs  , fShader1, fColorFilter, 0xFF },
            { fColors,  nullptr, fShader2, fColorFilter, 0xFF },
            { nullptr,  fTexs  , fShader2, fColorFilter, 0xFF },
            { fColors,  fTexs  , fShader2, fColorFilter, 0xFF },
            { fColors,  nullptr, fShader1, nullptr     , 0x80 },
            { nullptr,  fTexs  , fShader1, nullptr     , 0x80 },
            { fColors,  fTexs  , fShader1, nullptr     , 0x80 },
            { fColors,  nullptr, fShader2, nullptr     , 0x80 },
            { nullptr,  fTexs  , fShader2, nullptr     , 0x80 },
            { fColors,  fTexs  , fShader2, nullptr     , 0x80 },
            { fColors,  nullptr, fShader1, fColorFilter, 0x80 },
            { nullptr,  fTexs  , fShader1, fColorFilter, 0x80 },
            { fColors,  fTexs  , fShader1, fColorFilter, 0x80 },
            { fColors,  nullptr, fShader2, fColorFilter, 0x80 },
            { nullptr,  fTexs  , fShader2, fColorFilter, 0x80 },
            { fColors,  fTexs  , fShader2, fColorFilter, 0x80 },
        };

        const SkXfermode::Mode modes[] = {
            SkXfermode::kClear_Mode,
            SkXfermode::kSrc_Mode,
            SkXfermode::kDst_Mode,
            SkXfermode::kSrcOver_Mode,
            SkXfermode::kDstOver_Mode,
            SkXfermode::kSrcIn_Mode,
            SkXfermode::kDstIn_Mode,
            SkXfermode::kSrcOut_Mode,
            SkXfermode::kDstOut_Mode,
            SkXfermode::kSrcATop_Mode,
            SkXfermode::kDstATop_Mode,
            SkXfermode::kXor_Mode,
            SkXfermode::kPlus_Mode,
            SkXfermode::kModulate_Mode,
            SkXfermode::kScreen_Mode,
            SkXfermode::kOverlay_Mode,
            SkXfermode::kDarken_Mode,
            SkXfermode::kLighten_Mode,
            SkXfermode::kColorDodge_Mode,
            SkXfermode::kColorBurn_Mode,
            SkXfermode::kHardLight_Mode,
            SkXfermode::kSoftLight_Mode,
            SkXfermode::kDifference_Mode,
            SkXfermode::kExclusion_Mode,
            SkXfermode::kMultiply_Mode,
            SkXfermode::kHue_Mode,
            SkXfermode::kSaturation_Mode,
            SkXfermode::kColor_Mode,
            SkXfermode::kLuminosity_Mode,
        };

        SkPaint paint;

        canvas->translate(4, 4);
        int x = 0;
        for (size_t j = 0; j < SK_ARRAY_COUNT(modes); ++j) {
            auto xfer = SkXfermode::Make(modes[j]);
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
                paint.setShader(rec[i].fShader);
                paint.setColorFilter(rec[i].fColorFilter);
                paint.setAlpha(rec[i].fAlpha);
                //if (2 == x)
                canvas->drawVertices(SkCanvas::kTriangleFan_VertexMode,
                                     SK_ARRAY_COUNT(fPts), fPts,
                                     rec[i].fTexs, rec[i].fColors,
                                     xfer, fan, SK_ARRAY_COUNT(fan), paint);
                canvas->translate(40, 0);
                ++x;
            }
            canvas->restore();
            canvas->translate(0, 40);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////

DEF_GM(return new VerticesGM();)

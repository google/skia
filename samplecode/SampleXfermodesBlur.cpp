/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sample.h"
#include "SkBitmap.h"
#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkCornerPathEffect.h"
#include "SkFont.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUTF.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTextUtils.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkStream.h"
#include "SkColorPriv.h"
#include "SkBlurMaskFilter.h"

static void setNamedTypeface(SkFont* font, const char name[]) {
    font->setTypeface(SkTypeface::MakeFromName(name, SkFontStyle()));
}

static uint16_t gBG[] = { 0xFFFF, 0xCCCF, 0xCCCF, 0xFFFF };

class XfermodesBlurView : public Sample {
    SkBitmap    fBG;
    SkBitmap    fSrcB, fDstB;

    void draw_mode(SkCanvas* canvas, SkBlendMode mode, int alpha, SkScalar x, SkScalar y) {
        SkPaint p;
        p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                               SkBlurMask::ConvertRadiusToSigma(5)));

        SkScalar ww = SkIntToScalar(W);
        SkScalar hh = SkIntToScalar(H);

        // draw a circle covering the upper
        // left three quarters of the canvas
        p.setColor(0xFFCC44FF);
        SkRect r;
        r.set(0, 0, ww*3/4, hh*3/4);
        r.offset(x, y);
        canvas->drawOval(r, p);

        p.setBlendMode(mode);

        // draw a square overlapping the circle
        // in the lower right of the canvas
        p.setColor(0x00AA6633 | alpha << 24);
        r.set(ww/3, hh/3, ww*19/20, hh*19/20);
        r.offset(x, y);
        canvas->drawRect(r, p);
    }

public:
    const static int W = 64;
    const static int H = 64;
    XfermodesBlurView() {
        fBG.installPixels(SkImageInfo::Make(2, 2, kARGB_4444_SkColorType, kPremul_SkAlphaType),
                          gBG, 4);
    }

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "XfermodesBlur");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        const SkBlendMode gModes[] = {
            SkBlendMode::kClear,
            SkBlendMode::kSrc,
            SkBlendMode::kDst,
            SkBlendMode::kSrcOver,
            SkBlendMode::kDstOver,
            SkBlendMode::kSrcIn,
            SkBlendMode::kDstIn,
            SkBlendMode::kSrcOut,
            SkBlendMode::kDstOut,
            SkBlendMode::kSrcATop,
            SkBlendMode::kDstATop,
            SkBlendMode::kXor,
            SkBlendMode::kPlus,
        };

        const SkScalar w = SkIntToScalar(W);
        const SkScalar h = SkIntToScalar(H);
        SkMatrix m;
        m.setScale(SkIntToScalar(6), SkIntToScalar(6));
        auto s = fBG.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &m);

        SkFont font;
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        setNamedTypeface(&font, "Menlo Regular");

        const int W = 5;

        SkScalar x0 = 0;
        for (int twice = 0; twice < 2; twice++) {
            SkScalar x = x0, y = 0;
            for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
                SkRect r;
                r.set(x, y, x+w, y+h);

                SkPaint p;
                p.setStyle(SkPaint::kFill_Style);
                p.setShader(s);
                canvas->drawRect(r, p);

                canvas->saveLayer(&r, nullptr);
                draw_mode(canvas, gModes[i], twice ? 0x88 : 0xFF, r.fLeft, r.fTop);
                canvas->restore();

                r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
                p.setStyle(SkPaint::kStroke_Style);
                p.setShader(nullptr);
                canvas->drawRect(r, p);

                const char* label = SkBlendMode_Name(gModes[i]);
                SkTextUtils::DrawString(canvas, label, x + w/2, y - font.getSize()/2, font, SkPaint(),
                                        SkTextUtils::kCenter_Align);
                x += w + SkIntToScalar(10);
                if ((i % W) == W - 1) {
                    x = x0;
                    y += h + SkIntToScalar(30);
                }
            }
            x0 += SkIntToScalar(400);
        }
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new XfermodesBlurView(); )

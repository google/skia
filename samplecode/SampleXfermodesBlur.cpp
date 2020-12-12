/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"
#include "include/utils/SkTextUtils.h"
#include "samplecode/Sample.h"
#include "src/core/SkBlurMask.h"
#include "src/utils/SkUTF.h"

#include "include/core/SkColorPriv.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkStream.h"

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
        r.setLTRB(0, 0, ww*3/4, hh*3/4);
        r.offset(x, y);
        canvas->drawOval(r, p);

        p.setBlendMode(mode);

        // draw a square overlapping the circle
        // in the lower right of the canvas
        p.setColor(0x00AA6633 | alpha << 24);
        r.setLTRB(ww/3, hh/3, ww*19/20, hh*19/20);
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
    SkString name() override { return SkString("XfermodesBlur"); }

    void onDrawContent(SkCanvas* canvas) override {
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
        auto s = fBG.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions(), m);

        SkFont font;
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        setNamedTypeface(&font, "Menlo Regular");

        const int W = 5;

        SkScalar x0 = 0;
        for (int twice = 0; twice < 2; twice++) {
            SkScalar x = x0, y = 0;
            for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
                SkRect r;
                r.setLTRB(x, y, x+w, y+h);

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
    using INHERITED = Sample;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new XfermodesBlurView(); )

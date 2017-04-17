/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkCornerPathEffect.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkStream.h"
#include "SkColorPriv.h"
#include "SkBlurMaskFilter.h"

static void setNamedTypeface(SkPaint* paint, const char name[]) {
    paint->setTypeface(SkTypeface::MakeFromName(name, SkFontStyle()));
}

static uint16_t gBG[] = { 0xFFFF, 0xCCCF, 0xCCCF, 0xFFFF };

class XfermodesBlurView : public SampleView {
    SkBitmap    fBG;
    SkBitmap    fSrcB, fDstB;

    void draw_mode(SkCanvas* canvas, SkBlendMode mode, int alpha, SkScalar x, SkScalar y) {
        SkPaint p;
        p.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                               SkBlurMask::ConvertRadiusToSigma(5),
                                               SkBlurMaskFilter::kNone_BlurFlag));

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
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "XfermodesBlur");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        if (false) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setTextSize(50);
            paint.setTypeface(SkTypeface::MakeFromName("Arial Unicode MS", SkFontStyle()));
            char buffer[10];
            size_t len = SkUTF8_FromUnichar(0x8500, buffer);
            canvas->drawText(buffer, len, 40, 40, paint);
            return;
        }
        if (false) {
            SkPaint paint;
            paint.setAntiAlias(true);

            SkRect r0 = { 0, 0, 10.5f, 20 };
            SkRect r1 = { 10.5f, 10, 20, 30 };
            paint.setColor(SK_ColorRED);
            canvas->drawRect(r0, paint);
            paint.setColor(SK_ColorBLUE);
            canvas->drawRect(r1, paint);
            return;
        }

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
        auto s = SkShader::MakeBitmapShader(fBG, SkShader::kRepeat_TileMode,
                                            SkShader::kRepeat_TileMode, &m);

        SkPaint labelP;
        labelP.setAntiAlias(true);
        labelP.setLCDRenderText(true);
        labelP.setTextAlign(SkPaint::kCenter_Align);
        setNamedTypeface(&labelP, "Menlo Regular");

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
                canvas->drawString(label,
                                 x + w/2, y - labelP.getTextSize()/2, labelP);
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
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new XfermodesBlurView; }
static SkViewRegister reg(MyFactory);

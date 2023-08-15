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
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"

class LcdTextGM : public skiagm::GM {
    static constexpr SkScalar kTextHeight = 36;
    SkScalar fY = kTextHeight;

    SkString getName() const override { return SkString("lcdtext"); }

    SkISize getISize() override { return {640, 480}; }

    void onDraw(SkCanvas* canvas) override {
        fY = kTextHeight;
        drawText(canvas, SkString("TEXT: SubpixelTrue LCDRenderTrue"),
                 true,  true);
        drawText(canvas, SkString("TEXT: SubpixelTrue LCDRenderFalse"),
                 true,  false);
        drawText(canvas, SkString("TEXT: SubpixelFalse LCDRenderTrue"),
                 false, true);
        drawText(canvas, SkString("TEXT: SubpixelFalse LCDRenderFalse"),
                 false, false);
    }

    void drawText(SkCanvas* canvas, const SkString& string,
                  bool subpixelTextEnabled, bool lcdRenderTextEnabled) {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setDither(true);
        SkFont font(nullptr, kTextHeight);
        if (subpixelTextEnabled) {
            font.setSubpixel(true);
        }
        if (lcdRenderTextEnabled) {
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        }
        canvas->drawString(string, 0, fY, font, paint);
        fY += kTextHeight;
    }
};

/*
 *  Skia will automatically disable LCD requests if the total size exceeds some limit
 *  (hard coded in this test for now, as it is now avaiable as an API)
 *
 *  Test this both by changing "textsize" and by changing the computed size (textsize * CTM)
 */
class LcdTextSizeGM : public skiagm::GM {
    static void ScaleAbout(SkCanvas* canvas, SkScalar sx, SkScalar sy, SkScalar px, SkScalar py) {
        SkMatrix m;
        m.setScale(sx, sy, px, py);
        canvas->concat(m);
    }

    SkString getName() const override { return SkString("lcdtextsize"); }

    SkISize getISize() override { return {320, 120}; }

    void onDraw(SkCanvas* canvas) override {
        const char* lcd_text = "LCD";
        const char* gray_text = "GRAY";

        constexpr static float kLCDTextSizeLimit = 48;

        const struct {
            SkPoint     fLoc;
            SkScalar    fTextSize;
            SkScalar    fScale;
            const char* fText;
        } rec[] = {
            { {  10,  50 }, kLCDTextSizeLimit - 1,     1,  lcd_text },
            { { 160,  50 }, kLCDTextSizeLimit + 1,     1,  gray_text },
            { {  10, 100 }, kLCDTextSizeLimit / 2, 1.99f,  lcd_text },
            { { 160, 100 }, kLCDTextSizeLimit / 2, 2.01f,  gray_text },
        };

        for (size_t i = 0; i < std::size(rec); ++i) {
            const SkPoint loc = rec[i].fLoc;
            SkAutoCanvasRestore acr(canvas, true);

            SkFont font(nullptr, rec[i].fTextSize);
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

            ScaleAbout(canvas, rec[i].fScale, rec[i].fScale, loc.x(), loc.y());
            canvas->drawString(rec[i].fText, loc.x(), loc.y(), font, SkPaint());
        }
    }
};

class SaveLayerPreserveLCDTextGM : public skiagm::GM {
    static constexpr SkScalar kTextHeight = 36;

    SkString getName() const override { return SkString("savelayerpreservelcdtext"); }

    SkISize getISize() override { return {620, 300}; }

    void onDraw(SkCanvas* canvas) override {
        drawText(canvas, SkString("SaveLayer PreserveLCDText"), 50,
                 SkCanvas::kPreserveLCDText_SaveLayerFlag);
        drawText(canvas, SkString("SaveLayer Default (LCDText not preserved)"), 150, 0);
    }

    void drawText(SkCanvas* canvas,
                  const SkString& string,
                  int y,
                  SkCanvas::SaveLayerFlags saveLayerFlags) {
        SkCanvas::SaveLayerRec rec(nullptr, nullptr, saveLayerFlags);
        canvas->saveLayer(rec);
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        canvas->drawRect(SkRect::MakeXYWH(0, y - 10, 640, kTextHeight + 20), paint);
        paint.setColor(SK_ColorBLACK);
        SkFont font(nullptr, kTextHeight);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        canvas->drawString(string, 10, y, font, paint);
        canvas->restore();
    }
};

DEF_GM(return new LcdTextGM;)
DEF_GM(return new LcdTextSizeGM;)
DEF_GM(return new SaveLayerPreserveLCDTextGM;)

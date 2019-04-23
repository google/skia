/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "tools/ToolUtils.h"

#include "include/core/SkShader.h"

// Inspired by svg/as-border-image/svg-as-border-image.html. Draws a four-color checker board bitmap
// such that it is stretched and repeat tiled with different filter qualities. It is testing whether
// the bmp filter respects the repeat mode at the tile seams.
class BmpFilterQualityRepeat : public skiagm::GM {
public:
    BmpFilterQualityRepeat() { this->setBGColor(ToolUtils::color_to_565(0xFFCCBBAA)); }

protected:

    void onOnceBeforeDraw() override {
        fBmp.allocN32Pixels(40, 40, true);
        SkCanvas canvas(fBmp);
        SkBitmap colorBmp;
        colorBmp.allocN32Pixels(20, 20, true);
        colorBmp.eraseColor(0xFFFF0000);
        canvas.drawBitmap(colorBmp, 0, 0);
        colorBmp.eraseColor(ToolUtils::color_to_565(0xFF008200));
        canvas.drawBitmap(colorBmp, 20, 0);
        colorBmp.eraseColor(ToolUtils::color_to_565(0xFFFF9000));
        canvas.drawBitmap(colorBmp, 0, 20);
        colorBmp.eraseColor(ToolUtils::color_to_565(0xFF2000FF));
        canvas.drawBitmap(colorBmp, 20, 20);
    }

    SkString onShortName() override { return SkString("bmp_filter_quality_repeat"); }

    SkISize onISize() override { return SkISize::Make(1000, 400); }

    void onDraw(SkCanvas* canvas) override {
        this->drawAll(canvas, 2.5f);
        canvas->translate(0, 250);
        canvas->scale(0.5, 0.5);
        this->drawAll(canvas, 1);
    }

private:
    void drawAll(SkCanvas* canvas, SkScalar scaleX) const {
        constexpr struct {
            SkFilterQuality fQuality;
            const char* fName;
        } kQualities[] = {
            {kNone_SkFilterQuality, "none"},
            {kLow_SkFilterQuality, "low"},
            {kMedium_SkFilterQuality, "medium"},
            {kHigh_SkFilterQuality, "high"},
        };

        SkRect rect = SkRect::MakeLTRB(20, 60, 220, 210);
        SkMatrix lm = SkMatrix::I();
        lm.setScaleX(scaleX);
        lm.setTranslateX(423);
        lm.setTranslateY(330);

        SkPaint textPaint;
        textPaint.setAntiAlias(true);

        SkPaint bmpPaint(textPaint);

        SkFont font(ToolUtils::create_portable_typeface());

        SkAutoCanvasRestore acr(canvas, true);

        for (size_t q = 0; q < SK_ARRAY_COUNT(kQualities); ++q) {
            constexpr SkTileMode kTM = SkTileMode::kRepeat;
            bmpPaint.setShader(fBmp.makeShader(kTM, kTM, &lm));
            bmpPaint.setFilterQuality(kQualities[q].fQuality);
            canvas->drawRect(rect, bmpPaint);
            canvas->drawString(kQualities[q].fName, 20, 40, font, textPaint);
            canvas->translate(250, 0);
        }

    }

    SkBitmap    fBmp;

    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BmpFilterQualityRepeat;)

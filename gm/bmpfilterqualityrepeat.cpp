/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkShader.h"

// Inspired by svg/as-border-image/svg-as-border-image.html. Draws a four-color checker board bitmap
// such that it is stretched and repeat tiled with different filter qualities. It is testing whether
// the bmp filter respects the repeat mode at the tile seams.
class BmpFilterQualityRepeat : public skiagm::GM {
public:
    BmpFilterQualityRepeat() { this->setBGColor(sk_tool_utils::color_to_565(0xFFCCBBAA)); }

protected:

    void onOnceBeforeDraw() override {
        fBmp.allocN32Pixels(40, 40, true);
        SkCanvas canvas(fBmp);
        SkBitmap colorBmp;
        colorBmp.allocN32Pixels(20, 20, true);
        colorBmp.eraseColor(0xFFFF0000);
        canvas.drawBitmap(colorBmp, 0, 0);
        colorBmp.eraseColor(sk_tool_utils::color_to_565(0xFF008200));
        canvas.drawBitmap(colorBmp, 20, 0);
        colorBmp.eraseColor(sk_tool_utils::color_to_565(0xFFFF9000));
        canvas.drawBitmap(colorBmp, 0, 20);
        colorBmp.eraseColor(sk_tool_utils::color_to_565(0xFF2000FF));
        canvas.drawBitmap(colorBmp, 20, 20);
    }

    SkString onShortName() override { return SkString("bmp_filter_quality_repeat"); }

    SkISize onISize() override { return SkISize::Make(1000, 235); }

    void onDraw(SkCanvas* canvas) override {

        static const struct { 
            SkFilterQuality fQuality;
            const char* fName;
        } kQualities[] = {
            {kNone_SkFilterQuality, "none"},
            {kLow_SkFilterQuality, "low"},
            {kMedium_SkFilterQuality, "medium"},
            {kHigh_SkFilterQuality, "high"},
        };

        for (size_t q = 0; q < SK_ARRAY_COUNT(kQualities); ++q) {
            SkPaint paint;
            sk_tool_utils::set_portable_typeface(&paint);
            paint.setFilterQuality(kQualities[q].fQuality);
            SkPaint bmpPaint(paint);
            SkMatrix lm = SkMatrix::I();
            lm.setScaleX(2.5);
            lm.setTranslateX(423);
            lm.setTranslateY(330);

            static const SkShader::TileMode kTM = SkShader::kRepeat_TileMode;
            bmpPaint.setShader(SkShader::CreateBitmapShader(fBmp, kTM, kTM, &lm))->unref();
            SkRect rect = SkRect::MakeLTRB(20, 60, 220, 210);
            canvas->drawRect(rect, bmpPaint);
            paint.setAntiAlias(true);
            canvas->drawText(kQualities[q].fName, strlen(kQualities[q].fName), 20, 40, paint);
            canvas->translate(250, 0);
        }
    }

private:
    SkBitmap    fBmp;

    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BmpFilterQualityRepeat;)

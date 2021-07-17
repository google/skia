/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/ToolUtils.h"

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
        canvas.drawImage(colorBmp.asImage(), 0, 0);
        colorBmp.eraseColor(ToolUtils::color_to_565(0xFF008200));
        canvas.drawImage(colorBmp.asImage(), 20, 0);
        colorBmp.eraseColor(ToolUtils::color_to_565(0xFFFF9000));
        canvas.drawImage(colorBmp.asImage(), 0, 20);
        colorBmp.eraseColor(ToolUtils::color_to_565(0xFF2000FF));
        canvas.drawImage(colorBmp.asImage(), 20, 20);
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

        const struct {
            const char* name;
            SkSamplingOptions sampling;
        } recs[] = {
            { "none",   SkSamplingOptions(SkFilterMode::kNearest) },
            { "low",    SkSamplingOptions(SkFilterMode::kLinear) },
            { "medium", SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear) },
            { "high",   SkSamplingOptions(SkCubicResampler::Mitchell()) },
        };

        for (const auto& rec : recs) {
            constexpr SkTileMode kTM = SkTileMode::kRepeat;
            bmpPaint.setShader(fBmp.makeShader(kTM, kTM, rec.sampling, lm));
            canvas->drawRect(rect, bmpPaint);
            canvas->drawString(rec.name, 20, 40, font, textPaint);
            canvas->translate(250, 0);
        }

    }

    SkBitmap    fBmp;

    using INHERITED = skiagm::GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BmpFilterQualityRepeat;)

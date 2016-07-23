/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/*
 * Tests overlapping LCD text
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkTextBlob.h"

namespace skiagm {

static const int kWidth = 750;
static const int kHeight = 750;

class LcdOverlapGM : public skiagm::GM {
public:
    LcdOverlapGM() {
        const int kPointSize = 25;
        fTextHeight = SkIntToScalar(kPointSize);
    }

protected:
    SkString onShortName() override {
        return SkString("lcdoverlap");
    }

    void onOnceBeforeDraw() override {
        // build text blob
        SkTextBlobBuilder builder;

        SkPaint paint;
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(32);
        const char* text = "able was I ere I saw elba";
        paint.setAntiAlias(true);
        paint.setSubpixelText(true);
        paint.setLCDRenderText(true);
        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, 0);
        fBlob.reset(builder.build());
    }

    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }

    void drawTestCase(SkCanvas* canvas, SkScalar x, SkScalar y, SkXfermode::Mode mode,
                      SkXfermode::Mode mode2) {
        const SkColor colors[] {
                SK_ColorRED,
                SK_ColorGREEN,
                SK_ColorBLUE,
                SK_ColorYELLOW,
                SK_ColorCYAN,
                SK_ColorMAGENTA,
        };

        sk_sp<SkXfermode> xfermode(SkXfermode::Make(mode));
        sk_sp<SkXfermode> xfermode2(SkXfermode::Make(mode2));
        for (size_t i = 0; i < SK_ARRAY_COUNT(colors); i++) {
            canvas->save();
            canvas->translate(x, y);
            canvas->rotate(360.0f / SK_ARRAY_COUNT(colors) * i);
            canvas->translate(-fBlob->bounds().width() / 2.0f + 0.5f, 0);

            SkPaint textPaint;
            textPaint.setColor(colors[i]);
            textPaint.setXfermode(i % 2 == 0 ? xfermode : xfermode2);
            canvas->drawTextBlob(fBlob, 0, 0, textPaint);
            canvas->restore();
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar offsetX = kWidth / 4.0f;
        SkScalar offsetY = kHeight / 4.0f;
        drawTestCase(canvas, offsetX, offsetY,  SkXfermode::kSrc_Mode, SkXfermode::kSrc_Mode);
        drawTestCase(canvas, 3 * offsetX, offsetY,  SkXfermode::kSrcOver_Mode,
                     SkXfermode::kSrcOver_Mode);
        drawTestCase(canvas, offsetX, 3 * offsetY,  SkXfermode::kHardLight_Mode,
                     SkXfermode::kLuminosity_Mode);
        drawTestCase(canvas, 3 * offsetX, 3 * offsetY,  SkXfermode::kSrcOver_Mode,
                     SkXfermode::kSrc_Mode);
    }

private:
    SkScalar fTextHeight;
    SkAutoTUnref<const SkTextBlob> fBlob;
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new LcdOverlapGM; )
}

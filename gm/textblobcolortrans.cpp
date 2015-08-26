/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

namespace skiagm {
class TextBlobColorTrans : public GM {
public:
    // This gm tests that textblobs can be translated and have their colors regenerated
    // correctly.  With smaller atlas sizes, it can also trigger regeneration of texture coords on
    // the GPU backend
    TextBlobColorTrans() { }

protected:
    void onOnceBeforeDraw() override {
        SkTextBlobBuilder builder;

        // make textblob
        // Large text is used to trigger atlas eviction
        SkPaint paint;
        paint.setTextSize(256);
        const char* text = "AB";
        sk_tool_utils::set_portable_typeface(&paint);

        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);

        SkScalar yOffset = bounds.height();
        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, yOffset - 30);

        // A8
        paint.setTextSize(28);
        text = "The quick brown fox jumps over the lazy dog.";
        paint.setSubpixelText(false);
        paint.setLCDRenderText(false);
        paint.measureText(text, strlen(text), &bounds);
        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, yOffset - 8);

        // build
        fBlob.reset(builder.build());
    }

    SkString onShortName() override {
        return SkString("textblobcolortrans");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorGRAY));

        SkPaint paint;
        canvas->translate(10, 40);

        SkRect bounds = fBlob->bounds();

        // Colors were chosen to map to pairs of canonical colors.  The GPU Backend will cache A8
        // Texture Blobs based on the canonical color they map to.  Canonical colors are used to
        // create masks.  For A8 there are 8 of them.
        SkColor colors[] = {SK_ColorCYAN, sk_tool_utils::color_to_565(SK_ColorLTGRAY),
                SK_ColorYELLOW, SK_ColorWHITE};

        size_t count = SK_ARRAY_COUNT(colors);
        size_t colorIndex = 0;
        for (int y = 0; y + SkScalarFloorToInt(bounds.height()) < kHeight;
             y += SkScalarFloorToInt(bounds.height())) {
            paint.setColor(colors[colorIndex++ % count]);
            canvas->save();
            canvas->translate(0, SkIntToScalar(y));
            canvas->drawTextBlob(fBlob, 0, 0, paint);
            canvas->restore();
        }
    }

private:
    SkAutoTUnref<const SkTextBlob> fBlob;

    static const int kWidth = 675;
    static const int kHeight = 1600;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobColorTrans;)
}

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ToolUtils.h"
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
        SkFont font(ToolUtils::create_portable_typeface(), 256);
        font.setEdging(SkFont::Edging::kAlias);
        const char* text = "AB";

        SkRect bounds;
        font.measureText(text, strlen(text), kUTF8_SkTextEncoding, &bounds);

        SkScalar yOffset = bounds.height();
        ToolUtils::add_to_text_blob(&builder, text, font, 0, yOffset - 30);

        // A8
        font.setSize(28);
        text = "The quick brown fox jumps over the lazy dog.";
        font.measureText(text, strlen(text), kUTF8_SkTextEncoding, &bounds);
        ToolUtils::add_to_text_blob(&builder, text, font, 0, yOffset - 8);

        // build
        fBlob = builder.make();
    }

    SkString onShortName() override {
        return SkString("textblobcolortrans");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;
        canvas->translate(10, 40);

        SkRect bounds = fBlob->bounds();

        // Colors were chosen to map to pairs of canonical colors.  The GPU Backend will cache A8
        // Texture Blobs based on the canonical color they map to.  Canonical colors are used to
        // create masks.  For A8 there are 8 of them.
        SkColor colors[] = {SK_ColorCYAN, SK_ColorLTGRAY, SK_ColorYELLOW, SK_ColorWHITE};

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
    sk_sp<SkTextBlob> fBlob;

    static constexpr int kWidth = 675;
    static constexpr int kHeight = 1600;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobColorTrans;)
}

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "tools/ToolUtils.h"
#include "tools/Resources.h"

#include <string.h>
#include <initializer_list>

namespace skiagm {

class ColrV1GM : public GM {
public:
    ColrV1GM() { }

protected:
    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        const uint16_t fGlyphs[8] = {19, 33, 20, 21, 22, 23, 24, 25 };
        const size_t fGlyphs_bytesize = SK_ARRAY_COUNT(fGlyphs) * sizeof(uint16_t);
    } fEmojiFont;

    void onOnceBeforeDraw() override {
      fEmojiFont.fTypeface = MakeResourceAsTypeface("fonts/colrv1_samples.ttf");
      SkASSERT(fEmojiFont.fTypeface);
    }

    SkString onShortName() override {
        return SkString("colrv1");
    }

    SkISize onISize() override { return SkISize::Make(1200, 1200); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        SkPaint paint;
        SkFont font(fEmojiFont.fTypeface);
        // font.setEdging(SkFont::Edging::kAlias);

        SkFontMetrics metrics;
        SkScalar y = 0;
        for (SkScalar textSize : { 12, 18 }) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            y += -metrics.fAscent;
            canvas->drawSimpleText(fEmojiFont.fGlyphs, fEmojiFont.fGlyphs_bytesize, SkTextEncoding::kGlyphID, 10, y, font, paint);
            y += metrics.fDescent + metrics.fLeading;
        }

    }

private:
    using INHERITED = GM;
};

DEF_GM(return new ColrV1GM;)

}  // namespace skiagm

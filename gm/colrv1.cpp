/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include "include/core/SkStream.h"
#include "modules/svg/include/SkSVGDOM.h"

#include <string.h>
#include <initializer_list>

namespace skiagm {

class ColrV1GM : public GM {
public:
  ColrV1GM(SkScalar skewX, SkScalar rotateDeg) : fSkewX(skewX), fRotateDeg(rotateDeg) { }

protected:
    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        const uint16_t fGlyphs[10] = { 19, 33, 34, 35, 20, 21, 22, 23, 24, 25 };
        const size_t fGlyphs_bytesize =
          SK_ARRAY_COUNT(fGlyphs) * sizeof(uint16_t);
    } fEmojiFont;

    void onOnceBeforeDraw() override {
        fEmojiFont.fTypeface =
          MakeResourceAsTypeface("fonts/colrv1_samples.ttf");

        std::unique_ptr<SkStreamAsset> svg_file_stream = SkStream::MakeFromFile("emoji_u1f9fa.svg");
        fSvgDom = SkSVGDOM::MakeFromStream(*svg_file_stream);
        fSvgDom->setContainerSize(SkSize::Make(400, 400));
    }

    SkString onShortName() override {
        if (fSkewX != 0 || fRotateDeg != 0) {
            return SkStringPrintf("colrv1_skew_%g_rotate_%g",
                                  fSkewX,
                                  fRotateDeg);
        }
        return SkString("colrv1");
    }

    SkISize onISize() override { return SkISize::Make(1400, 600); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;

        canvas->translate(200, 20);

        if (!fEmojiFont.fTypeface) {
          *errorMsg = "Did not recognize COLR v1 font format.";
          return DrawResult::kSkip;
        }

        canvas->rotate(fRotateDeg);
        canvas->skew(fSkewX, 0);

        SkFont font(fEmojiFont.fTypeface);

        SkFontMetrics metrics;
        SkScalar y = 0;
        for (SkScalar textSize : { 12, 18, 30, 120 }) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            y += -metrics.fAscent;
            canvas->drawSimpleText(fEmojiFont.fGlyphs,
                                   fEmojiFont.fGlyphs_bytesize,
                                   SkTextEncoding::kGlyphID,
                                   10, y, font, paint);
            y += metrics.fDescent + metrics.fLeading;
        }

        canvas->translate(100, 100);
        fSvgDom->render(canvas);

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;
    SkScalar fSkewX;
    SkScalar fRotateDeg;

  sk_sp<SkSVGDOM> fSvgDom;
};

DEF_GM(return new ColrV1GM(0.f, 0.f);)
DEF_GM(return new ColrV1GM(-0.5f, 0.f);)
DEF_GM(return new ColrV1GM(0.f, 20.f);)
DEF_GM(return new ColrV1GM(-0.5f, 20.f);)

}  // namespace skiagm

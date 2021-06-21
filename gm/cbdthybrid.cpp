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

#include <string.h>
#include <initializer_list>

namespace skiagm {

class CbdtHybridGM : public GM {
public:

  CbdtHybridGM(SkScalar skewX, SkScalar rotateDeg) : fSkewX(skewX), fRotateDeg(rotateDeg) { }

protected:
    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        std::vector<uint16_t> fGlyphs;
        size_t bytesize() { return fGlyphs.size() * sizeof(uint16_t); }
    } fTestFont;

    void onOnceBeforeDraw() override {
      fTestFont.fTypeface = MakeResourceAsTypeface("fonts/Unnamed-Regular-CBDT.ttf");
      fTestFont.fGlyphs = {4, 5};
    }

    SkString onShortName() override {
        SkString gm_name = SkStringPrintf("cbdt_glyf_hybrid");
        if (fSkewX) {
            gm_name.append("_skew");
        }

        if (fRotateDeg) {
            gm_name.append("_rotate");
        }
        return gm_name;
    }

    SkISize onISize() override { return SkISize::Make(1400, 600); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;

        canvas->translate(200, 20);

        if (!fTestFont.fTypeface) {
          *errorMsg = "Did not recognize test font format.";
          return DrawResult::kSkip;
        }

        canvas->rotate(fRotateDeg);
        canvas->skew(fSkewX, 0);

        SkFont font(fTestFont.fTypeface);

        SkFontMetrics metrics;
        SkScalar y = 0;
        for (SkScalar textSize : { 12, 18, 30, 120 }) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            y += -metrics.fAscent;
            canvas->drawSimpleText(fTestFont.fGlyphs.data(),
                                   fTestFont.bytesize(),
                                   SkTextEncoding::kGlyphID,
                                   10, y, font, paint);
            y += metrics.fDescent + metrics.fLeading;
        }
        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;
    SkScalar fSkewX;
    SkScalar fRotateDeg;
};

DEF_GM(return new CbdtHybridGM(0.f, 0.f);)

}  // namespace skiagm

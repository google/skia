/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "src/ports/SkTypeface_fontations.h"

namespace skiagm {

namespace {
const SkScalar kTextSizes[] = {12, 18, 30, 120};

}  // namespace

class FontationsTypefaceGM : public GM {
public:
    FontationsTypefaceGM() { this->setBGColor(SK_ColorWHITE); }

protected:
    void onOnceBeforeDraw() override { fTypeface = sk_sp<SkTypeface>(new SkTypeface_Fontations()); }

    SkString onShortName() override { return SkString("typeface_fontations"); }

    SkISize onISize() override { return SkISize::Make(400, 200); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        if (!fTypeface) {
            *errorMsg = "Unable to initialize typeface.";
            return DrawResult::kSkip;
        }

        SkFont font(fTypeface);
        uint16_t glyphs[] = {1, 1, 1};
        SkScalar x = 100;
        SkScalar y = 150;

        for (SkScalar textSize : kTextSizes) {
            font.setSize(textSize);
            y += font.getSpacing();

            /* Draw origin marker as a green dot. */
            paint.setColor(SK_ColorGREEN);
            canvas->drawRect(SkRect::MakeXYWH(x, y, 2, 2), paint);
            paint.setColor(SK_ColorBLACK);

            canvas->drawSimpleText(glyphs,
                                   sizeof(uint16_t) * std::size(glyphs),
                                   SkTextEncoding::kGlyphID,
                                   x,
                                   y,
                                   font,
                                   paint);
        }

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    sk_sp<SkTypeface> fTypeface;
};

DEF_GM(return new FontationsTypefaceGM();)

}  // namespace skiagm

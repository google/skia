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
#include "tools/Resources.h"

namespace skiagm {

namespace {
const SkScalar kTextSizes[] = {12, 18, 30, 120};
const char kTestFontName[] = "fonts/Roboto-Regular.ttf";
}  // namespace

class FontationsTypefaceGM : public GM {
public:
    FontationsTypefaceGM() { this->setBGColor(SK_ColorWHITE); }

protected:
    void onOnceBeforeDraw() override {
        std::unique_ptr<SkStreamAsset> fontStream = GetResourceAsStream(kTestFontName);
        fTypeface = sk_sp<SkTypeface>(new SkTypeface_Fontations(std::move(fontStream)));
    }

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
        const char32_t testText[] = U"xyz";
        size_t testTextBytesize = sizeof(testText) / sizeof(char32_t) * sizeof(char32_t);
        SkScalar x = 100;
        SkScalar y = 150;

        for (SkScalar textSize : kTextSizes) {
            font.setSize(textSize);
            y += font.getSpacing();

            /* Draw origin marker as a green dot. */
            paint.setColor(SK_ColorGREEN);
            canvas->drawRect(SkRect::MakeXYWH(x, y, 2, 2), paint);
            paint.setColor(SK_ColorBLACK);

            canvas->drawSimpleText(
                    testText, testTextBytesize, SkTextEncoding::kUTF32, x, y, font, paint);
        }

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    sk_sp<SkTypeface> fTypeface;
};

DEF_GM(return new FontationsTypefaceGM();)

}  // namespace skiagm

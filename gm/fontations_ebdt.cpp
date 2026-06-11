/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "tools/Resources.h"

namespace skiagm {

// Renders monochrome EBDT bitmap glyphs from all four supported formats at
// multiple sizes. Each row is a different format (1, 2, 6, 7), each column
// is a different font size. This exercises the byte-aligned and bit-aligned
// 1bpp decode paths through the Fontations backend.
class FontationsEbdtGM : public GM {
public:
    FontationsEbdtGM() { this->setBGColor(SK_ColorWHITE); }

protected:
    SkString getName() const override { return SkString("fontations_ebdt"); }

    SkISize getISize() override { return SkISize::Make(400, 300); }

    void onOnceBeforeDraw() override {
        const char* fonts[] = {
                "fonts/ebdt_fmt1.ttf",
                "fonts/ebdt_fmt2.ttf",
                "fonts/ebdt_fmt6.ttf",
                "fonts/ebdt_fmt7.ttf",
        };
        for (const char* fontFile : fonts) {
            fTypefaces.push_back(SkTypeface_Make_Fontations(
                    GetResourceAsStream(fontFile), SkFontArguments()));
        }
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        const char* labels[] = {"fmt1", "fmt2", "fmt6", "fmt7"};
        const SkScalar sizes[] = {12, 16, 24, 32};
        // U+2662 WHITE DIAMOND SUIT — present in all test fonts.
        const char* diamond = "\xE2\x99\xA2";

        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        SkFont labelFont;
        labelFont.setSize(12);

        SkScalar y = 30;
        for (size_t row = 0; row < fTypefaces.size(); ++row) {
            if (!fTypefaces[row]) {
                *errorMsg = SkStringPrintf("Failed to load typeface %zu", row);
                return DrawResult::kSkip;
            }

            canvas->drawSimpleText(labels[row], strlen(labels[row]),
                                   SkTextEncoding::kUTF8, 10, y, labelFont, paint);

            SkScalar x = 60;
            for (SkScalar size : sizes) {
                SkFont font(fTypefaces[row], size);
                canvas->drawSimpleText(diamond, 3, SkTextEncoding::kUTF8, x, y, font, paint);
                x += size + 20;
            }
            y += 50;
        }
        return DrawResult::kOk;
    }

private:
    std::vector<sk_sp<SkTypeface>> fTypefaces;
    using INHERITED = GM;
};

DEF_GM(return new FontationsEbdtGM();)

}  // namespace skiagm

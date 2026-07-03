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

// ebdt_glyf.ttf has, for U+2662, both a monochrome EBDT embedded bitmap (a
// filled diamond, at strikes 16/64/128 ppem) and a glyf outline (a hollow
// diamond). The Fontations backend must use the embedded bitmap only at a
// strike ppem and the outline at every other size, rather than scaling a
// distant strike. This GM renders the glyph across strike (16/64/128) and
// non-strike (32/96/160) sizes: the bitmap (filled) should appear at the
// strike sizes and the outline (hollow) at the others.
class FontationsEbdtGlyfGM : public GM {
public:
    FontationsEbdtGlyfGM() { this->setBGColor(SK_ColorWHITE); }

protected:
    SkString getName() const override { return SkString("fontations_ebdt_glyf"); }

    SkISize getISize() override { return SkISize::Make(640, 280); }

    void onOnceBeforeDraw() override {
        fTypeface = SkTypeface_Make_Fontations(GetResourceAsStream("fonts/ebdt_glyf.ttf"),
                                               SkFontArguments());
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!fTypeface) {
            *errorMsg = "Failed to load fonts/ebdt_glyf.ttf";
            return DrawResult::kSkip;
        }

        // U+2662 WHITE DIAMOND SUIT, present as both bitmap and outline.
        const char* diamond = "\xE2\x99\xA2";
        // 16/64/128 match an embedded strike (-> bitmap); the others do not
        // (-> outline).
        const SkScalar sizes[] = {16, 32, 64, 96, 128, 160};

        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);

        SkFont labelFont;
        labelFont.setSize(11);

        const SkScalar baseline = 190;
        SkScalar x = 10;
        for (SkScalar size : sizes) {
            SkFont font(fTypeface, size);
            font.setEmbeddedBitmaps(true);
            canvas->drawSimpleText(diamond, 3, SkTextEncoding::kUTF8, x, baseline, font, paint);

            SkString label;
            label.printf("%.0f", size);
            canvas->drawSimpleText(label.c_str(), label.size(), SkTextEncoding::kUTF8,
                                   x, baseline + 24, labelFont, paint);
            x += size + 16;
        }
        return DrawResult::kOk;
    }

private:
    sk_sp<SkTypeface> fTypeface;
    using INHERITED = GM;
};

DEF_GM(return new FontationsEbdtGlyfGM();)

}  // namespace skiagm

/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "tools/fonts/FontToolUtils.h"

// Visual regression coverage for ParagraphStyle::setRenderSoftHyphens. The GM
// draws four labeled rows on a single canvas so Skia Gold can detect changes
// in hyphen positioning, especially for RTL where the hyphen must appear at
// the visual *left* edge of the line.
//
// Rows:
//   1. LTR: "inter\u00ADnational" with rendering enabled, narrow width.
//   2. RTL Arabic: "\u0643\u0644\u0645\u0629\u00AD\u0637\u0648\u064A\u0644\u0629"
//      with rendering enabled, narrow width.
//   3. LTR multiple soft hyphens: only the one at the actual line break renders.
//   4. LTR with rendering disabled (default): no visible hyphen at the break.
class SoftHyphenGM : public skiagm::GM {
public:
    SoftHyphenGM() {}

protected:
    SkString getName() const override { return SkString("softhyphen"); }

    SkISize getISize() override { return SkISize::Make(400, 400); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        sk_sp<SkFontMgr> fontmgr = ToolUtils::TestFontMgr();
        if (fontmgr->countFamilies() == 0) {
            *errorMsg = "No fonts available.";
            return DrawResult::kSkip;
        }
        auto unicode = SkShapers::BestAvailableUnicode();
        if (!unicode) {
            *errorMsg = "No SkUnicode implementation available.";
            return DrawResult::kSkip;
        }

        auto collection = sk_make_sp<skia::textlayout::FontCollection>();
        collection->setDefaultFontManager(fontmgr);

        canvas->clear(SK_ColorWHITE);

        SkPaint labelPaint;
        labelPaint.setColor(SK_ColorDKGRAY);
        SkFont labelFont = ToolUtils::DefaultPortableFont();
        labelFont.setSize(11);

        constexpr SkScalar kColX = 10;
        constexpr SkScalar kRowH = 90;
        // Wide enough for the longest unbreakable part ("national"), so the LTR
        // rows only break at soft hyphens.
        constexpr SkScalar kLayoutWidth = 100;
        // Both Arabic words fit in kLayoutWidth together, so the RTL row uses a
        // narrower width to break at its soft hyphen.
        constexpr SkScalar kArabicLayoutWidth = 60;

        auto drawRow = [&](int rowIndex,
                           const char* label,
                           const char* text,
                           bool render,
                           skia::textlayout::TextDirection dir,
                           const char* fontFamily,
                           SkScalar layoutWidth) {
            SkScalar y = 10 + rowIndex * kRowH;
            canvas->drawString(label, kColX, y, labelFont, labelPaint);

            skia::textlayout::ParagraphStyle paraStyle;
            paraStyle.setRenderSoftHyphens(render);
            paraStyle.setTextDirection(dir);

            skia::textlayout::TextStyle textStyle;
            textStyle.setColor(SK_ColorBLACK);
            textStyle.setFontFamilies({SkString(fontFamily)});
            textStyle.setFontSize(20);
            paraStyle.setTextStyle(textStyle);

            auto builder =
                    skia::textlayout::ParagraphBuilderImpl::make(paraStyle, collection, unicode);
            if (!builder) {
                return;
            }
            builder->addText(text, strlen(text));
            auto paragraph = builder->Build();
            paragraph->layout(layoutWidth);
            paragraph->paint(canvas, kColX, y + 14);
        };

        // Row 1: LTR, render hyphens
        drawRow(0,
                "1. LTR render=true",
                "inter\xC2\xAD"
                "national",
                true,
                skia::textlayout::TextDirection::kLtr,
                "sans-serif",
                kLayoutWidth);

        // Row 2: RTL Arabic, render hyphens
        drawRow(1,
                "2. RTL Arabic render=true",
                "\xD9\x83\xD9\x84\xD9\x85\xD8\xA9"           // كلمة
                "\xC2\xAD"                                   // U+00AD
                "\xD8\xB7\xD9\x88\xD9\x8A\xD9\x84\xD8\xA9",  // طويلة
                true,
                skia::textlayout::TextDirection::kRtl,
                "sans-serif",
                kArabicLayoutWidth);

        // Row 3: multiple soft hyphens, only one breaks
        drawRow(2,
                "3. LTR multi render=true",
                "su\xC2\xAD"
                "per\xC2\xAD"
                "cali\xC2\xAD"
                "frag",
                true,
                skia::textlayout::TextDirection::kLtr,
                "sans-serif",
                kLayoutWidth);

        // Row 4: default off (no hyphen at the break)
        drawRow(3,
                "4. LTR render=false (default)",
                "inter\xC2\xAD"
                "national",
                false,
                skia::textlayout::TextDirection::kLtr,
                "sans-serif",
                kLayoutWidth);

        return DrawResult::kOk;
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM(return new SoftHyphenGM;)

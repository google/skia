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
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
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
class ScaledEmojiRenderingGM : public GM {
public:
    ScaledEmojiRenderingGM() {}

private:
    sk_sp<SkTypeface> typefaces[4];

    void onOnceBeforeDraw() override {
        sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefDefault();
        typefaces[0] = MakeResourceAsTypeface(*fontMgr, "fonts/colr.ttf");
        if (!typefaces[0] && fontMgr->canMake(SkFontFormat::TT_glyf_COLR)) {
            fErrorMsg = "could not create COLR font";
        }
        typefaces[1] = MakeResourceAsTypeface(*fontMgr, "fonts/sbix.ttf");
        if (!typefaces[1] && fontMgr->canMake(SkFontFormat::TT_sbix_Png)) {
            fErrorMsg = "could not create sbix font";
        }
        typefaces[2] = MakeResourceAsTypeface(*fontMgr, "fonts/cbdt.ttf");
        if (!typefaces[2] && fontMgr->canMake(SkFontFormat::TT_CBDT_Png)) {
            fErrorMsg = "could not create CBDT font";
        }
        typefaces[3] = ToolUtils::create_portable_typeface("Emoji", SkFontStyle());
        if (!typefaces[3]) {
            fErrorMsg = "no portable emoji font";
        }
    }

    SkString onShortName() override {
        return SkString("scaledemoji_rendering");
    }

    SkISize onISize() override { return SkISize::Make(1200, 1200); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (fErrorMsg) {
            *errorMsg = fErrorMsg;
            return DrawResult::kFail;
        }
        canvas->drawColor(SK_ColorGRAY);
        SkScalar y = 0;

        for (const auto& typeface: typefaces) {
            if (!typeface) {
                continue;
            }

            SkFont font(typeface);
            font.setEdging(SkFont::Edging::kAlias);

            SkPaint paint;
            const char*   text = ToolUtils::emoji_sample_text();
            SkFontMetrics metrics;

            for (SkScalar textSize : { 70, 150 }) {
                font.setSize(textSize);
                font.getMetrics(&metrics);
                // All typefaces should support subpixel mode
                font.setSubpixel(true);
                y += -metrics.fAscent;

                canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8,
                                       10, y, font, paint);
                y += metrics.fDescent + metrics.fLeading;
            }
        }
        return DrawResult::kOk;
    }

    const char* fErrorMsg = nullptr;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ScaledEmojiRenderingGM;)
}

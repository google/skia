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
#include "src/base/SkUTF.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <initializer_list>

static sk_sp<SkTextBlob> make_hpos_test_blob_utf8(const char* text, const SkFont& font) {
    constexpr SkTextEncoding enc = SkTextEncoding::kUTF8;
    SkTextBlobBuilder builder;
    size_t len = strlen(text);
    int glyphCount = font.countText(text, len, enc);
    const auto& buffer = builder.allocRunPosH(font, glyphCount, 0);
    (void)font.textToGlyphs(text, len, enc, buffer.glyphs, glyphCount);
    font.getXPos(buffer.glyphs, glyphCount, buffer.pos);
    return builder.make();
}

namespace skiagm {

class ScaledEmojiGM : public GM {
public:
    ScaledEmojiGM(ToolUtils::EmojiFontFormat format) : fFormat(format) {}

protected:
    ToolUtils::EmojiTestSample fEmojiFont;

    void onOnceBeforeDraw() override { fEmojiFont = ToolUtils::EmojiSample(fFormat); }

    SkString getName() const override {
        return SkString("scaledemoji_") += ToolUtils::NameForFontFormat(fFormat);
    }

    SkISize getISize() override { return SkISize::Make(1200, 1200); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!fEmojiFont.typeface) {
            *errorMsg = SkStringPrintf("Unable to instantiate emoji test font of format %s.",
                                       ToolUtils::NameForFontFormat(fFormat).c_str());
            return DrawResult::kSkip;
        }

        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;
        SkFont font(fEmojiFont.typeface);
        font.setEdging(SkFont::Edging::kAlias);

        const char* text = fEmojiFont.sampleText;

        // draw text at different point sizes
        // Testing GPU bitmap path, SDF path with no scaling,
        // SDF path with scaling, path rendering with scaling
        SkFontMetrics metrics;
        SkScalar y = 0;
        for (SkScalar textSize : {70, 180, 270, 340}) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            y += -metrics.fAscent;
            canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, 10, y, font, paint);
            y += metrics.fDescent + metrics.fLeading;
        }

        return DrawResult::kOk;
    }

private:
    ToolUtils::EmojiFontFormat fFormat;
    using INHERITED = GM;
};

class ScaledEmojiPosGM : public GM {
public:
    ScaledEmojiPosGM(ToolUtils::EmojiFontFormat format) : fFormat(format) {}

protected:
    ToolUtils::EmojiTestSample fEmojiFont;

    void onOnceBeforeDraw() override { fEmojiFont = ToolUtils::EmojiSample(fFormat); }

    SkString getName() const override {
        return SkString("scaledemojipos_") += ToolUtils::NameForFontFormat(fFormat);
    }

    SkISize getISize() override { return SkISize::Make(1200, 1200); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!fEmojiFont.typeface) {
            *errorMsg = SkStringPrintf("Unable to instantiate emoji test font of format %s.",
                                       ToolUtils::NameForFontFormat(fFormat).c_str());
            return DrawResult::kSkip;
        }

        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;
        SkFont font(fEmojiFont.typeface, 12);
        const char* text = fEmojiFont.sampleText;

        // draw text at different point sizes
        // Testing GPU bitmap path, SDF path with no scaling,
        // SDF path with scaling, path rendering with scaling
        SkFontMetrics metrics;
        SkScalar y = 0;
        for (SkScalar textSize : {70, 180, 270, 340}) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            y += -metrics.fAscent;

            sk_sp<SkTextBlob> blob = make_hpos_test_blob_utf8(text, font);
            // Draw with an origin.
            canvas->drawTextBlob(blob, 10, y, paint);

            // Draw with shifted canvas.
            canvas->save();
            canvas->translate(750, 0);
            canvas->drawTextBlob(blob, 10, y, paint);
            canvas->restore();

            y += metrics.fDescent + metrics.fLeading;
        }

        return DrawResult::kOk;
    }

private:
    ToolUtils::EmojiFontFormat fFormat;
    using INHERITED = GM;
};

class ScaledEmojiPerspectiveGM : public GM {
public:
    ScaledEmojiPerspectiveGM(ToolUtils::EmojiFontFormat format) : fFormat(format) {}

protected:
    ToolUtils::EmojiTestSample fEmojiFont;
    SkString fStripSpacesSampleText;

    void onOnceBeforeDraw() override {
        fEmojiFont = ToolUtils::EmojiSample(fFormat);

        int count = 0;
        const char* ch_ptr = fEmojiFont.sampleText;
        const char* ch_end = ch_ptr + strlen(ch_ptr);
        while (ch_ptr < ch_end && count < 2) {
            SkUnichar ch = SkUTF::NextUTF8(&ch_ptr, ch_end);
            if (ch != ' ') {
                fStripSpacesSampleText.appendUnichar(ch);
                ++count;
            }
        }
    }

    SkString getName() const override {
        return SkString("scaledemojiperspective_") += ToolUtils::NameForFontFormat(fFormat);
    }

    SkISize getISize() override { return SkISize::Make(1200, 1200); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!fEmojiFont.typeface) {
            *errorMsg = SkStringPrintf("Unable to instantiate emoji test font of format %s.",
                                       ToolUtils::NameForFontFormat(fFormat).c_str());
            return DrawResult::kSkip;
        }

        canvas->drawColor(SK_ColorGRAY);
        SkMatrix taper;
        taper.setPerspY(-0.0025f);

        SkPaint paint;
        SkFont font(fEmojiFont.typeface, 40);
        sk_sp<SkTextBlob> blob = make_hpos_test_blob_utf8(fStripSpacesSampleText.c_str(), font);

        // draw text at different point sizes
        // Testing GPU bitmap path, SDF path with no scaling,
        // SDF path with scaling, path rendering with scaling
        SkFontMetrics metrics;
        font.getMetrics(&metrics);
        for (auto rotate : {0.0, 45.0, 90.0, 135.0, 180.0, 225.0, 270.0, 315.0}) {
            canvas->save();
            SkMatrix perspective;
            perspective.postTranslate(-600, -600);
            perspective.postConcat(taper);
            perspective.postRotate(rotate);
            perspective.postTranslate(600, 600);
            canvas->concat(perspective);
            SkScalar y = 670;
            for (int i = 0; i < 5; i++) {

                y += -metrics.fAscent;

                // Draw with an origin.
                canvas->drawTextBlob(blob, 565, y, paint);

                y += metrics.fDescent + metrics.fLeading;
            }
            canvas->restore();
        }

        return DrawResult::kOk;
    }

private:
    ToolUtils::EmojiFontFormat fFormat;
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ScaledEmojiGM(ToolUtils::EmojiFontFormat::Cbdt);)
DEF_GM(return new ScaledEmojiPosGM(ToolUtils::EmojiFontFormat::Cbdt);)
DEF_GM(return new ScaledEmojiPerspectiveGM(ToolUtils::EmojiFontFormat::Cbdt);)
DEF_GM(return new ScaledEmojiGM(ToolUtils::EmojiFontFormat::Sbix);)
DEF_GM(return new ScaledEmojiPosGM(ToolUtils::EmojiFontFormat::Sbix);)
DEF_GM(return new ScaledEmojiPerspectiveGM(ToolUtils::EmojiFontFormat::Sbix);)
DEF_GM(return new ScaledEmojiGM(ToolUtils::EmojiFontFormat::ColrV0);)
DEF_GM(return new ScaledEmojiPosGM(ToolUtils::EmojiFontFormat::ColrV0);)
DEF_GM(return new ScaledEmojiPerspectiveGM(ToolUtils::EmojiFontFormat::ColrV0);)
DEF_GM(return new ScaledEmojiGM(ToolUtils::EmojiFontFormat::Svg);)
DEF_GM(return new ScaledEmojiPosGM(ToolUtils::EmojiFontFormat::Svg);)
DEF_GM(return new ScaledEmojiPerspectiveGM(ToolUtils::EmojiFontFormat::Svg);)
DEF_GM(return new ScaledEmojiGM(ToolUtils::EmojiFontFormat::Test);)
DEF_GM(return new ScaledEmojiPosGM(ToolUtils::EmojiFontFormat::Test);)
DEF_GM(return new ScaledEmojiPerspectiveGM(ToolUtils::EmojiFontFormat::Test);)

}  // namespace skiagm

/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/GrContextOptions.h"
#include "src/core/SkFontPriv.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/ContextOptions.h"
#endif

#include <string.h>
#include <initializer_list>
#include <utility>

/*
 * Spits out an arbitrary gradient to test blur with shader on paint
 */
static sk_sp<SkShader> MakeLinear() {
    constexpr SkPoint     kPts[] = { { 0, 0 }, { 32, 32 } };
    constexpr SkScalar    kPos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    constexpr SkColor kColors[] = {0x80F00080, 0xF0F08000, 0x800080F0 };
    return SkGradientShader::MakeLinear(kPts, kColors, kPos, std::size(kColors),
                                        SkTileMode::kClamp);
}

static sk_sp<SkImageFilter> make_grayscale(sk_sp<SkImageFilter> input) {
    float matrix[20];
    memset(matrix, 0, 20 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    sk_sp<SkColorFilter> filter(SkColorFilters::Matrix(matrix));
    return SkImageFilters::ColorFilter(std::move(filter), std::move(input));
}

static sk_sp<SkImageFilter> make_blur(float amount, sk_sp<SkImageFilter> input) {
    return SkImageFilters::Blur(amount, amount, std::move(input));
}

static sk_sp<SkColorFilter> make_color_filter() {
    return SkColorFilters::Lighting(SkColorSetRGB(0x00, 0x80, 0xFF),
                                    SkColorSetRGB(0xFF, 0x20, 0x00));
}

namespace skiagm {

class ColorEmojiGM : public GM {
public:
    ColorEmojiGM() { }

protected:
    struct EmojiFont {
        sk_sp<SkTypeface> typeface;
        const char* text;
    } emojiFont;
    void onOnceBeforeDraw() override {
        emojiFont.typeface = ToolUtils::EmojiTypeface();
        emojiFont.text     = ToolUtils::EmojiSampleText();
    }

    SkString getName() const override { return SkString("coloremoji"); }

    SkISize getISize() override { return SkISize::Make(650, 1200); }

    void modifyGrContextOptions(GrContextOptions* ctxOptions) override {
        // This will force multitexturing to verify that color text works with this,
        // as well as with any additional color transformations.
        ctxOptions->fGlyphCacheTextureMaximumBytes = 256 * 256 * 4;
    }

#if defined(SK_GRAPHITE)
    void modifyGraphiteContextOptions(skgpu::graphite::ContextOptions* ctxOptions) const override {
        // This will force multitexturing to verify that color text works with this,
        // as well as with any additional color transformations.
        ctxOptions->fGlyphCacheTextureMaximumBytes = 256 * 256 * 4;
    }
#endif

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorGRAY);

        SkFont font(emojiFont.typeface);
        char const * const text = emojiFont.text;
        size_t textLen = strlen(text);

        // draw text at different point sizes
        constexpr SkScalar textSizes[] = { 10, 30, 50 };
        SkFontMetrics metrics;
        SkScalar y = 0;
        for (const bool& fakeBold : { false, true }) {
            font.setEmbolden(fakeBold);
            for (const SkScalar& textSize : textSizes) {
                font.setSize(textSize);
                font.getMetrics(&metrics);
                y += -metrics.fAscent;
                canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8,
                                       10, y, font, SkPaint());
                y += metrics.fDescent + metrics.fLeading;
            }
        }

        // draw one more big one to max out one Plot
        font.setSize(256);
        font.getMetrics(&metrics);
        canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8,
                               190, -metrics.fAscent, font, SkPaint());

        y += 20;
        SkScalar savedY = y;
        // draw with shaders and image filters
        for (int makeLinear = 0; makeLinear < 2; makeLinear++) {
            for (int makeBlur = 0; makeBlur < 2; makeBlur++) {
                for (int makeGray = 0; makeGray < 2; makeGray++) {
                    for (int makeMode = 0; makeMode < 2; ++makeMode) {
                        for (int alpha = 0; alpha < 2; ++alpha) {
                            SkFont shaderFont(SkFontPriv::RefTypefaceOrDefault(font));
                            SkPaint shaderPaint;
                            if (SkToBool(makeLinear)) {
                                shaderPaint.setShader(MakeLinear());
                            }

                            if (SkToBool(makeBlur) && SkToBool(makeGray)) {
                                sk_sp<SkImageFilter> grayScale(make_grayscale(nullptr));
                                sk_sp<SkImageFilter> blur(make_blur(3.0f, std::move(grayScale)));
                                shaderPaint.setImageFilter(std::move(blur));
                            } else if (SkToBool(makeBlur)) {
                                shaderPaint.setImageFilter(make_blur(3.0f, nullptr));
                            } else if (SkToBool(makeGray)) {
                                shaderPaint.setImageFilter(make_grayscale(nullptr));
                            }
                            if (makeMode) {
                                shaderPaint.setColorFilter(make_color_filter());
                            }
                            if (alpha) {
                                shaderPaint.setAlphaf(0.5f);
                            }
                            shaderFont.setSize(30);
                            shaderFont.getMetrics(&metrics);
                            y += -metrics.fAscent;
                            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, 380, y,
                                                   shaderFont, shaderPaint);
                            y += metrics.fDescent + metrics.fLeading;
                        }
                    }
                }
            }
        }
        // setup work needed to draw text with different clips
        canvas->translate(10, savedY);
        font.setSize(40);

        // compute the bounds of the text
        SkRect bounds;
        font.measureText(text, textLen, SkTextEncoding::kUTF8, &bounds);

        const SkScalar boundsHalfWidth = bounds.width() * SK_ScalarHalf;
        const SkScalar boundsHalfHeight = bounds.height() * SK_ScalarHalf;
        const SkScalar boundsQuarterWidth = boundsHalfWidth * SK_ScalarHalf;
        const SkScalar boundsQuarterHeight = boundsHalfHeight * SK_ScalarHalf;

        SkRect upperLeftClip = SkRect::MakeXYWH(bounds.left(), bounds.top(),
                                                boundsHalfWidth, boundsHalfHeight);
        SkRect lowerRightClip = SkRect::MakeXYWH(bounds.centerX(), bounds.centerY(),
                                                 boundsHalfWidth, boundsHalfHeight);
        SkRect interiorClip = bounds;
        interiorClip.inset(boundsQuarterWidth, boundsQuarterHeight);

        const SkRect clipRects[] = { bounds, upperLeftClip, lowerRightClip, interiorClip };

        SkPaint clipHairline;
        clipHairline.setColor(SK_ColorWHITE);
        clipHairline.setStyle(SkPaint::kStroke_Style);

        SkPaint paint;
        for (const SkRect& clipRect : clipRects) {
            canvas->translate(0, bounds.height());
            canvas->save();
            canvas->drawRect(clipRect, clipHairline);
            paint.setAlpha(0x20);
            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, 0, 0, font, paint);
            canvas->clipRect(clipRect);
            paint.setAlphaf(1.0f);
            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, 0, 0, font, paint);
            canvas->restore();
            canvas->translate(0, SkIntToScalar(25));
        }
    }

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ColorEmojiGM;)

}  // namespace skiagm

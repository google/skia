/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "Resources.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkGradientShader.h"
#include "SkStream.h"
#include "SkTypeface.h"

/*
 * Spits out a dummy gradient to test blur with shader on paint
 */
static sk_sp<SkShader> MakeLinear() {
    constexpr SkPoint     kPts[] = { { 0, 0 }, { 32, 32 } };
    constexpr SkScalar    kPos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    constexpr SkColor kColors[] = {0x80F00080, 0xF0F08000, 0x800080F0 };
    return SkGradientShader::MakeLinear(kPts, kColors, kPos, SK_ARRAY_COUNT(kColors),
                                        SkShader::kClamp_TileMode);
}

static sk_sp<SkImageFilter> make_grayscale(sk_sp<SkImageFilter> input) {
    SkScalar matrix[20];
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    sk_sp<SkColorFilter> filter(SkColorFilter::MakeMatrixFilterRowMajor255(matrix));
    return SkColorFilterImageFilter::Make(std::move(filter), std::move(input));
}

static sk_sp<SkImageFilter> make_blur(float amount, sk_sp<SkImageFilter> input) {
    return SkBlurImageFilter::Make(amount, amount, std::move(input));
}

static sk_sp<SkColorFilter> make_color_filter() {
    return SkColorMatrixFilter::MakeLightingFilter(SkColorSetRGB(0x00, 0x80, 0xFF),
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
    virtual void onOnceBeforeDraw() override {
        emojiFont.typeface = sk_tool_utils::emoji_typeface();
        emojiFont.text = sk_tool_utils::emoji_sample_text();
    }

    SkString onShortName() override {
        SkString name("coloremoji");
        name.append(sk_tool_utils::platform_os_emoji());
        return name;
    }

    SkISize onISize() override { return SkISize::Make(650, 1200); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorGRAY));

        SkPaint paint;
        paint.setTypeface(emojiFont.typeface);
        const char* text = emojiFont.text;

        // draw text at different point sizes
        constexpr SkScalar textSizes[] = { 10, 30, 50, };
        SkPaint::FontMetrics metrics;
        SkScalar y = 0;
        for (const SkScalar& textSize : textSizes) {
            paint.setTextSize(textSize);
            paint.getFontMetrics(&metrics);
            y += -metrics.fAscent;
            canvas->drawString(text, 10, y, paint);
            y += metrics.fDescent + metrics.fLeading;
        }

        y += 20;
        SkScalar savedY = y;
        // draw with shaders and image filters
        for (int makeLinear = 0; makeLinear < 2; makeLinear++) {
            for (int makeBlur = 0; makeBlur < 2; makeBlur++) {
                for (int makeGray = 0; makeGray < 2; makeGray++) {
                    for (int makeMode = 0; makeMode < 2; ++makeMode) {
                        for (int alpha = 0; alpha < 2; ++alpha) {
                            SkPaint shaderPaint;
                            shaderPaint.setTypeface(sk_ref_sp(paint.getTypeface()));
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
                                shaderPaint.setAlpha(0x80);
                            }
                            shaderPaint.setTextSize(30);
                            shaderPaint.getFontMetrics(&metrics);
                            y += -metrics.fAscent;
                            canvas->drawString(text, 380, y, shaderPaint);
                            y += metrics.fDescent + metrics.fLeading;
                        }
                    }
                }
            }
        }
        // setup work needed to draw text with different clips
        canvas->translate(10, savedY);
        paint.setTextSize(40);

        // compute the bounds of the text
        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);

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

        for (const SkRect& clipRect : clipRects) {
            canvas->translate(0, bounds.height());
            canvas->save();
            canvas->drawRect(clipRect, clipHairline);
            paint.setAlpha(0x20);
            canvas->drawString(text, 0, 0, paint);
            canvas->clipRect(clipRect);
            paint.setAlpha(0xFF);
            canvas->drawString(text, 0, 0, paint);
            canvas->restore();
            canvas->translate(0, SkIntToScalar(25));
        }
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ColorEmojiGM;)

}

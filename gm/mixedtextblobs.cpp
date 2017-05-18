/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

namespace skiagm {

static void draw_blob(SkCanvas* canvas, const SkTextBlob* blob, const SkPaint& skPaint,
                      const SkRect& clipRect) {
    SkPaint clipHairline;
    clipHairline.setColor(SK_ColorWHITE);
    clipHairline.setStyle(SkPaint::kStroke_Style);

    SkPaint paint(skPaint);
    canvas->save();
    canvas->drawRect(clipRect, clipHairline);
    paint.setAlpha(0x20);
    canvas->drawTextBlob(blob, 0, 0, paint);
    canvas->clipRect(clipRect);
    paint.setAlpha(0xFF);
    canvas->drawTextBlob(blob, 0, 0, paint);
    canvas->restore();
}

class MixedTextBlobsGM : public GM {
public:
    MixedTextBlobsGM() { }

protected:
    void onOnceBeforeDraw() override {
        fEmojiTypeface = sk_tool_utils::emoji_typeface();
        fEmojiText = sk_tool_utils::emoji_sample_text();
        fReallyBigATypeface = MakeResourceAsTypeface("/fonts/ReallyBigA.ttf");

        SkTextBlobBuilder builder;

        // make textblob
        // Text so large we draw as paths
        SkPaint paint;
        paint.setTextSize(385);
        const char* text = "O";
        sk_tool_utils::set_portable_typeface(&paint);

        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);

        SkScalar yOffset = bounds.height();
        sk_tool_utils::add_to_text_blob(&builder, text, paint, 10, yOffset);
        SkScalar corruptedAx = bounds.width();
        SkScalar corruptedAy = yOffset;

        const SkScalar boundsHalfWidth = bounds.width() * SK_ScalarHalf;
        const SkScalar boundsHalfHeight = bounds.height() * SK_ScalarHalf;

        SkScalar xOffset = boundsHalfWidth;
        yOffset = boundsHalfHeight;

        // LCD
        paint.setTextSize(32);
        text = "LCD!!!!!";
        paint.setAntiAlias(true);
        paint.setSubpixelText(true);
        paint.setLCDRenderText(true);
        paint.measureText(text, strlen(text), &bounds);
        sk_tool_utils::add_to_text_blob(&builder, text, paint, xOffset - bounds.width() * 0.25f,
                                        yOffset - bounds.height() * 0.5f);
        yOffset += bounds.height();

        // color emoji
        if (fEmojiTypeface) {
            paint.setAntiAlias(false);
            paint.setSubpixelText(false);
            paint.setLCDRenderText(false);
            paint.setTypeface(fEmojiTypeface);
            text = fEmojiText;
            paint.measureText(text, strlen(text), &bounds);
            sk_tool_utils::add_to_text_blob(&builder, text, paint, xOffset - bounds.width() * 0.3f,
                                            yOffset);
        }

        // Corrupted font
        paint.setTextSize(12);
        text = "aA";
        paint.setTypeface(fReallyBigATypeface);
        sk_tool_utils::add_to_text_blob(&builder, text, paint, corruptedAx, corruptedAy);
        fBlob = builder.make();
    }

    SkString onShortName() override {
        SkString name("mixedtextblobs");
        name.append(sk_tool_utils::platform_os_emoji());
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorGRAY));

        SkPaint paint;

        // setup work needed to draw text with different clips
        paint.setColor(SK_ColorBLACK);
        canvas->translate(10, 40);

        paint.setTextSize(40);

        // compute the bounds of the text and setup some clips
        SkRect bounds = fBlob->bounds();

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

        const SkRect clipRects[] = { bounds, upperLeftClip, lowerRightClip, interiorClip};

        size_t count = sizeof(clipRects) / sizeof(SkRect);
        for (size_t x = 0; x < count; ++x) {
            draw_blob(canvas, fBlob.get(), paint, clipRects[x]);
            if (x == (count >> 1) - 1) {
                canvas->translate(SkScalarFloorToScalar(bounds.width() + SkIntToScalar(25)),
                                  -(x * SkScalarFloorToScalar(bounds.height() +
                                    SkIntToScalar(25))));
            } else {
                canvas->translate(0, SkScalarFloorToScalar(bounds.height() + SkIntToScalar(25)));
            }
        }
    }

private:
    sk_sp<SkTypeface> fEmojiTypeface;
    sk_sp<SkTypeface> fReallyBigATypeface;
    const char* fEmojiText;
    sk_sp<SkTextBlob> fBlob;

    static constexpr int kWidth = 1250;
    static constexpr int kHeight = 700;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new MixedTextBlobsGM;)
}

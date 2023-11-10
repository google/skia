/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>

namespace skiagm {

static void draw_blob(SkCanvas* canvas, const SkTextBlob* blob, const SkPaint& skPaint,
                      const SkRect& clipRect) {
    SkPaint clipHairline;
    clipHairline.setColor(SK_ColorWHITE);
    clipHairline.setStyle(SkPaint::kStroke_Style);

    SkPaint paint(skPaint);
    canvas->save();
    canvas->drawRect(clipRect, clipHairline);
    paint.setAlphaf(0.125f);
    canvas->drawTextBlob(blob, 0, 0, paint);
    canvas->clipRect(clipRect);
    paint.setAlphaf(1.0f);
    canvas->drawTextBlob(blob, 0, 0, paint);
    canvas->restore();
}

class MixedTextBlobsGM : public GM {
public:
    MixedTextBlobsGM() { }

protected:
    void onOnceBeforeDraw() override {
        fEmojiTypeface      = ToolUtils::PlanetTypeface();
        fEmojiText = "♁♃";
        fReallyBigATypeface = ToolUtils::CreateTypefaceFromResource("fonts/ReallyBigA.ttf");
        if (!fReallyBigATypeface) {
            fReallyBigATypeface = ToolUtils::DefaultPortableTypeface();
        }

        SkTextBlobBuilder builder;

        // make textblob
        // Text so large we draw as paths
        SkFont font(ToolUtils::DefaultPortableTypeface(), 385);
        font.setEdging(SkFont::Edging::kAlias);
        const char* text = "O";

        SkRect bounds;
        font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);

        SkScalar yOffset = bounds.height();
        ToolUtils::add_to_text_blob(&builder, text, font, 10, yOffset);
        SkScalar corruptedAx = bounds.width();
        SkScalar corruptedAy = yOffset;

        const SkScalar boundsHalfWidth = bounds.width() * SK_ScalarHalf;
        const SkScalar boundsHalfHeight = bounds.height() * SK_ScalarHalf;

        SkScalar xOffset = boundsHalfWidth;
        yOffset = boundsHalfHeight;

        // LCD
        font.setSize(32);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);
        text = "LCD!!!!!";
        font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);
        ToolUtils::add_to_text_blob(&builder,
                                    text,
                                    font,
                                    xOffset - bounds.width() * 0.25f,
                                    yOffset - bounds.height() * 0.5f);

        // color emoji font with large glyph
        if (fEmojiTypeface) {
            font.setEdging(SkFont::Edging::kAlias);
            font.setSubpixel(false);
            font.setTypeface(fEmojiTypeface);
            font.measureText(fEmojiText, strlen(fEmojiText), SkTextEncoding::kUTF8, &bounds);
            ToolUtils::add_to_text_blob(&builder, fEmojiText, font, xOffset, yOffset);
        }

        // outline font with large glyph
        font.setSize(12);
        text = "aA";
        font.setTypeface(fReallyBigATypeface);
        ToolUtils::add_to_text_blob(&builder, text, font, corruptedAx, corruptedAy);
        fBlob = builder.make();
    }

    SkString getName() const override { return SkString("mixedtextblobs"); }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;

        // setup work needed to draw text with different clips
        paint.setColor(SK_ColorBLACK);
        canvas->translate(10, 40);

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

    inline static constexpr int kWidth = 1250;
    inline static constexpr int kHeight = 700;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new MixedTextBlobsGM;)
}  // namespace skiagm

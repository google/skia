/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Sample.h"
#include "ToolUtils.h"

#include "AnimTimer.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkRandom.h"
#include "SkTypeface.h"

#include <cmath>

// Implementation in C++ of Animated Emoji
// See https://t.d3fc.io/status/705212795936247808
// See https://crbug.com/848616

class GlyphTransformView : public Sample {
public:
    GlyphTransformView() {}

protected:
    void onOnceBeforeDraw() override {
        fEmojiFont.fTypeface = ToolUtils::emoji_typeface();
        fEmojiFont.fText     = ToolUtils::emoji_sample_text();
    }

    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Glyph Transform");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;

        SkFont font(fEmojiFont.fTypeface);
        const char* text = fEmojiFont.fText;

        double baseline = this->height() / 2;
        canvas->drawLine(0, baseline, this->width(), baseline, paint);

        SkMatrix ctm;
        ctm.setRotate(fRotate); // d3 rotate takes degrees
        ctm.postScale(fScale * 4, fScale * 4);
        ctm.postTranslate(fTranslate.fX  + this->width() * 0.8, fTranslate.fY + baseline);
        canvas->concat(ctm);

        // d3 by default anchors text around the middle
        SkRect bounds;
        font.measureText(text, strlen(text), kUTF8_SkTextEncoding, &bounds);
        canvas->drawSimpleText(text, strlen(text), kUTF8_SkTextEncoding, -bounds.centerX(), -bounds.centerY(),
                               font, paint);
    }

    bool onAnimate(const AnimTimer& timer) override {
        constexpr SkScalar maxt = 100000;
        double t = timer.pingPong(20, 0, 0, maxt); // d3 t is in milliseconds

        fTranslate.set(sin(t / 3000) - t * this->width() * 0.7 / maxt, sin(t / 999) / t);
        fScale = 4.5 - std::sqrt(t) / 99;
        fRotate = sin(t / 734);

        return true;
    }

private:
    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        const char* fText;
    } fEmojiFont;

    SkVector fTranslate;
    SkScalar fScale;
    SkScalar fRotate;

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new GlyphTransformView(); )

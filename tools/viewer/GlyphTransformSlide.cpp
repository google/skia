/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkTypeface.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/Slide.h"

#include <cmath>

// Implementation in C++ of Animated Emoji
// See https://t.d3fc.io/status/705212795936247808
// See https://crbug.com/848616

class GlyphTransformView : public Slide {
public:
    GlyphTransformView() { fName = "Glyph Transform"; }

    void load(SkScalar w, SkScalar h) override {
        fEmojiFont.fTypeface = ToolUtils::EmojiTypeface();
        fEmojiFont.fText     = ToolUtils::EmojiSampleText();
        fSize = {w, h};
    }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void draw(SkCanvas* canvas) override {
        SkPaint paint;

        SkFont font(fEmojiFont.fTypeface);
        const char* text = fEmojiFont.fText;

        double baseline = fSize.height() / 2;
        canvas->drawLine(0, baseline, fSize.width(), baseline, paint);

        SkMatrix ctm;
        ctm.setRotate(fRotate); // d3 rotate takes degrees
        ctm.postScale(fScale * 4, fScale * 4);
        ctm.postTranslate(fTranslate.fX  + fSize.width() * 0.8, fTranslate.fY + baseline);
        canvas->concat(ctm);

        // d3 by default anchors text around the middle
        SkRect bounds;
        font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);
        canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, -bounds.centerX(), -bounds.centerY(),
                               font, paint);
    }

    bool animate(double nanos) override {
        constexpr SkScalar maxt = 100000;
        double t = TimeUtils::PingPong(1e-9 * nanos, 20, 0, 0, maxt); // d3 t is in milliseconds

        fTranslate.set(sin(t / 3000) - t * fSize.width() * 0.7 / maxt, sin(t / 999) / t);
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
    SkSize fSize;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new GlyphTransformView(); )

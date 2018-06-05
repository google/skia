/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "sk_tool_utils.h"

#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRRect.h"
#include "SkTypeface.h"

// Implementation in C++ of Animated Emoji
// See https://t.d3fc.io/status/705212795936247808

class GlyphTransformView : public SampleView {
public:
    GlyphTransformView() {}

protected:
    void onOnceBeforeDraw() override {
        fEmojiFont.fTypeface = sk_tool_utils::emoji_typeface();
        fEmojiFont.fText = sk_tool_utils::emoji_sample_text();
    }

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Glyph Transform");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setTypeface(fEmojiFont.fTypeface);
        const char* text = fEmojiFont.fText;

        canvas->scale(4, 4);

        canvas->drawLine(0, 200, 600, 200, paint);
        SkMatrix ctm;
        ctm.setRotate(SkRadiansToDegrees(fRotate));
        ctm.postScale(fScale, fScale);
        ctm.postTranslate(fTranslate.fX, fTranslate.fY);
        canvas->concat(ctm);
        canvas->drawString(text, 0, 0, paint);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        double t = timer.secs();

        fTranslate.set(99 + sin(t / 3.0e3) - t / 1024, 200 + sin(t / 999) / t);
        fScale = 4.5 - t*t / 99;
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

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new GlyphTransformView; }
static SkViewRegister reg(MyFactory);

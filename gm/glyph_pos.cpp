/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkTypeface.h"

/* This test tries to define the effect of using hairline strokes on text.
 * Provides non-hairline images for reference and consistency checks.
 * glyph_pos_(h/n)_(s/f/b)
 *   -> test hairline/non-hairline stroke/fill/stroke+fill.
 */
static const SkScalar kTextHeight = 14.0f;
static const char kText[] = "Proportional Hamburgefons #% fi";

namespace skiagm {

class GlyphPosGM : public GM {
public:
    GlyphPosGM(SkScalar strokeWidth, SkPaint::Style strokeStyle)
        : fStrokeWidth(strokeWidth)
        , fStrokeStyle(strokeStyle) {
        }

protected:

    SkString onShortName() override {
        SkString str("glyph_pos");
        if (fStrokeWidth == 0.0f) {
            str.append("_h"); // h == Hairline.
        } else {
            str.append("_n"); // n == Normal.
        }
        if (fStrokeStyle == SkPaint::kStroke_Style) {
            str.append("_s");
        } else if (fStrokeStyle == SkPaint::kFill_Style) {
            str.append("_f");
        } else {
            str.append("_b"); // b == Both.
        }
        return str;
    }

    SkISize onISize() override { return SkISize::Make(800, 600); }

    void onDraw(SkCanvas* canvas) override {

        // There's a black pixel at 40, 40 for reference.
        canvas->drawPoint(40.0f, 40.0f, SK_ColorBLACK);

        // Two reference images.
        canvas->translate(50.0f, 50.0f);
        drawTestCase(canvas, 1.0f);

        canvas->translate(0.0f, 50.0f);
        drawTestCase(canvas, 3.0f);

        // Uniform scaling test.
        canvas->translate(0.0f, 100.0f);
        canvas->save();
        canvas->scale(3.0f, 3.0f);
        drawTestCase(canvas, 1.0f);
        canvas->restore();

        // Non-uniform scaling test.
        canvas->translate(0.0f, 100.0f);
        canvas->save();
        canvas->scale(3.0f, 6.0f);
        drawTestCase(canvas, 1.0f);
        canvas->restore();

        // Skew test.
        canvas->translate(0.0f, 80.0f);
        canvas->save();
        canvas->scale(3.0f, 3.0f);
        SkMatrix skew;
        skew.setIdentity();
        skew.setSkewX(8.0f / 25.0f);
        skew.setSkewY(2.0f / 25.0f);
        canvas->concat(skew);
        drawTestCase(canvas, 1.0f);
        canvas->restore();

        // Perspective test.
        canvas->translate(0.0f, 80.0f);
        canvas->save();
        SkMatrix perspective;
        perspective.setIdentity();
        perspective.setPerspX(-SkScalarInvert(340));
        perspective.setSkewX(8.0f / 25.0f);
        perspective.setSkewY(2.0f / 25.0f);


        canvas->concat(perspective);
        drawTestCase(canvas, 1.0f);
        canvas->restore();
    }

    void drawTestCase(SkCanvas* canvas, SkScalar textScale) {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);
        paint.setTextSize(kTextHeight * textScale);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setStrokeWidth(fStrokeWidth);
        paint.setStyle(fStrokeStyle);

        // This demonstrates that we can not measure the text if there's a device transform. The
        // canvas total matrix will end up being a device transform.
        bool drawRef = !(canvas->getTotalMatrix().getType() &
                         ~(SkMatrix::kIdentity_Mask | SkMatrix::kTranslate_Mask));

        SkRect bounds;
        if (drawRef) {
            SkScalar advance = paint.measureText(kText, sizeof(kText) - 1, &bounds);

            paint.setStrokeWidth(0.0f);
            paint.setStyle(SkPaint::kStroke_Style);

            // Green box is the measured text bounds.
            paint.setColor(SK_ColorGREEN);
            canvas->drawRect(bounds, paint);

            // Red line is the measured advance from the 0,0 of the text position.
            paint.setColor(SK_ColorRED);
            canvas->drawLine(0.0f, 0.0f, advance, 0.0f, paint);
        }

        // Black text is the testcase, eg. the text.
        paint.setColor(SK_ColorBLACK);
        paint.setStrokeWidth(fStrokeWidth);
        paint.setStyle(fStrokeStyle);
        canvas->drawText(kText, sizeof(kText) - 1, 0.0f, 0.0f, paint);

        if (drawRef) {
            SkScalar widths[sizeof(kText) - 1];
            paint.getTextWidths(kText, sizeof(kText) - 1, widths, NULL);

            paint.setStrokeWidth(0.0f);
            paint.setStyle(SkPaint::kStroke_Style);

            // Magenta lines are the positions for the characters.
            paint.setColor(SK_ColorMAGENTA);
            SkScalar w = bounds.x();
            for (size_t i = 0; i < sizeof(kText) - 1; ++i) {
                canvas->drawLine(w, 0.0f, w, 5.0f, paint);
                w += widths[i];
            }
        }
    }

private:
    SkScalar fStrokeWidth;
    SkPaint::Style fStrokeStyle;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* GlyphPosHairlineStrokeAndFillFactory(void*) {
    return new GlyphPosGM(0.0f, SkPaint::kStrokeAndFill_Style);
}
static GM* GlyphPosStrokeAndFillFactory(void*) {
    return new GlyphPosGM(1.2f, SkPaint::kStrokeAndFill_Style);
}
static GM* GlyphPosHairlineStrokeFactory(void*) {
    return new GlyphPosGM(0.0f, SkPaint::kStroke_Style);
}
static GM* GlyphPosStrokeFactory(void*) {
    return new GlyphPosGM(1.2f, SkPaint::kStroke_Style);
}
static GM* GlyphPosHairlineFillFactory(void*) {
    return new GlyphPosGM(0.0f, SkPaint::kFill_Style);
}
static GM* GlyphPosFillFactory(void*) {
    return new GlyphPosGM(1.2f, SkPaint::kFill_Style);
}

static GMRegistry reg1(GlyphPosHairlineStrokeAndFillFactory);
static GMRegistry reg2(GlyphPosStrokeAndFillFactory);
static GMRegistry reg3(GlyphPosHairlineStrokeFactory);
static GMRegistry reg4(GlyphPosStrokeFactory);
static GMRegistry reg5(GlyphPosHairlineFillFactory);
static GMRegistry reg6(GlyphPosFillFactory);


}

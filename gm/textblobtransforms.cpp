/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

namespace skiagm {
class TextBlobTransforms : public GM {
public:
    // This gm tests that textblobs can be translated, rotated, and scaled
    TextBlobTransforms() {}

protected:
    void onOnceBeforeDraw() override {
        SkTextBlobBuilder builder;

        // make textblob.  To stress distance fields, we choose sizes appropriately
        SkPaint paint;
        paint.setTextSize(162);
        const char* text = "A";
        sk_tool_utils::set_portable_typeface(&paint);

        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);
        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, 0);

        // Medium
        SkScalar xOffset = bounds.width() + 5;
        paint.setTextSize(72);
        text = "B";
        sk_tool_utils::add_to_text_blob(&builder, text, paint, xOffset, 0);

        paint.measureText(text, strlen(text), &bounds);
        SkScalar yOffset = bounds.height();

        // Small
        paint.setTextSize(32);
        text = "C";
        sk_tool_utils::add_to_text_blob(&builder, text, paint, xOffset, -yOffset - 10);

        // build
        fBlob.reset(builder.build());
    }

    SkString onShortName() override {
        return SkString("textblobtransforms");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorGRAY));

        SkPaint paint;

        SkRect bounds = fBlob->bounds();
        canvas->translate(20, 20);

        // Colors were chosen to map to pairs of canonical colors.  The GPU Backend will cache A8
        // Texture Blobs based on the canonical color they map to.  Canonical colors are used to
        // create masks.  For A8 there are 8 of them.
        //SkColor colors[] = {SK_ColorCYAN, sk_tool_utils::color_to_565(SK_ColorLTGRAY), SK_ColorYELLOW, SK_ColorWHITE};

        SkScalar xOffset = SkScalarCeilToScalar(bounds.width());
        SkScalar yOffset = SkScalarCeilToScalar(bounds.height());
        // first translate
        canvas->translate(xOffset, 2 * yOffset);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->translate(-xOffset, 0);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->translate(2 * xOffset, 0);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->translate(-xOffset, -yOffset);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->translate(0, 2 * yOffset);
        canvas->drawTextBlob(fBlob, 0, 0, paint);

        // now rotate
        canvas->translate(4 * xOffset, -yOffset);
        canvas->rotate(180.f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->rotate(-180.f);
        canvas->translate(0, -yOffset);
        canvas->rotate(-180.f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->rotate(270.f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->rotate(-90.f);
        canvas->translate(-xOffset, yOffset);
        canvas->rotate(-90.f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->rotate(90.f);

        // and scales
        canvas->translate(- 3 * xOffset, 3 * yOffset);
        canvas->scale(1.5f, 1.5f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->translate(xOffset, 0);
        canvas->scale(.25f, .25f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
        canvas->translate(xOffset, 0);
        canvas->scale(3.f, 2.f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);

        // finally rotates, scales, and translates together
        canvas->translate(xOffset, 0);
        canvas->rotate(23.f);
        canvas->scale(.33f, .5f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);

        canvas->rotate(-46.f);
        canvas->translate(xOffset, 0);
        canvas->scale(1.2f, 1.1f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);

        canvas->rotate(46.f);
        canvas->translate(xOffset, 0);
        canvas->scale(1.1f, 1.2f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);

        canvas->rotate(46.f);
        canvas->translate(xOffset, 0);
        canvas->scale(.95f, 1.1f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);

        canvas->rotate(46.f);
        canvas->translate(xOffset, 0);
        canvas->scale(1.3f, .7f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);

        canvas->rotate(46.f);
        canvas->translate(xOffset, 0);
        canvas->scale(.8f, 1.1f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);

        canvas->rotate(10.f);
        canvas->translate(xOffset, 0);
        canvas->scale(1.f, 5.f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);

        canvas->rotate(5.f);
        canvas->translate(xOffset, 0);
        canvas->scale(5.f, 1.f);
        canvas->drawTextBlob(fBlob, 0, 0, paint);
    }

private:
    SkAutoTUnref<const SkTextBlob> fBlob;

    static const int kWidth = 1000;
    static const int kHeight = 1200;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobTransforms;)
}

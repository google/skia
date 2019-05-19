/*
 * Copyright 2015 Google Inc.
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
#include "tools/ToolUtils.h"

#include <string.h>

namespace skiagm {
class TextBlobTransforms : public GM {
public:
    // This gm tests that textblobs can be translated, rotated, and scaled
    TextBlobTransforms() {}

protected:
    void onOnceBeforeDraw() override {
        SkTextBlobBuilder builder;

        // make textblob.  To stress distance fields, we choose sizes appropriately
        SkFont font(ToolUtils::create_portable_typeface(), 162);
        font.setEdging(SkFont::Edging::kAlias);
        const char* text = "A";

        SkRect bounds;
        font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);
        ToolUtils::add_to_text_blob(&builder, text, font, 0, 0);

        // Medium
        SkScalar xOffset = bounds.width() + 5;
        font.setSize(72);
        text = "B";
        ToolUtils::add_to_text_blob(&builder, text, font, xOffset, 0);

        font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);
        SkScalar yOffset = bounds.height();

        // Small
        font.setSize(32);
        text = "C";
        ToolUtils::add_to_text_blob(&builder, text, font, xOffset, -yOffset - 10);

        // build
        fBlob = builder.make();
    }

    SkString onShortName() override {
        return SkString("textblobtransforms");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;

        SkRect bounds = fBlob->bounds();
        canvas->translate(20, 20);

        // Colors were chosen to map to pairs of canonical colors.  The GPU Backend will cache A8
        // Texture Blobs based on the canonical color they map to.  Canonical colors are used to
        // create masks.  For A8 there are 8 of them.
        //SkColor colors[] = {SK_ColorCYAN, SK_ColorLTGRAY, SK_ColorYELLOW, SK_ColorWHITE};

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
    sk_sp<SkTextBlob> fBlob;

    static constexpr int kWidth = 1000;
    static constexpr int kHeight = 1200;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobTransforms;)
}

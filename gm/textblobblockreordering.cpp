/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkCanvas.h"
#include "SkTextBlob.h"

namespace skiagm {
class TextBlobBlockReordering : public GM {
public:
    // This gm tests that textblobs translate properly when their draw order is different from their
    // flush order
    TextBlobBlockReordering() { }

protected:
    void onOnceBeforeDraw() override {
        SkTextBlobBuilder builder;

        // make textblob
        // Large text is used to trigger atlas eviction
        SkPaint paint;
        paint.setTextSize(56);
        const char* text = "AB";
        sk_tool_utils::set_portable_typeface(&paint);

        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);

        SkScalar yOffset = bounds.height();
        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, yOffset - 30);

        // build
        fBlob = builder.make();
    }

    SkString onShortName() override {
        return SkString("textblobblockreordering");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    // This draws the same text blob 3 times.  The second draw used a different xfer mode so its
    // GrDrawOp doesn't get combined with the first and third. Ultimately, they will be flushed in
    // the order first, third, and then second.
    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorGRAY));

        SkPaint paint;
        canvas->translate(10, 40);

        SkRect bounds = fBlob->bounds();
        const int yDelta = SkScalarFloorToInt(bounds.height()) + 20;
        const int xDelta = SkScalarFloorToInt(bounds.width());

        canvas->drawTextBlob(fBlob, 0, 0, paint);

        canvas->translate(SkIntToScalar(xDelta), SkIntToScalar(yDelta));

        // Draw a rect where the text should be, and then twiddle the xfermode so we don't combine.
        SkPaint redPaint;
        redPaint.setColor(SK_ColorRED);
        canvas->drawRect(bounds, redPaint);
        SkPaint srcInPaint(paint);
        srcInPaint.setBlendMode(SkBlendMode::kSrcIn);
        canvas->drawTextBlob(fBlob, 0, 0, srcInPaint);

        canvas->translate(SkIntToScalar(xDelta), SkIntToScalar(yDelta));
        canvas->drawTextBlob(fBlob, 0, 0, paint);
    }

private:
    sk_sp<SkTextBlob> fBlob;

    static constexpr int kWidth = 275;
    static constexpr int kHeight = 200;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobBlockReordering;)
}

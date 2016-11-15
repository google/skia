/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkTextBlob.h"
#include "SkCanvas.h"

#define WIDTH 800
#define HEIGHT 800


// This test ensures that glyphs that are too large for the atlas
// are both translated and clipped correctly.
class ClipErrorGM : public skiagm::GM {
public:
    ClipErrorGM() {}

protected:
    SkString onShortName() override { return SkString("cliperror"); }

    SkISize onISize() override { return SkISize::Make(WIDTH, HEIGHT); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);

        const char text[] = "hambur";

        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(256);
        paint.setAntiAlias(true);

        // setup up maskfilter
        const SkScalar kSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(50));

        SkPaint blurPaint(paint);
        blurPaint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, kSigma));

        SkTextBlobBuilder builder;

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, 0);

        sk_sp<SkTextBlob> blob(builder.make());

        SkPaint clearPaint(paint);
        clearPaint.setColor(SK_ColorWHITE);

        canvas->save();
        canvas->translate(0, 0);
        canvas->clipRect(SkRect::MakeLTRB(0, 0, 800, 256));
        {
            canvas->save();
            canvas->clipRect(SkRect::MakeLTRB(0, 0, 1081, 665));
            canvas->drawRect(SkRect::MakeLTRB(0, 0, 1081, 665), clearPaint);
            // draw as blurred to push glyph to be too large for atlas
            canvas->drawTextBlob(blob, 0, 256, blurPaint);
            canvas->drawTextBlob(blob, 0, 477, paint);
            canvas->restore();
        }
        canvas->restore();

        canvas->save();
        canvas->translate(0, 256);
        canvas->clipRect(SkRect::MakeLTRB(0, 256, 800, 510));
        {
            canvas->save();
            canvas->clipRect(SkRect::MakeLTRB(0, 0, 1081, 665));
            canvas->drawRect(SkRect::MakeLTRB(0, 0, 1081, 665), clearPaint);
            // draw as blurred to push glyph to be too large for atlas
            canvas->drawTextBlob(blob, 0, 256, blurPaint);
            canvas->drawTextBlob(blob, 0, 477, paint);
            canvas->restore();
        }
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new ClipErrorGM;)

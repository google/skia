/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkTextBlob.h"
#include "src/core/SkBlurMask.h"
#include "tools/ToolUtils.h"

#define WIDTH 800
#define HEIGHT 800

static void draw_text(SkCanvas* canvas, sk_sp<SkTextBlob> blob,
                      const SkPaint& paint, const SkPaint& blurPaint,
                      const SkPaint& clearPaint) {
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(0, 0, 1081, 665));
    canvas->drawRect(SkRect::MakeLTRB(0, 0, 1081, 665), clearPaint);
    // draw as blurred to push glyph to be too large for atlas
    canvas->drawTextBlob(blob, 0, 256, blurPaint);
    canvas->drawTextBlob(blob, 0, 477, paint);
    canvas->restore();
}

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

        SkFont font(ToolUtils::create_portable_typeface(), 256);

        // setup up maskfilter
        const SkScalar kSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(50));

        SkPaint blurPaint(paint);
        blurPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, kSigma));

        const char text[] = "hambur";
        auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);

        SkPaint clearPaint(paint);
        clearPaint.setColor(SK_ColorWHITE);

        canvas->save();
        canvas->translate(0, 0);
        canvas->clipRect(SkRect::MakeLTRB(0, 0, WIDTH, 256));
        draw_text(canvas, blob, paint, blurPaint, clearPaint);
        canvas->restore();

        canvas->save();
        canvas->translate(0, 256);
        canvas->clipRect(SkRect::MakeLTRB(0, 256, WIDTH, 510));
        draw_text(canvas, blob, paint, blurPaint, clearPaint);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new ClipErrorGM;)

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkBlurMask.h"
#include "tools/ToolUtils.h"

#include <string.h>

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

// Reproduces the canvas-clip-rule.html layout test in Blink's web tests
DEF_SIMPLE_GM(canvas_clip_rule, canvas, 200, 200) {
    SkPaint p;
    p.setStyle(SkPaint::kFill_Style);
    p.setAntiAlias(true);

    // Default fill type (winding)
    p.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 100), p);

    p.setColor(SK_ColorGREEN);
    SkPath path;
    path.addRect(SkRect::MakeXYWH(0, 0, 100, 100));
    path.addRect(SkRect::MakeXYWH(25, 25, 50, 50));
    path.setFillType(SkPathFillType::kWinding);

    canvas->clipPath(path, true);
    path.reset();
    canvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 100), p);

    // Explicit 'nonzero' fill type (winding)
    p.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 100), p);

    p.setColor(SK_ColorGREEN);
    path.reset();
    path.addRect(SkRect::MakeXYWH(0, 0, 100, 100));
    path.addRect(SkRect::MakeXYWH(25, 25, 50, 50));
    path.setFillType(SkPathFillType::kWinding);

    canvas->clipPath(path, true);
    path.reset();
    canvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 100), p);

    // Even-odd fill type
    p.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 100), p);

    p.setColor(SK_ColorGREEN);
    path.reset();
    path.addRect(SkRect::MakeXYWH(0, 0, 100, 100));
    path.addRect(SkRect::MakeXYWH(25, 25, 50, 50));
    path.setFillType(SkPathFillType::kEvenOdd);

    canvas->clipPath(path, true);
    path.reset();
    canvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 100), p);
}

DEF_SIMPLE_GM(clip_video_failure, canvas, 130, 40) {
    auto drawClippedImage = [canvas](const SkImage* img, float x, float y) {
        // drawImage(x,y)
        // - these are what the element size and default dst size work out to for html video elements
        SkRect src = SkRect::MakeXYWH(0.f, 0.f, img->width(), img->height());
        SkRect dst = SkRect::MakeXYWH(x, y, img->width(), img->height());
        // drawImageInternal
        canvas->save();
        canvas->clipRect(dst); // aa doesn't seem to matter

        canvas->translate(dst.x(), dst.y());
        canvas->scale(dst.width() / src.width(), dst.height() / src.height());
        canvas->translate(-src.x(), -src.y());

        // paintCurrentFrame
        // - since everything passed was already sized to the video element, and I'm assuming the
        // video within the element for this test didn't have any complex sizing going on, that
        // no additional transforms were added before the actual drawImage call.
        canvas->drawImage(img, 0.f, 0.f);

        canvas->restore();
    };

    // Make a "video frame" that's 150x60
    SkBitmap video = ToolUtils::create_checkerboard_bitmap(150, 60, SK_ColorLTGRAY, SK_ColorGRAY, 30);
    sk_sp<SkImage> frame = SkImage::MakeFromBitmap(video);

    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SkColorSetARGB(255, 0, 100, 255));
    canvas->drawRect(SkRect::MakeXYWH(5, 5, 120, 30), p);

    SkPath clip;
    clip.moveTo(0, 0);
    clip.lineTo(0, 45);
    clip.lineTo(80, 45);
    clip.lineTo(80, 0);
    canvas->clipPath(clip); // aa doesn't seem to matter

    canvas->translate(40, -10);
    canvas->scale(0.4f, 0.6f);
    canvas->rotate(22.5f);
    canvas->translate(-10, -10);

    drawClippedImage(frame.get(), 10.f, 10.f);
}

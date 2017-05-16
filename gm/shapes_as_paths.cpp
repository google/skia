/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkAutoPixmapStorage.h"
#include "SkImage.h"
#include "SkPath.h"
#include "SkSurface.h"

namespace skiagm {

static bool draw_diff(SkCanvas* canvas, SkImage* imgA, SkImage* imgB) {
    SkASSERT(imgA->dimensions() == imgB->dimensions());

    int w = imgA->width(), h = imgA->height();

    // First, draw the two images faintly overlaid
    SkPaint paint;
    paint.setAlpha(64);
    paint.setBlendMode(SkBlendMode::kPlus);
    canvas->drawImage(imgA, 0, 0, &paint);
    canvas->drawImage(imgB, 0, 0, &paint);

    // Next, read the pixels back, figure out if there are any differences
    // TODO: Read F16 directly, etc...
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    SkAutoPixmapStorage pmapA;
    SkAutoPixmapStorage pmapB;
    pmapA.alloc(info);
    pmapB.alloc(info);
    if (!imgA->readPixels(pmapA, 0, 0) || !imgB->readPixels(pmapB, 0, 0)) {
        return false;
    }

    int dX = 0, dY = 0, maxDiff = 0;
    SkBitmap highlight;
    highlight.allocN32Pixels(w, h);
    highlight.eraseColor(SK_ColorTRANSPARENT);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t pixelA = *pmapA.addr32(x, y);
            uint32_t pixelB = *pmapB.addr32(x, y);
            if (pixelA != pixelB) {
                int diff =
                    SkTAbs((int)(SkColorGetR(pixelA) - SkColorGetR(pixelB))) +
                    SkTAbs((int)(SkColorGetG(pixelA) - SkColorGetG(pixelB))) +
                    SkTAbs((int)(SkColorGetB(pixelA) - SkColorGetB(pixelB))) +
                    SkTAbs((int)(SkColorGetA(pixelA) - SkColorGetA(pixelB)));
                if (diff > maxDiff) {
                    dX = x;
                    dY = y;
                    maxDiff = diff;
                }
                *highlight.getAddr32(x, y) = SkPackARGB32(0xA0, 0xA0, 0x00, 0x00);
            }
        }
    }

    if (maxDiff == 0) {
        // Big green circle to make it obvious that things are correct when triaging
        SkPaint matchPaint;
        matchPaint.setColor(SK_ColorGREEN);
        canvas->drawCircle(w * 1.5f + 10, h * 0.5f, SkTMin(w, h) * 0.4f, matchPaint);
        return true;
    } else {
        // Call extra attention to the region we're going to zoom
        SkPMColor yellow = SkPackARGB32(0xFF, 0xFF, 0xFF, 0x00);
        *highlight.getAddr32(dX, dY) = yellow;
        *highlight.getAddr32(SkTMax(dX - 1, 0), dY) = yellow;
        *highlight.getAddr32(dX, SkTMax(dY - 1, 0)) = yellow;
        *highlight.getAddr32(SkTMin(dX + 1, w - 1), dY) = yellow;
        *highlight.getAddr32(dX, SkTMin(dY + 1, h - 1)) = yellow;

        // Draw the overlay
        canvas->drawBitmap(highlight, 0, 0);

        // Draw zoom of largest pixel diff
        canvas->drawImageRect(imgA, SkRect::MakeXYWH(dX - 5, dY - 5, 10, 10),
                              SkRect::MakeXYWH(w + 10, 0, w, h), nullptr);
        canvas->drawImageRect(imgB, SkRect::MakeXYWH(dX - 5, dY - 5, 10, 10),
                              SkRect::MakeXYWH(w + 10 + w, 0, w, h), nullptr);

        // Put hairline boxes around zoom
        SkPaint outline;
        outline.setColor(0xFF7f7f7f);
        outline.setAntiAlias(true);
        outline.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(SkRect::MakeXYWH(w + 10, 0, w, h), outline);
        canvas->drawRect(SkRect::MakeXYWH(w + 10 + w, 0, w, h), outline);
        return false;
    }
}

DEF_SIMPLE_GM(shapes_as_paths, canvas, 275, 310) {
    // Drawing a shape vs. adding shape to a path and drawing a path, should produce same results.
    //
    // Variables:
    // - Shape type (rect, oval, arc, rrect, line)
    // - Fill, hairline, wide stroke (3)
    // - Axis aligned, rotated, scaled, scaled negative, perspective (5?)
    // - Source geometry (normal, collapsed, inverted) (3)
    // - AntiAlias (2)

    canvas->clear(SK_ColorBLACK);
    canvas->translate(10, 10);

    SkImageInfo info = canvas->imageInfo().makeWH(50, 50);
    if (kUnknown_SkColorType == info.colorType()) {
        return;
    }
    auto surface = canvas->makeSurface(info);

    SkRect r = SkRect::MakeXYWH(10, 10, 30, 0);

    // Do first draw
    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5);
    surface->getCanvas()->clear(SK_ColorBLACK);
    surface->getCanvas()->drawRect(r, paint);
    auto imgA = surface->makeImageSnapshot();

    surface->getCanvas()->clear(SK_ColorBLACK);
    SkPath path;
    path.addRect(r);
    surface->getCanvas()->drawPath(path, paint);
    auto imgB = surface->makeImageSnapshot();

    draw_diff(canvas, imgA.get(), imgB.get());
}

static void draw_degenerate_rects(SkCanvas* canvas,
                                  std::function<void(SkCanvas*, const SkRect&,
                                                     const SkPaint&)> drawFunc) {
    const int kPad = 15;

    canvas->translate(kPad, kPad);

    // Width is always 50, exercise different aspect ratios, flipped, etc...
    const int rectHeights[] = { 50, 40, 5, 0, -5, -40, -50 };

    // Various stroking/filling setups
    struct StyleParams {
        SkPaint::Style fStyle;
        SkScalar fStrokeWidth;
    } kStyles[] = {
        { SkPaint::kFill_Style, 0 },
        { SkPaint::kStroke_Style, 0 },
        { SkPaint::kStroke_Style, 5 },
        { SkPaint::kStrokeAndFill_Style, 5 },
    };

    for (auto style : kStyles) {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(style.fStyle);
        p.setStrokeWidth(style.fStrokeWidth);

        canvas->save();
        for (int h : rectHeights) {
            SkRect r = SkRect::MakeXYWH(0, SkTMax(0, -h), 50, h);
            drawFunc(canvas, r, p);
            canvas->translate(0, SkTAbs(h) + kPad);
        }
        canvas->restore();
        canvas->translate(50 + kPad, 0);
    }
}

DEF_SIMPLE_GM(degenerate_rects, canvas, 275, 310) {
    auto drawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        canvas->drawRect(rect, paint);
    };
    draw_degenerate_rects(canvas, drawFunc);
}

DEF_SIMPLE_GM(degenerate_rects_as_paths, canvas, 275, 310) {
    auto drawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        SkPath path;
        path.addRect(rect);
        canvas->drawPath(path, paint);
    };
    draw_degenerate_rects(canvas, drawFunc);
}

DEF_SIMPLE_GM(degenerate_ovals, canvas, 275, 310) {
    auto drawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        canvas->drawOval(rect, paint);
    };
    draw_degenerate_rects(canvas, drawFunc);
}

DEF_SIMPLE_GM(degenerate_ovals_as_paths, canvas, 275, 310) {
    auto drawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        SkPath path;
        path.addOval(rect);
        canvas->drawPath(path, paint);
    };
    draw_degenerate_rects(canvas, drawFunc);
}

DEF_SIMPLE_GM(degenerate_arcs, canvas, 275, 310) {
    auto drawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        canvas->drawArc(rect, 0, 270, false, paint);
    };
    draw_degenerate_rects(canvas, drawFunc);
}

DEF_SIMPLE_GM(degenerate_arcs_with_center, canvas, 275, 310) {
    auto drawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        canvas->drawArc(rect, 0, 270, true, paint);
    };
    draw_degenerate_rects(canvas, drawFunc);
}

DEF_SIMPLE_GM(degenerate_rrects, canvas, 275, 310) {
    auto drawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        canvas->drawRoundRect(rect, 5, 5, paint);
    };
    draw_degenerate_rects(canvas, drawFunc);
}

}

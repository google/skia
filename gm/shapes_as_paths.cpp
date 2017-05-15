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

static void draw_diff(SkCanvas* canvas, SkImage* imgA, SkImage* imgB) {
    SkASSERT(imgA->dimensions() == imgB->dimensions());

    SkScalar w = imgA->width(), h = imgA->height();

    // First, draw the two images faintly overlaid
    SkPaint paint;
    paint.setAlpha(64);
    paint.setBlendMode(SkBlendMode::kPlus);
    canvas->drawImage(imgA, 0, 0, &paint);
    canvas->drawImage(imgB, 0, 0, &paint);

    // Next, read the pixels back, figure out if there are any differences
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    SkAutoPixmapStorage pmapA;
    SkAutoPixmapStorage pmapB;
    pmapA.alloc(info);
    pmapB.alloc(info);
    if (!imgA->readPixels(pmapA, 0, 0) || !imgB->readPixels(pmapB, 0, 0)) {
        return;
    }

    int dX = -1, dY = -1;
    SkBitmap highlight;
    highlight.allocN32Pixels(w, h);
    highlight.eraseColor(SK_ColorTRANSPARENT);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t pixelA = *pmapA.addr32(x, y);
            uint32_t pixelB = *pmapB.addr32(x, y);
            if (pixelA != pixelB) {
                if (dX < 0) {
                    dX = x;
                    dY = y;
                }
                *highlight.getAddr32(x, y) = SK_ColorRED;
            }
        }
    }

    // Draw the overlay
    canvas->drawBitmap(highlight, 0, 0);

    if (dX < 0) {
        // Big green circle to make it obvious that things are correct when triaging
        SkPaint matchPaint;
        matchPaint.setColor(SK_ColorGREEN);
        canvas->drawCircle(w * 1.5f + 10, h * 0.5f, SkTMin(w, h) * 0.4f, matchPaint);
    } else {
        // Draw zoom of first region of interest
        // TODO: Cluster mis-matched pixels, or at least find pixel with largest diff?
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
    }
}

DEF_SIMPLE_GM(diff_gm_test, canvas, 275, 310) {
    canvas->clear(SK_ColorBLACK);

    SkImageInfo info = canvas->imageInfo().makeWH(50, 50);
    if (kUnknown_SkColorType == info.colorType()) {
        return;
    }
    auto surface = canvas->makeSurface(info);

    // Do first draw
    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(true);
    surface->getCanvas()->clear(SK_ColorBLACK);
    surface->getCanvas()->drawRect(SkRect::MakeXYWH(10, 10, 30, 30), paint);
    auto imgA = surface->makeImageSnapshot();

    int x2 = canvas->getGrContext() ? 11 : 10;
    surface->getCanvas()->clear(SK_ColorBLACK);
    surface->getCanvas()->drawRect(SkRect::MakeXYWH(x2, 10, 30, 30), paint);
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

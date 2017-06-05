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

    int w = imgA->width(), h = imgA->height();

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

    int maxDiffX = 0, maxDiffY = 0, maxDiff = 0;
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
                    maxDiffX = x;
                    maxDiffY = y;
                    maxDiff = diff;
                }
                *highlight.getAddr32(x, y) = SkPackARGB32(0xA0, 0xA0, 0x00, 0x00);
            }
        }
    }

    SkPaint outline;
    outline.setStyle(SkPaint::kStroke_Style);
    outline.setColor(maxDiff == 0 ? 0xFF007F00 : 0xFF7F0000);

    if (maxDiff > 0) {
        // Call extra attention to the region we're going to zoom
        SkPMColor yellow = SkPackARGB32(0xFF, 0xFF, 0xFF, 0x00);
        *highlight.getAddr32(maxDiffX, maxDiffY) = yellow;
        *highlight.getAddr32(SkTMax(maxDiffX - 1, 0), maxDiffY) = yellow;
        *highlight.getAddr32(maxDiffX, SkTMax(maxDiffY - 1, 0)) = yellow;
        *highlight.getAddr32(SkTMin(maxDiffX + 1, w - 1), maxDiffY) = yellow;
        *highlight.getAddr32(maxDiffX, SkTMin(maxDiffY + 1, h - 1)) = yellow;

        // Draw the overlay
        canvas->drawBitmap(highlight, 0, 0);

        // Draw zoom of largest pixel diff
        SkBitmap bmpA, bmpB;
        SkAssertResult(bmpA.installPixels(pmapA));
        SkAssertResult(bmpB.installPixels(pmapB));
        canvas->drawBitmapRect(bmpA, SkRect::MakeXYWH(maxDiffX - 5, maxDiffY - 5, 10, 10),
                               SkRect::MakeXYWH(w, 0, w, h), nullptr);
        canvas->drawBitmapRect(bmpB, SkRect::MakeXYWH(maxDiffX - 5, maxDiffY - 5, 10, 10),
                               SkRect::MakeXYWH(2 * w, 0, w, h), nullptr);

        // Add lines to separate zoom boxes
        canvas->drawLine(w, 0, w, h, outline);
        canvas->drawLine(2 * w, 0, 2 * w, h, outline);
    }

    // Draw outline of whole test region
    canvas->drawRect(SkRect::MakeWH(3 * w, h), outline);
}

namespace {
typedef std::function<void(SkCanvas*, const SkRect&, const SkPaint&)> ShapeDrawFunc;
}

/**
 *  Iterates over a variety of rect shapes, paint parameters, and matrices, calling two different
 *  user-supplied draw callbacks. Produces a grid clearly showing if the two callbacks produce the
 *  same visual results in all cases.
 */
static void draw_rect_geom_diff_grid(SkCanvas* canvas, ShapeDrawFunc f1, ShapeDrawFunc f2) {
    // Variables:
    // - Fill, hairline, wide stroke
    // - Axis aligned, rotated, scaled, scaled negative, perspective
    // - Source geometry (normal, collapsed, inverted)
    //
    // Things not (yet?) tested:
    // - AntiAlias on/off
    // - StrokeAndFill
    // - Cap/join
    // - Anything even more elaborate...

    const SkRect kRects[] = {
        SkRect::MakeXYWH(10, 10, 30, 30),  // Normal
        SkRect::MakeXYWH(10, 25, 30, 0),   // Collapsed
        SkRect::MakeXYWH(10, 40, 30, -30), // Inverted
    };

    const struct { SkPaint::Style fStyle; SkScalar fStrokeWidth; } kStyles[] = {
        { SkPaint::kFill_Style, 0 },   // Filled
        { SkPaint::kStroke_Style, 0 }, // Hairline
        { SkPaint::kStroke_Style, 5 }, // Wide stroke
    };

    SkMatrix mI = SkMatrix::I();
    SkMatrix mRot;
    mRot.setRotate(30, 25, 25);
    SkMatrix mScale;
    mScale.setScaleTranslate(0.5f, 1, 12.5f, 0);
    SkMatrix mFlipX;
    mFlipX.setScaleTranslate(-1, 1, 50, 0);
    SkMatrix mFlipY;
    mFlipY.setScaleTranslate(1, -1, 0, 50);
    SkMatrix mFlipXY;
    mFlipXY.setScaleTranslate(-1, -1, 50, 50);
    SkMatrix mPersp;
    mPersp.setIdentity();
    mPersp.setPerspY(0.002f);

    const SkMatrix* kMatrices[] = { &mI, &mRot, &mScale, &mFlipX, &mFlipY, &mFlipXY, &mPersp, };

    canvas->translate(10, 10);

    SkImageInfo info = canvas->imageInfo().makeWH(50, 50);
    auto surface = canvas->makeSurface(info);
    if (!surface) {
        surface = SkSurface::MakeRasterN32Premul(50, 50);
    }

    for (const SkRect& rect : kRects) {
        for (const auto& style : kStyles) {
            canvas->save();

            for (const SkMatrix* mat : kMatrices) {
                SkPaint paint;
                paint.setColor(SK_ColorWHITE);
                paint.setAntiAlias(true);
                paint.setStyle(style.fStyle);
                paint.setStrokeWidth(style.fStrokeWidth);

                // Do first draw
                surface->getCanvas()->clear(SK_ColorBLACK);
                surface->getCanvas()->save();
                surface->getCanvas()->concat(*mat);
                f1(surface->getCanvas(), rect, paint);
                surface->getCanvas()->restore();
                auto imgA = surface->makeImageSnapshot();

                // Do second draw
                surface->getCanvas()->clear(SK_ColorBLACK);
                surface->getCanvas()->save();
                surface->getCanvas()->concat(*mat);
                f2(surface->getCanvas(), rect, paint);
                surface->getCanvas()->restore();
                auto imgB = surface->makeImageSnapshot();

                draw_diff(canvas, imgA.get(), imgB.get());
                canvas->translate(160, 0);
            }
            canvas->restore();
            canvas->translate(0, 60);
        }
    }
}

static const int kNumRows = 9;
static const int kNumColumns = 7;
static const int kTotalWidth = kNumColumns * 160 + 10;
static const int kTotalHeight = kNumRows * 60 + 10;

DEF_SIMPLE_GM_BG(rects_as_paths, canvas, kTotalWidth, kTotalHeight, SK_ColorBLACK) {
    // Drawing a rect vs. adding it to a path and drawing the path, should produce same results.
    auto rectDrawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        canvas->drawRect(rect, paint);
    };
    auto pathDrawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        SkPath path;
        path.addRect(rect);
        canvas->drawPath(path, paint);
    };

    draw_rect_geom_diff_grid(canvas, rectDrawFunc, pathDrawFunc);
}

DEF_SIMPLE_GM_BG(ovals_as_paths, canvas, kTotalWidth, kTotalHeight, SK_ColorBLACK) {
    // Drawing an oval vs. adding it to a path and drawing the path, should produce same results.
    auto ovalDrawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        canvas->drawOval(rect, paint);
    };
    auto pathDrawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        SkPath path;
        path.addOval(rect);
        canvas->drawPath(path, paint);
    };

    draw_rect_geom_diff_grid(canvas, ovalDrawFunc, pathDrawFunc);
}

DEF_SIMPLE_GM_BG(arcs_as_paths, canvas, kTotalWidth, kTotalHeight, SK_ColorBLACK) {
    // Drawing an arc vs. adding it to a path and drawing the path, should produce same results.
    auto arcDrawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        canvas->drawArc(rect, 10, 200, false, paint);
    };
    auto pathDrawFunc = [](SkCanvas* canvas, const SkRect& rect, const SkPaint& paint) {
        SkPath path;
        path.addArc(rect, 10, 200);
        canvas->drawPath(path, paint);
    };

    draw_rect_geom_diff_grid(canvas, arcDrawFunc, pathDrawFunc);
}

}

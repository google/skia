/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkSurface.h"

#define ZOOM    32
#define SMALL_W 9
#define SMALL_H 3
#define REPEAT_LOOP 5

static sk_sp<SkSurface> new_surface(int width, int height) {
    return SkSurface::MakeRasterN32Premul(width, height);
}

static void draw_pixel_centers(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(sk_tool_utils::color_to_565(0xFF0088FF));
    paint.setAntiAlias(true);

    for (int y = 0; y < SMALL_H; ++y) {
        for (int x = 0; x < SMALL_W; ++x) {
            canvas->drawCircle(x + 0.5f, y + 0.5f, 1.5f / ZOOM, paint);
        }
    }
}

static void draw_fatpath(SkCanvas* canvas, SkSurface* surface, const SkPath& path) {
    SkPaint paint;

    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
    surface->getCanvas()->drawPath(path, paint);
    surface->draw(canvas, 0, 0, nullptr);

    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);

    draw_pixel_centers(canvas);
}

DEF_SIMPLE_GM(fatpathfill, canvas,
              SMALL_W * ZOOM,
              SMALL_H * ZOOM * REPEAT_LOOP) {
        auto surface(new_surface(SMALL_W, SMALL_H));

        canvas->scale(ZOOM, ZOOM);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1);

        for (int i = 0; i < REPEAT_LOOP; ++i) {
            SkPath line, path;
            line.moveTo(1, 2);
            line.lineTo(SkIntToScalar(4 + i), 1);
            paint.getFillPath(line, &path);
            draw_fatpath(canvas, surface.get(), path);

            canvas->translate(0, SMALL_H);
        }
}

DEF_SIMPLE_GM(precision_circles, canvas, 256, 256) {
    SkPaint strokePaint;
    strokePaint.setAntiAlias(true);
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(.1f);
    SkPath strokePath;
    strokePath.moveTo(.08f, .08f);
    strokePath.quadTo(.09f, .08f, .17f, .17f);
    SkPath fillPath;
    SkPaint outlinePaint(strokePaint);
    outlinePaint.setStrokeWidth(2);
    SkMatrix scale = SkMatrix::MakeScale(300, 300);
    for (SkScalar precision : { 0.001f, 0.01f, .1f, 1.f, 10.f, 100.f } ) {
        strokePaint.getFillPath(strokePath, &fillPath, nullptr, precision);
        fillPath.dump();
        SkDebugf("\n");
        fillPath.transform(scale);
        canvas->drawPath(fillPath, outlinePaint);
        canvas->translate(60, 0);
        if (1.f == precision) canvas->translate(-180, 100);
    }
}

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

static SkSurface* new_surface(int width, int height) {
    return SkSurface::NewRasterPMColor(width, height);
}

static void draw_pixel_centers(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(0xFF0088FF);
    paint.setAntiAlias(true);

    for (int y = 0; y < SMALL_H; ++y) {
        for (int x = 0; x < SMALL_W; ++x) {
            canvas->drawCircle(x + 0.5f, y + 0.5f, 1.5f / ZOOM, paint);
        }
    }
}

static void draw_fatpath(SkCanvas* canvas, SkSurface* surface,
                         const SkPath paths[], int count) {
    SkPaint paint;

    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
    for (int i = 0; i < count; ++i) {
        surface->getCanvas()->drawPath(paths[i], paint);
    }
    surface->draw(canvas, 0, 0, NULL);

    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    for (int j = 0; j < count; ++j) {
        canvas->drawPath(paths[j], paint);
    }

    draw_pixel_centers(canvas);
}

class FatPathFillGM : public skiagm::GM {
public:
    FatPathFillGM() {}

protected:
    virtual SkString onShortName() {
        return SkString("fatpathfill");
    }

    virtual SkISize onISize() {
        return SkISize::Make(SMALL_W * ZOOM, SMALL_H * ZOOM * REPEAT_LOOP);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkAutoTUnref<SkSurface> surface(new_surface(SMALL_W, SMALL_H));

        canvas->scale(ZOOM, ZOOM);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1);

        for (int i = 0; i < REPEAT_LOOP; ++i) {
            SkPath line, path;
            line.moveTo(SkIntToScalar(1), SkIntToScalar(2));
            line.lineTo(SkIntToScalar(4 + i), SkIntToScalar(1));
            paint.getFillPath(line, &path);
            draw_fatpath(canvas, surface, &path, 1);

            canvas->translate(0, SMALL_H);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM(return new FatPathFillGM;)

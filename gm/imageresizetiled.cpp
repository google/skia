/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkImageFilter.h"
#include "SkRandom.h"

#define WIDTH 640
#define HEIGHT 480

#define RESIZE_FACTOR SkIntToScalar(2)

DEF_SIMPLE_GM(imageresizetiled, canvas, WIDTH, HEIGHT) {
        SkPaint paint;
        SkMatrix matrix;
        matrix.setScale(RESIZE_FACTOR, RESIZE_FACTOR);
        SkAutoTUnref<SkImageFilter> imageFilter(
            SkImageFilter::CreateMatrixFilter(matrix, kNone_SkFilterQuality));
        paint.setImageFilter(imageFilter.get());
        const SkScalar tile_size = SkIntToScalar(100);
        SkRect bounds;
        canvas->getClipBounds(&bounds);
        for (SkScalar y = 0; y < HEIGHT; y += tile_size) {
            for (SkScalar x = 0; x < WIDTH; x += tile_size) {
                canvas->save();
                canvas->clipRect(SkRect::MakeXYWH(x, y, tile_size, tile_size));
                canvas->scale(SkScalarInvert(RESIZE_FACTOR),
                              SkScalarInvert(RESIZE_FACTOR));
                canvas->saveLayer(nullptr, &paint);
                const char* str[] = {
                    "The quick",
                    "brown fox",
                    "jumped over",
                    "the lazy dog.",
                };
                SkPaint textPaint;
                textPaint.setAntiAlias(true);
                sk_tool_utils::set_portable_typeface(&textPaint);
                textPaint.setTextSize(SkIntToScalar(100));
                int posY = 0;
                for (unsigned i = 0; i < SK_ARRAY_COUNT(str); i++) {
                    posY += 100;
                    canvas->drawText(str[i], strlen(str[i]), SkIntToScalar(0),
                                     SkIntToScalar(posY), textPaint);
                }
                canvas->restore();
                canvas->restore();
            }
        }
}

// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(strokerect_gm, 1400, 740, false, 0) {
void draw(SkCanvas* canvas) {
    constexpr SkScalar kStrokeWidth = 20;
    constexpr SkPaint::Join gJoins[] = {SkPaint::kMiter_Join, SkPaint::kRound_Join,
                                        SkPaint::kBevel_Join};
    constexpr SkScalar W = 80;
    constexpr SkScalar H = 80;
    constexpr SkRect gRects[] = {
            {0, 0, W, H},
            {W, 0, 0, H},
            {0, H, W, 0},
            {0, 0, kStrokeWidth, H},
            {0, 0, W, kStrokeWidth},
            {0, 0, kStrokeWidth / 2, kStrokeWidth / 2},
            {0, 0, W, 0},
            {0, 0, 0, H},
            {0, 0, 0, 0},
            {0, 0, W, FLT_EPSILON},
            {0, 0, FLT_EPSILON, H},
            {0, 0, FLT_EPSILON, FLT_EPSILON},
    };
    canvas->translate(kStrokeWidth * 3 / 2, kStrokeWidth * 3 / 2);
    for (int doFill = 0; doFill <= 1; ++doFill) {
        SkPaint::Style style = doFill ? SkPaint::kStrokeAndFill_Style : SkPaint::kStroke_Style;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gJoins); ++i) {
            SkPaint::Join join = gJoins[i];
            for (size_t j = 0; j < SK_ARRAY_COUNT(gRects); ++j) {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(
                        j * (W + 2 * kStrokeWidth),
                        (i + doFill * SK_ARRAY_COUNT(gJoins)) * (H + 2 * kStrokeWidth));
                const SkRect& rect = gRects[j];

                SkPath path, fillPath;
                path.addRect(rect);
                SkPaint paint;

                paint.setStrokeWidth(kStrokeWidth);
                paint.setStyle(style);
                paint.setStrokeJoin(join);
                paint.getFillPath(path, &fillPath);

                paint.setAntiAlias(true);
                paint.setColor(0xFF8C8A8C);
                canvas->drawRect(rect, paint);

                paint.setStyle(SkPaint::kStroke_Style);
                paint.setStrokeWidth(0);
                paint.setColor(SK_ColorRED);
                canvas->drawPath(fillPath, paint);

                paint.setStrokeWidth(3);
                paint.setStrokeJoin(SkPaint::kMiter_Join);
                int n = fillPath.countPoints();
                SkAutoTArray<SkPoint> points(n);
                fillPath.getPoints(points.get(), n);
                canvas->drawPoints(SkCanvas::kPoints_PointMode, n, points.get(), paint);
            }
        }
    }
}
}  // END FIDDLE

/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkImageFilters.h"

#include <utility>

static SkPath generate_square(SkScalar cx, SkScalar cy, SkScalar w) {
    return SkPath::Rect(SkRect::MakeXYWH(cx - w / 2, cy - w / 2, w, w));
}

static SkPath generate_rect_line(SkScalar cx, SkScalar cy, SkScalar l) {
    return SkPath::Rect(SkRect::MakeXYWH(cx - l / 2, cy, l, 0));
}

static SkPath generate_circle(SkScalar cx, SkScalar cy, SkScalar d) {
    return SkPath::Circle(cx, cy, d/2, SkPathDirection::kCW);
}

static SkPath generate_line(SkScalar cx, SkScalar cy, SkScalar l) {
    return SkPath::Line({cx - l / 2, cy}, {cx + l / 2, cy});
}

namespace {
struct Style {
    Style(SkPaint::Style paintStyle, sk_sp<SkPathEffect> pe = sk_sp<SkPathEffect>())
        : fPaintStyle(paintStyle)
        , fPathEffect(std::move(pe)) {}
    SkPaint::Style      fPaintStyle;
    sk_sp<SkPathEffect> fPathEffect;
};

sk_sp<SkPathEffect> make_dash() {
    constexpr SkScalar kIntervals[] = { 4.f, 3.f };
    return SkDashPathEffect::Make(kIntervals, std::size(kIntervals), 0);
}

Style styles[] {
    {SkPaint::kStroke_Style},
    {SkPaint::kStrokeAndFill_Style},
    {SkPaint::kFill_Style},
    {SkPaint::kStroke_Style, make_dash()},
};

SkScalar pathSizes[] = {
        40,
        10,
        0
};
SkScalar strokeWidths[] = {
        10,
        0
};
SkPath (*paths[])(SkScalar, SkScalar, SkScalar) = {
        generate_square,
        generate_rect_line,
        generate_circle,
        generate_line
};

const SkScalar slideWidth = 90, slideHeight = 90;
const SkScalar slideBoundary = 5;

}  // namespace

DEF_SIMPLE_GM(inverse_paths, canvas, 800, 1200) {
    SkScalar cx = slideWidth / 2 + slideBoundary;
    SkScalar cy = slideHeight / 2 + slideBoundary;
    SkScalar dx = slideWidth + 2 * slideBoundary;
    SkScalar dy = slideHeight + 2 * slideBoundary;

    SkRect clipRect = SkRect::MakeLTRB(slideBoundary, slideBoundary,
                                       slideBoundary + slideWidth,
                                       slideBoundary + slideHeight);
    SkPaint clipPaint;
    clipPaint.setStyle(SkPaint::kStroke_Style);
    clipPaint.setStrokeWidth(SkIntToScalar(2));

    SkPaint outlinePaint;
    outlinePaint.setColor(0x40000000);
    outlinePaint.setStyle(SkPaint::kStroke_Style);
    outlinePaint.setStrokeWidth(SkIntToScalar(0));

    for (size_t styleIndex = 0; styleIndex < std::size(styles);
            styleIndex++) {
        for (size_t sizeIndex = 0; sizeIndex < std::size(pathSizes);
                sizeIndex++) {
            SkScalar size = pathSizes[sizeIndex];

            canvas->save();

            for (size_t widthIndex = 0;
                    widthIndex < std::size(strokeWidths);
                    widthIndex++) {
                SkPaint paint;
                paint.setColor(0xff007000);
                paint.setStrokeWidth(strokeWidths[widthIndex]);
                paint.setStyle(styles[styleIndex].fPaintStyle);
                paint.setPathEffect(styles[styleIndex].fPathEffect);

                for (size_t pathIndex = 0;
                        pathIndex < std::size(paths);
                        pathIndex++) {
                    canvas->drawRect(clipRect, clipPaint);

                    canvas->save();
                    canvas->clipRect(clipRect);

                    SkPath path = paths[pathIndex](cx, cy, size);
                    path.setFillType(SkPathFillType::kInverseWinding);
                    canvas->drawPath(path, paint);

                    path.setFillType(SkPathFillType::kWinding);
                    canvas->drawPath(path, outlinePaint);

                    canvas->restore();
                    canvas->translate(dx, 0);
                }
            }
            canvas->restore();
            canvas->translate(0, dy);
        }
    }
}

DEF_SIMPLE_GM(inverse_fill_filters, canvas, 384, 128) {
    auto draw = [canvas](const SkPaint& paint) {
        SkPath path = SkPath::Circle(65.f, 65.f, 30.f);
        path.setFillType(SkPathFillType::kInverseWinding);

        canvas->save();
        canvas->clipRect({0, 0, 128, 128});
        canvas->drawPath(path, paint);
        canvas->restore();

        SkPaint stroke;
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setColor(SK_ColorWHITE);
        canvas->drawRect({0, 0, 128, 128}, stroke);
    };

    SkPaint paint;
    paint.setAntiAlias(true);

    draw(paint);

    canvas->translate(128, 0);
    paint.setImageFilter(SkImageFilters::Blur(5.f, 5.f, nullptr));
    draw(paint);

    canvas->translate(128, 0);
    paint.setImageFilter(nullptr);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5));
    draw(paint);
}

DEF_SIMPLE_GM(inverse_windingmode_filters, canvas, 256, 100) {
    SkPath path;
    path.addRect({10, 10, 30, 30}, SkPathDirection::kCW);
    path.addRect({20, 20, 40, 40}, SkPathDirection::kCW);
    path.addRect({10, 60, 30, 80}, SkPathDirection::kCW);
    path.addRect({20, 70, 40, 90}, SkPathDirection::kCCW);
    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    SkRect clipRect = {0, 0, 51, 99};
    canvas->drawPath(path, strokePaint);
    SkPaint fillPaint;
    fillPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f));
    for (auto fillType : { SkPathFillType::kWinding,
                           SkPathFillType::kEvenOdd,
                           SkPathFillType::kInverseWinding,
                           SkPathFillType::kInverseEvenOdd } ) {
        canvas->translate(51, 0);
        canvas->save();
        canvas->clipRect(clipRect);
        path.setFillType(fillType);
        canvas->drawPath(path, fillPaint);
        canvas->restore();
        SkPaint clipPaint;
        clipPaint.setColor(SK_ColorRED);
        clipPaint.setStyle(SkPaint::kStroke_Style);
        clipPaint.setStrokeWidth(1.f);
        canvas->drawRect(clipRect, clipPaint);
    }
}

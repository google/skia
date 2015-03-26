/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

namespace skiagm {

static SkPath generate_square(SkScalar cx, SkScalar cy, SkScalar w) {
    SkRect rect = SkRect::MakeXYWH(cx - w / 2, cy - w / 2, w, w);
    SkPath path;
    path.addRect(rect);
    return path;
}

static SkPath generate_rect_line(SkScalar cx, SkScalar cy, SkScalar l) {
    SkRect rect = SkRect::MakeXYWH(cx - l / 2, cy, l, 0);
    SkPath path;
    path.addRect(rect);
    return path;
}

static SkPath generate_circle(SkScalar cx, SkScalar cy, SkScalar d) {
    SkPath path;
    path.addCircle(cx, cy, d/2, SkPath::kCW_Direction);
    return path;
}

static SkPath generate_line(SkScalar cx, SkScalar cy, SkScalar l) {
    SkPath path;
    path.moveTo(cx - l / 2, cy);
    path.lineTo(cx + l / 2, cy);
    return path;
}

SkPaint::Style styles[] = {
        SkPaint::kStroke_Style,
        SkPaint::kStrokeAndFill_Style,
        SkPaint::kFill_Style
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
SkPath ((*paths[])(SkScalar, SkScalar, SkScalar)) = {
        generate_square,
        generate_rect_line,
        generate_circle,
        generate_line
};

const SkScalar slideWidth = 90, slideHeight = 90;
const SkScalar slideBoundary = 5;


class InversePathsGM : public GM {
public:
    InversePathsGM() {

    }

protected:

    SkString onShortName() override {
        return SkString("inverse_paths");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 900);
    }

    void onDraw(SkCanvas* canvas) override {
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

        for (size_t styleIndex = 0; styleIndex < SK_ARRAY_COUNT(styles);
                styleIndex++) {
            for (size_t sizeIndex = 0; sizeIndex < SK_ARRAY_COUNT(pathSizes);
                    sizeIndex++) {
                SkScalar size = pathSizes[sizeIndex];

                canvas->save();

                for (size_t widthIndex = 0;
                        widthIndex < SK_ARRAY_COUNT(strokeWidths);
                        widthIndex++) {
                    SkPaint paint;
                    paint.setColor(0xff007000);
                    paint.setStrokeWidth(strokeWidths[widthIndex]);
                    paint.setStyle(styles[styleIndex]);

                    for (size_t pathIndex = 0;
                            pathIndex < SK_ARRAY_COUNT(paths);
                            pathIndex++) {
                        canvas->drawRect(clipRect, clipPaint);

                        canvas->save();
                        canvas->clipRect(clipRect);

                        SkPath path = paths[pathIndex](cx, cy, size);
                        path.setFillType(SkPath::kInverseWinding_FillType);
                        canvas->drawPath(path, paint);

                        path.setFillType(SkPath::kWinding_FillType);
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

private:
    typedef GM INHERITED;
};

DEF_GM( return new InversePathsGM; )
}

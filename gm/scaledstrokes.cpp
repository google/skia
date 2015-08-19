/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkRRect.h"
#include "SkPath.h"

class ScaledStrokesGM : public skiagm::GM {
public:
    ScaledStrokesGM() {}

protected:

    SkString onShortName() override {
        return SkString("scaledstrokes");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 320);
    }

    static void draw_path(SkScalar size, SkCanvas* canvas, SkPaint paint) {
        SkScalar c = 0.551915024494f * size;
        SkPath path;
        path.moveTo(0.0f, size);
        path.cubicTo(c, size, size, c, size, 0.0f);
        path.cubicTo(size, -c, c, -size, 0.0f, -size);
        path.cubicTo(-c, -size, -size, -c, -size, 0.0f);
        path.cubicTo(-size, c, -c, size, 0.0f, size);
        canvas->drawPath(path, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        SkPath path;
        paint.setStyle(SkPaint::Style::kStroke_Style);
        canvas->translate(5.0f, 5.0f);
        const SkScalar size = 60.0f;
        for (int i = 0; i < 2; i++) {
            paint.setAntiAlias(i == 1);
            canvas->save();
            for (int j = 0; j < 4; j++) {
                SkScalar scale = 4.0f - j;
                paint.setStrokeWidth(4.0f / scale);
                canvas->save();
                canvas->translate(size / 2.0f, size / 2.0f);
                canvas->scale(scale, scale);
                draw_path(size / 2.0f / scale, canvas, paint);
                canvas->restore();

                canvas->save();
                canvas->translate(size / 2.0f, 80.0f + size / 2.0f);
                canvas->scale(scale, scale);
                canvas->drawCircle(0.0f, 0.0f, size / 2.0f / scale, paint);
                canvas->restore();

                canvas->save();
                canvas->translate(0.0f, 160.0f);
                canvas->scale(scale, scale);
                canvas->drawRect(SkRect::MakeXYWH(0.0f, 0.0f, size / scale, size / scale), paint);
                canvas->restore();

                canvas->save();
                canvas->translate(0.0f, 240.0f);
                canvas->scale(scale, scale);
                canvas->drawLine(0.0f, 0.0f, size / scale, size / scale, paint);
                canvas->restore();

                canvas->translate(80.0f, 0.0f);
            }
        }

    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new ScaledStrokesGM; )

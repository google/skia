/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"

class ScaledStrokesGM : public skiagm::GM {
public:
    ScaledStrokesGM() {}

protected:
    SkString getName() const override { return SkString("scaledstrokes"); }

    SkISize getISize() override { return SkISize::Make(640, 320); }

    static void draw_path(SkScalar size, SkCanvas* canvas, SkPaint paint) {
        SkScalar c = 0.551915024494f * size;
        SkPathBuilder path;
        path.moveTo(0.0f, size);
        path.cubicTo(c, size, size, c, size, 0.0f);
        path.cubicTo(size, -c, c, -size, 0.0f, -size);
        path.cubicTo(-c, -size, -size, -c, -size, 0.0f);
        path.cubicTo(-size, c, -c, size, 0.0f, size);
        canvas->drawPath(path.detach(), paint);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        SkPath path;
        paint.setStyle(SkPaint::Style::kStroke_Style);
        canvas->translate(5.0f, 5.0f);
        const SkScalar size = 60.0f;
        for (int i = 0; i < 2; i++) {
            paint.setAntiAlias(i == 1);
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
    using INHERITED = GM;
};

DEF_GM( return new ScaledStrokesGM; )

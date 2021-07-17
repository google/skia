/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"

#define RESIZE_FACTOR_X SkIntToScalar(2)
#define RESIZE_FACTOR_Y SkIntToScalar(5)

namespace skiagm {

class ImageFiltersStrokedGM : public GM {
public:
    ImageFiltersStrokedGM() {
        this->setBGColor(0x00000000);
    }

protected:

    SkString onShortName() override {
        return SkString("imagefiltersstroked");
    }

    SkISize onISize() override {
        return SkISize::Make(860, 500);
    }

    static void draw_circle(SkCanvas* canvas, const SkRect& r, const SkPaint& paint) {
        canvas->drawCircle(r.centerX(), r.centerY(),
                           r.width() * 2 / 5, paint);
    }

    static void draw_line(SkCanvas* canvas, const SkRect& r, const SkPaint& paint) {
        canvas->drawLine(r.fLeft, r.fBottom, r.fRight, r.fTop, paint);
    }

    static void draw_rect(SkCanvas* canvas, const SkRect& r, const SkPaint& paint) {
        canvas->drawRect(r, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        void (*drawProc[])(SkCanvas*, const SkRect&, const SkPaint&) = {
            draw_line, draw_rect, draw_circle,
        };

        canvas->clear(SK_ColorBLACK);

        SkMatrix resizeMatrix;
        resizeMatrix.setScale(RESIZE_FACTOR_X, RESIZE_FACTOR_Y);

        sk_sp<SkImageFilter> filters[] = {
            SkImageFilters::Blur(5, 5, nullptr),
            SkImageFilters::DropShadow(10, 10, 3, 3, SK_ColorGREEN, nullptr),
            SkImageFilters::Offset(-16, 32, nullptr),
            SkImageFilters::MatrixTransform(resizeMatrix, SkSamplingOptions(), nullptr),
        };

        SkRect r = SkRect::MakeWH(64, 64);
        SkScalar margin = 32;
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(10);
        paint.setStyle(SkPaint::kStroke_Style);

        for (size_t i = 0; i < SK_ARRAY_COUNT(drawProc); ++i) {
            canvas->translate(0, margin);
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(filters); ++j) {
                canvas->translate(margin, 0);
                canvas->save();
                if (2 == j) {
                    canvas->translate(16, -32);
                } else if (3 == j) {
                    canvas->scale(SkScalarInvert(RESIZE_FACTOR_X),
                                  SkScalarInvert(RESIZE_FACTOR_Y));
                }
                paint.setImageFilter(filters[j]);
                drawProc[i](canvas, r, paint);
                canvas->restore();
                canvas->translate(r.width() + margin, 0);
            }
            canvas->restore();
            canvas->translate(0, r.height());
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageFiltersStrokedGM;)

}  // namespace skiagm

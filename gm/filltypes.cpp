/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"

namespace skiagm {

class FillTypeGM : public GM {
    SkPath fPath;
public:
    FillTypeGM() {
        this->setBGColor(0xFFDDDDDD);
    }

    void makePath() {
        if (fPath.isEmpty()) {
            const SkScalar radius = SkIntToScalar(45);
            fPath.addCircle(SkIntToScalar(50), SkIntToScalar(50), radius);
            fPath.addCircle(SkIntToScalar(100), SkIntToScalar(100), radius);
        }
    }

protected:

    SkString onShortName() override {
        return SkString("filltypes");
    }

    SkISize onISize() override {
        return SkISize::Make(835, 840);
    }

    void showPath(SkCanvas* canvas, int x, int y, SkPathFillType ft,
                  SkScalar scale, const SkPaint& paint) {
        const SkRect r = { 0, 0, SkIntToScalar(150), SkIntToScalar(150) };

        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(r);
        canvas->drawColor(SK_ColorWHITE);
        fPath.setFillType(ft);
        canvas->translate(r.centerX(), r.centerY());
        canvas->scale(scale, scale);
        canvas->translate(-r.centerX(), -r.centerY());
        canvas->drawPath(fPath, paint);
        canvas->restore();
    }

    void showFour(SkCanvas* canvas, SkScalar scale, const SkPaint& paint) {
        showPath(canvas,   0,   0, SkPathFillType::kWinding,
                 scale, paint);
        showPath(canvas, 200,   0, SkPathFillType::kEvenOdd,
                 scale, paint);
        showPath(canvas,  00, 200, SkPathFillType::kInverseWinding,
                 scale, paint);
        showPath(canvas, 200, 200, SkPathFillType::kInverseEvenOdd,
                 scale, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        this->makePath();

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        SkPaint paint;
        const SkScalar scale = SkIntToScalar(5)/4;

        paint.setAntiAlias(false);

        showFour(canvas, SK_Scalar1, paint);
        canvas->translate(SkIntToScalar(450), 0);
        showFour(canvas, scale, paint);

        paint.setAntiAlias(true);

        canvas->translate(SkIntToScalar(-450), SkIntToScalar(450));
        showFour(canvas, SK_Scalar1, paint);
        canvas->translate(SkIntToScalar(450), 0);
        showFour(canvas, scale, paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FillTypeGM; )

}

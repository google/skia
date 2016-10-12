/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorFilter.h"
#include "SkPath.h"
#include "SkSurface.h"

class ColorFilterGM : public skiagm::GM {
public:
    ColorFilterGM() {}

protected:
    SkString onShortName() override {
        return SkString("srcincolorfilter");
    }

    SkISize onISize() override {
        return SkISize::Make(30, 30);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPath path;
        path.lineTo(25.0f, 3.0f);
        path.lineTo(25.0f, 25.0f);
        path.lineTo(3.0f, 25.0f);

        SkPaint paint;
        paint.setColor(0xFFFFFFFF);
        paint.setColorFilter(SkColorFilter::MakeModeFilter(0xFF8080FF, SkXfermode::kSrcIn_Mode));
        paint.setStrokeWidth(3.0f);

        canvas->drawPath(path, paint);
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ColorFilterGM; )

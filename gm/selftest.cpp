/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * Pathologically simple drawing tests, designed to generate consistent
 * output images across platforms for gm/tests/run.sh
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPaint.h"

class SelfTestGM : public skiagm::GM {
public:
    SelfTestGM(const char name[], SkColor color) :
        fName(name), fColor(color) {}
    const static int kWidth = 300;
    const static int kHeight = 200;

protected:
    SkString onShortName() {
        return fName;
    }

    SkISize onISize() { return SkISize::Make(kWidth, kHeight); }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(fColor);
        canvas->drawRectCoords(0, 0, SkIntToScalar(kWidth), SkIntToScalar(kHeight), paint);
    }

private:
    const SkString fName;
    const SkColor fColor;
};

//////////////////////////////////////////////////////////////////////////////

// We use translucent colors to make sure we are properly handling cases like
// those which caused https://code.google.com/p/skia/issues/detail?id=1079
// ('gm generating spurious pixel_error messages as of r7258')
static SkColor kTranslucentGreen = 0x7700EE00;
static SkColor kTranslucentBlue  = 0x770000DD;

DEF_GM( return new SelfTestGM("selftest1", kTranslucentGreen); )
DEF_GM( return new SelfTestGM("selftest2", kTranslucentBlue); )

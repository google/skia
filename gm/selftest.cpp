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
    SelfTestGM(const char name[], SkColor color) : fName(name), fColor(color) {}
    const static int kWidth = 300;
    const static int kHeight = 200;

protected:
    SkString onShortName() {
        return fName;
    }

    SkISize onISize() { return skiagm::make_isize(kWidth, kHeight); }

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

static skiagm::GM* F1(void*) { return new SelfTestGM("selftest1", SK_ColorGREEN); }
static skiagm::GM* F2(void*) { return new SelfTestGM("selftest2", SK_ColorBLUE); }

static skiagm::GMRegistry gR1(F1);
static skiagm::GMRegistry gR2(F2);

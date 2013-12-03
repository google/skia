/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkBicubicImageFilter.h"

// This bench exercises SkBicubicImageFilter, upsampling a 40x40 input to
// 100x100, 400x100, 100x400, and 400x400.

class BicubicBench : public SkBenchmark {
    SkSize         fScale;
    SkString       fName;

public:
    BicubicBench(float x, float y)
        : fScale(SkSize::Make(x, y)) {
        fName.printf("bicubic_%gx%g",
                     SkScalarToFloat(fScale.fWidth), SkScalarToFloat(fScale.fHeight));
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setAntiAlias(true);

        SkRect r = SkRect::MakeWH(40, 40);
        SkAutoTUnref<SkImageFilter> bicubic(SkBicubicImageFilter::CreateMitchell(fScale));
        paint.setImageFilter(bicubic);

        for (int i = 0; i < loops; i++) {
            canvas->save();
            canvas->clipRect(r);
            canvas->drawOval(r, paint);
            canvas->restore();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH( return new BicubicBench(10.0f, 10.0f); )
DEF_BENCH( return new BicubicBench(2.5f, 10.0f); )
DEF_BENCH( return new BicubicBench(10.0f, 2.5f); )
DEF_BENCH( return new BicubicBench(2.5f, 2.5f); )

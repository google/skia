/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"

class BitmapFractionalBench: public Benchmark {
    int                  fInputSize;
    int                  fOutputSize;
    SkPaint::FilterLevel fFilterLevel;
    SkString             fName;

public:
    BitmapFractionalBench( int is, const char *name, SkPaint::FilterLevel filterLevel )  {
        fInputSize = is;
        fOutputSize = 2*is;
        fFilterLevel = filterLevel;
        fName.printf( "bitmap_fractional_bench_%s", name );
    }

protected:

    SkBitmap fInputBitmap, fOutputBitmap;
    SkMatrix fMatrix;

    virtual const char* onGetName() {
        return fName.c_str();
    }

    int inputSize() const {
        return fInputSize;
    }

    int outputSize() const {
        return fOutputSize;
    }

    SkIPoint onGetSize() SK_OVERRIDE {
        return SkIPoint::Make( fOutputSize, fOutputSize );
    }

    virtual void onPreDraw() {
        fInputBitmap.allocN32Pixels(fInputSize, fInputSize, true);
        fInputBitmap.eraseColor(SK_ColorWHITE);

        fOutputBitmap.allocN32Pixels(fOutputSize, fOutputSize, true);
    }

    virtual void onDraw(const int loops, SkCanvas*) {
        SkPaint paint;
        this->setupPaint(&paint);

        preBenchSetup();

        SkCanvas canvas( fOutputBitmap );
        paint.setFilterLevel(fFilterLevel);
        fInputBitmap.notifyPixelsChanged();

        for (int i = 0; i < loops; i++) {
            // up-scale the image by a variety of close, fractional scales
            for (int j = 0 ; j < 20 ; j++) {
                fMatrix = SkMatrix::I();
                fMatrix.setScale( 1 + j/500.f, 1 + j/500.f );
                canvas.drawBitmapMatrix( fInputBitmap, fMatrix, &paint );
            }
            // down-scale the image by a variety of close, fractional scales
            for (int j = 0 ; j < 20 ; j++) {
                fMatrix = SkMatrix::I();
                fMatrix.setScale( 1 - j/500.f, 1 - j/500.f );
                canvas.drawBitmapMatrix( fInputBitmap, fMatrix, &paint );
            }
            // Now try some fractional translates
            for (int j = 0 ; j < 20 ; j++) {
                fMatrix = SkMatrix::I();
                fMatrix.setTranslate( j/3.f, j/3.f );
                canvas.drawBitmapMatrix( fInputBitmap, fMatrix, &paint );
            }
            // Finally, some fractional translates with non-identity scale.
            for (int j = 0 ; j < 20 ; j++) {
                fMatrix = SkMatrix::I();
                fMatrix.setTranslate( j/3.f, j/3.f );
                fMatrix.preScale( 1.5f, 1.5f );
                canvas.drawBitmapMatrix( fInputBitmap, fMatrix, &paint );
            }
        }
    }

    virtual void preBenchSetup() {}
private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new BitmapFractionalBench(256, "high", SkPaint::kHigh_FilterLevel);)
DEF_BENCH(return new BitmapFractionalBench(256, "medium", SkPaint::kMedium_FilterLevel);)
DEF_BENCH(return new BitmapFractionalBench(256, "low", SkPaint::kLow_FilterLevel);)

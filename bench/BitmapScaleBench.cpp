/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkBlurMask.h"

class BitmapScaleBench: public SkBenchmark {
    int         fLoopCount;
    int         fInputSize;
    int         fOutputSize;
    SkString    fName;

public:
    BitmapScaleBench(void *param, int is, int os) : INHERITED(param) {
        fInputSize = is;
        fOutputSize = os;

        fLoopCount = 20;
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

    float scale() const {
        return float(outputSize())/inputSize();
    }

    SkIPoint onGetSize() SK_OVERRIDE {
        return SkIPoint::Make( fOutputSize, fOutputSize );
    }

    void setName(const char * name) {
        fName.printf( "bitmap_scale_%s_%d_%d", name, fInputSize, fOutputSize );
    }

    virtual void onPreDraw() {
        fInputBitmap.setConfig(SkBitmap::kARGB_8888_Config, fInputSize, fInputSize);
        fInputBitmap.allocPixels();
        fInputBitmap.eraseColor(SK_ColorWHITE);
        fInputBitmap.setIsOpaque(true);

        fOutputBitmap.setConfig(SkBitmap::kARGB_8888_Config, fOutputSize, fOutputSize);
        fOutputBitmap.allocPixels();
        fOutputBitmap.setIsOpaque(true);

        fMatrix.setScale( scale(), scale() );
    }

    virtual void onDraw(SkCanvas*) {
        SkPaint paint;
        this->setupPaint(&paint);

        preBenchSetup();

        for (int i = 0; i < SkBENCHLOOP(fLoopCount); i++) {
            doScaleImage();
        }
    }

    virtual void doScaleImage() = 0;
    virtual void preBenchSetup() {}
private:
    typedef SkBenchmark INHERITED;
};

class BitmapFilterScaleBench: public BitmapScaleBench {
 public:
    BitmapFilterScaleBench(void *param, int is, int os) : INHERITED(param, is, os) {
        setName( "filter" );
    }
protected:
    virtual void doScaleImage() SK_OVERRIDE {
        SkCanvas canvas( fOutputBitmap );
        SkPaint paint;

        paint.setFilterLevel(SkPaint::kHigh_FilterLevel);
        canvas.drawBitmapMatrix( fInputBitmap, fMatrix, &paint );
    }
private:
    typedef BitmapScaleBench INHERITED;
};

DEF_BENCH(return new BitmapFilterScaleBench(p, 10, 90);)
DEF_BENCH(return new BitmapFilterScaleBench(p, 30, 90);)
DEF_BENCH(return new BitmapFilterScaleBench(p, 80, 90);)
DEF_BENCH(return new BitmapFilterScaleBench(p, 90, 90);)
DEF_BENCH(return new BitmapFilterScaleBench(p, 90, 80);)
DEF_BENCH(return new BitmapFilterScaleBench(p, 90, 30);)
DEF_BENCH(return new BitmapFilterScaleBench(p, 90, 10);)
DEF_BENCH(return new BitmapFilterScaleBench(p, 256, 64);)
DEF_BENCH(return new BitmapFilterScaleBench(p, 64, 256);)

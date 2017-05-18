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

class BitmapScaleBench: public Benchmark {
    int         fLoopCount;
    int         fInputSize;
    int         fOutputSize;
    SkString    fName;

public:
    BitmapScaleBench( int is, int os)  {
        fInputSize = is;
        fOutputSize = os;

        fLoopCount = 20;
    }

protected:

    SkBitmap fInputBitmap, fOutputBitmap;
    SkMatrix fMatrix;

    const char* onGetName() override {
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

    SkIPoint onGetSize() override {
        return SkIPoint::Make( fOutputSize, fOutputSize );
    }

    void setName(const char * name) {
        fName.printf( "bitmap_scale_%s_%d_%d", name, fInputSize, fOutputSize );
    }

    void onDelayedSetup() override {
        fInputBitmap.allocN32Pixels(fInputSize, fInputSize, true);
        fInputBitmap.eraseColor(SK_ColorWHITE);

        fOutputBitmap.allocN32Pixels(fOutputSize, fOutputSize, true);

        fMatrix.setScale( scale(), scale() );
    }

    void onDraw(int loops, SkCanvas*) override {
        SkPaint paint;
        this->setupPaint(&paint);

        preBenchSetup();

        for (int i = 0; i < loops; i++) {
            doScaleImage();
        }
    }

    virtual void doScaleImage() = 0;
    virtual void preBenchSetup() {}
private:
    typedef Benchmark INHERITED;
};

class BitmapFilterScaleBench: public BitmapScaleBench {
 public:
    BitmapFilterScaleBench( int is, int os) : INHERITED(is, os) {
        setName( "filter" );
    }
protected:
    void doScaleImage() override {
        SkCanvas canvas( fOutputBitmap );
        SkPaint paint;

        paint.setFilterQuality(kHigh_SkFilterQuality);
        fInputBitmap.notifyPixelsChanged();
        canvas.concat(fMatrix);
        canvas.drawBitmap(fInputBitmap, 0, 0, &paint );
    }
private:
    typedef BitmapScaleBench INHERITED;
};

DEF_BENCH(return new BitmapFilterScaleBench(10, 90);)
DEF_BENCH(return new BitmapFilterScaleBench(30, 90);)
DEF_BENCH(return new BitmapFilterScaleBench(80, 90);)
DEF_BENCH(return new BitmapFilterScaleBench(90, 90);)
DEF_BENCH(return new BitmapFilterScaleBench(90, 80);)
DEF_BENCH(return new BitmapFilterScaleBench(90, 30);)
DEF_BENCH(return new BitmapFilterScaleBench(90, 10);)
DEF_BENCH(return new BitmapFilterScaleBench(256, 64);)
DEF_BENCH(return new BitmapFilterScaleBench(64, 256);)

///////////////////////////////////////////////////////////////////////////////////////////////

#include "SkBitmapScaler.h"

class PixmapScalerBench: public Benchmark {
    SkBitmapScaler::ResizeMethod    fMethod;
    SkString                        fName;
    SkBitmap                        fSrc, fDst;

public:
    PixmapScalerBench(SkBitmapScaler::ResizeMethod method, const char suffix[]) : fMethod(method) {
        fName.printf("pixmapscaler_%s", suffix);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    SkIPoint onGetSize() override { return{ 100, 100 }; }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDelayedSetup() override {
        fSrc.allocN32Pixels(640, 480);
        fSrc.eraseColor(SK_ColorWHITE);
        fDst.allocN32Pixels(300, 250);
    }

    void onDraw(int loops, SkCanvas*) override {
        SkPixmap src, dst;
        fSrc.peekPixels(&src);
        fDst.peekPixels(&dst);
        for (int i = 0; i < loops * 16; i++) {
            SkBitmapScaler::Resize(dst, src, fMethod);
        }
    }

private:
    typedef Benchmark INHERITED;
};
DEF_BENCH( return new PixmapScalerBench(SkBitmapScaler::RESIZE_LANCZOS3, "lanczos");  )
DEF_BENCH( return new PixmapScalerBench(SkBitmapScaler::RESIZE_MITCHELL, "mitchell"); )
DEF_BENCH( return new PixmapScalerBench(SkBitmapScaler::RESIZE_HAMMING,  "hamming");  )
DEF_BENCH( return new PixmapScalerBench(SkBitmapScaler::RESIZE_TRIANGLE, "triangle"); )
DEF_BENCH( return new PixmapScalerBench(SkBitmapScaler::RESIZE_BOX,      "box");      )

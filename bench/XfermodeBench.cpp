
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
#include "SkString.h"
#include "SkXfermode.h"

// Benchmark that draws non-AA rects with an SkXfermode::Mode
class XfermodeBench : public SkBenchmark {
public:
    XfermodeBench(void* param, SkXfermode::Mode mode) : SkBenchmark(param) {
        fXfermode.reset(SkXfermode::Create(mode));
        SkASSERT(NULL != fXfermode.get() || SkXfermode::kSrcOver_Mode == mode);
        fName.printf("Xfermode_%s", SkXfermode::ModeName(mode));
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE { return fName.c_str(); }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkISize size = canvas->getDeviceSize();
        SkMWCRandom random;
        for (int i = 0; i < kNumRects; ++i) {
            SkPaint paint;
            paint.setXfermode(fXfermode.get());
            paint.setColor(random.nextU());
            SkScalar w = random.nextRangeScalar(SkIntToScalar(kMinSize), SkIntToScalar(kMaxSize));
            SkScalar h = random.nextRangeScalar(SkIntToScalar(kMinSize), SkIntToScalar(kMaxSize));
            SkRect rect = SkRect::MakeXYWH(
                random.nextUScalar1() * (size.fWidth - w),
                random.nextUScalar1() * (size.fHeight - h),
                w,
                h
            );
            canvas->drawRect(rect, paint);
        }
    }

private:
    enum {
        kNumRects = SkBENCHLOOP(1000),
        kMinSize = 10,
        kMaxSize = 400,
    };
    SkAutoTUnref<SkXfermode> fXfermode;
    SkString fName;
    
    typedef SkBenchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////


// These modes were chosen because they are expected to be successively harder for the GPU.
// kSrc can disable blending, kSrcOver cannot, kDarken requires reading the dst pixel in the shader.
static SkBenchmark* Fact0(void* p) { return new XfermodeBench(p, SkXfermode::kSrc_Mode); }
static SkBenchmark* Fact1(void* p) { return new XfermodeBench(p, SkXfermode::kSrcOver_Mode); }
static SkBenchmark* Fact2(void* p) { return new XfermodeBench(p, SkXfermode::kDarken_Mode); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);


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
    XfermodeBench(SkXfermode::Mode mode) {
        fXfermode.reset(SkXfermode::Create(mode));
        SkASSERT(NULL != fXfermode.get() || SkXfermode::kSrcOver_Mode == mode);
        fName.printf("Xfermode_%s", SkXfermode::ModeName(mode));
    }

    XfermodeBench(SkXfermode* xferMode, const char* name) {
        SkASSERT(NULL != xferMode);
        fXfermode.reset(xferMode);
        fName.printf("Xfermode_%s", name);
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE { return fName.c_str(); }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkISize size = canvas->getDeviceSize();
        SkRandom random;
        for (int i = 0; i < loops; ++i) {
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
        kMinSize = 50,
        kMaxSize = 100,
    };
    SkAutoTUnref<SkXfermode> fXfermode;
    SkString fName;

    typedef SkBenchmark INHERITED;
};

class XferCreateBench : public SkBenchmark {
public:
    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE { return "xfermode_create"; }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int outer = 0; outer < loops * 10; ++outer) {
            for (int i = 0; i <= SkXfermode::kLastMode; ++i) {
                SkXfermode* xfer = SkXfermode::Create(SkXfermode::Mode(i));
                SkSafeUnref(xfer);
            }
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

#define CONCAT_I(x, y) x ## y
#define CONCAT(x, y) CONCAT_I(x, y) // allow for macro expansion
#define BENCH(...) \
    DEF_BENCH( return new XfermodeBench(__VA_ARGS__); );\


BENCH(SkXfermode::kClear_Mode)
BENCH(SkXfermode::kSrc_Mode)
BENCH(SkXfermode::kDst_Mode)
BENCH(SkXfermode::kSrcOver_Mode)
BENCH(SkXfermode::kDstOver_Mode)
BENCH(SkXfermode::kSrcIn_Mode)
BENCH(SkXfermode::kDstIn_Mode)
BENCH(SkXfermode::kSrcOut_Mode)
BENCH(SkXfermode::kDstOut_Mode)
BENCH(SkXfermode::kSrcATop_Mode)
BENCH(SkXfermode::kDstATop_Mode)
BENCH(SkXfermode::kXor_Mode)

BENCH(SkXfermode::kPlus_Mode)
BENCH(SkXfermode::kModulate_Mode)
BENCH(SkXfermode::kScreen_Mode)

BENCH(SkXfermode::kOverlay_Mode)
BENCH(SkXfermode::kDarken_Mode)
BENCH(SkXfermode::kLighten_Mode)
BENCH(SkXfermode::kColorDodge_Mode)
BENCH(SkXfermode::kColorBurn_Mode)
BENCH(SkXfermode::kHardLight_Mode)
BENCH(SkXfermode::kSoftLight_Mode)
BENCH(SkXfermode::kDifference_Mode)
BENCH(SkXfermode::kExclusion_Mode)
BENCH(SkXfermode::kMultiply_Mode)

BENCH(SkXfermode::kHue_Mode)
BENCH(SkXfermode::kSaturation_Mode)
BENCH(SkXfermode::kColor_Mode)
BENCH(SkXfermode::kLuminosity_Mode)

DEF_BENCH(return new XferCreateBench;)

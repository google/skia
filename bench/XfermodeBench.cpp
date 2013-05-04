
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
        kNumRects = SkBENCHLOOP(75),
        kMinSize = 50,
        kMaxSize = 100,
    };
    SkAutoTUnref<SkXfermode> fXfermode;
    SkString fName;

    typedef SkBenchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact00(void* p) { return new XfermodeBench(p, SkXfermode::kClear_Mode); }
static SkBenchmark* Fact01(void* p) { return new XfermodeBench(p, SkXfermode::kSrc_Mode); }
static SkBenchmark* Fact02(void* p) { return new XfermodeBench(p, SkXfermode::kDst_Mode); }
static SkBenchmark* Fact03(void* p) { return new XfermodeBench(p, SkXfermode::kSrcOver_Mode); }
static SkBenchmark* Fact04(void* p) { return new XfermodeBench(p, SkXfermode::kDstOver_Mode); }
static SkBenchmark* Fact05(void* p) { return new XfermodeBench(p, SkXfermode::kSrcIn_Mode); }
static SkBenchmark* Fact06(void* p) { return new XfermodeBench(p, SkXfermode::kDstIn_Mode); }
static SkBenchmark* Fact07(void* p) { return new XfermodeBench(p, SkXfermode::kSrcOut_Mode); }
static SkBenchmark* Fact08(void* p) { return new XfermodeBench(p, SkXfermode::kDstOut_Mode); }
static SkBenchmark* Fact09(void* p) { return new XfermodeBench(p, SkXfermode::kSrcATop_Mode); }
static SkBenchmark* Fact10(void* p) { return new XfermodeBench(p, SkXfermode::kDstATop_Mode); }
static SkBenchmark* Fact11(void* p) { return new XfermodeBench(p, SkXfermode::kXor_Mode); }
static SkBenchmark* Fact12(void* p) { return new XfermodeBench(p, SkXfermode::kPlus_Mode); }
static SkBenchmark* Fact13(void* p) { return new XfermodeBench(p, SkXfermode::kModulate_Mode); }
static SkBenchmark* Fact14(void* p) { return new XfermodeBench(p, SkXfermode::kScreen_Mode); }
static SkBenchmark* Fact15(void* p) { return new XfermodeBench(p, SkXfermode::kOverlay_Mode); }
static SkBenchmark* Fact16(void* p) { return new XfermodeBench(p, SkXfermode::kDarken_Mode); }
static SkBenchmark* Fact17(void* p) { return new XfermodeBench(p, SkXfermode::kLighten_Mode); }
static SkBenchmark* Fact18(void* p) { return new XfermodeBench(p, SkXfermode::kColorDodge_Mode); }
static SkBenchmark* Fact19(void* p) { return new XfermodeBench(p, SkXfermode::kColorBurn_Mode); }
static SkBenchmark* Fact20(void* p) { return new XfermodeBench(p, SkXfermode::kHardLight_Mode); }
static SkBenchmark* Fact21(void* p) { return new XfermodeBench(p, SkXfermode::kSoftLight_Mode); }
static SkBenchmark* Fact22(void* p) { return new XfermodeBench(p, SkXfermode::kDifference_Mode); }
static SkBenchmark* Fact23(void* p) { return new XfermodeBench(p, SkXfermode::kExclusion_Mode); }
static SkBenchmark* Fact24(void* p) { return new XfermodeBench(p, SkXfermode::kMultiply_Mode); }
static SkBenchmark* Fact25(void* p) { return new XfermodeBench(p, SkXfermode::kHue_Mode); }
static SkBenchmark* Fact26(void* p) { return new XfermodeBench(p, SkXfermode::kSaturation_Mode); }
static SkBenchmark* Fact27(void* p) { return new XfermodeBench(p, SkXfermode::kColor_Mode); }
static SkBenchmark* Fact28(void* p) { return new XfermodeBench(p, SkXfermode::kLuminosity_Mode); }

static BenchRegistry gReg00(Fact00);
static BenchRegistry gReg01(Fact01);
static BenchRegistry gReg02(Fact02);
static BenchRegistry gReg03(Fact03);
static BenchRegistry gReg04(Fact04);
static BenchRegistry gReg05(Fact05);
static BenchRegistry gReg06(Fact06);
static BenchRegistry gReg07(Fact07);
static BenchRegistry gReg08(Fact08);
static BenchRegistry gReg09(Fact09);
static BenchRegistry gReg10(Fact10);
static BenchRegistry gReg11(Fact11);
static BenchRegistry gReg12(Fact12);
static BenchRegistry gReg13(Fact13);
static BenchRegistry gReg14(Fact14);
static BenchRegistry gReg15(Fact15);
static BenchRegistry gReg16(Fact16);
static BenchRegistry gReg17(Fact17);
static BenchRegistry gReg18(Fact18);
static BenchRegistry gReg19(Fact19);
static BenchRegistry gReg20(Fact20);
static BenchRegistry gReg21(Fact21);
static BenchRegistry gReg22(Fact22);
static BenchRegistry gReg23(Fact23);
static BenchRegistry gReg24(Fact24);
static BenchRegistry gReg25(Fact25);
static BenchRegistry gReg26(Fact26);
static BenchRegistry gReg27(Fact27);
static BenchRegistry gReg28(Fact28);

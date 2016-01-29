
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkString.h"
#include "SkXfer4f.h"

#define INNER_LOOPS 100

// Benchmark that draws non-AA rects or AA text with an SkXfermode::Mode.
class Xfer4fBench : public Benchmark {
public:
    Xfer4fBench(SkXfermode::Mode mode, const char name[], bool doN, uint32_t flags) : fDoN(doN) {
        fProc1 = SkPM4fXfer1ProcFactory(mode, flags);
        fProcN = SkPM4fXferNProcFactory(mode, flags);
        fName.printf("xfer4f_%s_%c_%s_%s", name, fDoN ? 'N' : '1',
                     (flags & kSrcIsOpaque_SkXfer4fFlag) ? "opaque" : "alpha",
                     (flags & kDstIsSRGB_SkXfer4fFlag) ? "srgb" : "linear");

        SkPM4f c;
        c.fVec[0] = 1; c.fVec[1] = 1; c.fVec[2] = 1; c.fVec[3] = 1;
        for (int i = 0; i < N; ++i) {
            fSrc[i] = c;
            fDst[i] = 0;
        }
    }

protected:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < INNER_LOOPS; ++j) {
                if (fDoN) {
                    fProcN(fDst, fSrc, N);
                } else {
                    fProc1(fDst, fSrc[0], N);
                }
            }
        }
    }

private:
    SkString        fName;
    SkPM4fXfer1Proc fProc1;
    SkPM4fXferNProc fProcN;
    bool            fDoN;

    enum {
        N = 1000,
    };
    SkPM4f      fSrc[N];
    SkPMColor   fDst[N];

    typedef Benchmark INHERITED;
};

DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", false, kDstIsSRGB_SkXfer4fFlag); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", false, 0); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", false, kDstIsSRGB_SkXfer4fFlag | kSrcIsOpaque_SkXfer4fFlag); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", false, kSrcIsOpaque_SkXfer4fFlag); )

DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", true,  kDstIsSRGB_SkXfer4fFlag); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", true,  0); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", true,  kDstIsSRGB_SkXfer4fFlag | kSrcIsOpaque_SkXfer4fFlag); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", true,  kSrcIsOpaque_SkXfer4fFlag); )

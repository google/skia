/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkPM4f.h"
#include "SkString.h"
#include "SkXfermode.h"

#define USE_AA      (1 << 31)   // merge with Xfermode::PMFlags w/o conflict

#define INNER_LOOPS 1000

// Benchmark that draws non-AA rects or AA text with an SkXfermode::Mode.
class XferF16Bench : public Benchmark {
public:
    XferF16Bench(SkXfermode::Mode mode, const char name[], bool doN, uint32_t flags)
        : fDoN(doN)
        , fFlags(flags & ~USE_AA)
    {
        fXfer = SkXfermode::Make(mode);

        fProc1 = SkXfermode::GetF16Proc(fXfer, fFlags | SkXfermode::kSrcIsSingle_F16Flag);
        fProcN = SkXfermode::GetF16Proc(fXfer, fFlags);
        fName.printf("xferF16_%s_%s_%c_%s",
                     name,
                     (flags & USE_AA) ? "aa" : "bw",
                     fDoN ? 'N' : '1',
                     (flags & SkXfermode::kSrcIsOpaque_F16Flag) ? "opaque" : "alpha");

        for (int i = 0; i < N; ++i) {
            fSrc[i] = {{ 1, 1, 1, 1 }};
            fDst[i] = 0;
            fAAStorage[i] = i * 255 / (N - 1);
        }

        if (flags & USE_AA) {
            fAA = fAAStorage;
        } else {
            fAA = nullptr;
        }
    }

protected:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops * INNER_LOOPS; ++i) {
            if (fDoN) {
                fProcN(fXfer.get(), fDst, fSrc, N, fAA);
            } else {
                fProc1(fXfer.get(), fDst, fSrc, N, fAA);
            }
        }
    }

private:
    sk_sp<SkXfermode>   fXfer;
    SkString            fName;
    SkXfermode::F16Proc fProc1;
    SkXfermode::F16Proc fProcN;
    const SkAlpha*      fAA;
    bool                fDoN;
    uint32_t            fFlags;

    enum {
        N = 1000,
    };
    SkPM4f      fSrc[N];
    uint64_t    fDst[N];
    uint8_t     fAAStorage[N];

    typedef Benchmark INHERITED;
};

#define F00 0
#define F01 (SkXfermode::kSrcIsOpaque_F16Flag)

#define MODE    SkXfermode::kSrcOver_Mode
#define NAME    "srcover"

DEF_BENCH( return new XferF16Bench(MODE, NAME, true,  F00 | USE_AA); )
DEF_BENCH( return new XferF16Bench(MODE, NAME, true,  F01 | USE_AA); )
DEF_BENCH( return new XferF16Bench(MODE, NAME, true,  F00); )
DEF_BENCH( return new XferF16Bench(MODE, NAME, true,  F01); )

DEF_BENCH( return new XferF16Bench(MODE, NAME, false, F00 | USE_AA); )
DEF_BENCH( return new XferF16Bench(MODE, NAME, false, F01 | USE_AA); )
DEF_BENCH( return new XferF16Bench(MODE, NAME, false, F00); )
DEF_BENCH( return new XferF16Bench(MODE, NAME, false, F01); )

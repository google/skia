
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkString.h"
#include "SkXfermode.h"

#define INNER_LOOPS 1000

// Benchmark that draws non-AA rects or AA text with an SkXfermode::Mode.
class Xfer4fBench : public Benchmark {
public:
    Xfer4fBench(SkXfermode::Mode mode, const char name[], bool doN, uint32_t flags)
        : fDoN(doN)
        , fFlags(flags)
    {
        fProc1 = SkXfermode::GetPM4fProc1(mode, flags);
        fProcN = SkXfermode::GetPM4fProcN(mode, flags);
        fName.printf("xfer4f_%s_%c_%s_%s", name, fDoN ? 'N' : '1',
                     (flags & SkXfermode::kSrcIsOpaque_PM4fFlag) ? "opaque" : "alpha",
                     (flags & SkXfermode::kDstIsSRGB_PM4fFlag) ? "srgb" : "linear");

        for (int i = 0; i < N; ++i) {
            fSrc[i] = {{ 1, 1, 1, 1 }};
            fDst[i] = 0;
        }
    }

protected:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        const SkXfermode::PM4fState state{ nullptr, fFlags };
        const uint8_t* aa = nullptr;

        for (int i = 0; i < loops * INNER_LOOPS; ++i) {
            if (fDoN) {
                fProcN(state, fDst, fSrc, N, aa);
            } else {
                fProc1(state, fDst, fSrc[0], N, aa);
            }
        }
    }

private:
    SkString        fName;
    SkXfermode::PM4fProc1 fProc1;
    SkXfermode::PM4fProcN fProcN;
    bool            fDoN;
    uint32_t        fFlags;

    enum {
        N = 1000,
    };
    SkPM4f      fSrc[N];
    SkPMColor   fDst[N];

    typedef Benchmark INHERITED;
};

#define F00 0
#define F01 (SkXfermode::kSrcIsOpaque_PM4fFlag)
#define F10 (SkXfermode::kDstIsSRGB_PM4fFlag)
#define F11 (SkXfermode::kSrcIsOpaque_PM4fFlag | SkXfermode::kDstIsSRGB_PM4fFlag)

DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", false, F10); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", false, F00); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", false, F11); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", false, F01); )

DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", true,  F10); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", true,  F00); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", true,  F11); )
DEF_BENCH( return new Xfer4fBench(SkXfermode::kSrcOver_Mode, "srcover", true,  F01); )

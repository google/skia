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
class XferD32Bench : public Benchmark {
public:
    XferD32Bench(SkXfermode::Mode mode, const char name[], bool doN, uint32_t flags)
        : fDoN(doN)
        , fFlags(flags & ~USE_AA)
    {
        fXfer = SkXfermode::Make(mode);
        fProc1 = SkXfermode::GetD32Proc(fXfer, fFlags | SkXfermode::kSrcIsSingle_D32Flag);
        fProcN = SkXfermode::GetD32Proc(fXfer, fFlags);
        fName.printf("xfer4f_%s_%s_%c_%s_%s",
                     name,
                     (flags & USE_AA) ? "aa" : "bw",
                     fDoN ? 'N' : '1',
                     (flags & SkXfermode::kSrcIsOpaque_D32Flag) ? "opaque" : "alpha",
                     (flags & SkXfermode::kDstIsSRGB_D32Flag) ? "srgb" : "linear");

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
    SkString             fName;
    SkXfermode::D32Proc  fProc1;
    SkXfermode::D32Proc  fProcN;
    const SkAlpha*       fAA;
    bool                 fDoN;
    uint32_t             fFlags;

    enum {
        N = 1000,
    };
    SkPM4f      fSrc[N];
    SkPMColor   fDst[N];
    uint8_t     fAAStorage[N];

    typedef Benchmark INHERITED;
};

#define F00 0
#define F01 (SkXfermode::kSrcIsOpaque_D32Flag)
#define F10 (SkXfermode::kDstIsSRGB_D32Flag)
#define F11 (SkXfermode::kSrcIsOpaque_D32Flag | SkXfermode::kDstIsSRGB_D32Flag)

DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", false, F10); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", false, F00); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", false, F11); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", false, F01); )

DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", true,  F10); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", true,  F00); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", true,  F11); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", true,  F01); )

DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", false, F10 | USE_AA); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", false, F00 | USE_AA); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", false, F11 | USE_AA); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", false, F01 | USE_AA); )

DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", true,  F10 | USE_AA); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", true,  F00 | USE_AA); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", true,  F11 | USE_AA); )
DEF_BENCH( return new XferD32Bench(SkXfermode::kSrcOver_Mode, "srcover", true,  F01 | USE_AA); )

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkString.h"
#include "SkXfermode.h"

#define USE_AA      (1 << 31)   // merge with Xfermode::PMFlags w/o conflict

#define INNER_LOOPS 1000

// Benchmark that draws non-AA rects or AA text with an SkXfermode::Mode.
class XferU64Bench : public Benchmark {
public:
    XferU64Bench(bool doN, uint32_t flags)
        : fDoN(doN)
        , fFlags(flags & ~USE_AA)
    {
        SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode;

        fProc1 = SkXfermode::GetU64Proc1(mode, fFlags);
        fProcN = SkXfermode::GetU64ProcN(mode, fFlags);
        fName.printf("xferu64_%s_%c_%s_%s",
                     (flags & USE_AA) ? "aa" : "bw",
                     fDoN ? 'N' : '1',
                     (flags & SkXfermode::kSrcIsOpaque_U64Flag) ? "opaque" : "alpha",
                     (flags & SkXfermode::kDstIsFloat16_U64Flag) ? "f16" : "u16");

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
        const SkXfermode::U64State state{ nullptr, fFlags };

        for (int i = 0; i < loops * INNER_LOOPS; ++i) {
            if (fDoN) {
                fProcN(state, fDst, fSrc, N, fAA);
            } else {
                fProc1(state, fDst, fSrc[0], N, fAA);
            }
        }
    }

private:
    SkString             fName;
    SkXfermode::U64Proc1 fProc1;
    SkXfermode::U64ProcN fProcN;
    const SkAlpha*  fAA;
    bool            fDoN;
    uint32_t        fFlags;

    enum {
        N = 1000,
    };
    SkPM4f      fSrc[N];
    uint64_t    fDst[N];
    uint8_t     fAAStorage[N];

    typedef Benchmark INHERITED;
};

#define F00 0
#define F01 (SkXfermode::kSrcIsOpaque_U64Flag)
#define F10 (SkXfermode::kDstIsFloat16_U64Flag)
#define F11 (SkXfermode::kDstIsFloat16_U64Flag | SkXfermode::kSrcIsOpaque_U64Flag)

#if 0
DEF_BENCH( return new XferU64Bench(true,  F10 | USE_AA); )
DEF_BENCH( return new XferU64Bench(true,  F11 | USE_AA); )
DEF_BENCH( return new XferU64Bench(true,  F10); )
DEF_BENCH( return new XferU64Bench(true,  F11); )

DEF_BENCH( return new XferU64Bench(true,  F00 | USE_AA); )
DEF_BENCH( return new XferU64Bench(true,  F01 | USE_AA); )
DEF_BENCH( return new XferU64Bench(true,  F00); )
DEF_BENCH( return new XferU64Bench(true,  F01); )
#endif

DEF_BENCH( return new XferU64Bench(false, F10 | USE_AA); )
DEF_BENCH( return new XferU64Bench(false, F11 | USE_AA); )
DEF_BENCH( return new XferU64Bench(false, F10); )
DEF_BENCH( return new XferU64Bench(false, F11); )

DEF_BENCH( return new XferU64Bench(false, F00 | USE_AA); )
DEF_BENCH( return new XferU64Bench(false, F01 | USE_AA); )
DEF_BENCH( return new XferU64Bench(false, F00); )
DEF_BENCH( return new XferU64Bench(false, F01); )

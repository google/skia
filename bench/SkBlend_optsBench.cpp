/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <tuple>

#include "Benchmark.h"
#include "Resources.h"
#include "SkCpu.h"
#include "SkImage.h"
#include "SkImage_Base.h"
#include "SkNx.h"
#include "SkOpts.h"
#include "SkString.h"

#define INNER_LOOPS 10

namespace sk_default {
extern void brute_force_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);
}

class SrcOverVSkOptsBruteForce {
public:
    static SkString Name() { return SkString{"VSkOptsBruteForce"}; }
    static bool WorksOnCpu() { return true; }
    static void BlendN(uint32_t* dst, int count, const uint32_t* src) {
        sk_default::brute_force_srcover_srgb_srgb(dst, src, count, count);
    }
};

namespace sk_default {
extern void trivial_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);
}

class SrcOverVSkOptsTrivial {
public:
    static SkString Name() { return SkString{"VSkOptsTrivial"}; }
    static bool WorksOnCpu() { return true; }
    static void BlendN(uint32_t* dst, int count, const uint32_t* src) {
        sk_default::trivial_srcover_srgb_srgb(dst, src, count, count);
    }
};

namespace sk_default {
extern void best_non_simd_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);
}

class SrcOverVSkOptsNonSimdCore {
public:
    static SkString Name() { return SkString{"VSkOptsNonSimdCore"}; }
    static bool WorksOnCpu() { return true; }
    static void BlendN(uint32_t* dst, int count, const uint32_t* src) {
        sk_default::best_non_simd_srcover_srgb_srgb(dst, src, count, count);
    }
};

namespace sk_default {
extern void srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);
}

class SrcOverVSkOptsDefault {
public:
    static SkString Name() { return SkString{"VSkOptsDefault"}; }
    static bool WorksOnCpu() { return true; }
    static void BlendN(uint32_t* dst, int count, const uint32_t* src) {
        sk_default::srcover_srgb_srgb(dst, src, count, count);
    }
};

namespace sk_sse41 {
   extern void srcover_srgb_srgb(
       uint32_t* dst, const uint32_t* const srcStart, int ndst, const int nsrc);
}

class SrcOverVSkOptsSSE41 {
public:
    static SkString Name() { return SkString{"VSkOptsSSE41"}; }
    static bool WorksOnCpu() { return SkCpu::Supports(SkCpu::SSE41); }
    static void BlendN(uint32_t* dst, int count, const uint32_t* src) {
        sk_sse41::srcover_srgb_srgb(dst, src, count, count);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Blender>
class LinearSrcOverBench : public Benchmark {
public:
    LinearSrcOverBench(const char* fileName) : fFileName(fileName) {
        fName = "LinearSrcOver";
        fName.append(fileName);
        fName.append(Blender::Name());
    }

protected:
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend && Blender::WorksOnCpu();
    }
    const char* onGetName() override { return fName.c_str(); }

    void onPreDraw(SkCanvas*) override {
        if (!fPixmap.addr()) {
            sk_sp<SkImage> image = GetResourceAsImage(fFileName.c_str());
            SkBitmap bm;
            if (!as_IB(image)->getROPixels(&bm)) {
                SkFAIL("Could not read resource");
            }
            bm.peekPixels(&fPixmap);
            fCount = fPixmap.rowBytesAsPixels();
            fDst.reset(fCount);
            memset(fDst.get(), 0, fPixmap.rowBytes());
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        SkASSERT(fPixmap.colorType() == kN32_SkColorType);

        const int width = fPixmap.rowBytesAsPixels();

        for (int i = 0; i < loops * INNER_LOOPS; ++i) {
            const uint32_t* src = fPixmap.addr32();
            for (int y = 0; y < fPixmap.height(); y++) {
                Blender::BlendN(fDst.get(), width, src);
                src += width;
            }
        }
    }

    void onPostDraw(SkCanvas*) override {
        // Make sure the compiler does not optimize away the operation.
        volatile uint32_t v = 0;
        for (int i = 0; i < fCount; i++) {
            v ^= fDst[i];
        }
    }

private:
    int fCount;
    SkAutoTArray<uint32_t> fDst;
    SkString fFileName;
    SkString fName;
    SkPixmap fPixmap;

    typedef Benchmark INHERITED;
};

#if defined(SK_CPU_X86) && !defined(SK_BUILD_NO_OPTS)
#define BENCHES(fileName)                                                        \
DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsBruteForce>(fileName); )  \
DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsTrivial>(fileName); )     \
DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsNonSimdCore>(fileName); ) \
DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsDefault>(fileName); )     \
DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsSSE41>(fileName); )
#else
#define BENCHES(fileName)                                                        \
DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsBruteForce>(fileName); )  \
DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsTrivial>(fileName); )     \
DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsNonSimdCore>(fileName); ) \
DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsDefault>(fileName); )
#endif

BENCHES("yellow_rose.png")
BENCHES("baby_tux.png")
BENCHES("plane.png")
BENCHES("mandrill_512.png")
BENCHES("iconstrip.png")

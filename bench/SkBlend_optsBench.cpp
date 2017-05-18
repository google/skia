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
#include "SkPM4fPriv.h"
#include "SkString.h"

#define INNER_LOOPS 10

static inline void brute_srcover_srgb_srgb_1(uint32_t* dst, uint32_t src) {
    auto d = Sk4f_fromS32(*dst),
         s = Sk4f_fromS32( src);
    *dst = Sk4f_toS32(s + d * (1.0f - s[3]));
}

static inline void srcover_srgb_srgb_1(uint32_t* dst, uint32_t src) {
    if (src >= 0xFF000000) {
        *dst = src;
        return;
    }
    brute_srcover_srgb_srgb_1(dst, src);
}

static void brute_force_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
    while (ndst > 0) {
        int n = SkTMin(ndst, nsrc);

        for (int i = 0; i < n; i++) {
            brute_srcover_srgb_srgb_1(dst++, src[i]);
        }
        ndst -= n;
    }
}

static void trivial_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
    while (ndst > 0) {
        int n = SkTMin(ndst, nsrc);

        for (int i = 0; i < n; i++) {
            srcover_srgb_srgb_1(dst++, src[i]);
        }
        ndst -= n;
    }
}

static void best_non_simd_srcover_srgb_srgb(
    uint32_t* dst, const uint32_t* const src, int ndst, const int nsrc) {
    uint64_t* ddst = reinterpret_cast<uint64_t*>(dst);

    auto srcover_srgb_srgb_2 = [](uint32_t* dst, const uint32_t* src) {
        srcover_srgb_srgb_1(dst++, *src++);
        srcover_srgb_srgb_1(dst, *src);
    };

    while (ndst >0) {
        int count = SkTMin(ndst, nsrc);
        ndst -= count;
        const uint64_t* dsrc = reinterpret_cast<const uint64_t*>(src);
        const uint64_t* end = dsrc + (count >> 1);
        do {
            if ((~*dsrc & 0xFF000000FF000000) == 0) {
                do {
                    *ddst++ = *dsrc++;
                } while (dsrc < end && (~*dsrc & 0xFF000000FF000000) == 0);
            } else if ((*dsrc & 0xFF000000FF000000) == 0) {
                do {
                    dsrc++;
                    ddst++;
                } while (dsrc < end && (*dsrc & 0xFF000000FF000000) == 0);
            } else {
                srcover_srgb_srgb_2(reinterpret_cast<uint32_t*>(ddst++),
                                    reinterpret_cast<const uint32_t*>(dsrc++));
            }
        } while (dsrc < end);

        if ((count & 1) != 0) {
            uint32_t s1;
            memcpy(&s1, dsrc, 4);
            srcover_srgb_srgb_1(reinterpret_cast<uint32_t*>(ddst), s1);
        }
    }
}

class SrcOverVSkOptsBruteForce {
public:
    static SkString Name() { return SkString{"VSkOptsBruteForce"}; }
    static void BlendN(uint32_t* dst, const uint32_t* src, int count) {
        brute_force_srcover_srgb_srgb(dst, src, count, count);
    }
};

class SrcOverVSkOptsTrivial {
public:
    static SkString Name() { return SkString{"VSkOptsTrivial"}; }
    static void BlendN(uint32_t* dst, const uint32_t* src, int count) {
        trivial_srcover_srgb_srgb(dst, src, count, count);
    }
};

class SrcOverVSkOptsNonSimdCore {
public:
    static SkString Name() { return SkString{"VSkOptsNonSimdCore"}; }
    static void BlendN(uint32_t* dst, const uint32_t* src, int count) {
        best_non_simd_srcover_srgb_srgb(dst, src, count, count);
    }
};

class SrcOverVSkOptsDefault {
public:
    static SkString Name() { return SkString{"VSkOptsDefault"}; }
    static void BlendN(uint32_t* dst, const uint32_t* src, int count) {
        SkOpts::srcover_srgb_srgb(dst, src, count, count);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Blender>
class LinearSrcOverBench : public Benchmark {
public:
    LinearSrcOverBench(const char* fileName) : fFileName(fileName) {
        fName = "LinearSrcOver_";
        fName.append(fileName);
        fName.append(Blender::Name());
    }

protected:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return fName.c_str(); }

    void onPreDraw(SkCanvas*) override {
        if (!fPixmap.addr()) {
            sk_sp<SkImage> image = GetResourceAsImage(fFileName.c_str());
            SkBitmap bm;
            SkColorSpace* legacyColorSpace = nullptr;
            if (!as_IB(image)->getROPixels(&bm, legacyColorSpace)) {
                SkFAIL("Could not read resource");
            }
            bm.peekPixels(&fPixmap);
            fCount = fPixmap.rowBytesAsPixels();
            fDst.reset(fCount);
            sk_bzero(fDst.get(), fPixmap.rowBytes());
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        SkASSERT(fPixmap.colorType() == kN32_SkColorType);

        const int width = fPixmap.rowBytesAsPixels();

        for (int i = 0; i < loops * INNER_LOOPS; ++i) {
            const uint32_t* src = fPixmap.addr32();
            for (int y = 0; y < fPixmap.height(); y++) {
                Blender::BlendN(fDst.get(), src, width);
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

#define BENCHES(fileName)                                                            \
    DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsBruteForce>(fileName); )  \
    DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsTrivial>(fileName); )     \
    DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsNonSimdCore>(fileName); ) \
    DEF_BENCH( return new LinearSrcOverBench<SrcOverVSkOptsDefault>(fileName); )

BENCHES("yellow_rose.png")
BENCHES("baby_tux.png")
BENCHES("plane.png")
BENCHES("mandrill_512.png")
BENCHES("iconstrip.png")

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkTypes.h"

/**
 * There's a good variety of ways to pack from int down to uint16_t with SSE,
 * depending on the specific instructions available.
 *
 * SSE2 offers an int -> int16_t pack instruction.  We can use this in two ways:
 *    - subtract off 32768, int -> int16_t, add 32768 back                                  (sse2_a)
 *    - first artificially sign extend the (positive) value in our int, then int -> int16_t (sse2_b)
 * SSSE3 adds a byte shuffle, so we just put the bytes where we want them.                  (ssse3)
 * SSE41 added an int -> uint16_t pack instruction.                                         (sse41)
 *
 * Findings so far:
 *   - sse41 < ssse3 <<< sse2_b < sse2_a;
 *   - the ssse3 version is only slightly slower than the sse41 version, maybe not at all
 *   - the sse2_a is only slightly slower than the sse2_b version
 *   - the ssse3 and sse41 versions are about 3x faster than either sse2 version
 *   - the sse41 version seems to cause some code generation trouble.
 */

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

#include <immintrin.h>

template <__m128i (kernel)(__m128i)>
class pack_int_uint16_t_Bench : public Benchmark {
public:
    pack_int_uint16_t_Bench(const char* impl) {
        fName.append("pack_int_uint16_t_");
        fName.append(impl);
    }

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        __m128i x = _mm_set1_epi32(0x42424242);
        while (loops --> 0) {
            x = kernel(x);
        }

        volatile int blackhole = 0;
        blackhole ^= _mm_cvtsi128_si32(x);
    }

    SkString fName;
};

namespace {
    __m128i sse2_a(__m128i x) {
        x = _mm_sub_epi32(x, _mm_set1_epi32(0x8000));
        return _mm_add_epi16(_mm_packs_epi32(x,x), _mm_set1_epi16((short)0x8000));
    }
}
DEF_BENCH( return new pack_int_uint16_t_Bench<sse2_a>("sse2_a"); )

namespace {
    __m128i sse2_b(__m128i x) {
        x = _mm_srai_epi32(_mm_slli_epi32(x, 16), 16);
        return _mm_packs_epi32(x,x);
    }
}
DEF_BENCH( return new pack_int_uint16_t_Bench<sse2_b>("sse2_b"); )

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
namespace {
    __m128i ssse3(__m128i x) {
        // TODO: Can we force the bench to load the mask inside the loop?  Would be more realistic.
        const int _ = ~0;
        return _mm_shuffle_epi8(x, _mm_setr_epi8(0,1, 4,5, 8,9, 12,13, _,_,_,_,_,_,_,_));
    }
}
DEF_BENCH( return new pack_int_uint16_t_Bench<ssse3>("ssse3"); )
#endif

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
namespace {
    __m128i sse41(__m128i x) {
        return _mm_packus_epi32(x,x);
    }
}
DEF_BENCH( return new pack_int_uint16_t_Bench<sse41>("sse41"); )
#endif

#endif  // SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

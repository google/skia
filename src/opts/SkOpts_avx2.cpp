/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#define SK_OPTS_NS sk_avx2

#ifndef SK_SUPPORT_LEGACY_X86_BLITS

namespace sk_avx2 {

// AVX2 has masked loads and stores.  We'll use them for N<4 pixels.
static __m128i mask(int n) {
    static const int masks[][4] = {
        { 0, 0, 0, 0},
        {~0, 0, 0, 0},
        {~0,~0, 0, 0},
        {~0,~0,~0, 0},
    };
    return _mm_load_si128((const __m128i*)masks+n);
}

// Load 8, 4, or 1-3 constant pixels or coverages (4x replicated).
static __m256i next8(    uint32_t val) { return _mm256_set1_epi32(val); }
static __m128i next4(    uint32_t val) { return    _mm_set1_epi32(val); }
static __m128i tail(int, uint32_t val) { return    _mm_set1_epi32(val); }

static __m256i next8(    uint8_t val) { return _mm256_set1_epi8(val); }
static __m128i next4(    uint8_t val) { return    _mm_set1_epi8(val); }
static __m128i tail(int, uint8_t val) { return    _mm_set1_epi8(val); }

// Load 8, 4, or 1-3 variable pixels or coverages (4x replicated).
// next8() and next4() increment their pointer past what they just read.  tail() doesn't bother.
static __m256i next8(const uint32_t*& ptr) {
    auto r = _mm256_loadu_si256((const __m256i*)ptr);
    ptr += 8;
    return r;
}
static __m128i next4(const uint32_t*& ptr) {
    auto r = _mm_loadu_si128((const __m128i*)ptr);
    ptr += 4;
    return r;
}
static __m128i tail(int n, const uint32_t* ptr) {
    return _mm_maskload_epi32((const int*)ptr, mask(n));
}

static __m256i next8(const uint8_t*& ptr) {
    auto r = _mm256_cvtepu8_epi32(_mm_loadl_epi64((const __m128i*)ptr));
    r = _mm256_shuffle_epi8(r, _mm256_setr_epi8(0,0,0,0, 4,4,4,4, 8,8,8,8, 12,12,12,12,
                                                0,0,0,0, 4,4,4,4, 8,8,8,8, 12,12,12,12));
    ptr += 8;
    return r;
}
static __m128i next4(const uint8_t*& ptr) {
    auto r = _mm_shuffle_epi8(_mm_cvtsi32_si128(*(const uint32_t*)ptr),
                              _mm_setr_epi8(0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3));
    ptr += 4;
    return r;
}
static __m128i tail(int n, const uint8_t* ptr) {
    uint32_t x = 0;
    switch (n) {
        case 3: x |= (uint32_t)ptr[2] << 16;
        case 2: x |= (uint32_t)ptr[1] <<  8;
        case 1: x |= (uint32_t)ptr[0] <<  0;
    }
    auto p = (const uint8_t*)&x;
    return next4(p);
}

// For i = 0...n, tgt = fn(dst,src,cov), where Dst,Src,and Cov can be constants or arrays.
template <typename Dst, typename Src, typename Cov, typename Fn>
static void loop(int n, uint32_t* t, const Dst dst, const Src src, const Cov cov, Fn&& fn) {
    // We don't want to muck with the callers' pointers, so we make them const and copy here.
    Dst d = dst;
    Src s = src;
    Cov c = cov;

    // Writing this as a single while-loop helps hoist loop invariants from fn.
    while (n) {
        if (n >= 8) {
            _mm256_storeu_si256((__m256i*)t, fn(next8(d), next8(s), next8(c)));
            t += 8;
            n -= 8;
            continue;
        }
        if (n >= 4) {
            _mm_storeu_si128((__m128i*)t, fn(next4(d), next4(s), next4(c)));
            t += 4;
            n -= 4;
        }
        if (n) {
            _mm_maskstore_epi32((int*)t, mask(n), fn(tail(n,d), tail(n,s), tail(n,c)));
        }
        return;
    }
}

//                                       packed                                              //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//                                      unpacked                                             //

// Everything on the packed side of the squiggly line deals with densely packed 8-bit data,
// e.g [ BGRA bgra ... ] for pixels or [ CCCC cccc ... ] for coverage.
//
// Everything on the unpacked side of the squiggly line deals with unpacked 8-bit data,
// e.g. [ B_G_ R_A_ b_g_ r_a_ ... ] for pixels or [ C_C_ C_C_ c_c_ c_c_ ... ] for coverage,
// where _ is a zero byte.
//
// Adapt<Fn> / adapt(fn) allow the two sides to interoperate,
// by unpacking arguments, calling fn, then packing the results.
//
// This lets us write most of our code in terms of unpacked inputs (considerably simpler)
// and all the packing and unpacking is handled automatically.

template <typename Fn>
struct Adapt {
    Fn fn;

    __m256i operator()(__m256i d, __m256i s, __m256i c) {
        auto lo = [](__m256i x) { return _mm256_unpacklo_epi8(x, _mm256_setzero_si256()); };
        auto hi = [](__m256i x) { return _mm256_unpackhi_epi8(x, _mm256_setzero_si256()); };
        return _mm256_packus_epi16(fn(lo(d), lo(s), lo(c)),
                                   fn(hi(d), hi(s), hi(c)));
    }

    __m128i operator()(__m128i d, __m128i s, __m128i c) {
        auto unpack = [](__m128i x) { return _mm256_cvtepu8_epi16(x); };
        auto   pack = [](__m256i x) {
            auto x01 = x,
                 x23 = _mm256_permute4x64_epi64(x, 0xe);  // 0b1110
            return _mm256_castsi256_si128(_mm256_packus_epi16(x01, x23));
        };
        return pack(fn(unpack(d), unpack(s), unpack(c)));
    }
};

template <typename Fn>
static Adapt<Fn> adapt(Fn&& fn) { return { fn }; }

// These helpers all work exclusively with unpacked 8-bit values,
// except div255() which is 16-bit -> unpacked 8-bit, and mul255() which is the reverse.

// Divide by 255 with rounding.
// (x+127)/255 == ((x+128)*257)>>16.
// Sometimes we can be more efficient by breaking this into two parts.
static __m256i div255_part1(__m256i x) { return _mm256_add_epi16  (x, _mm256_set1_epi16(128)); }
static __m256i div255_part2(__m256i x) { return _mm256_mulhi_epu16(x, _mm256_set1_epi16(257)); }
static __m256i div255(__m256i x) { return div255_part2(div255_part1(x)); }

// (x*y+127)/255, a byte multiply.
static __m256i scale(__m256i x, __m256i y) { return div255(_mm256_mullo_epi16(x, y)); }

// (255 * x).
static __m256i mul255(__m256i x) { return _mm256_sub_epi16(_mm256_slli_epi16(x, 8), x); }

// (255 - x).
static __m256i inv(__m256i x) { return _mm256_xor_si256(_mm256_set1_epi16(0x00ff), x); }

// ARGB argb ... -> AAAA aaaa ...
static __m256i alphas(__m256i px) {
    const int a = 2 * (SK_A32_SHIFT/8);  // SK_A32_SHIFT is typically 24, so this is typically 6.
    const int _ = ~0;
    return _mm256_shuffle_epi8(px, _mm256_setr_epi8(a+0,_,a+0,_,a+0,_,a+0,_,
                                                    a+8,_,a+8,_,a+8,_,a+8,_,
                                                    a+0,_,a+0,_,a+0,_,a+0,_,
                                                    a+8,_,a+8,_,a+8,_,a+8,_));
}


// SrcOver, with a constant source and full coverage.
static void blit_row_color32(SkPMColor* tgt, const SkPMColor* dst, int n, SkPMColor src) {
    // We want to calculate s + (d * inv(alphas(s)) + 127)/255.
    // We'd generally do that div255 as s + ((d * inv(alphas(s)) + 128)*257)>>16.

    // But we can go one step further to ((s*255 + 128 + d*inv(alphas(s)))*257)>>16.
    // This lets us hoist (s*255+128) and inv(alphas(s)) out of the loop.
    auto s = _mm256_cvtepu8_epi16(_mm_set1_epi32(src)),
         s_255_128 = div255_part1(mul255(s)),
         A = inv(alphas(s));

    const uint8_t cov = 0xff;
    loop(n, tgt, dst, src, cov, adapt([=](__m256i d, __m256i, __m256i) {
        return div255_part2(_mm256_add_epi16(s_255_128, _mm256_mullo_epi16(d, A)));
    }));
}

// SrcOver, with a constant source and variable coverage.
// If the source is opaque, SrcOver becomes Src.
static void blit_mask_d32_a8(SkPMColor* dst,     size_t dstRB,
                             const SkAlpha* cov, size_t covRB,
                             SkColor color, int w, int h) {
    if (SkColorGetA(color) == 0xFF) {
        const SkPMColor src = SkSwizzle_BGRA_to_PMColor(color);
        while (h --> 0) {
            loop(w, dst, (const SkPMColor*)dst, src, cov,
                    adapt([](__m256i d, __m256i s, __m256i c) {
                // Src blend mode: a simple lerp from d to s by c.
                // TODO: try a pmaddubsw version?
                return div255(_mm256_add_epi16(_mm256_mullo_epi16(inv(c),d),
                                               _mm256_mullo_epi16(    c ,s)));
            }));
            dst += dstRB / sizeof(*dst);
            cov += covRB / sizeof(*cov);
        }
    } else {
        const SkPMColor src = SkPreMultiplyColor(color);
        while (h --> 0) {
            loop(w, dst, (const SkPMColor*)dst, src, cov,
                    adapt([](__m256i d, __m256i s, __m256i c) {
                // SrcOver blend mode, with coverage folded into source alpha.
                auto sc = scale(s,c),
                     AC = inv(alphas(sc));
                return _mm256_add_epi16(sc, scale(d,AC));
            }));
            dst += dstRB / sizeof(*dst);
            cov += covRB / sizeof(*cov);
        }
    }
}

}  // namespace sk_avx2

#endif

namespace SkOpts {
    void Init_avx2() {
    #ifndef SK_SUPPORT_LEGACY_X86_BLITS
        blit_row_color32 = sk_avx2::blit_row_color32;
        blit_mask_d32_a8 = sk_avx2::blit_mask_d32_a8;
    #endif
    }
}

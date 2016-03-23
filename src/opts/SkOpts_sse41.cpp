/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sk_sse41
#include "SkBlurImageFilter_opts.h"
#include "SkBlitRow_opts.h"

#ifndef SK_SUPPORT_LEGACY_X86_BLITS

namespace sk_sse41_new {

// An SSE register holding at most 64 bits of useful data in the low lanes.
struct m64i {
    __m128i v;
    /*implicit*/ m64i(__m128i v) : v(v) {}
    operator __m128i() const { return v; }
};

// Load 4, 2, or 1 constant pixels or coverages (4x replicated).
static __m128i next4(uint32_t val) { return _mm_set1_epi32(val); }
static m64i    next2(uint32_t val) { return _mm_set1_epi32(val); }
static m64i    next1(uint32_t val) { return _mm_set1_epi32(val); }

static __m128i next4(uint8_t val) { return _mm_set1_epi8(val); }
static m64i    next2(uint8_t val) { return _mm_set1_epi8(val); }
static m64i    next1(uint8_t val) { return _mm_set1_epi8(val); }

// Load 4, 2, or 1 variable pixels or coverages (4x replicated),
// incrementing the pointer past what we read.
static __m128i next4(const uint32_t*& ptr) {
    auto r = _mm_loadu_si128((const __m128i*)ptr);
    ptr += 4;
    return r;
}
static m64i next2(const uint32_t*& ptr) {
    auto r = _mm_loadl_epi64((const __m128i*)ptr);
    ptr += 2;
    return r;
}
static m64i next1(const uint32_t*& ptr) {
    auto r = _mm_cvtsi32_si128(*ptr);
    ptr += 1;
    return r;
}

// xyzw -> xxxx yyyy zzzz wwww
static __m128i replicate_coverage(__m128i xyzw) {
    return _mm_shuffle_epi8(xyzw, _mm_setr_epi8(0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3));
}

static __m128i next4(const uint8_t*& ptr) {
    auto r = replicate_coverage(_mm_cvtsi32_si128(*(const uint32_t*)ptr));
    ptr += 4;
    return r;
}
static m64i next2(const uint8_t*& ptr) {
    auto r = replicate_coverage(_mm_cvtsi32_si128(*(const uint16_t*)ptr));
    ptr += 2;
    return r;
}
static m64i next1(const uint8_t*& ptr) {
    auto r = replicate_coverage(_mm_cvtsi32_si128(*ptr));
    ptr += 1;
    return r;
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
        if (n >= 4) {
            _mm_storeu_si128((__m128i*)t, fn(next4(d), next4(s), next4(c)));
            t += 4;
            n -= 4;
            continue;
        }
        if (n & 2) {
            _mm_storel_epi64((__m128i*)t, fn(next2(d), next2(s), next2(c)));
            t += 2;
        }
        if (n & 1) {
            *t = _mm_cvtsi128_si32(fn(next1(d), next1(s), next1(c)));
        }
        return;
    }
}

//                                             packed
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//                                            unpacked

// Everything on the packed side of the squiggly line deals with densely packed 8-bit data,
// e.g. [BGRA bgra ... ] for pixels or [ CCCC cccc ... ] for coverage.
//
// Everything on the unpacked side of the squiggly line deals with unpacked 8-bit data,
// e.g [B_G_ R_A_ b_g_ r_a_ ] for pixels or [ C_C_ C_C_ c_c_ c_c_ c_c_ ] for coverage,
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

    __m128i operator()(__m128i d, __m128i s, __m128i c) {
        auto lo = [](__m128i x) { return _mm_unpacklo_epi8(x, _mm_setzero_si128()); };
        auto hi = [](__m128i x) { return _mm_unpackhi_epi8(x, _mm_setzero_si128()); };
        return _mm_packus_epi16(fn(lo(d), lo(s), lo(c)),
                                fn(hi(d), hi(s), hi(c)));
    }

    m64i operator()(const m64i& d, const m64i& s, const m64i& c) {
        auto lo = [](__m128i x) { return _mm_unpacklo_epi8(x, _mm_setzero_si128()); };
        auto r = fn(lo(d), lo(s), lo(c));
        return _mm_packus_epi16(r, r);
    }
};

template <typename Fn>
static Adapt<Fn> adapt(Fn&& fn) { return { fn }; }

// These helpers all work exclusively with unpacked 8-bit values,
// except div255() with is 16-bit -> unpacked 8-bit, and mul255() which is the reverse.

// Divide by 255 with rounding.
// (x+127)/255 == ((x+128)*257)>>16.
// Sometimes we can be more efficient by breaking this into two parts.
static __m128i div255_part1(__m128i x) { return _mm_add_epi16(x, _mm_set1_epi16(128)); }
static __m128i div255_part2(__m128i x) { return _mm_mulhi_epu16(x, _mm_set1_epi16(257)); }
static __m128i div255(__m128i x) { return div255_part2(div255_part1(x)); }

// (x*y+127)/255, a byte multiply.
static __m128i scale(__m128i x, __m128i y) { return div255(_mm_mullo_epi16(x, y)); }

// (255 * x).
static __m128i mul255(__m128i x) { return _mm_sub_epi16(_mm_slli_epi16(x, 8), x); }

// (255 - x).
static __m128i inv(__m128i x) { return _mm_xor_si128(_mm_set1_epi16(0x00ff), x); }

// ARGB argb -> AAAA aaaa
static __m128i alphas(__m128i px) {
    const int a = 2 * (SK_A32_SHIFT/8);  // SK_A32_SHIFT is typically 24, so this is typically 6.
    const int _ = ~0;
    return _mm_shuffle_epi8(px, _mm_setr_epi8(a+0,_,a+0,_,a+0,_,a+0,_, a+8,_,a+8,_,a+8,_,a+8,_));
}

// SrcOver, with a constant source and full coverage.
static void blit_row_color32(SkPMColor* tgt, const SkPMColor* dst, int n, SkPMColor src) {
    // We want to calculate s + (d * inv(alphas(s)) + 127)/255.
    // We'd generally do that div255 as s + ((d * inv(alphas(s)) + 128)*257)>>16.

    // But we can go one step further to ((s*255 + 128 + d*inv(alphas(s)))*257)>>16.
    // This lets us hoist (s*255+128) and inv(alphas(s)) out of the loop.
    __m128i s = _mm_unpacklo_epi8(_mm_set1_epi32(src), _mm_setzero_si128()),
            s_255_128 = div255_part1(mul255(s)),
            A = inv(alphas(s));

    const uint8_t cov = 0xff;
    loop(n, tgt, dst, src, cov, adapt([=](__m128i d, __m128i, __m128i) {
        return div255_part2(_mm_add_epi16(s_255_128, _mm_mullo_epi16(d, A)));
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
                    adapt([](__m128i d, __m128i s, __m128i c) {
                // Src blend mode: a simple lerp from d to s by c.
                // TODO: try a pmaddubsw version?
                return div255(_mm_add_epi16(_mm_mullo_epi16(inv(c),d),
                                            _mm_mullo_epi16(    c ,s)));
            }));
            dst += dstRB / sizeof(*dst);
            cov += covRB / sizeof(*cov);
        }
    } else {
        const SkPMColor src = SkPreMultiplyColor(color);
        while (h --> 0) {
            loop(w, dst, (const SkPMColor*)dst, src, cov,
                    adapt([](__m128i d, __m128i s, __m128i c) {
                // SrcOver blend mode, with coverage folded into source alpha.
                __m128i sc = scale(s,c),
                        AC = inv(alphas(sc));
                return _mm_add_epi16(sc, scale(d,AC));
            }));
            dst += dstRB / sizeof(*dst);
            cov += covRB / sizeof(*cov);
        }
    }
}

}  // namespace sk_sse41_new

#endif

namespace SkOpts {
    void Init_sse41() {
        box_blur_xx = sk_sse41::box_blur_xx;
        box_blur_xy = sk_sse41::box_blur_xy;
        box_blur_yx = sk_sse41::box_blur_yx;

    #ifndef SK_SUPPORT_LEGACY_X86_BLITS
        blit_row_color32 = sk_sse41_new::blit_row_color32;
        blit_mask_d32_a8 = sk_sse41_new::blit_mask_d32_a8;
    #endif
        blit_row_s32a_opaque = sk_sse41::blit_row_s32a_opaque;
    }
}

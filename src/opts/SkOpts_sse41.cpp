/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sk_sse41
#include "SkBlurImageFilter_opts.h"

#ifndef SK_SUPPORT_LEGACY_X86_BLITS

// This file deals mostly with unpacked 8-bit values,
// i.e. values between 0 and 255, but in 16-bit lanes with 0 at the top.

// So __m128i typically represents 1 or 2 pixels, and m128ix2 represents 4.
struct m128ix2 { __m128i lo, hi; };

// unpack{lo,hi}() get our raw pixels unpacked, from half of 4 packed pixels to 2 unpacked pixels.
static inline __m128i unpacklo(__m128i x) { return _mm_cvtepu8_epi16(x); }
static inline __m128i unpackhi(__m128i x) { return _mm_unpackhi_epi8(x, _mm_setzero_si128()); }

// pack() converts back, from 4 unpacked pixels to 4 packed pixels.
static inline __m128i pack(__m128i lo, __m128i hi) { return _mm_packus_epi16(lo, hi); }

// These nextN() functions abstract over the difference between iterating over
// an array of values and returning a constant value, for uint8_t and uint32_t.
// The nextN() taking pointers increment that pointer past where they read.
//
// nextN() returns N unpacked pixels or 4N unpacked coverage values.

static inline __m128i next1(uint8_t val) { return _mm_set1_epi16(val); }
static inline __m128i next2(uint8_t val) { return _mm_set1_epi16(val); }
static inline m128ix2 next4(uint8_t val) { return { next2(val), next2(val) }; }

static inline __m128i next1(uint32_t val) { return unpacklo(_mm_cvtsi32_si128(val)); }
static inline __m128i next2(uint32_t val) { return unpacklo(_mm_set1_epi32(val)); }
static inline m128ix2 next4(uint32_t val) { return { next2(val), next2(val) }; }

static inline __m128i next1(const uint8_t*& ptr) { return _mm_set1_epi16(*ptr++); }
static inline __m128i next2(const uint8_t*& ptr) {
    auto r = _mm_cvtsi32_si128(*(const uint16_t*)ptr);
    ptr += 2;
    const int _ = ~0;
    return _mm_shuffle_epi8(r, _mm_setr_epi8(0,_,0,_,0,_,0,_, 1,_,1,_,1,_,1,_));
}
static inline m128ix2 next4(const uint8_t*& ptr) {
    auto r = _mm_cvtsi32_si128(*(const uint32_t*)ptr);
    ptr += 4;
    const int _ = ~0;
    auto lo = _mm_shuffle_epi8(r, _mm_setr_epi8(0,_,0,_,0,_,0,_, 1,_,1,_,1,_,1,_)),
         hi = _mm_shuffle_epi8(r, _mm_setr_epi8(2,_,2,_,2,_,2,_, 3,_,3,_,3,_,3,_));
    return { lo, hi };
}

static inline __m128i next1(const uint32_t*& ptr) { return unpacklo(_mm_cvtsi32_si128(*ptr++)); }
static inline __m128i next2(const uint32_t*& ptr) {
    auto r = unpacklo(_mm_loadl_epi64((const __m128i*)ptr));
    ptr += 2;
    return r;
}
static inline m128ix2 next4(const uint32_t*& ptr) {
    auto packed = _mm_loadu_si128((const __m128i*)ptr);
    ptr += 4;
    return { unpacklo(packed), unpackhi(packed) };
}

// Divide by 255 with rounding.
// (x+127)/255 == ((x+128)*257)>>16.
// Sometimes we can be more efficient by breaking this into two parts.
static inline __m128i div255_part1(__m128i x) { return _mm_add_epi16(x, _mm_set1_epi16(128)); }
static inline __m128i div255_part2(__m128i x) { return _mm_mulhi_epu16(x, _mm_set1_epi16(257)); }
static inline __m128i div255(__m128i x) { return div255_part2(div255_part1(x)); }

// (x*y+127)/255, a byte multiply.
static inline __m128i scale(__m128i x, __m128i y) {
    return div255(_mm_mullo_epi16(x, y));
}

// (255 - x).
static inline __m128i inv(__m128i x) {
    return _mm_xor_si128(_mm_set1_epi16(0x00ff), x);  // This seems a bit faster than _mm_sub_epi16.
}

// ARGB argb -> AAAA aaaa
static inline __m128i alphas(__m128i px) {
    const int a = 2 * (SK_A32_SHIFT/8);  // SK_A32_SHIFT is typically 24, so this is typically 6.
    const int _ = ~0;
    return _mm_shuffle_epi8(px, _mm_setr_epi8(a+0,_,a+0,_,a+0,_,a+0,_, a+8,_,a+8,_,a+8,_,a+8,_));
}

// For i = 0...n, tgt = fn(dst,src,cov), where Dst,Src,and Cov can be constants or arrays.
template <typename Dst, typename Src, typename Cov, typename Fn>
static inline void loop(int n, uint32_t* t, const Dst dst, const Src src, const Cov cov, Fn&& fn) {
    // We don't want to muck with the callers' pointers, so we make them const and copy here.
    Dst d = dst;
    Src s = src;
    Cov c = cov;

    // Writing this as a single while-loop helps hoist loop invariants from fn.
    while (n) {
        if (n >= 4) {
            auto d4 = next4(d),
                 s4 = next4(s),
                 c4 = next4(c);
            auto lo = fn(d4.lo, s4.lo, c4.lo),
                 hi = fn(d4.hi, s4.hi, c4.hi);
            _mm_storeu_si128((__m128i*)t, pack(lo,hi));
            t += 4;
            n -= 4;
            continue;
        }
        if (n & 2) {
            auto r = fn(next2(d), next2(s), next2(c));
            _mm_storel_epi64((__m128i*)t, pack(r,r));
            t += 2;
        }
        if (n & 1) {
            auto r = fn(next1(d), next1(s), next1(c));
            *t = _mm_cvtsi128_si32(pack(r,r));
        }
        return;
    }
}

namespace sk_sse41 {

// SrcOver, with a constant source and full coverage.
static void blit_row_color32(SkPMColor* tgt, const SkPMColor* dst, int n, SkPMColor src) {
    // We want to calculate s + (d * inv(alphas(s)) + 127)/255.
    // We'd generally do that div255 as s + ((d * inv(alphas(s)) + 128)*257)>>16.

    // But we can go one step further to ((s*255 + 128 + d*inv(alphas(s)))*257)>>16.
    // This lets us hoist (s*255+128) and inv(alphas(s)) out of the loop.
    __m128i s = next2(src),
            s_255_128 = div255_part1(_mm_mullo_epi16(s, _mm_set1_epi16(255))),
            A = inv(alphas(s));

    const uint8_t cov = 0xff;
    loop(n, tgt, dst, src, cov, [=](__m128i d, __m128i, __m128i) {
        return div255_part2(_mm_add_epi16(s_255_128, _mm_mullo_epi16(d, A)));
    });
}

// SrcOver, with a constant source and variable coverage.
// If the source is opaque, SrcOver becomes Src.
static void blit_mask_d32_a8(SkPMColor* dst,     size_t dstRB,
                             const SkAlpha* cov, size_t covRB,
                             SkColor color, int w, int h) {
    if (SkColorGetA(color) == 0xFF) {
        const SkPMColor src = SkSwizzle_BGRA_to_PMColor(color);
        while (h --> 0) {
            loop(w, dst, (const SkPMColor*)dst, src, cov, [](__m128i d, __m128i s, __m128i c) {
                // Src blend mode: a simple lerp from d to s by c.
                // TODO: try a pmaddubsw version?
                return div255(_mm_add_epi16(_mm_mullo_epi16(inv(c),d), _mm_mullo_epi16(c,s)));
            });
            dst += dstRB / sizeof(*dst);
            cov += covRB / sizeof(*cov);
        }
    } else {
        const SkPMColor src = SkPreMultiplyColor(color);
        while (h --> 0) {
            loop(w, dst, (const SkPMColor*)dst, src, cov, [](__m128i d, __m128i s, __m128i c) {
                // SrcOver blend mode, with coverage folded into source alpha.
                __m128i sc = scale(s,c),
                        AC = inv(alphas(sc));
                return _mm_add_epi16(sc, scale(d,AC));
            });
            dst += dstRB / sizeof(*dst);
            cov += covRB / sizeof(*cov);
        }
    }
}

}  // namespace sk_sse41
#endif

namespace SkOpts {
    void Init_sse41() {
        box_blur_xx = sk_sse41::box_blur_xx;
        box_blur_xy = sk_sse41::box_blur_xy;
        box_blur_yx = sk_sse41::box_blur_yx;

    #ifndef SK_SUPPORT_LEGACY_X86_BLITS
        blit_row_color32 = sk_sse41::blit_row_color32;
        blit_mask_d32_a8 = sk_sse41::blit_mask_d32_a8;
    #endif
    }
}

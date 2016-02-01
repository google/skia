/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPngFilters.h"

// Functions in this file look at most 3 pixels (a,b,c) to predict the fourth (d).
// They're positioned like this:
//     prev:  c b
//     row:   a d
// The Sub filter predicts d=a, Avg d=(a+b)/2,  and Paeth predicts d to be whichever
// of a, b, or c is closest to p=a+b-c.  (Up also exists, predicting d=b.)

#if defined(__SSE2__)

    template <int bpp>
    static __m128i load(const void* p) {
        static_assert(bpp <= 4, "");

        uint32_t packed;
        memcpy(&packed, p, bpp);
        return _mm_cvtsi32_si128(packed);
    }

    template <int bpp>
    static void store(void* p, __m128i v) {
        static_assert(bpp <= 4, "");

        uint32_t packed = _mm_cvtsi128_si32(v);
        memcpy(p, &packed, bpp);
    }

    template <int bpp>
    static void sk_sub_sse2(png_row_infop row_info, uint8_t* row, const uint8_t*) {
        // The Sub filter predicts each pixel as the previous pixel, a.
        // There is no pixel to the left of the first pixel.  It's encoded directly.
        // That works with our main loop if we just say that left pixel was zero.
        __m128i a, d = _mm_setzero_si128();

        int rb = row_info->rowbytes;
        while (rb > 0) {
            a = d; d = load<bpp>(row);
            d = _mm_add_epi8(d, a);
            store<bpp>(row, d);

            row += bpp;
            rb  -= bpp;
        }
    }

    template <int bpp>
    void sk_avg_sse2(png_row_infop row_info, uint8_t* row, const uint8_t* prev) {
        // The Avg filter predicts each pixel as the (truncated) average of a and b.
        // There's no pixel to the left of the first pixel.  Luckily, it's
        // predicted to be half of the pixel above it.  So again, this works
        // perfectly with our loop if we make sure a starts at zero.
        const __m128i zero = _mm_setzero_si128();
        __m128i    b;
        __m128i a, d = zero;

        int rb = row_info->rowbytes;
        while (rb > 0) {
                   b = load<bpp>(prev);
            a = d; d = load<bpp>(row );

            // PNG requires a truncating average here, so sadly we can't just use _mm_avg_epu8...
            __m128i avg = _mm_avg_epu8(a,b);
            // ...but we can fix it up by subtracting off 1 if it rounded up.
            avg = _mm_sub_epi8(avg, _mm_and_si128(_mm_xor_si128(a,b), _mm_set1_epi8(1)));

            d = _mm_add_epi8(d, avg);
            store<bpp>(row, d);

            prev += bpp;
            row  += bpp;
            rb   -= bpp;
        }
    }

    // Returns |x| for 16-bit lanes.
    static __m128i abs_i16(__m128i x) {
    #if defined(__SSSE3__)
        return _mm_abs_epi16(x);
    #else
        // Read this all as, return x<0 ? -x : x.
        // To negate two's complement, you flip all the bits then add 1.
        __m128i is_negative = _mm_cmplt_epi16(x, _mm_setzero_si128());
        x = _mm_xor_si128(x, is_negative);                      // Flip negative lanes.
        x = _mm_add_epi16(x, _mm_srli_epi16(is_negative, 15));  // +1 to negative lanes, else +0.
        return x;
    #endif
    }

    // Bytewise c ? t : e.
    static __m128i if_then_else(__m128i c, __m128i t, __m128i e) {
    #if 0 && defined(__SSE4_1__)  // Make sure we have a bot testing this before enabling.
        return _mm_blendv_epi8(e,t,c);
    #else
        return _mm_or_si128(_mm_and_si128(c, t), _mm_andnot_si128(c, e));
    #endif
    }

    template <int bpp>
    void sk_paeth_sse2(png_row_infop row_info, uint8_t* row, const uint8_t* prev) {
        // Paeth tries to predict pixel d using the pixel to the left of it, a,
        // and two pixels from the previous row, b and c:
        //   prev: c b
        //   row:  a d
        // The Paeth function predicts d to be whichever of a, b, or c is nearest to p=a+b-c.

        // The first pixel has no left context, and so uses an Up filter, p = b.
        // This works naturally with our main loop's p = a+b-c if we force a and c to zero.
        // Here we zero b and d, which become c and a respectively at the start of the loop.
        const __m128i zero = _mm_setzero_si128();
        __m128i c, b = zero,
                a, d = zero;

        int rb = row_info->rowbytes;
        while (rb > 0) {
            // It's easiest to do this math (particularly, deal with pc) with 16-bit intermediates.
            c = b; b = _mm_unpacklo_epi8(load<bpp>(prev), zero);
            a = d; d = _mm_unpacklo_epi8(load<bpp>(row ), zero);

            __m128i pa = _mm_sub_epi16(b,c),   // (p-a) == (a+b-c - a) == (b-c)
                    pb = _mm_sub_epi16(a,c),   // (p-b) == (a+b-c - b) == (a-c)
                    pc = _mm_add_epi16(pa,pb); // (p-c) == (a+b-c - c) == (a+b-c-c) == (b-c)+(a-c)

            pa = abs_i16(pa);  // |p-a|
            pb = abs_i16(pb);  // |p-b|
            pc = abs_i16(pc);  // |p-c|

            __m128i smallest = _mm_min_epi16(pc, _mm_min_epi16(pa, pb));

            // Paeth breaks ties favoring a over b over c.
            __m128i nearest  = if_then_else(_mm_cmpeq_epi16(smallest, pa), a,
                               if_then_else(_mm_cmpeq_epi16(smallest, pb), b,
                                                                           c));

            d = _mm_add_epi8(d, nearest);  // Note `_epi8`: we need addition to wrap modulo 255.
            store<bpp>(row, _mm_packus_epi16(d,d));

            prev += bpp;
            row  += bpp;
            rb   -= bpp;
        }
    }

    void sk_sub3_sse2(png_row_infop row_info, uint8_t* row, const uint8_t* prev) {
        sk_sub_sse2<3>(row_info, row, prev);
    }
    void sk_sub4_sse2(png_row_infop row_info, uint8_t* row, const uint8_t* prev) {
        sk_sub_sse2<4>(row_info, row, prev);
    }

    void sk_avg3_sse2(png_row_infop row_info, uint8_t* row, const uint8_t* prev) {
        sk_avg_sse2<3>(row_info, row, prev);
    }
    void sk_avg4_sse2(png_row_infop row_info, uint8_t* row, const uint8_t* prev) {
        sk_avg_sse2<4>(row_info, row, prev);
    }

    void sk_paeth3_sse2(png_row_infop row_info, uint8_t* row, const uint8_t* prev) {
        sk_paeth_sse2<3>(row_info, row, prev);
    }
    void sk_paeth4_sse2(png_row_infop row_info, uint8_t* row, const uint8_t* prev) {
        sk_paeth_sse2<4>(row_info, row, prev);
    }

#endif

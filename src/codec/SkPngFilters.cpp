/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPngFilters.h"
#include "SkTypes.h"

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
    static void sk_sub_sse2(png_row_infop row_info, png_bytep row, png_const_bytep) {
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
    void sk_avg_sse2(png_row_infop row_info, png_bytep row, png_const_bytep prev) {
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

    // Returns bytewise |x-y|.
    static __m128i absdiff_u8(__m128i x, __m128i y) {
        // One of these two saturated subtractions will be the answer, the other zero.
        return _mm_or_si128(_mm_subs_epu8(x,y), _mm_subs_epu8(y,x));
    }

    // Bytewise c ? t : e.
    static __m128i if_then_else(__m128i c, __m128i t, __m128i e) {
        // SSE 4.1+ would be: return _mm_blendv_epi8(e,t,c);
        return _mm_or_si128(_mm_and_si128(c, t), _mm_andnot_si128(c, e));
    }

    template <int bpp>
    void sk_paeth_sse2(png_row_infop row_info, png_bytep row, png_const_bytep prev) {
        // Paeth tries to predict pixel d using the pixel to the left of it, a,
        // and two pixels from the previous row, b and c:
        //   prev: c b
        //   row:  a d
        // The Paeth function predicts d to be whichever of a, b, or c is nearest to p=a+b-c.

        // The first pixel has no left context, and so uses an Up filter, p = b.
        // This works naturally with our main loop's p = a+b-c if we force a and c to zero.
        // Here we zero b and d, which become c and a respectively at the start of the loop.
        __m128i c, b = _mm_setzero_si128(),
                a, d = _mm_setzero_si128();

        int rb = row_info->rowbytes;
        while (rb > 0) {
            c = b; b = load<bpp>(prev);
            a = d; d = load<bpp>(row );

            // We can't express p in 8 bits, but luckily we can use this faux p instead.
            // (I have no deep insight here... I just proved this with brute force.)
            __m128i min = _mm_min_epu8(a,b),
                    max = _mm_max_epu8(a,b),
                    faux_p = _mm_adds_epu8(min, _mm_subs_epu8(max, c));

            // We could use faux_p for calculating all three of pa, pb, and pc,
            // but it's a little quicker to calculate the correct pa and pb directly,
            // and the predictor remains the same.  (Again, brute force.)
            __m128i pa = absdiff_u8(b,c),             // |a+b-c - a| == |b-c|
                    pb = absdiff_u8(a,c),             // |a+b-c - b| == |a-c|
               faux_pc = absdiff_u8(faux_p, c);

            // From here, things are straightforward.  Find the smallest distance to p...
            __m128i smallest = _mm_min_epu8(_mm_min_epu8(pa, pb), faux_pc);

            // ... then the predictor is the input corresponding to that smallest distance,
            // breaking ties in favor of a over b over c.
            __m128i nearest = if_then_else(_mm_cmpeq_epi8(smallest, pa), a,
                              if_then_else(_mm_cmpeq_epi8(smallest, pb), b,
                                                                         c));

            // We've reconstructed d!  Leave it for next round to become a, and write it out.
            d = _mm_add_epi8(d, nearest);
            store<bpp>(row, d);

            prev += bpp;
            row  += bpp;
            rb   -= bpp;
        }
    }

    void sk_sub3_sse2(png_row_infop row_info, png_bytep row, png_const_bytep prev) {
        sk_sub_sse2<3>(row_info, row, prev);
    }
    void sk_sub4_sse2(png_row_infop row_info, png_bytep row, png_const_bytep prev) {
        sk_sub_sse2<4>(row_info, row, prev);
    }

    void sk_avg3_sse2(png_row_infop row_info, png_bytep row, png_const_bytep prev) {
        sk_avg_sse2<3>(row_info, row, prev);
    }
    void sk_avg4_sse2(png_row_infop row_info, png_bytep row, png_const_bytep prev) {
        sk_avg_sse2<4>(row_info, row, prev);
    }

    void sk_paeth3_sse2(png_row_infop row_info, png_bytep row, png_const_bytep prev) {
        sk_paeth_sse2<3>(row_info, row, prev);
    }
    void sk_paeth4_sse2(png_row_infop row_info, png_bytep row, png_const_bytep prev) {
        sk_paeth_sse2<4>(row_info, row, prev);
    }

#endif

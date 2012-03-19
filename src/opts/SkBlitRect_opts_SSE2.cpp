/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRect_opts_SSE2.h"
#include "SkBlitRow.h"
#include "SkColorPriv.h"

#include <emmintrin.h>

/** Simple blitting of opaque rectangles less than 31 pixels wide:
    inlines and merges sections of Color32_SSE2 and sk_memset32_SSE2.
*/
void BlitRect32_OpaqueNarrow_SSE2(SkPMColor* SK_RESTRICT destination,
                                  int width, int height,
                                  size_t rowBytes, uint32_t color) {
    SkASSERT(255 == SkGetPackedA32(color));
    SkASSERT(width > 0);
    SkASSERT(width < 31);

    while (--height >= 0) {
        SkPMColor* dst = destination;
        int count = width;

        while (count > 4) {
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            count -= 4;
        }

        while (count > 0) {
            *dst++ = color;
            --count;
        }

        destination = (uint32_t*)((char*)destination + rowBytes);
    }
}

/**
  Fast blitting of opaque rectangles at least 31 pixels wide:
  inlines and merges sections of Color32_SSE2 and sk_memset32_SSE2.
  A 31 pixel rectangle is guaranteed to have at least one
  16-pixel aligned span that can take advantage of mm_store.
*/
void BlitRect32_OpaqueWide_SSE2(SkPMColor* SK_RESTRICT destination,
                                int width, int height,
                                size_t rowBytes, uint32_t color) {
    SkASSERT(255 == SkGetPackedA32(color));
    SkASSERT(width >= 31);

    __m128i color_wide = _mm_set1_epi32(color);
    while (--height >= 0) {
        // Prefetching one row ahead to L1 cache can equal hardware
        // performance for large/tall rects, but never *beats*
        // hardware performance.
        SkPMColor* dst = destination;
        int count = width;

        while (((size_t)dst) & 0x0F) {
            *dst++ = color;
            --count;
        }
        __m128i *d = reinterpret_cast<__m128i*>(dst);

        // Googling suggests _mm_stream is only going to beat _mm_store
        // for things that wouldn't fit in L2 cache anyway, typically
        // >500kB, and precisely fill cache lines.  For us, with
        // arrays > 100k elements _mm_stream is still 100%+ slower than
        // mm_store.

        // Unrolling to count >= 64 is a break-even for most
        // input patterns; we seem to be saturating the bus and having
        // low enough overhead at 32.

        while (count >= 32) {
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            count -= 32;
        }
        if (count >= 16) {
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            _mm_store_si128(d++, color_wide);
            count -= 16;
        }
        dst = reinterpret_cast<uint32_t*>(d);

        // Unrolling the loop in the Narrow code is a significant performance
        // gain, but unrolling this loop appears to make no difference in
        // benchmarks with either mm_store_si128 or individual sets.

        while (count > 0) {
            *dst++ = color;
            --count;
        }

        destination = (uint32_t*)((char*)destination + rowBytes);
    }
}

void ColorRect32_SSE2(SkPMColor* destination,
                      int width, int height,
                      size_t rowBytes, uint32_t color) {
    if (0 == height || 0 == width || 0 == color) {
        return;
    }
    unsigned colorA = SkGetPackedA32(color);
    //if (255 == colorA) {
        //if (width < 31) {
            //BlitRect32_OpaqueNarrow_SSE2(destination, width, height,
                                         //rowBytes, color);
        //} else {
            //BlitRect32_OpaqueWide_SSE2(destination, width, height,
                                       //rowBytes, color);
        //}
    //} else {
        SkBlitRow::ColorRect32(destination, width, height, rowBytes, color);
    //}
}


/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkUnPreMultiply.h"
#include "SkShader.h"

#include "SkBitmapFilter_opts_SSE2.h"

#include <emmintrin.h>

#if 0
static inline void print128i(__m128i value) {
    int *v = (int*) &value;
    printf("% .11d % .11d % .11d % .11d\n", v[0], v[1], v[2], v[3]);
}

static inline void print128i_16(__m128i value) {
    short *v = (short*) &value;
    printf("% .5d % .5d % .5d % .5d % .5d % .5d % .5d % .5d\n", v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
}

static inline void print128i_8(__m128i value) {
    unsigned char *v = (unsigned char*) &value;
    printf("%.3u %.3u %.3u %.3u %.3u %.3u %.3u %.3u %.3u %.3u %.3u %.3u %.3u %.3u %.3u %.3u\n",
           v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7],
           v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]
           );
}

static inline void print128f(__m128 value) {
    float *f = (float*) &value;
    printf("%3.4f %3.4f %3.4f %3.4f\n", f[0], f[1], f[2], f[3]);
}
#endif

// because the border is handled specially, this is guaranteed to have all 16 pixels
// available to it without running off the bitmap's edge.

int debug_x = 20;
int debug_y = 255;

void highQualityFilter_SSE2(const SkBitmapProcState& s, int x, int y,
                        SkPMColor* SK_RESTRICT colors, int count) {

    const int maxX = s.fBitmap->width() - 1;
    const int maxY = s.fBitmap->height() - 1;

    while (count-- > 0) {
        SkPoint srcPt;
        s.fInvProc(*s.fInvMatrix, SkIntToScalar(x),
                    SkIntToScalar(y), &srcPt);
        srcPt.fX -= SK_ScalarHalf;
        srcPt.fY -= SK_ScalarHalf;

        int sx = SkScalarFloorToInt(srcPt.fX);
        int sy = SkScalarFloorToInt(srcPt.fY);

        __m128 weight = _mm_setzero_ps();
        __m128 accum = _mm_setzero_ps();

        int y0 = SkTMax(0, int(ceil(sy-s.getBitmapFilter()->width() + 0.5f)));
        int y1 = SkTMin(maxY, int(floor(sy+s.getBitmapFilter()->width() + 0.5f)));
        int x0 = SkTMax(0, int(ceil(sx-s.getBitmapFilter()->width() + 0.5f)));
        int x1 = SkTMin(maxX, int(floor(sx+s.getBitmapFilter()->width() + 0.5f)));

        for (int src_y = y0; src_y <= y1; src_y++) {
            float yweight = s.getBitmapFilter()->lookupFloat( (srcPt.fY - src_y) );

            for (int src_x = x0; src_x <= x1 ; src_x++) {
                float xweight = s.getBitmapFilter()->lookupFloat( (srcPt.fX - src_x) );

                float combined_weight = xweight * yweight;

                SkPMColor color = *s.fBitmap->getAddr32(src_x, src_y);

                __m128i c = _mm_cvtsi32_si128( color );
                c = _mm_unpacklo_epi8(c, _mm_setzero_si128());
                c = _mm_unpacklo_epi16(c, _mm_setzero_si128());

                __m128 cfloat = _mm_cvtepi32_ps( c );

                __m128 weightVector = _mm_set1_ps(combined_weight);

                accum = _mm_add_ps(accum, _mm_mul_ps(cfloat, weightVector));
                weight = _mm_add_ps( weight, weightVector );
            }
        }

        accum = _mm_div_ps(accum, weight);
        accum = _mm_add_ps(accum, _mm_set1_ps(0.5f));

        __m128i accumInt = _mm_cvtps_epi32( accum );

        int localResult[4];
        _mm_storeu_si128((__m128i *) (localResult), accumInt);
        int a = SkClampMax(localResult[0], 255);
        int r = SkClampMax(localResult[1], a);
        int g = SkClampMax(localResult[2], a);
        int b = SkClampMax(localResult[3], a);

        *colors++ = SkPackARGB32(a, r, g, b);

        x++;
    }
}

void highQualityFilter_ScaleOnly_SSE2(const SkBitmapProcState &s, int x, int y,
                             SkPMColor *SK_RESTRICT colors, int count) {
    const int maxX = s.fBitmap->width() - 1;
    const int maxY = s.fBitmap->height() - 1;

    SkPoint srcPt;
    s.fInvProc(*s.fInvMatrix, SkIntToScalar(x),
                SkIntToScalar(y), &srcPt);
    srcPt.fY -= SK_ScalarHalf;
    int sy = SkScalarFloorToInt(srcPt.fY);

    int y0 = SkTMax(0, int(ceil(sy-s.getBitmapFilter()->width() + 0.5f)));
    int y1 = SkTMin(maxY, int(floor(sy+s.getBitmapFilter()->width() + 0.5f)));

    while (count-- > 0) {
        srcPt.fX -= SK_ScalarHalf;
        srcPt.fY -= SK_ScalarHalf;

        int sx = SkScalarFloorToInt(srcPt.fX);

        float weight = 0;
        __m128 accum = _mm_setzero_ps();

        int x0 = SkTMax(0, int(ceil(sx-s.getBitmapFilter()->width() + 0.5f)));
        int x1 = SkTMin(maxX, int(floor(sx+s.getBitmapFilter()->width() + 0.5f)));

        for (int src_y = y0; src_y <= y1; src_y++) {
            float yweight = s.getBitmapFilter()->lookupFloat( (srcPt.fY - src_y) );

            for (int src_x = x0; src_x <= x1 ; src_x++) {
                float xweight = s.getBitmapFilter()->lookupFloat( (srcPt.fX - src_x) );

                float combined_weight = xweight * yweight;

                SkPMColor color = *s.fBitmap->getAddr32(src_x, src_y);

                __m128 c = _mm_set_ps((float)SkGetPackedB32(color),
                                      (float)SkGetPackedG32(color),
                                      (float)SkGetPackedR32(color),
                                      (float)SkGetPackedA32(color));

                __m128 weightVector = _mm_set1_ps(combined_weight);

                accum = _mm_add_ps(accum, _mm_mul_ps(c, weightVector));
                weight += combined_weight;
            }
        }

        __m128 totalWeightVector = _mm_set1_ps(weight);
        accum = _mm_div_ps(accum, totalWeightVector);
        accum = _mm_add_ps(accum, _mm_set1_ps(0.5f));

        float localResult[4];
        _mm_storeu_ps(localResult, accum);
        int a = SkClampMax(int(localResult[0]), 255);
        int r = SkClampMax(int(localResult[1]), a);
        int g = SkClampMax(int(localResult[2]), a);
        int b = SkClampMax(int(localResult[3]), a);

        *colors++ = SkPackARGB32(a, r, g, b);

        x++;

        s.fInvProc(*s.fInvMatrix, SkIntToScalar(x),
                    SkIntToScalar(y), &srcPt);

    }
}

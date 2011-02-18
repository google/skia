/* libs/graphics/effects/SkBlurMask.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkBlurMask.h"
#include "SkTemplates.h"

#if 0
static void dump_sum_buffer(const uint32_t sum[], const int w, const int h) {
    printf("---- sum buffer\n");
    for (int y = 0; y <= h; y++) {
        for (int x = 0; x <= w; x++) {
            printf(" %5d", sum[x]);
        }
        printf("\n");
        sum += w+1;
    }
}
#else
#define dump_sum_buffer(sum, w, h)
#endif

/** The sum buffer is an array of u32 to hold the accumulated sum of all of the
    src values at their position, plus all values above and to the left.
    When we sample into this buffer, we need an initial row and column of 0s,
    so we have an index correspondence as follows:
 
    src[i, j] == sum[i+1, j+1]
    sum[0, j] == sum[i, 0] == 0
 
    We assume that the sum buffer's stride == its width
 */
static void build_sum_buffer(uint32_t sum[], int srcW, int srcH, const uint8_t src[], int srcRB) {
    int sumW = srcW + 1;

    SkASSERT(srcRB >= srcW);
    // mod srcRB so we can apply it after each row
    srcRB -= srcW;

    int x, y;

    // zero out the top row and column
    memset(sum, 0, sumW * sizeof(sum[0]));
    sum += sumW;

    // special case first row
    uint32_t X = 0;
    *sum++ = 0; // initialze the first column to 0
    for (x = srcW - 1; x >= 0; --x)
    {
        X = *src++ + X;
        *sum++ = X;
    }
    src += srcRB;

    // now do the rest of the rows
    for (y = srcH - 1; y > 0; --y)
    {
        uint32_t L = 0;
        uint32_t C = 0;
        *sum++ = 0; // initialze the first column to 0
        for (x = srcW - 1; x >= 0; --x)
        {
            uint32_t T = sum[-sumW];
            X = *src++ + L + T - C;
            *sum++ = X;
            L = X;
            C = T;
        }
        src += srcRB;
    }
}

/*  sw and sh are the width and height of the src. Since the sum buffer
    matches that, but has an extra row and col at the beginning (with zeros),
    we can just use sw and sh as our "max" values for pinning coordinates
    when sampling into sum[][]
 */
static void apply_kernel(uint8_t dst[], int rx, int ry, const uint32_t sum[],
                         int sw, int sh) {
    uint32_t scale = (1 << 24) / ((2*rx + 1)*(2*ry + 1));

    int sumStride = sw + 1;

    int dw = sw + 2*rx;
    int dh = sh + 2*ry;

    int prev_y = -2*ry;
    int next_y = 1;

    for (int y = 0; y < dh; y++) {
        int py = SkClampPos(prev_y) * sumStride;
        int ny = SkFastMin32(next_y, sh) * sumStride;

        int prev_x = -2*rx;
        int next_x = 1;

        for (int x = 0; x < dw; x++) {
            int px = SkClampPos(prev_x);
            int nx = SkFastMin32(next_x, sw);

            uint32_t tmp = sum[px+py] + sum[nx+ny] - sum[nx+py] - sum[px+ny];
            *dst++ = SkToU8(tmp * scale >> 24);

            prev_x += 1;
            next_x += 1;
        }
        prev_y += 1;
        next_y += 1;
    }
}

/*  sw and sh are the width and height of the src. Since the sum buffer
 matches that, but has an extra row and col at the beginning (with zeros),
 we can just use sw and sh as our "max" values for pinning coordinates
 when sampling into sum[][]
 */
static void apply_kernel_interp(uint8_t dst[], int rx, int ry,
                const uint32_t sum[], int sw, int sh, U8CPU outer_weight) {
    SkASSERT(rx > 0 && ry > 0);
    SkASSERT(outer_weight <= 255);

    int inner_weight = 255 - outer_weight;

    // round these guys up if they're bigger than 127
    outer_weight += outer_weight >> 7;
    inner_weight += inner_weight >> 7;

    uint32_t outer_scale = (outer_weight << 16) / ((2*rx + 1)*(2*ry + 1));
    uint32_t inner_scale = (inner_weight << 16) / ((2*rx - 1)*(2*ry - 1));

    int sumStride = sw + 1;

    int dw = sw + 2*rx;
    int dh = sh + 2*ry;

    int prev_y = -2*ry;
    int next_y = 1;

    for (int y = 0; y < dh; y++) {
        int py = SkClampPos(prev_y) * sumStride;
        int ny = SkFastMin32(next_y, sh) * sumStride;

        int ipy = SkClampPos(prev_y + 1) * sumStride;
        int iny = SkClampMax(next_y - 1, sh) * sumStride;

        int prev_x = -2*rx;
        int next_x = 1;

        for (int x = 0; x < dw; x++) {
            int px = SkClampPos(prev_x);
            int nx = SkFastMin32(next_x, sw);

            int ipx = SkClampPos(prev_x + 1);
            int inx = SkClampMax(next_x - 1, sw);

            uint32_t outer_sum = sum[px+py] + sum[nx+ny] - sum[nx+py] - sum[px+ny];
            uint32_t inner_sum = sum[ipx+ipy] + sum[inx+iny] - sum[inx+ipy] - sum[ipx+iny];
            *dst++ = SkToU8((outer_sum * outer_scale + inner_sum * inner_scale) >> 24);

            prev_x += 1;
            next_x += 1;
        }
        prev_y += 1;
        next_y += 1;
    }
}

#include "SkColorPriv.h"

static void merge_src_with_blur(uint8_t dst[], int dstRB,
                                const uint8_t src[], int srcRB,
                                const uint8_t blur[], int blurRB,
                                int sw, int sh) {
    dstRB -= sw;
    srcRB -= sw;
    blurRB -= sw;
    while (--sh >= 0) {
        for (int x = sw - 1; x >= 0; --x) {
            *dst = SkToU8(SkAlphaMul(*blur, SkAlpha255To256(*src)));
            dst += 1;
            src += 1;
            blur += 1;
        }
        dst += dstRB;
        src += srcRB;
        blur += blurRB;
    }
}

static void clamp_with_orig(uint8_t dst[], int dstRowBytes,
                            const uint8_t src[], int srcRowBytes,
                            int sw, int sh,
                            SkBlurMask::Style style) {
    int x;
    while (--sh >= 0) {
        switch (style) {
        case SkBlurMask::kSolid_Style:
            for (x = sw - 1; x >= 0; --x) {
                int s = *src;
                int d = *dst;
                *dst = SkToU8(s + d - SkMulDiv255Round(s, d));
                dst += 1;
                src += 1;
            }
            break;
        case SkBlurMask::kOuter_Style:
            for (x = sw - 1; x >= 0; --x) {
                if (*src) {
                    *dst = SkToU8(SkAlphaMul(*dst, SkAlpha255To256(255 - *src)));
                }
                dst += 1;
                src += 1;
            }
            break;
        default:
            SkASSERT(!"Unexpected blur style here");
            break;
        }
        dst += dstRowBytes - sw;
        src += srcRowBytes - sw;
    }
}

////////////////////////////////////////////////////////////////////////

// we use a local funciton to wrap the class static method to work around
// a bug in gcc98
void SkMask_FreeImage(uint8_t* image);
void SkMask_FreeImage(uint8_t* image)
{
    SkMask::FreeImage(image);
}

bool SkBlurMask::Blur(SkMask* dst, const SkMask& src,
                      SkScalar radius, Style style, Quality quality)
{
    if (src.fFormat != SkMask::kA8_Format)
        return false;

    // Force high quality off for small radii (performance)
    if (radius < SkIntToScalar(3)) quality = kLow_Quality;

    // highQuality: use three box blur passes as a cheap way to approximate a Gaussian blur
    int passCount = (quality == kHigh_Quality) ? 3 : 1;
    SkScalar passRadius = SkScalarDiv(radius, SkScalarSqrt(SkIntToScalar(passCount)));

    int rx = SkScalarCeil(passRadius);
    int outer_weight = 255 - SkScalarRound((SkIntToScalar(rx) - passRadius) * 255);

    SkASSERT(rx >= 0);
    SkASSERT((unsigned)outer_weight <= 255);
    if (rx <= 0) {
        return false;
    }

    int ry = rx;    // only do square blur for now

    int padx = passCount * rx;
    int pady = passCount * ry;
    dst->fBounds.set(src.fBounds.fLeft - padx, src.fBounds.fTop - pady,
        src.fBounds.fRight + padx, src.fBounds.fBottom + pady);
    dst->fRowBytes = dst->fBounds.width();
    dst->fFormat = SkMask::kA8_Format;
    dst->fImage = NULL;

    if (src.fImage) {
        size_t dstSize = dst->computeImageSize();
        if (0 == dstSize) {
            return false;   // too big to allocate, abort
        }

        int             sw = src.fBounds.width();
        int             sh = src.fBounds.height();
        const uint8_t*  sp = src.fImage;
        uint8_t*        dp = SkMask::AllocImage(dstSize);

        SkAutoTCallVProc<uint8_t, SkMask_FreeImage> autoCall(dp);

        // build the blurry destination
        {
            SkAutoTMalloc<uint32_t> storage((sw + 2 * (passCount - 1) * rx + 1) * (sh + 2 * (passCount - 1) * ry + 1));
            uint32_t*               sumBuffer = storage.get();

            //pass1: sp is source, dp is destination
            build_sum_buffer(sumBuffer, sw, sh, sp, src.fRowBytes);
            dump_sum_buffer(sumBuffer, sw, sh);
            if (outer_weight == 255)
                apply_kernel(dp, rx, ry, sumBuffer, sw, sh);
            else
                apply_kernel_interp(dp, rx, ry, sumBuffer, sw, sh, outer_weight);

            if (quality == kHigh_Quality)
            {
                //pass2: dp is source, tmpBuffer is destination
                int tmp_sw = sw + 2 * rx;
                int tmp_sh = sh + 2 * ry;
                SkAutoTMalloc<uint8_t>  tmpBuffer(dstSize);
                build_sum_buffer(sumBuffer, tmp_sw, tmp_sh, dp, tmp_sw);
                if (outer_weight == 255)
                    apply_kernel(tmpBuffer.get(), rx, ry, sumBuffer, tmp_sw, tmp_sh);
                else
                    apply_kernel_interp(tmpBuffer.get(), rx, ry, sumBuffer, tmp_sw, tmp_sh, outer_weight);

                //pass3: tmpBuffer is source, dp is destination
                tmp_sw += 2 * rx;
                tmp_sh += 2 * ry;
                build_sum_buffer(sumBuffer, tmp_sw, tmp_sh, tmpBuffer.get(), tmp_sw);
                if (outer_weight == 255)
                    apply_kernel(dp, rx, ry, sumBuffer, tmp_sw, tmp_sh);
                else
                    apply_kernel_interp(dp, rx, ry, sumBuffer, tmp_sw, tmp_sh, outer_weight);
            }
        }

        dst->fImage = dp;
        // if need be, alloc the "real" dst (same size as src) and copy/merge
        // the blur into it (applying the src)
        if (style == kInner_Style) {
            // now we allocate the "real" dst, mirror the size of src
            size_t srcSize = src.computeImageSize();
            if (0 == srcSize) {
                return false;   // too big to allocate, abort
            }
            dst->fImage = SkMask::AllocImage(srcSize);
            merge_src_with_blur(dst->fImage, src.fRowBytes,
                                sp, src.fRowBytes,
                                dp + passCount * (rx + ry * dst->fRowBytes), dst->fRowBytes,
                                sw, sh);
            SkMask::FreeImage(dp);
        } else if (style != kNormal_Style) {
            clamp_with_orig(dp + passCount * (rx + ry * dst->fRowBytes), dst->fRowBytes,
                            sp, src.fRowBytes, sw, sh,
                            style);
        }
        (void)autoCall.detach();
    }

    if (style == kInner_Style) {
        dst->fBounds = src.fBounds; // restore trimmed bounds
        dst->fRowBytes = src.fRowBytes;
    }

#if 0
    if (gamma && dst->fImage) {
        uint8_t*    image = dst->fImage;
        uint8_t*    stop = image + dst->computeImageSize();

        for (; image < stop; image += 1) {
            *image = gamma[*image];
        }
    }
#endif
    return true;
}

#if 0
void SkBlurMask::BuildSqrtGamma(uint8_t gamma[256], SkScalar percent)
{
    SkASSERT(gamma);
    SkASSERT(percent >= 0 && percent <= SK_Scalar1);

    int scale = SkScalarRound(percent * 256);

    for (int i = 0; i < 256; i++)
    {
        SkFixed n = i * 257;
        n += n >> 15;
        SkASSERT(n >= 0 && n <= SK_Fixed1);
        n = SkFixedSqrt(n);
        n = n * 255 >> 16;
        n = SkAlphaBlend(n, i, scale);
        gamma[i] = SkToU8(n);
    }
}

void SkBlurMask::BuildSqrGamma(uint8_t gamma[256], SkScalar percent)
{
    SkASSERT(gamma);
    SkASSERT(percent >= 0 && percent <= SK_Scalar1);

    int     scale = SkScalarRound(percent * 256);
    SkFixed div255 = SK_Fixed1 / 255;

    for (int i = 0; i < 256; i++)
    {
        int square = i * i;
        int linear = i * 255;
        int n = SkAlphaBlend(square, linear, scale);
        gamma[i] = SkToU8(n * div255 >> 16);
    }
}
#endif

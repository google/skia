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

static void build_sum_buffer(uint32_t dst[], int w, int h, const uint8_t src[], int srcRB)
{
    SkASSERT(srcRB >= w);
    // mod srcRB so we can apply it after each row
    srcRB -= w;

    int x, y;

    // special case first row
    uint32_t X = 0;
    for (x = w - 1; x >= 0; --x)
    {
        X = *src++ + X;
        *dst++ = X;
    }
    src += srcRB;

    // now do the rest of the rows
    for (y = h - 1; y > 0; --y)
    {
        uint32_t L = 0;
        uint32_t C = 0;
        for (x = w - 1; x >= 0; --x)
        {
            uint32_t T = dst[-w];
            X = *src++ + L + T - C;
            *dst++ = X;
            L = X;
            C = T;
        }
        src += srcRB;
    }
}

static void apply_kernel(uint8_t dst[], int rx, int ry, const uint32_t src[], int sw, int sh)
{
    uint32_t scale = (1 << 24) / ((2*rx + 1)*(2*ry + 1));

    int rowBytes = sw;

    int dw = sw + 2*rx;
    int dh = sh + 2*ry;

    sw -= 1;    // now it is max_x
    sh -= 1;    // now it is max_y

    int prev_y = -ry - 1    -ry;
    int next_y = ry         -ry;

    for (int y = 0; y < dh; y++)
    {
        int py = SkClampPos(prev_y) * rowBytes;
        int ny = SkFastMin32(next_y, sh) * rowBytes;

        int prev_x = -rx - 1    -rx;
        int next_x = rx         -rx;

        for (int x = 0; x < dw; x++)
        {
            int px = SkClampPos(prev_x);
            int nx = SkFastMin32(next_x, sw);

            uint32_t sum = src[px+py] + src[nx+ny] - src[nx+py] - src[px+ny];
            *dst++ = SkToU8(sum * scale >> 24);

            prev_x += 1;
            next_x += 1;
        }
        prev_y += 1;
        next_y += 1;
    }
}

static void apply_kernel_interp(uint8_t dst[], int rx, int ry, const uint32_t src[], int sw, int sh, U8CPU outer_weight)
{
    SkASSERT(rx > 0 && ry > 0);
    SkASSERT(outer_weight <= 255);

    int inner_weight = 255 - outer_weight;

    // round these guys up if they're bigger than 127
    outer_weight += outer_weight >> 7;
    inner_weight += inner_weight >> 7;

    uint32_t outer_scale = (outer_weight << 16) / ((2*rx + 1)*(2*ry + 1));
    uint32_t inner_scale = (inner_weight << 16) / ((2*rx - 1)*(2*ry - 1));

    int rowBytes = sw;

    int dw = sw + 2*rx;
    int dh = sh + 2*ry;

    sw -= 1;    // now it is max_x
    sh -= 1;    // now it is max_y

    int prev_y = -ry - 1    -ry;
    int next_y = ry         -ry;

    for (int y = 0; y < dh; y++)
    {
        int py = SkClampPos(prev_y) * rowBytes;
        int ny = SkFastMin32(next_y, sh) * rowBytes;

        int ipy = SkClampPos(prev_y + 1) * rowBytes;
        int iny = SkClampMax(next_y - 1, sh) * rowBytes;

        int prev_x = -rx - 1    -rx;
        int next_x = rx         -rx;

        for (int x = 0; x < dw; x++)
        {
            int px = SkClampPos(prev_x);
            int nx = SkFastMin32(next_x, sw);

            int ipx = SkClampPos(prev_x + 1);
            int inx = SkClampMax(next_x - 1, sw);

            uint32_t outer_sum = src[px+py] + src[nx+ny] - src[nx+py] - src[px+ny];
            uint32_t inner_sum = src[ipx+ipy] + src[inx+iny] - src[inx+ipy] - src[ipx+iny];
            *dst++ = SkToU8((outer_sum * outer_scale + inner_sum * inner_scale) >> 24);

            prev_x += 1;
            next_x += 1;
        }
        prev_y += 1;
        next_y += 1;
    }
}

#include "SkColorPriv.h"

static void merge_src_with_blur(uint8_t dst[],
                                const uint8_t src[], int sw, int sh,
                                const uint8_t blur[], int blurRowBytes)
{
    while (--sh >= 0)
    {
        for (int x = sw - 1; x >= 0; --x)
        {
            *dst = SkToU8(SkAlphaMul(*blur, SkAlpha255To256(*src)));
            dst += 1;
            src += 1;
            blur += 1;
        }
        blur += blurRowBytes - sw;
    }
}

static void clamp_with_orig(uint8_t dst[], int dstRowBytes,
                            const uint8_t src[], int sw, int sh,
                            SkBlurMask::Style style)
{
    int x;
    while (--sh >= 0)
    {
        switch (style) {
        case SkBlurMask::kSolid_Style:
            for (x = sw - 1; x >= 0; --x)
            {
                *dst = SkToU8(*src + SkAlphaMul(*dst, SkAlpha255To256(255 - *src)));
                dst += 1;
                src += 1;
            }
            break;
        case SkBlurMask::kOuter_Style:
            for (x = sw - 1; x >= 0; --x)
            {
                if (*src)
                    *dst = SkToU8(SkAlphaMul(*dst, SkAlpha255To256(255 - *src)));
                dst += 1;
                src += 1;
            }
            break;
        default:
            SkASSERT(!"Unexpected blur style here");
            break;
        }
        dst += dstRowBytes - sw;
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
                      SkScalar radius, Style style)
{
    if (src.fFormat != SkMask::kA8_Format)
        return false;

    int rx = SkScalarCeil(radius);
    int outer_weight = 255 - SkScalarRound((SkIntToScalar(rx) - radius) * 255);

    SkASSERT(rx >= 0);
    SkASSERT((unsigned)outer_weight <= 255);

    if (rx == 0)
        return false;

    int ry = rx;    // only do square blur for now

    dst->fBounds.set(src.fBounds.fLeft - rx, src.fBounds.fTop - ry,
                        src.fBounds.fRight + rx, src.fBounds.fBottom + ry);
    dst->fRowBytes = SkToU16(dst->fBounds.width());
    dst->fFormat = SkMask::kA8_Format;
    dst->fImage = NULL;

    if (src.fImage)
    {
        int             sw = src.fBounds.width();
        int             sh = src.fBounds.height();
        const uint8_t*  sp = src.fImage;
        uint8_t*        dp = SkMask::AllocImage(dst->computeImageSize());

        SkAutoTCallVProc<uint8_t, SkMask_FreeImage> autoCall(dp);

        // build the blurry destination
        {
            SkAutoTMalloc<uint32_t> storage(sw * sh);
            uint32_t*               sumBuffer = storage.get();

            build_sum_buffer(sumBuffer, sw, sh, sp, src.fRowBytes);
            if (outer_weight == 255)
                apply_kernel(dp, rx, ry, sumBuffer, sw, sh);
            else
                apply_kernel_interp(dp, rx, ry, sumBuffer, sw, sh, outer_weight);
        }

        dst->fImage = dp;
        // if need be, alloc the "real" dst (same size as src) and copy/merge
        // the blur into it (applying the src)
        if (style == kInner_Style)
        {
            dst->fImage = SkMask::AllocImage(src.computeImageSize());
            merge_src_with_blur(dst->fImage, sp, sw, sh,
                                dp + rx + ry*dst->fBounds.width(),
                                dst->fBounds.width());
            SkMask::FreeImage(dp);
        }
        else if (style != kNormal_Style)
        {
            clamp_with_orig(dp + rx + ry*dst->fBounds.width(),
                            dst->fBounds.width(),
                            sp, sw, sh,
                            style);
        }
        (void)autoCall.detach();
    }

    if (style == kInner_Style)
    {
        dst->fBounds = src.fBounds; // restore trimmed bounds
        dst->fRowBytes = SkToU16(dst->fBounds.width());
    }

#if 0
    if (gamma && dst->fImage)
    {
        uint8_t*    image = dst->fImage;
        uint8_t*    stop = image + dst->computeImageSize();

        for (; image < stop; image += 1)
            *image = gamma[*image];
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

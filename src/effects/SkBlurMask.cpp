
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBlurMask.h"
#include "SkMath.h"
#include "SkTemplates.h"
#include "SkEndian.h"

// Unrolling the integer blur kernel seems to give us a ~15% speedup on Windows,
// breakeven on Mac, and ~15% slowdown on Linux.
// Reading a word at a time when bulding the sum buffer seems to give
// us no appreciable speedup on Windows or Mac, and 2% slowdown on Linux.
#if defined(SK_BUILD_FOR_WIN32)
#define UNROLL_KERNEL_LOOP 1
#endif

/** The sum buffer is an array of u32 to hold the accumulated sum of all of the
    src values at their position, plus all values above and to the left.
    When we sample into this buffer, we need an initial row and column of 0s,
    so we have an index correspondence as follows:
 
    src[i, j] == sum[i+1, j+1]
    sum[0, j] == sum[i, 0] == 0
 
    We assume that the sum buffer's stride == its width
 */
static void build_sum_buffer(uint32_t sum[], int srcW, int srcH,
                             const uint8_t src[], int srcRB) {
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
    for (x = srcW - 1; x >= 0; --x) {
        X = *src++ + X;
        *sum++ = X;
    }
    src += srcRB;

    // now do the rest of the rows
    for (y = srcH - 1; y > 0; --y) {
        uint32_t L = 0;
        uint32_t C = 0;
        *sum++ = 0; // initialze the first column to 0

        for (x = srcW - 1; !SkIsAlign4((intptr_t) src) && x >= 0; x--) {
            uint32_t T = sum[-sumW];
            X = *src++ + L + T - C;
            *sum++ = X;
            L = X;
            C = T;
        }

        for (; x >= 4; x-=4) {
            uint32_t T = sum[-sumW];
            X = *src++ + L + T - C;
            *sum++ = X;
            L = X;
            C = T;
            T = sum[-sumW];
            X = *src++ + L + T - C;
            *sum++ = X;
            L = X;
            C = T;
            T = sum[-sumW];
            X = *src++ + L + T - C;
            *sum++ = X;
            L = X;
            C = T;
            T = sum[-sumW];
            X = *src++ + L + T - C;
            *sum++ = X;
            L = X;
            C = T;
        }

        for (; x >= 0; --x) {
            uint32_t T = sum[-sumW];
            X = *src++ + L + T - C;
            *sum++ = X;
            L = X;
            C = T;
        }
        src += srcRB;
    }
}

/**
 * This is the path for apply_kernel() to be taken when the kernel
 * is wider than the source image.
 */
static void kernel_clamped(uint8_t dst[], int rx, int ry, const uint32_t sum[],
                           int sw, int sh) {
    SkASSERT(2*rx > sw);

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
/**
 *  sw and sh are the width and height of the src. Since the sum buffer
 *  matches that, but has an extra row and col at the beginning (with zeros),
 *  we can just use sw and sh as our "max" values for pinning coordinates
 *  when sampling into sum[][]
 *
 *  The inner loop is conceptually simple; we break it into several sections
 *  to improve performance. Here's the original version:
        for (int x = 0; x < dw; x++) {
            int px = SkClampPos(prev_x);
            int nx = SkFastMin32(next_x, sw);

            uint32_t tmp = sum[px+py] + sum[nx+ny] - sum[nx+py] - sum[px+ny];
            *dst++ = SkToU8(tmp * scale >> 24);

            prev_x += 1;
            next_x += 1;
        }
 *  The sections are:
 *     left-hand section, where prev_x is clamped to 0
 *     center section, where neither prev_x nor next_x is clamped
 *     right-hand section, where next_x is clamped to sw
 *  On some operating systems, the center section is unrolled for additional
 *  speedup.
*/
static void apply_kernel(uint8_t dst[], int rx, int ry, const uint32_t sum[],
                         int sw, int sh) {
    if (2*rx > sw) {
        kernel_clamped(dst, rx, ry, sum, sw, sh);
        return;
    }

    uint32_t scale = (1 << 24) / ((2*rx + 1)*(2*ry + 1));

    int sumStride = sw + 1;

    int dw = sw + 2*rx;
    int dh = sh + 2*ry;

    int prev_y = -2*ry;
    int next_y = 1;

    SkASSERT(2*rx <= dw - 2*rx);

    for (int y = 0; y < dh; y++) {
        int py = SkClampPos(prev_y) * sumStride;
        int ny = SkFastMin32(next_y, sh) * sumStride;

        int prev_x = -2*rx;
        int next_x = 1;
        int x = 0;

        for (; x < 2*rx; x++) {
            SkASSERT(prev_x <= 0);
            SkASSERT(next_x <= sw);

            int px = 0;
            int nx = next_x;

            uint32_t tmp = sum[px+py] + sum[nx+ny] - sum[nx+py] - sum[px+ny];
            *dst++ = SkToU8(tmp * scale >> 24);

            prev_x += 1;
            next_x += 1;
        }

        int i0 = prev_x + py;
        int i1 = next_x + ny;
        int i2 = next_x + py;
        int i3 = prev_x + ny;

#if UNROLL_KERNEL_LOOP
        for (; x < dw - 2*rx - 4; x += 4) {
            SkASSERT(prev_x >= 0);
            SkASSERT(next_x <= sw);

            uint32_t tmp = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            *dst++ = SkToU8(tmp * scale >> 24);
            tmp = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            *dst++ = SkToU8(tmp * scale >> 24);
            tmp = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            *dst++ = SkToU8(tmp * scale >> 24);
            tmp = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            *dst++ = SkToU8(tmp * scale >> 24);

            prev_x += 4;
            next_x += 4;
        }
#endif

        for (; x < dw - 2*rx; x++) {
            SkASSERT(prev_x >= 0);
            SkASSERT(next_x <= sw);

            uint32_t tmp = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            *dst++ = SkToU8(tmp * scale >> 24);

            prev_x += 1;
            next_x += 1;
        }

        for (; x < dw; x++) {
            SkASSERT(prev_x >= 0);
            SkASSERT(next_x > sw);

            int px = prev_x;
            int nx = sw;

            uint32_t tmp = sum[px+py] + sum[nx+ny] - sum[nx+py] - sum[px+ny];
            *dst++ = SkToU8(tmp * scale >> 24);

            prev_x += 1;
            next_x += 1;
        }

        prev_y += 1;
        next_y += 1;
    }
}

/**
 * This is the path for apply_kernel_interp() to be taken when the kernel
 * is wider than the source image.
 */
static void kernel_interp_clamped(uint8_t dst[], int rx, int ry,
                const uint32_t sum[], int sw, int sh, U8CPU outer_weight) {
    SkASSERT(2*rx > sw);

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

            uint32_t outer_sum = sum[px+py] + sum[nx+ny]
                               - sum[nx+py] - sum[px+ny];
            uint32_t inner_sum = sum[ipx+ipy] + sum[inx+iny]
                               - sum[inx+ipy] - sum[ipx+iny];
            *dst++ = SkToU8((outer_sum * outer_scale
                           + inner_sum * inner_scale) >> 24);

            prev_x += 1;
            next_x += 1;
        }
        prev_y += 1;
        next_y += 1;
    }
}

/**
 *  sw and sh are the width and height of the src. Since the sum buffer
 *  matches that, but has an extra row and col at the beginning (with zeros),
 *  we can just use sw and sh as our "max" values for pinning coordinates
 *  when sampling into sum[][]
 *
 *  The inner loop is conceptually simple; we break it into several variants
 *  to improve performance. Here's the original version:
        for (int x = 0; x < dw; x++) {
            int px = SkClampPos(prev_x);
            int nx = SkFastMin32(next_x, sw);

            int ipx = SkClampPos(prev_x + 1);
            int inx = SkClampMax(next_x - 1, sw);

            uint32_t outer_sum = sum[px+py] + sum[nx+ny]
                               - sum[nx+py] - sum[px+ny];
            uint32_t inner_sum = sum[ipx+ipy] + sum[inx+iny]
                               - sum[inx+ipy] - sum[ipx+iny];
            *dst++ = SkToU8((outer_sum * outer_scale
                           + inner_sum * inner_scale) >> 24);

            prev_x += 1;
            next_x += 1;
        }
 *  The sections are:
 *     left-hand section, where prev_x is clamped to 0
 *     center section, where neither prev_x nor next_x is clamped
 *     right-hand section, where next_x is clamped to sw
 *  On some operating systems, the center section is unrolled for additional
 *  speedup.
*/
static void apply_kernel_interp(uint8_t dst[], int rx, int ry,
                const uint32_t sum[], int sw, int sh, U8CPU outer_weight) {
    SkASSERT(rx > 0 && ry > 0);
    SkASSERT(outer_weight <= 255);

    if (2*rx > sw) {
        kernel_interp_clamped(dst, rx, ry, sum, sw, sh, outer_weight);
        return;
    }

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

    SkASSERT(2*rx <= dw - 2*rx);

    for (int y = 0; y < dh; y++) {
        int py = SkClampPos(prev_y) * sumStride;
        int ny = SkFastMin32(next_y, sh) * sumStride;

        int ipy = SkClampPos(prev_y + 1) * sumStride;
        int iny = SkClampMax(next_y - 1, sh) * sumStride;

        int prev_x = -2*rx;
        int next_x = 1;
        int x = 0;

        for (; x < 2*rx; x++) {
            SkASSERT(prev_x < 0);
            SkASSERT(next_x <= sw);

            int px = 0;
            int nx = next_x;

            int ipx = 0;
            int inx = next_x - 1;

            uint32_t outer_sum = sum[px+py] + sum[nx+ny]
                               - sum[nx+py] - sum[px+ny];
            uint32_t inner_sum = sum[ipx+ipy] + sum[inx+iny]
                               - sum[inx+ipy] - sum[ipx+iny];
            *dst++ = SkToU8((outer_sum * outer_scale
                           + inner_sum * inner_scale) >> 24);

            prev_x += 1;
            next_x += 1;
        }

        int i0 = prev_x + py;
        int i1 = next_x + ny;
        int i2 = next_x + py;
        int i3 = prev_x + ny;
        int i4 = prev_x + 1 + ipy;
        int i5 = next_x - 1 + iny;
        int i6 = next_x - 1 + ipy;
        int i7 = prev_x + 1 + iny;

#if UNROLL_KERNEL_LOOP
        for (; x < dw - 2*rx - 4; x += 4) {
            SkASSERT(prev_x >= 0);
            SkASSERT(next_x <= sw);

            uint32_t outer_sum = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            uint32_t inner_sum = sum[i4++] + sum[i5++] - sum[i6++] - sum[i7++];
            *dst++ = SkToU8((outer_sum * outer_scale
                           + inner_sum * inner_scale) >> 24);
            outer_sum = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            inner_sum = sum[i4++] + sum[i5++] - sum[i6++] - sum[i7++];
            *dst++ = SkToU8((outer_sum * outer_scale
                           + inner_sum * inner_scale) >> 24);
            outer_sum = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            inner_sum = sum[i4++] + sum[i5++] - sum[i6++] - sum[i7++];
            *dst++ = SkToU8((outer_sum * outer_scale
                           + inner_sum * inner_scale) >> 24);
            outer_sum = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            inner_sum = sum[i4++] + sum[i5++] - sum[i6++] - sum[i7++];
            *dst++ = SkToU8((outer_sum * outer_scale
                           + inner_sum * inner_scale) >> 24);

            prev_x += 4;
            next_x += 4;
        }
#endif

        for (; x < dw - 2*rx; x++) {
            SkASSERT(prev_x >= 0);
            SkASSERT(next_x <= sw);

            uint32_t outer_sum = sum[i0++] + sum[i1++] - sum[i2++] - sum[i3++];
            uint32_t inner_sum = sum[i4++] + sum[i5++] - sum[i6++] - sum[i7++];
            *dst++ = SkToU8((outer_sum * outer_scale
                           + inner_sum * inner_scale) >> 24);

            prev_x += 1;
            next_x += 1;
        }

        for (; x < dw; x++) {
            SkASSERT(prev_x >= 0);
            SkASSERT(next_x > sw);

            int px = prev_x;
            int nx = sw;

            int ipx = prev_x + 1;
            int inx = sw;

            uint32_t outer_sum = sum[px+py] + sum[nx+ny]
                               - sum[nx+py] - sum[px+ny];
            uint32_t inner_sum = sum[ipx+ipy] + sum[inx+iny]
                               - sum[inx+ipy] - sum[ipx+iny];
            *dst++ = SkToU8((outer_sum * outer_scale
                           + inner_sum * inner_scale) >> 24);

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
            SkDEBUGFAIL("Unexpected blur style here");
            break;
        }
        dst += dstRowBytes - sw;
        src += srcRowBytes - sw;
    }
}

///////////////////////////////////////////////////////////////////////////////

// we use a local funciton to wrap the class static method to work around
// a bug in gcc98
void SkMask_FreeImage(uint8_t* image);
void SkMask_FreeImage(uint8_t* image) {
    SkMask::FreeImage(image);
}

bool SkBlurMask::Blur(SkMask* dst, const SkMask& src,
                      SkScalar radius, Style style, Quality quality,
                      SkIPoint* margin)
{
    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

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
    if (margin) {
        margin->set(padx, pady);
    }
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
            const size_t storageW = sw + 2 * (passCount - 1) * rx + 1;
            const size_t storageH = sh + 2 * (passCount - 1) * ry + 1;
            SkAutoTMalloc<uint32_t> storage(storageW * storageH);
            uint32_t*               sumBuffer = storage.get();

            //pass1: sp is source, dp is destination
            build_sum_buffer(sumBuffer, sw, sh, sp, src.fRowBytes);
            if (outer_weight == 255) {
                apply_kernel(dp, rx, ry, sumBuffer, sw, sh);
            } else {
                apply_kernel_interp(dp, rx, ry, sumBuffer, sw, sh, outer_weight);
            }

            if (quality == kHigh_Quality) {
                //pass2: dp is source, tmpBuffer is destination
                int tmp_sw = sw + 2 * rx;
                int tmp_sh = sh + 2 * ry;
                SkAutoTMalloc<uint8_t>  tmpBuffer(dstSize);
                build_sum_buffer(sumBuffer, tmp_sw, tmp_sh, dp, tmp_sw);
                if (outer_weight == 255)
                    apply_kernel(tmpBuffer.get(), rx, ry, sumBuffer, tmp_sw, tmp_sh);
                else
                    apply_kernel_interp(tmpBuffer.get(), rx, ry, sumBuffer,
                                        tmp_sw, tmp_sh, outer_weight);

                //pass3: tmpBuffer is source, dp is destination
                tmp_sw += 2 * rx;
                tmp_sh += 2 * ry;
                build_sum_buffer(sumBuffer, tmp_sw, tmp_sh, tmpBuffer.get(), tmp_sw);
                if (outer_weight == 255)
                    apply_kernel(dp, rx, ry, sumBuffer, tmp_sw, tmp_sh);
                else
                    apply_kernel_interp(dp, rx, ry, sumBuffer, tmp_sw, tmp_sh,
                                        outer_weight);
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
                                dp + passCount * (rx + ry * dst->fRowBytes),
                                dst->fRowBytes, sw, sh);
            SkMask::FreeImage(dp);
        } else if (style != kNormal_Style) {
            clamp_with_orig(dp + passCount * (rx + ry * dst->fRowBytes),
                            dst->fRowBytes, sp, src.fRowBytes, sw, sh, style);
        }
        (void)autoCall.detach();
    }

    if (style == kInner_Style) {
        dst->fBounds = src.fBounds; // restore trimmed bounds
        dst->fRowBytes = src.fRowBytes;
    }

    return true;
}


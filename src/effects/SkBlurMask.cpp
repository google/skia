
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

// scale factor for the blur radius to match the behavior of the all existing blur
// code (both on the CPU and the GPU).  This magic constant is  1/sqrt(3).

// TODO: get rid of this fudge factor and move any required fudging up into
// the calling library

#define kBlurRadiusFudgeFactor SkFloatToScalar( .57735f )

#define UNROLL_SEPARABLE_LOOPS

/**
 * This function performs a box blur in X, of the given radius.  If the
 * "transpose" parameter is true, it will transpose the pixels on write,
 * such that X and Y are swapped. Reads are always performed from contiguous
 * memory in X, for speed. The destination buffer (dst) must be at least
 * (width + leftRadius + rightRadius) * height bytes in size.
 */
static int boxBlur(const uint8_t* src, int src_y_stride, uint8_t* dst,
                   int leftRadius, int rightRadius, int width, int height,
                   bool transpose)
{
    int diameter = leftRadius + rightRadius;
    int kernelSize = diameter + 1;
    int border = SkMin32(width, diameter);
    uint32_t scale = (1 << 24) / kernelSize;
    int new_width = width + SkMax32(leftRadius, rightRadius) * 2;
    int dst_x_stride = transpose ? height : 1;
    int dst_y_stride = transpose ? 1 : new_width;
    for (int y = 0; y < height; ++y) {
        int sum = 0;
        uint8_t* dptr = dst + y * dst_y_stride;
        const uint8_t* right = src + y * src_y_stride;
        const uint8_t* left = right;
        for (int x = 0; x < rightRadius - leftRadius; x++) {
            *dptr = 0;
            dptr += dst_x_stride;
        }
#define LEFT_BORDER_ITER \
            sum += *right++; \
            *dptr = (sum * scale) >> 24; \
            dptr += dst_x_stride;

        int x = 0;
#ifdef UNROLL_SEPARABLE_LOOPS
        for (; x < border - 16; x += 16) {
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
        }
#endif
        for (; x < border; ++x) {
            LEFT_BORDER_ITER
        }
#undef LEFT_BORDER_ITER
#define TRIVIAL_ITER \
            *dptr = (sum * scale) >> 24; \
            dptr += dst_x_stride;
        x = width;
#ifdef UNROLL_SEPARABLE_LOOPS
        for (; x < diameter - 16; x += 16) {
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
            TRIVIAL_ITER
        }
#endif
        for (; x < diameter; ++x) {
            TRIVIAL_ITER
        }
#undef TRIVIAL_ITER
#define CENTER_ITER \
            sum += *right++; \
            *dptr = (sum * scale) >> 24; \
            sum -= *left++; \
            dptr += dst_x_stride;

        x = diameter;
#ifdef UNROLL_SEPARABLE_LOOPS
        for (; x < width - 16; x += 16) {
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
        }
#endif
        for (; x < width; ++x) {
            CENTER_ITER
        }
#undef CENTER_ITER
#define RIGHT_BORDER_ITER \
            *dptr = (sum * scale) >> 24; \
            sum -= *left++; \
            dptr += dst_x_stride;

        x = 0;
#ifdef UNROLL_SEPARABLE_LOOPS
        for (; x < border - 16; x += 16) {
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
        }
#endif
        for (; x < border; ++x) {
            RIGHT_BORDER_ITER
        }
#undef RIGHT_BORDER_ITER
        for (int x = 0; x < leftRadius - rightRadius; x++) {
            *dptr = 0;
            dptr += dst_x_stride;
        }
        SkASSERT(sum == 0);
    }
    return new_width;
}

/**
 * This variant of the box blur handles blurring of non-integer radii.  It
 * keeps two running sums: an outer sum for the rounded-up kernel radius, and
 * an inner sum for the rounded-down kernel radius.  For each pixel, it linearly
 * interpolates between them.  In float this would be:
 *  outer_weight * outer_sum / kernelSize +
 *  (1.0 - outer_weight) * innerSum / (kernelSize - 2)
 */
static int boxBlurInterp(const uint8_t* src, int src_y_stride, uint8_t* dst,
                         int radius, int width, int height,
                         bool transpose, uint8_t outer_weight)
{
    int diameter = radius * 2;
    int kernelSize = diameter + 1;
    int border = SkMin32(width, diameter);
    int inner_weight = 255 - outer_weight;
    outer_weight += outer_weight >> 7;
    inner_weight += inner_weight >> 7;
    uint32_t outer_scale = (outer_weight << 16) / kernelSize;
    uint32_t inner_scale = (inner_weight << 16) / (kernelSize - 2);
    int new_width = width + diameter;
    int dst_x_stride = transpose ? height : 1;
    int dst_y_stride = transpose ? 1 : new_width;
    for (int y = 0; y < height; ++y) {
        int outer_sum = 0, inner_sum = 0;
        uint8_t* dptr = dst + y * dst_y_stride;
        const uint8_t* right = src + y * src_y_stride;
        const uint8_t* left = right;
        int x = 0;

#define LEFT_BORDER_ITER \
            inner_sum = outer_sum; \
            outer_sum += *right++; \
            *dptr = (outer_sum * outer_scale + inner_sum * inner_scale) >> 24; \
            dptr += dst_x_stride;

#ifdef UNROLL_SEPARABLE_LOOPS
        for (;x < border - 16; x += 16) {
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
            LEFT_BORDER_ITER
        }
#endif

        for (;x < border; x++) {
            LEFT_BORDER_ITER
        }
#undef LEFT_BORDER_ITER
        for (int x = width; x < diameter; ++x) {
            *dptr = (outer_sum * outer_scale + inner_sum * inner_scale) >> 24;
            dptr += dst_x_stride;
        }
        x = diameter;

#define CENTER_ITER \
            inner_sum = outer_sum - *left; \
            outer_sum += *right++; \
            *dptr = (outer_sum * outer_scale + inner_sum * inner_scale) >> 24; \
            dptr += dst_x_stride; \
            outer_sum -= *left++;

#ifdef UNROLL_SEPARABLE_LOOPS
        for (; x < width - 16; x += 16) {
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
            CENTER_ITER
        }
#endif
        for (; x < width; ++x) {
            CENTER_ITER
        }
#undef CENTER_ITER

        #define RIGHT_BORDER_ITER \
            inner_sum = outer_sum - *left++; \
            *dptr = (outer_sum * outer_scale + inner_sum * inner_scale) >> 24; \
            dptr += dst_x_stride; \
            outer_sum = inner_sum;

        x = 0;
#ifdef UNROLL_SEPARABLE_LOOPS
        for (; x < border - 16; x += 16) {
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
            RIGHT_BORDER_ITER
        }
#endif
        for (; x < border; x++) {
            RIGHT_BORDER_ITER
        }
#undef RIGHT_BORDER_ITER
        SkASSERT(outer_sum == 0 && inner_sum == 0);
    }
    return new_width;
}

static void get_adjusted_radii(SkScalar passRadius, int *loRadius, int *hiRadius)
{
    *loRadius = *hiRadius = SkScalarCeil(passRadius);
    if (SkIntToScalar(*hiRadius) - passRadius > SkFloatToScalar(0.5f)) {
        *loRadius = *hiRadius - 1;
    }
}

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

// we use a local function to wrap the class static method to work around
// a bug in gcc98
void SkMask_FreeImage(uint8_t* image);
void SkMask_FreeImage(uint8_t* image) {
    SkMask::FreeImage(image);
}

bool SkBlurMask::Blur(SkMask* dst, const SkMask& src,
                      SkScalar radius, Style style, Quality quality,
                      SkIPoint* margin, bool separable)
{
    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    // Force high quality off for small radii (performance)
    if (radius < SkIntToScalar(3)) {
        quality = kLow_Quality;
    }

    // highQuality: use three box blur passes as a cheap way to approximate a Gaussian blur
    int passCount = (kHigh_Quality == quality) ? 3 : 1;
    SkScalar passRadius = (kHigh_Quality == quality) ? SkScalarMul( radius, kBlurRadiusFudgeFactor): radius;

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
        if (separable) {
            SkAutoTMalloc<uint8_t>  tmpBuffer(dstSize);
            uint8_t*                tp = tmpBuffer.get();
            int w = sw, h = sh;

            if (outer_weight == 255) {
                int loRadius, hiRadius;
                get_adjusted_radii(passRadius, &loRadius, &hiRadius);
                if (kHigh_Quality == quality) {
                    // Do three X blurs, with a transpose on the final one.
                    w = boxBlur(sp, src.fRowBytes, tp, loRadius, hiRadius, w, h, false);
                    w = boxBlur(tp, w,             dp, hiRadius, loRadius, w, h, false);
                    w = boxBlur(dp, w,             tp, hiRadius, hiRadius, w, h, true);
                    // Do three Y blurs, with a transpose on the final one.
                    h = boxBlur(tp, h,             dp, loRadius, hiRadius, h, w, false);
                    h = boxBlur(dp, h,             tp, hiRadius, loRadius, h, w, false);
                    h = boxBlur(tp, h,             dp, hiRadius, hiRadius, h, w, true);
                } else {
                    w = boxBlur(sp, src.fRowBytes, tp, rx, rx, w, h, true);
                    h = boxBlur(tp, h,             dp, ry, ry, h, w, true);
                }
            } else {
                if (kHigh_Quality == quality) {
                    // Do three X blurs, with a transpose on the final one.
                    w = boxBlurInterp(sp, src.fRowBytes, tp, rx, w, h, false, outer_weight);
                    w = boxBlurInterp(tp, w,             dp, rx, w, h, false, outer_weight);
                    w = boxBlurInterp(dp, w,             tp, rx, w, h, true, outer_weight);
                    // Do three Y blurs, with a transpose on the final one.
                    h = boxBlurInterp(tp, h,             dp, ry, h, w, false, outer_weight);
                    h = boxBlurInterp(dp, h,             tp, ry, h, w, false, outer_weight);
                    h = boxBlurInterp(tp, h,             dp, ry, h, w, true, outer_weight);
                } else {
                    w = boxBlurInterp(sp, src.fRowBytes, tp, rx, w, h, true, outer_weight);
                    h = boxBlurInterp(tp, h,             dp, ry, h, w, true, outer_weight);
                }
            }
        } else {
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

            if (kHigh_Quality == quality) {
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

bool SkBlurMask::BlurSeparable(SkMask* dst, const SkMask& src,
                               SkScalar radius, Style style, Quality quality,
                               SkIPoint* margin)
{
    return SkBlurMask::Blur(dst, src, radius, style, quality, margin, true);
}

bool SkBlurMask::Blur(SkMask* dst, const SkMask& src,
                     SkScalar radius, Style style, Quality quality,
                     SkIPoint* margin)
{
    return SkBlurMask::Blur(dst, src, radius, style, quality, margin, false);
}

/* Convolving a box with itself three times results in a piecewise
   quadratic function:

   0                              x <= -1.5
   9/8 + 3/2 x + 1/2 x^2   -1.5 < x <= 1.5
   3/4 - x^2                -.5 < x <= .5
   9/8 - 3/2 x + 1/2 x^2    0.5 < x <= 1.5
   0                        1.5 < x

   To get the profile curve of the blurred step function at the rectangle
   edge, we evaluate the indefinite integral, which is piecewise cubic:

   0                                        x <= -1.5
   5/8 + 9/8 x + 3/4 x^2 + 1/6 x^3   -1.5 < x <= -0.5
   1/2 + 3/4 x - 1/3 x^3              -.5 < x <= .5
   3/8 + 9/8 x - 3/4 x^2 + 1/6 x^3     .5 < x <= 1.5
   1                                  1.5 < x
*/

static float gaussian_integral( float x ) {
    if ( x > 1.5f ) {
        return 0.0f;
    }
    if ( x < -1.5f ) {
        return 1.0f;
    }

    float x2 = x*x;
    float x3 = x2*x;

    if ( x > 0.5f ) {
        return 0.5625f - ( x3 / 6.0f - 3.0f * x2 * 0.25f + 1.125f * x);
    }
    if ( x > -0.5f ) {
        return 0.5f - (0.75f * x - x3 / 3.0f);
    }
    return 0.4375f + (-x3 / 6.0f - 3.0f * x2 * 0.25f - 1.125f * x);
}

/*
    compute_profile allocates and fills in an array of floating
    point values between 0 and 255 for the profile signature of
    a blurred half-plane with the given blur radius.  Since we're
    going to be doing screened multiplications (i.e., 1 - (1-x)(1-y))
    all the time, we actually fill in the profile pre-inverted
    (already done 255-x).

    The function returns the size of the array allocated for the
    profile.  It's the responsibility of the caller to delete the
    memory returned in profile_out.
*/

static int compute_profile( SkScalar radius, unsigned int **profile_out ) {
    int size = SkScalarFloorToInt(radius * 3 + 1);
    int center = size >> 1;

    unsigned int *profile = SkNEW_ARRAY(unsigned int, size);

    float invr = 1.0f/radius;

    profile[0] = 255;
    for (int x = 1 ; x < size ; x++) {
        float scaled_x = ( center - x ) * invr;
        float gi = gaussian_integral( scaled_x );
        profile[x] = 255 - (uint8_t) ( 255.f * gi );
    }

    *profile_out = profile;
    return size;
}

// TODO MAYBE: Maintain a profile cache to avoid recomputing this for
// commonly used radii.  Consider baking some of the most common blur radii
// directly in as static data?

// Implementation adapted from Michael Herf's approach:
// http://stereopsis.com/shadowrect/

bool SkBlurMask::BlurRect(SkMask *dst, const SkRect &src,
                          SkScalar provided_radius, Style style, Quality quality,
                          SkIPoint *margin) {
    int profile_size;
    unsigned int *profile;


    float radius = SkScalarToFloat( SkScalarMul( provided_radius, kBlurRadiusFudgeFactor ) );

    profile_size = compute_profile( radius, &profile );
    SkAutoTDeleteArray<unsigned int> ada(profile);

    int pad = (int) (radius * 1.5f + 1);
    if (margin) {
        margin->set( pad, pad );
    }
    dst->fBounds = SkIRect::MakeWH(SkScalarFloorToInt(src.width()), SkScalarFloorToInt(src.height()));
    dst->fBounds.outset(pad, pad);

    dst->fRowBytes = dst->fBounds.width();
    dst->fFormat = SkMask::kA8_Format;
    dst->fImage = NULL;

    size_t dstSize = dst->computeImageSize();
    if (0 == dstSize) {
        return false;   // too big to allocate, abort
    }

    int             sw = SkScalarFloorToInt(src.width());
    int             sh = SkScalarFloorToInt(src.height());

    uint8_t*        dp = SkMask::AllocImage(dstSize);

    dst->fImage = dp;

    int dst_height = dst->fBounds.height();
    int dst_width = dst->fBounds.width();

    // nearest odd number less than the profile size represents the center
    // of the (2x scaled) profile
    int center = ( profile_size & ~1 ) - 1;

    int w = sw - center;
    int h = sh - center;

    uint8_t *outptr = dp;

    for (int y = 0 ; y < dst_height ; y++)
    {
        // time to fill in a scanline of the blurry rectangle.
        // to avoid floating point math, everything is multiplied by
        // 2 where needed.  This keeps things nice and integer-oriented.

        int dy = abs((y << 1) - dst_height) - h; // how far are we from the original edge?
        int oy = dy >> 1;
        if (oy < 0) oy = 0;

        unsigned int profile_y = profile[oy];

        for (int x = 0 ; x < (dst_width << 1) ; x += 2) {
            int dx = abs( x - dst_width ) - w;
            int ox = dx >> 1;
            if (ox < 0) ox = 0;

            unsigned int maskval = SkMulDiv255Round(profile[ox], profile_y);

            *(outptr++) = maskval;
        }
    }

    return true;
}

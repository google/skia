/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBlurEngine.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h" // IWYU pragma: keep
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFeatures.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkVx.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkDevice.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/core/SkSpecialImage.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <utility>


#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    #include <xmmintrin.h>
    #define SK_PREFETCH(ptr) _mm_prefetch(reinterpret_cast<const char*>(ptr), _MM_HINT_T0)
#elif defined(__GNUC__)
    #define SK_PREFETCH(ptr) __builtin_prefetch(ptr)
#else
    #define SK_PREFETCH(ptr)
#endif

// RasterBlurEngine
// ----------------------------------------------------------------------------

namespace {

class Pass {
public:
    explicit Pass(int border) : fBorder(border) {}
    virtual ~Pass() = default;

    void blur(int srcLeft, int srcRight, int dstRight,
              const uint32_t* src, int srcStride,
              uint32_t* dst, int dstStride) {
        this->startBlur();

        auto srcStart = srcLeft - fBorder,
                srcEnd   = srcRight - fBorder,
                dstEnd   = dstRight,
                srcIdx   = srcStart,
                dstIdx   = 0;

        const uint32_t* srcCursor = src;
        uint32_t* dstCursor = dst;

        if (dstIdx < srcIdx) {
            // The destination pixels are not effected by the src pixels,
            // change to zero as per the spec.
            // https://drafts.fxtf.org/filter-effects/#FilterPrimitivesOverviewIntro
            int commonEnd = std::min(srcIdx, dstEnd);
            while (dstIdx < commonEnd) {
                *dstCursor = 0;
                dstCursor += dstStride;
                SK_PREFETCH(dstCursor);
                dstIdx++;
            }
        } else if (srcIdx < dstIdx) {
            // The edge of the source is before the edge of the destination. Calculate the sums for
            // the pixels before the start of the destination.
            if (int commonEnd = std::min(dstIdx, srcEnd); srcIdx < commonEnd) {
                // Preload the blur with values from src before dst is entered.
                int n = commonEnd - srcIdx;
                this->blurSegment(n, srcCursor, srcStride, nullptr, 0);
                srcIdx += n;
                srcCursor += n * srcStride;
            }
            if (srcIdx < dstIdx) {
                // The weird case where src is out of pixels before dst is even started.
                int n = dstIdx - srcIdx;
                this->blurSegment(n, nullptr, 0, nullptr, 0);
                srcIdx += n;
            }
        }

        if (int commonEnd = std::min(dstEnd, srcEnd); dstIdx < commonEnd) {
            // Both srcIdx and dstIdx are in sync now, and can run in a 1:1 fashion. This is the
            // normal mode of operation.
            SkASSERT(srcIdx == dstIdx);

            int n = commonEnd - dstIdx;
            this->blurSegment(n, srcCursor, srcStride, dstCursor, dstStride);
            srcCursor += n * srcStride;
            dstCursor += n * dstStride;
            dstIdx += n;
            srcIdx += n;
        }

        // Drain the remaining blur values into dst assuming 0's for the leading edge.
        if (dstIdx < dstEnd) {
            int n = dstEnd - dstIdx;
            this->blurSegment(n, nullptr, 0, dstCursor, dstStride);
        }
    }

protected:
    virtual void startBlur() = 0;
    virtual void blurSegment(
            int n, const uint32_t* src, int srcStride, uint32_t* dst, int dstStride) = 0;

private:
    const int fBorder;
};

class PassMaker {
public:
    explicit PassMaker(int window) : fWindow{window} {}
    virtual ~PassMaker() = default;
    virtual Pass* makePass(void* buffer, SkArenaAlloc* alloc) const = 0;
    virtual size_t bufferSizeBytes() const = 0;
    int window() const {return fWindow;}

private:
    const int fWindow;
};

// Implement a scanline processor that uses a three-box filter to approximate a Gaussian blur.
// The GaussPass is limit to processing sigmas < 135.
class GaussPass final : public Pass {
public:
    // NB 136 is the largest sigma that will not cause a buffer full of 255 mask values to overflow
    // using the Gauss filter. It also limits the size of buffers used hold intermediate values.
    // Explanation of maximums:
    //   sum0 = window * 255
    //   sum1 = window * sum0 -> window * window * 255
    //   sum2 = window * sum1 -> window * window * window * 255 -> window^3 * 255
    //
    //   The value window^3 * 255 must fit in a uint32_t. So,
    //      window^3 < 2^32. window = 255.
    //
    //   window = floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5)
    //   For window <= 255, the largest value for sigma is 136.
    static PassMaker* MakeMaker(float sigma, SkArenaAlloc* alloc) {
        SkASSERT(0 <= sigma);
        int window = SkBlurEngine::BoxBlurWindow(sigma);
        if (255 <= window) {
            return nullptr;
        }

        class Maker : public PassMaker {
        public:
            explicit Maker(int window) : PassMaker{window} {}
            Pass* makePass(void* buffer, SkArenaAlloc* alloc) const override {
                return GaussPass::Make(this->window(), buffer, alloc);
            }

            size_t bufferSizeBytes() const override {
                int window = this->window();
                size_t onePassSize = window - 1;
                // If the window is odd, then there is an obvious middle element. For even sizes
                // 2 passes are shifted, and the last pass has an extra element. Like this:
                //       S
                //    aaaAaa
                //     bbBbbb
                //    cccCccc
                //       D
                size_t bufferCount = (window & 1) == 1 ? 3 * onePassSize : 3 * onePassSize + 1;
                return bufferCount * sizeof(skvx::Vec<4, uint32_t>);
            }
        };

        return alloc->make<Maker>(window);
    }

    static GaussPass* Make(int window, void* buffers, SkArenaAlloc* alloc) {
        // We don't need to store the trailing edge pixel in the buffer;
        int passSize = window - 1;
        skvx::Vec<4, uint32_t>* buffer0 = static_cast<skvx::Vec<4, uint32_t>*>(buffers);
        skvx::Vec<4, uint32_t>* buffer1 = buffer0 + passSize;
        skvx::Vec<4, uint32_t>* buffer2 = buffer1 + passSize;
        // If the window is odd just one buffer is needed, but if it's even, then there is one
        // more element on that pass.
        skvx::Vec<4, uint32_t>* buffersEnd = buffer2 + ((window & 1) ? passSize : passSize + 1);

        // Calculating the border is tricky. The border is the distance in pixels between the first
        // dst pixel and the first src pixel (or the last src pixel and the last dst pixel).
        // I will go through the odd case which is simpler, and then through the even case. Given a
        // stack of filters seven wide for the odd case of three passes.
        //
        //        S
        //     aaaAaaa
        //     bbbBbbb
        //     cccCccc
        //        D
        //
        // The furthest changed pixel is when the filters are in the following configuration.
        //
        //                 S
        //           aaaAaaa
        //        bbbBbbb
        //     cccCccc
        //        D
        //
        // The A pixel is calculated using the value S, the B uses A, and the C uses B, and
        // finally D is C. So, with a window size of seven the border is nine. In the odd case, the
        // border is 3*((window - 1)/2).
        //
        // For even cases the filter stack is more complicated. The spec specifies two passes
        // of even filters and a final pass of odd filters. A stack for a width of six looks like
        // this.
        //
        //       S
        //    aaaAaa
        //     bbBbbb
        //    cccCccc
        //       D
        //
        // The furthest pixel looks like this.
        //
        //               S
        //          aaaAaa
        //        bbBbbb
        //    cccCccc
        //       D
        //
        // For a window of six, the border value is eight. In the even case the border is 3 *
        // (window/2) - 1.
        int border = (window & 1) == 1 ? 3 * ((window - 1) / 2) : 3 * (window / 2) - 1;

        // If the window is odd then the divisor is just window ^ 3 otherwise,
        // it is window * window * (window + 1) = window ^ 3 + window ^ 2;
        int window2 = window * window;
        int window3 = window2 * window;
        int divisor = (window & 1) == 1 ? window3 : window3 + window2;
        return alloc->make<GaussPass>(buffer0, buffer1, buffer2, buffersEnd, border, divisor);
    }

    GaussPass(skvx::Vec<4, uint32_t>* buffer0,
              skvx::Vec<4, uint32_t>* buffer1,
              skvx::Vec<4, uint32_t>* buffer2,
              skvx::Vec<4, uint32_t>* buffersEnd,
              int border,
              int divisor)
        : Pass{border}
        , fBuffer0{buffer0}
        , fBuffer1{buffer1}
        , fBuffer2{buffer2}
        , fBuffersEnd{buffersEnd}
        , fDivider(divisor) {}

private:
    void startBlur() override {
        skvx::Vec<4, uint32_t> zero = {0u, 0u, 0u, 0u};
        zero.store(fSum0);
        zero.store(fSum1);
        auto half = fDivider.half();
        skvx::Vec<4, uint32_t>{half, half, half, half}.store(fSum2);
        sk_bzero(fBuffer0, (fBuffersEnd - fBuffer0) * sizeof(skvx::Vec<4, uint32_t>));

        fBuffer0Cursor = fBuffer0;
        fBuffer1Cursor = fBuffer1;
        fBuffer2Cursor = fBuffer2;
    }

    // GaussPass implements the common three pass box filter approximation of Gaussian blur,
    // but combines all three passes into a single pass. This approach is facilitated by three
    // circular buffers the width of the window which track values for trailing edges of each of
    // the three passes. This allows the algorithm to use more precision in the calculation
    // because the values are not rounded each pass. And this implementation also avoids a trap
    // that's easy to fall into resulting in blending in too many zeroes near the edge.
    //
    // In general, a window sum has the form:
    //     sum_n+1 = sum_n + leading_edge - trailing_edge.
    // If instead we do the subtraction at the end of the previous iteration, we can just
    // calculate the sums instead of having to do the subtractions too.
    //
    //      In previous iteration:
    //      sum_n+1 = sum_n - trailing_edge.
    //
    //      In this iteration:
    //      sum_n+1 = sum_n + leading_edge.
    //
    // Now we can stack all three sums and do them at once. Sum0 gets its leading edge from the
    // actual data. Sum1's leading edge is just Sum0, and Sum2's leading edge is Sum1. So, doing the
    // three passes at the same time has the form:
    //
    //    sum0_n+1 = sum0_n + leading edge
    //    sum1_n+1 = sum1_n + sum0_n+1
    //    sum2_n+1 = sum2_n + sum1_n+1
    //
    //    sum2_n+1 / window^3 is the new value of the destination pixel.
    //
    // Reduce the sums by the trailing edges which were stored in the circular buffers for the
    // next go around. This is the case for odd sized windows, even windows the the third
    // circular buffer is one larger then the first two circular buffers.
    //
    //    sum2_n+2 = sum2_n+1 - buffer2[i];
    //    buffer2[i] = sum1;
    //    sum1_n+2 = sum1_n+1 - buffer1[i];
    //    buffer1[i] = sum0;
    //    sum0_n+2 = sum0_n+1 - buffer0[i];
    //    buffer0[i] = leading edge
    void blurSegment(
            int n, const uint32_t* src, int srcStride, uint32_t* dst, int dstStride) override {
#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
        skvx::Vec<4, uint32_t>* buffer0Cursor = fBuffer0Cursor;
        skvx::Vec<4, uint32_t>* buffer1Cursor = fBuffer1Cursor;
        skvx::Vec<4, uint32_t>* buffer2Cursor = fBuffer2Cursor;
        v4u32 sum0 = __lsx_vld(fSum0, 0); // same as skvx::Vec<4, uint32_t>::Load(fSum0);
        v4u32 sum1 = __lsx_vld(fSum1, 0);
        v4u32 sum2 = __lsx_vld(fSum2, 0);

        auto processValue = [&](v4u32& vLeadingEdge){
          sum0 += vLeadingEdge;
          sum1 += sum0;
          sum2 += sum1;

          v4u32 divisorFactor = __lsx_vreplgr2vr_w(fDivider.divisorFactor());
          v4u32 blurred = __lsx_vmuh_w(divisorFactor, sum2);

          v4u32 buffer2Value = __lsx_vld(buffer2Cursor, 0); //Not fBuffer0Cursor, out of bounds.
          sum2 -= buffer2Value;
          __lsx_vst(sum1, (void *)buffer2Cursor, 0);
          buffer2Cursor = (buffer2Cursor + 1) < fBuffersEnd ? buffer2Cursor + 1 : fBuffer2;
          v4u32 buffer1Value = __lsx_vld(buffer1Cursor, 0);
          sum1 -= buffer1Value;
          __lsx_vst(sum0, (void *)buffer1Cursor, 0);
          buffer1Cursor = (buffer1Cursor + 1) < fBuffer2 ? buffer1Cursor + 1 : fBuffer1;
          v4u32 buffer0Value = __lsx_vld(buffer0Cursor, 0);
          sum0 -= buffer0Value;
          __lsx_vst(vLeadingEdge, (void *)buffer0Cursor, 0);
          buffer0Cursor = (buffer0Cursor + 1) < fBuffer1 ? buffer0Cursor + 1 : fBuffer0;

          v16u8 shuf = {0x0,0x4,0x8,0xc,0x0};
          v16u8 ret = __lsx_vshuf_b(blurred, blurred, shuf);
          return ret;
        };

        v4u32 zero = __lsx_vldi(0x0);
        if (!src && !dst) {
            while (n --> 0) {
                (void)processValue(zero);
            }
        } else if (src && !dst) {
            while (n --> 0) {
                v4u32 edge = __lsx_vinsgr2vr_w(zero, *src, 0);
                edge = __lsx_vilvl_b(zero, edge);
                edge = __lsx_vilvl_h(zero, edge);
                (void)processValue(edge);
                src += srcStride;
            }
        } else if (!src && dst) {
            while (n --> 0) {
                v4u32 ret = processValue(zero);
                __lsx_vstelm_w(ret, dst, 0, 0); // 3rd is offset, 4th is idx.
                dst += dstStride;
            }
        } else if (src && dst) {
            while (n --> 0) {
                v4u32 edge = __lsx_vinsgr2vr_w(zero, *src, 0);
                edge = __lsx_vilvl_b(zero, edge);
                edge = __lsx_vilvl_h(zero, edge);
                v4u32 ret = processValue(edge);
                __lsx_vstelm_w(ret, dst, 0, 0);
                src += srcStride;
                dst += dstStride;
            }
        }

        // Store the state
        fBuffer0Cursor = buffer0Cursor;
        fBuffer1Cursor = buffer1Cursor;
        fBuffer2Cursor = buffer2Cursor;

        __lsx_vst(sum0, fSum0, 0);
        __lsx_vst(sum1, fSum1, 0);
        __lsx_vst(sum2, fSum2, 0);
#else
        skvx::Vec<4, uint32_t>* buffer0Cursor = fBuffer0Cursor;
        skvx::Vec<4, uint32_t>* buffer1Cursor = fBuffer1Cursor;
        skvx::Vec<4, uint32_t>* buffer2Cursor = fBuffer2Cursor;
        skvx::Vec<4, uint32_t> sum0 = skvx::Vec<4, uint32_t>::Load(fSum0);
        skvx::Vec<4, uint32_t> sum1 = skvx::Vec<4, uint32_t>::Load(fSum1);
        skvx::Vec<4, uint32_t> sum2 = skvx::Vec<4, uint32_t>::Load(fSum2);

        // Given an expanded input pixel, move the window ahead using the leadingEdge value.
        auto processValue = [&](const skvx::Vec<4, uint32_t>& leadingEdge) {
            sum0 += leadingEdge;
            sum1 += sum0;
            sum2 += sum1;

            skvx::Vec<4, uint32_t> blurred = fDivider.divide(sum2);

            sum2 -= *buffer2Cursor;
            *buffer2Cursor = sum1;
            buffer2Cursor = (buffer2Cursor + 1) < fBuffersEnd ? buffer2Cursor + 1 : fBuffer2;
            sum1 -= *buffer1Cursor;
            *buffer1Cursor = sum0;
            buffer1Cursor = (buffer1Cursor + 1) < fBuffer2 ? buffer1Cursor + 1 : fBuffer1;
            sum0 -= *buffer0Cursor;
            *buffer0Cursor = leadingEdge;
            buffer0Cursor = (buffer0Cursor + 1) < fBuffer1 ? buffer0Cursor + 1 : fBuffer0;

            return skvx::cast<uint8_t>(blurred);
        };

        auto loadEdge = [&](const uint32_t* srcCursor) {
            return skvx::cast<uint32_t>(skvx::Vec<4, uint8_t>::Load(srcCursor));
        };

        if (!src && !dst) {
            while (n --> 0) {
                (void)processValue(0);
            }
        } else if (src && !dst) {
            while (n --> 0) {
                (void)processValue(loadEdge(src));
                src += srcStride;
            }
        } else if (!src && dst) {
            while (n --> 0) {
                processValue(0u).store(dst);
                dst += dstStride;
            }
        } else if (src && dst) {
            while (n --> 0) {
                processValue(loadEdge(src)).store(dst);
                src += srcStride;
                dst += dstStride;
            }
        }

        // Store the state
        fBuffer0Cursor = buffer0Cursor;
        fBuffer1Cursor = buffer1Cursor;
        fBuffer2Cursor = buffer2Cursor;

        sum0.store(fSum0);
        sum1.store(fSum1);
        sum2.store(fSum2);
#endif
    }

    skvx::Vec<4, uint32_t>* const fBuffer0;
    skvx::Vec<4, uint32_t>* const fBuffer1;
    skvx::Vec<4, uint32_t>* const fBuffer2;
    skvx::Vec<4, uint32_t>* const fBuffersEnd;
    const skvx::ScaledDividerU32 fDivider;

    // blur state
    char fSum0[sizeof(skvx::Vec<4, uint32_t>)];
    char fSum1[sizeof(skvx::Vec<4, uint32_t>)];
    char fSum2[sizeof(skvx::Vec<4, uint32_t>)];
    skvx::Vec<4, uint32_t>* fBuffer0Cursor;
    skvx::Vec<4, uint32_t>* fBuffer1Cursor;
    skvx::Vec<4, uint32_t>* fBuffer2Cursor;
};

// Implement a scanline processor that uses a two-box filter to approximate a Tent filter.
// The TentPass is limit to processing sigmas < 2183.
class TentPass final : public Pass {
public:
    // NB 2183 is the largest sigma that will not cause a buffer full of 255 mask values to overflow
    // using the Tent filter. It also limits the size of buffers used hold intermediate values.
    // Explanation of maximums:
    //   sum0 = window * 255
    //   sum1 = window * sum0 -> window * window * 255
    //
    //   The value window^2 * 255 must fit in a uint32_t. So,
    //      window^2 < 2^32. window = 4104.
    //
    //   window = floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5)
    //   For window <= 4104, the largest value for sigma is 2183.
    static PassMaker* MakeMaker(float sigma, SkArenaAlloc* alloc) {
        SkASSERT(0 <= sigma);
        int gaussianWindow = SkBlurEngine::BoxBlurWindow(sigma);
        // This is a naive method of using the window size for the Gaussian blur to calculate the
        // window size for the Tent blur. This seems to work well in practice.
        //
        // We can use a single pixel to generate the effective blur area given a window size. For
        // the Gaussian blur this is 3 * window size. For the Tent filter this is 2 * window size.
        int tentWindow = 3 * gaussianWindow / 2;
        if (tentWindow >= 4104) {
            return nullptr;
        }

        class Maker : public PassMaker {
        public:
            explicit Maker(int window) : PassMaker{window} {}
            Pass* makePass(void* buffer, SkArenaAlloc* alloc) const override {
                return TentPass::Make(this->window(), buffer, alloc);
            }

            size_t bufferSizeBytes() const override {
                size_t onePassSize = this->window() - 1;
                // If the window is odd, then there is an obvious middle element. For even sizes 2
                // passes are shifted, and the last pass has an extra element. Like this:
                //       S
                //    aaaAaa
                //     bbBbbb
                //       D
                size_t bufferCount = 2 * onePassSize;
                return bufferCount * sizeof(skvx::Vec<4, uint32_t>);
            }
        };

        return alloc->make<Maker>(tentWindow);
    }

    static TentPass* Make(int window, void* buffers, SkArenaAlloc* alloc) {
        if (window > 4104) {
            return nullptr;
        }

        // We don't need to store the trailing edge pixel in the buffer;
        int passSize = window - 1;
        skvx::Vec<4, uint32_t>* buffer0 = static_cast<skvx::Vec<4, uint32_t>*>(buffers);
        skvx::Vec<4, uint32_t>* buffer1 = buffer0 + passSize;
        skvx::Vec<4, uint32_t>* buffersEnd = buffer1 + passSize;

        // Calculating the border is tricky. The border is the distance in pixels between the first
        // dst pixel and the first src pixel (or the last src pixel and the last dst pixel).
        // I will go through the odd case which is simpler, and then through the even case. Given a
        // stack of filters seven wide for the odd case of three passes.
        //
        //        S
        //     aaaAaaa
        //     bbbBbbb
        //        D
        //
        // The furthest changed pixel is when the filters are in the following configuration.
        //
        //              S
        //        aaaAaaa
        //     bbbBbbb
        //        D
        //
        // The A pixel is calculated using the value S, the B uses A, and the D uses B.
        // So, with a window size of seven the border is nine. In the odd case, the border is
        // window - 1.
        //
        // For even cases the filter stack is more complicated. It uses two passes
        // of even filters offset from each other. A stack for a width of six looks like
        // this.
        //
        //       S
        //    aaaAaa
        //     bbBbbb
        //       D
        //
        // The furthest pixel looks like this.
        //
        //            S
        //       aaaAaa
        //     bbBbbb
        //       D
        //
        // For a window of six, the border value is 5. In the even case the border is
        // window - 1.
        int border = window - 1;

        int divisor = window * window;
        return alloc->make<TentPass>(buffer0, buffer1, buffersEnd, border, divisor);
    }

    TentPass(skvx::Vec<4, uint32_t>* buffer0,
             skvx::Vec<4, uint32_t>* buffer1,
             skvx::Vec<4, uint32_t>* buffersEnd,
             int border,
             int divisor)
         : Pass{border}
         , fBuffer0{buffer0}
         , fBuffer1{buffer1}
         , fBuffersEnd{buffersEnd}
         , fDivider(divisor) {}

private:
    void startBlur() override {
        skvx::Vec<4, uint32_t>{0u, 0u, 0u, 0u}.store(fSum0);
        auto half = fDivider.half();
        skvx::Vec<4, uint32_t>{half, half, half, half}.store(fSum1);
        sk_bzero(fBuffer0, (fBuffersEnd - fBuffer0) * sizeof(skvx::Vec<4, uint32_t>));

        fBuffer0Cursor = fBuffer0;
        fBuffer1Cursor = fBuffer1;
    }

    // TentPass implements the common two pass box filter approximation of Tent filter,
    // but combines all both passes into a single pass. This approach is facilitated by two
    // circular buffers the width of the window which track values for trailing edges of each of
    // both passes. This allows the algorithm to use more precision in the calculation
    // because the values are not rounded each pass. And this implementation also avoids a trap
    // that's easy to fall into resulting in blending in too many zeroes near the edge.
    //
    // In general, a window sum has the form:
    //     sum_n+1 = sum_n + leading_edge - trailing_edge.
    // If instead we do the subtraction at the end of the previous iteration, we can just
    // calculate the sums instead of having to do the subtractions too.
    //
    //      In previous iteration:
    //      sum_n+1 = sum_n - trailing_edge.
    //
    //      In this iteration:
    //      sum_n+1 = sum_n + leading_edge.
    //
    // Now we can stack all three sums and do them at once. Sum0 gets its leading edge from the
    // actual data. Sum1's leading edge is just Sum0, and Sum2's leading edge is Sum1. So, doing the
    // three passes at the same time has the form:
    //
    //    sum0_n+1 = sum0_n + leading edge
    //    sum1_n+1 = sum1_n + sum0_n+1
    //
    //    sum1_n+1 / window^2 is the new value of the destination pixel.
    //
    // Reduce the sums by the trailing edges which were stored in the circular buffers for the
    // next go around.
    //
    //    sum1_n+2 = sum1_n+1 - buffer1[i];
    //    buffer1[i] = sum0;
    //    sum0_n+2 = sum0_n+1 - buffer0[i];
    //    buffer0[i] = leading edge
    void blurSegment(
            int n, const uint32_t* src, int srcStride, uint32_t* dst, int dstStride) override {
        skvx::Vec<4, uint32_t>* buffer0Cursor = fBuffer0Cursor;
        skvx::Vec<4, uint32_t>* buffer1Cursor = fBuffer1Cursor;
        skvx::Vec<4, uint32_t> sum0 = skvx::Vec<4, uint32_t>::Load(fSum0);
        skvx::Vec<4, uint32_t> sum1 = skvx::Vec<4, uint32_t>::Load(fSum1);

        // Given an expanded input pixel, move the window ahead using the leadingEdge value.
        auto processValue = [&](const skvx::Vec<4, uint32_t>& leadingEdge) {
            sum0 += leadingEdge;
            sum1 += sum0;

            skvx::Vec<4, uint32_t> blurred = fDivider.divide(sum1);

            sum1 -= *buffer1Cursor;
            *buffer1Cursor = sum0;
            buffer1Cursor = (buffer1Cursor + 1) < fBuffersEnd ? buffer1Cursor + 1 : fBuffer1;
            sum0 -= *buffer0Cursor;
            *buffer0Cursor = leadingEdge;
            buffer0Cursor = (buffer0Cursor + 1) < fBuffer1 ? buffer0Cursor + 1 : fBuffer0;

            return skvx::cast<uint8_t>(blurred);
        };

        auto loadEdge = [&](const uint32_t* srcCursor) {
            return skvx::cast<uint32_t>(skvx::Vec<4, uint8_t>::Load(srcCursor));
        };

        if (!src && !dst) {
            while (n --> 0) {
                (void)processValue(0);
            }
        } else if (src && !dst) {
            while (n --> 0) {
                (void)processValue(loadEdge(src));
                src += srcStride;
            }
        } else if (!src && dst) {
            while (n --> 0) {
                processValue(0u).store(dst);
                dst += dstStride;
            }
        } else if (src && dst) {
            while (n --> 0) {
                processValue(loadEdge(src)).store(dst);
                src += srcStride;
                dst += dstStride;
            }
        }

        // Store the state
        fBuffer0Cursor = buffer0Cursor;
        fBuffer1Cursor = buffer1Cursor;
        sum0.store(fSum0);
        sum1.store(fSum1);
    }

    skvx::Vec<4, uint32_t>* const fBuffer0;
    skvx::Vec<4, uint32_t>* const fBuffer1;
    skvx::Vec<4, uint32_t>* const fBuffersEnd;
    const skvx::ScaledDividerU32 fDivider;

    // blur state
    char fSum0[sizeof(skvx::Vec<4, uint32_t>)];
    char fSum1[sizeof(skvx::Vec<4, uint32_t>)];
    skvx::Vec<4, uint32_t>* fBuffer0Cursor;
    skvx::Vec<4, uint32_t>* fBuffer1Cursor;
};

class Raster8888BlurAlgorithm : public SkBlurEngine::Algorithm {
public:
    // See analysis in description of TentPass for the max supported sigma.
    float maxSigma() const override {
        // TentPass supports a sigma up to 2183, and was added so that the CPU blur algorithm's
        // blur radius was as large as that supported by the GPU. GaussPass only supports up to 136.
        // However, there is a very apparent pop in blur weight when switching from successive box
        // blurs to the tent filter. The TentPass is preserved for legacy blurs, which do not use
        // FilterResult::rescale(). However, using kMaxSigma = 135 with the raster SkBlurEngine
        // ensures that the non-legacy raster blurs will always use the GaussPass implementation.
        // This is about 6-7x faster on large blurs to rescale a few times to a lower resolution
        // than it is to evaluate the much larger original window.
        static constexpr float kMaxSigma = 135.f;
        SkASSERT(SkBlurEngine::BoxBlurWindow(kMaxSigma) <= 255); // see GaussPass::MakeMaker().
        return kMaxSigma;
    }

    // TODO: Implement CPU backend for different fTileMode. This is still worth doing inline with
    // the blur; at the moment the tiling is applied via the CropImageFilter and carried as metadata
    // on the FilterResult. This is forcefully applied in FilterResult::Builder::blur() when
    // supportsOnlyDecalTiling() returns true.
    bool supportsOnlyDecalTiling() const override { return true; }

    sk_sp<SkSpecialImage> blur(SkSize sigma,
                               sk_sp<SkSpecialImage> input,
                               const SkIRect& originalSrcBounds,
                               SkTileMode tileMode,
                               const SkIRect& originalDstBounds) const override {
        // TODO: Enable this assert when the TentPass is no longer used for legacy blurs
        // (which supports blur sigmas larger than what's reported in maxSigma()).
        // SkASSERT(sigma.width() <= this->maxSigma() && sigma.height() <= this->maxSigma());
        SkASSERT(tileMode == SkTileMode::kDecal);

        SkASSERT(SkIRect::MakeSize(input->dimensions()).contains(originalSrcBounds));

        SkBitmap src;
        if (!SkSpecialImages::AsBitmap(input.get(), &src)) {
            return nullptr; // Should only have been called by CPU-backed images
        }
        // The blur engine should not have picked this algorithm for a non-32-bit color type
        SkASSERT(src.colorType() == kRGBA_8888_SkColorType ||
                 src.colorType() == kBGRA_8888_SkColorType);

        SkSTArenaAlloc<1024> alloc;
        auto makeMaker = [&](float sigma) -> PassMaker* {
            SkASSERT(0 <= sigma && sigma <= 2183); // should be guaranteed after map_sigma
            if (PassMaker* maker = GaussPass::MakeMaker(sigma, &alloc)) {
                return maker;
            }
            if (PassMaker* maker = TentPass::MakeMaker(sigma, &alloc)) {
                return maker;
            }
            SK_ABORT("Sigma is out of range.");
        };

        PassMaker* makerX = makeMaker(sigma.width());
        PassMaker* makerY = makeMaker(sigma.height());

#if !defined(SK_AVOID_SLOW_RASTER_PIPELINE_BLURS)
        // A blur with a sigma smaller than the successive box-blurs accuracy should have been
        // routed to the shader-based algorithm.
        SkASSERT(makerX->window() > 1 || makerY->window() > 1);
#endif

        SkIRect srcBounds = originalSrcBounds;
        SkIRect dstBounds = originalDstBounds;
        if (makerX->window() > 1) {
            // Inflate the dst by the window required for the Y pass so that the X pass can prepare
            // it. The Y pass will be offset to only write to the original rows in dstBounds, but
            // its window will access these extra rows calculated by the X pass. The SpecialImage
            // factory will then subset the bitmap so it appears to match 'originalDstBounds'
            // tightly. We make one slightly larger image to hold this extra data instead of two
            // separate images sized exactly to each pass because the CPU blur can write in place.
            dstBounds.outset(0, SkBlurEngine::SigmaToRadius(sigma.height()));
        }

        SkBitmap dst;
        const SkIPoint dstOrigin = dstBounds.topLeft();
        if (!dst.tryAllocPixels(src.info().makeWH(dstBounds.width(), dstBounds.height()))) {
            return nullptr;
        }
        dst.eraseColor(SK_ColorTRANSPARENT);

        auto buffer = alloc.makeBytesAlignedTo(std::max(makerX->bufferSizeBytes(),
                                                        makerY->bufferSizeBytes()),
                                            alignof(skvx::Vec<4, uint32_t>));

        // Basic Plan: The three cases to handle
        // * Horizontal and Vertical - blur horizontally while copying values from the source to
        //     the destination. Then, do an in-place vertical blur.
        // * Horizontal only - blur horizontally copying values from the source to the destination.
        // * Vertical only - blur vertically copying values from the source to the destination.

        // Initialize these assuming the Y-only case
        int loopStart  = std::max(srcBounds.left(),  dstBounds.left());
        int loopEnd    = std::min(srcBounds.right(), dstBounds.right());
        int dstYOffset = 0;

        if (makerX->window() > 1) {
            // First an X-only blur from src into dst, including the extra rows that will become
            // input for the second Y pass, which will then be performed in place.
            loopStart = std::max(srcBounds.top(),    dstBounds.top());
            loopEnd   = std::min(srcBounds.bottom(), dstBounds.bottom());

            auto srcAddr = src.getAddr32(0, loopStart - srcBounds.top());
            auto dstAddr = dst.getAddr32(0, loopStart - dstBounds.top());

            // Iterate over each row to calculate 1D blur along X.
            Pass* pass = makerX->makePass(buffer, &alloc);
            for (int y = loopStart; y < loopEnd; ++y) {
                pass->blur(srcBounds.left()  - dstBounds.left(),
                           srcBounds.right() - dstBounds.left(),
                           dstBounds.width(),
                           srcAddr, 1,
                           dstAddr, 1);
                srcAddr += src.rowBytesAsPixels();
                dstAddr += dst.rowBytesAsPixels();
            }

            // Set up the Y pass to blur from the full dst into the non-outset portion of dst
            src = dst;
            loopStart = originalDstBounds.left();
            loopEnd   = originalDstBounds.right();
            // The new 'dst' is equal to dst.extractSubset(originalDstBounds.offset(-dstOrigin)),
            // but by construction only the Y offset has an interesting value so this is a little
            // more efficient.
            dstYOffset = originalDstBounds.top() - dstBounds.top();

            srcBounds = dstBounds;
            dstBounds = originalDstBounds;
        }

        // Iterate over each column to calculate 1D blur along Y. This is either blurring from src
        // into dst for a 1D blur; or it's blurring from dst into dst for the second pass of a 2D
        // blur.
        if (makerY->window() > 1) {
            auto srcAddr = src.getAddr32(loopStart - srcBounds.left(), 0);
            auto dstAddr = dst.getAddr32(loopStart - dstBounds.left(), dstYOffset);

            Pass* pass = makerY->makePass(buffer, &alloc);
            for (int x = loopStart; x < loopEnd; ++x) {
                pass->blur(srcBounds.top()    - dstBounds.top(),
                           srcBounds.bottom() - dstBounds.top(),
                           dstBounds.height(),
                           srcAddr, src.rowBytesAsPixels(),
                           dstAddr, dst.rowBytesAsPixels());
                srcAddr += 1;
                dstAddr += 1;
            }
        }

#if defined(SK_AVOID_SLOW_RASTER_PIPELINE_BLURS)
        // When avoiding the shader-based algorithm, handle the box identity case.
        if (makerX->window() == 1 && makerY->window() == 1) {
            dst.writePixels(src.pixmap(),
                            srcBounds.left() - dstBounds.left(),
                            srcBounds.top()  - dstBounds.top());
        }
#endif

        dstBounds = originalDstBounds.makeOffset(-dstOrigin); // Make relative to dst's pixels
        return SkSpecialImages::MakeFromRaster(dstBounds, dst, SkSurfaceProps{});
    }

};

class RasterShaderBlurAlgorithm : public SkShaderBlurAlgorithm {
public:
    sk_sp<SkDevice> makeDevice(const SkImageInfo& imageInfo) const override {
        // This Device will only be used to draw blurs, so use default SkSurfaceProps. The pixel
        // geometry and font configuration do not matter. This is not a GPU surface, so DMSAA and
        // the kAlwaysDither surface property are also irrelevant.
        return SkBitmapDevice::Create(imageInfo, SkSurfaceProps{});
    }
};

class RasterBlurEngine : public SkBlurEngine {
public:
    const Algorithm* findAlgorithm(SkSize sigma,  SkColorType colorType) const override {
#if defined(SK_AVOID_SLOW_RASTER_PIPELINE_BLURS)
        // For large source images, the shader-based blur can be prohibitively slow so setting this
        // to zero means it'll never be used for 8888 color types.
        static constexpr float kBoxBlurMinSigma = 0.f;
#else
        static constexpr float kBoxBlurMinSigma = 2.f;

        // If the sigma is larger than kBoxBlurMinSigma, we should assume that we won't encounter
        // an identity window assertion later on.
        SkASSERT(SkBlurEngine::BoxBlurWindow(kBoxBlurMinSigma) > 1);
#endif

        // Using the shader-based blur for small blur sigmas only happens if both axes require a
        // small blur. It's assumed that any inaccuracy along one axis is hidden by the large enough
        // blur along the other axis.
        const bool smallBlur = sigma.width() < kBoxBlurMinSigma &&
                               sigma.height() < kBoxBlurMinSigma;
        // The box blur doesn't actually care about channel order as long as it's 4 8-bit channels.
        const bool rgba8Blur = colorType == kRGBA_8888_SkColorType ||
                               colorType == kBGRA_8888_SkColorType;
        // TODO: Specialize A8 color types as well by reusing the mask filter blur impl
        if (smallBlur || !rgba8Blur) {
            return &fShaderBlurAlgorithm;
        } else {
            return &fRGBA8BlurAlgorithm;
        }
    }

private:
    // For small sigmas and non-8888 or A8 color types, use the shader algorithm
    RasterShaderBlurAlgorithm fShaderBlurAlgorithm;
    // For large blurs with RGBA8 or BGRA8, use consecutive box blurs
    Raster8888BlurAlgorithm fRGBA8BlurAlgorithm;
};

} // anonymous namespace

const SkBlurEngine* SkBlurEngine::GetRasterBlurEngine() {
    static const RasterBlurEngine kInstance;
    return &kInstance;
}

// SkShaderBlurAlgorithm
// ----------------------------------------------------------------------------

void SkShaderBlurAlgorithm::Compute2DBlurKernel(SkSize sigma,
                                                SkISize radius,
                                                SkSpan<float> kernel) {
    // Callers likely had to calculate the radius prior to filling out the kernel value, which is
    // why it's provided; but make sure it's consistent with expectations.
    SkASSERT(SkBlurEngine::SigmaToRadius(sigma.width()) == radius.width() &&
             SkBlurEngine::SigmaToRadius(sigma.height()) == radius.height());

    // Callers are responsible for downscaling large sigmas to values that can be processed by the
    // effects, so ensure the radius won't overflow 'kernel'
    const int width = KernelWidth(radius.width());
    const int height = KernelWidth(radius.height());
    const size_t kernelSize = SkTo<size_t>(sk_64_mul(width, height));
    SkASSERT(kernelSize <= kernel.size());

    // And the definition of an identity blur should be sufficient that 2sigma^2 isn't near zero
    // when there's a non-trivial radius.
    const float twoSigmaSqrdX = 2.0f * sigma.width() * sigma.width();
    const float twoSigmaSqrdY = 2.0f * sigma.height() * sigma.height();
    SkASSERT((radius.width() == 0 || !SkScalarNearlyZero(twoSigmaSqrdX)) &&
             (radius.height() == 0 || !SkScalarNearlyZero(twoSigmaSqrdY)));

    // Setting the denominator to 1 when the radius is 0 automatically converts the remaining math
    // to the 1D Gaussian distribution. When both radii are 0, it correctly computes a weight of 1.0
    const float sigmaXDenom = radius.width() > 0 ? 1.0f / twoSigmaSqrdX : 1.f;
    const float sigmaYDenom = radius.height() > 0 ? 1.0f / twoSigmaSqrdY : 1.f;

    float sum = 0.0f;
    for (int x = 0; x < width; x++) {
        float xTerm = static_cast<float>(x - radius.width());
        xTerm = xTerm * xTerm * sigmaXDenom;
        for (int y = 0; y < height; y++) {
            float yTerm = static_cast<float>(y - radius.height());
            float xyTerm = std::exp(-(xTerm + yTerm * yTerm * sigmaYDenom));
            // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
            // is dropped here, since we renormalize the kernel below.
            kernel[y * width + x] = xyTerm;
            sum += xyTerm;
        }
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (size_t i = 0; i < kernelSize; ++i) {
        kernel[i] *= scale;
    }
    // Zero remainder of the array
    memset(kernel.data() + kernelSize, 0, sizeof(float)*(kernel.size() - kernelSize));
}

void SkShaderBlurAlgorithm::Compute2DBlurKernel(SkSize sigma,
                                                SkISize radii,
                                                std::array<SkV4, kMaxSamples/4>& kernel) {
    static_assert(sizeof(kernel) == sizeof(std::array<float, kMaxSamples>));
    static_assert(alignof(float) == alignof(SkV4));
    float* data = kernel[0].ptr();
    Compute2DBlurKernel(sigma, radii, SkSpan<float>(data, kMaxSamples));
}

void SkShaderBlurAlgorithm::Compute2DBlurOffsets(SkISize radius,
                                                 std::array<SkV4, kMaxSamples/2>& offsets) {
    const int kernelArea = KernelWidth(radius.width()) * KernelWidth(radius.height());
    SkASSERT(kernelArea <= kMaxSamples);

    SkSpan<float> offsetView{offsets[0].ptr(), kMaxSamples*2};

    int i = 0;
    for (int y = -radius.height(); y <= radius.height(); ++y) {
        for (int x = -radius.width(); x <= radius.width(); ++x) {
            offsetView[2*i]   = x;
            offsetView[2*i+1] = y;
            ++i;
        }
    }
    SkASSERT(i == kernelArea);
    const int lastValidOffset = 2*(kernelArea - 1);
    for (; i < kMaxSamples; ++i) {
        offsetView[2*i]   = offsetView[lastValidOffset];
        offsetView[2*i+1] = offsetView[lastValidOffset+1];
    }
}

void SkShaderBlurAlgorithm::Compute1DBlurLinearKernel(
        float sigma,
        int radius,
        std::array<SkV4, kMaxSamples/2>& offsetsAndKernel) {
    SkASSERT(sigma <= kMaxLinearSigma);
    SkASSERT(radius == SkBlurEngine::SigmaToRadius(sigma));
    SkASSERT(LinearKernelWidth(radius) <= kMaxSamples);

    // Given 2 adjacent gaussian points, they are blended as: Wi * Ci + Wj * Cj.
    // The GPU will mix Ci and Cj as Ci * (1 - x) + Cj * x during sampling.
    // Compute W', x such that W' * (Ci * (1 - x) + Cj * x) = Wi * Ci + Wj * Cj.
    // Solving W' * x = Wj, W' * (1 - x) = Wi:
    // W' = Wi + Wj
    // x = Wj / (Wi + Wj)
    auto get_new_weight = [](float* new_w, float* offset, float wi, float wj) {
        *new_w = wi + wj;
        *offset = wj / (wi + wj);
    };

    // Create a temporary standard kernel. The maximum blur radius that can be passed to this
    // function is (kMaxBlurSamples-1), so make an array large enough to hold the full kernel width.
    static constexpr int kMaxKernelWidth = KernelWidth(kMaxSamples - 1);
    SkASSERT(KernelWidth(radius) <= kMaxKernelWidth);
    std::array<float, kMaxKernelWidth> fullKernel;
    Compute1DBlurKernel(sigma, radius, SkSpan<float>{fullKernel.data(), KernelWidth(radius)});

    std::array<float, kMaxSamples> kernel;
    std::array<float, kMaxSamples> offsets;
    // Note that halfsize isn't just size / 2, but radius + 1. This is the size of the output array.
    int halfSize = LinearKernelWidth(radius);
    int halfRadius = halfSize / 2;
    int lowIndex = halfRadius - 1;

    // Compute1DGaussianKernel produces a full 2N + 1 kernel. Since the kernel can be mirrored,
    // compute only the upper half and mirror to the lower half.

    int index = radius;
    if (radius & 1) {
        // If N is odd, then use two samples.
        // The centre texel gets sampled twice, so halve its influence for each sample.
        // We essentially sample like this:
        // Texel edges
        // v    v    v    v
        // |    |    |    |
        // \-----^---/ Lower sample
        //      \---^-----/ Upper sample
        get_new_weight(&kernel[halfRadius],
                       &offsets[halfRadius],
                       fullKernel[index] * 0.5f,
                       fullKernel[index + 1]);
        kernel[lowIndex] = kernel[halfRadius];
        offsets[lowIndex] = -offsets[halfRadius];
        index++;
        lowIndex--;
    } else {
        // If N is even, then there are an even number of texels on either side of the centre texel.
        // Sample the centre texel directly.
        kernel[halfRadius] = fullKernel[index];
        offsets[halfRadius] = 0.0f;
    }
    index++;

    // Every other pair gets one sample.
    for (int i = halfRadius + 1; i < halfSize; index += 2, i++, lowIndex--) {
        get_new_weight(&kernel[i], &offsets[i], fullKernel[index], fullKernel[index + 1]);
        offsets[i] += static_cast<float>(index - radius);

        // Mirror to lower half.
        kernel[lowIndex] = kernel[i];
        offsets[lowIndex] = -offsets[i];
    }

    // Zero out remaining values in the kernel
    memset(kernel.data() + halfSize, 0, sizeof(float)*(kMaxSamples - halfSize));
    // But copy the last valid offset into the remaining offsets, to increase the chance that
    // over-iteration in a fragment shader will have a cache hit.
    for (int i = halfSize; i < kMaxSamples; ++i) {
        offsets[i] = offsets[halfSize - 1];
    }

    // Interleave into the output array to match the 1D SkSL effect
    for (int i = 0; i < kMaxSamples / 2; ++i) {
        offsetsAndKernel[i] = SkV4{offsets[2*i], kernel[2*i], offsets[2*i+1], kernel[2*i+1]};
    }
}

static SkKnownRuntimeEffects::StableKey to_stablekey(int kernelWidth, uint32_t baseKey) {
    SkASSERT(kernelWidth >= 2 && kernelWidth <= SkShaderBlurAlgorithm::kMaxSamples);
    switch(kernelWidth) {
        // Batch on multiples of 4 (skipping width=1, since that can't happen)
        case 2:  [[fallthrough]];
        case 3:  [[fallthrough]];
        case 4:  return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey);
        case 5:  [[fallthrough]];
        case 6:  [[fallthrough]];
        case 7:  [[fallthrough]];
        case 8:  return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+1);
        case 9:  [[fallthrough]];
        case 10: [[fallthrough]];
        case 11: [[fallthrough]];
        case 12: return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+2);
        case 13: [[fallthrough]];
        case 14: [[fallthrough]];
        case 15: [[fallthrough]];
        case 16: return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+3);
        case 17: [[fallthrough]];
        case 18: [[fallthrough]];
        case 19: [[fallthrough]];
        // With larger kernels, batch on multiples of eight so up to 7 wasted samples.
        case 20: return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+4);
        case 21: [[fallthrough]];
        case 22: [[fallthrough]];
        case 23: [[fallthrough]];
        case 24: [[fallthrough]];
        case 25: [[fallthrough]];
        case 26: [[fallthrough]];
        case 27: [[fallthrough]];
        case 28: return static_cast<SkKnownRuntimeEffects::StableKey>(baseKey+5);
        default:
            SkUNREACHABLE;
    }
}

const SkRuntimeEffect* SkShaderBlurAlgorithm::GetLinearBlur1DEffect(int radius) {
    return GetKnownRuntimeEffect(
            to_stablekey(LinearKernelWidth(radius),
                         static_cast<uint32_t>(SkKnownRuntimeEffects::StableKey::k1DBlurBase)));
}

const SkRuntimeEffect* SkShaderBlurAlgorithm::GetBlur2DEffect(const SkISize& radii) {
    int kernelArea = KernelWidth(radii.width()) * KernelWidth(radii.height());
    return GetKnownRuntimeEffect(
            to_stablekey(kernelArea,
                         static_cast<uint32_t>(SkKnownRuntimeEffects::StableKey::k2DBlurBase)));
}

sk_sp<SkSpecialImage> SkShaderBlurAlgorithm::renderBlur(SkRuntimeShaderBuilder* blurEffectBuilder,
                                                        SkFilterMode filter,
                                                        SkISize radii,
                                                        sk_sp<SkSpecialImage> input,
                                                        const SkIRect& srcRect,
                                                        SkTileMode tileMode,
                                                        const SkIRect& dstRect) const {
    SkImageInfo outII = SkImageInfo::Make({dstRect.width(), dstRect.height()},
                                          input->colorType(),
                                          kPremul_SkAlphaType,
                                          input->colorInfo().refColorSpace());
    sk_sp<SkDevice> device = this->makeDevice(outII);
    if (!device) {
        return nullptr;
    }

    SkIRect subset = SkIRect::MakeSize(dstRect.size());
    device->clipRect(SkRect::Make(subset), SkClipOp::kIntersect, /*aa=*/false);
    device->setLocalToDevice(SkM44::Translate(-dstRect.left(), -dstRect.top()));

    // renderBlur() will either mix multiple fast and strict draws to cover dstRect, or will issue
    // a single strict draw. While the SkShader object changes (really just strict mode), the rest
    // of the SkPaint remains the same.
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);

    SkIRect safeSrcRect = srcRect.makeInset(radii.width(), radii.height());
    SkIRect fastDstRect = dstRect;

    // Only consider the safeSrcRect for shader-based tiling if the original srcRect is different
    // from the backing store dimensions; when they match the full image we can use HW tiling.
    if (srcRect != SkIRect::MakeSize(input->backingStoreDimensions())) {
        if (fastDstRect.intersect(safeSrcRect)) {
            // If the area of the non-clamping shader is small, it's better to just issue a single
            // draw that performs shader tiling over the whole dst.
            if (fastDstRect != dstRect && fastDstRect.width() * fastDstRect.height() < 128 * 128) {
                fastDstRect.setEmpty();
            }
        } else {
            fastDstRect.setEmpty();
        }
    }

    if (!fastDstRect.isEmpty()) {
        // Fill as much as possible without adding shader tiling logic to each blur sample,
        // switching to clamp tiling if we aren't in this block due to HW tiling.
        SkIRect untiledSrcRect = srcRect.makeInset(1, 1);
        SkTileMode fastTileMode = untiledSrcRect.contains(fastDstRect) ? SkTileMode::kClamp
                                                                       : tileMode;
        blurEffectBuilder->child("child") = input->asShader(
                fastTileMode, filter, SkMatrix::I(), /*strict=*/false);
        paint.setShader(blurEffectBuilder->makeShader());
        device->drawRect(SkRect::Make(fastDstRect), paint);
    }

    // Switch to a strict shader if there are remaining pixels to fill
    if (fastDstRect != dstRect) {
        blurEffectBuilder->child("child") = input->makeSubset(srcRect)->asShader(
                tileMode, filter, SkMatrix::Translate(srcRect.left(), srcRect.top()));
        paint.setShader(blurEffectBuilder->makeShader());
    }

    if (fastDstRect.isEmpty()) {
        // Fill the entire dst with the strict shader
        device->drawRect(SkRect::Make(dstRect), paint);
    } else if (fastDstRect != dstRect) {
        // There will be up to four additional strict draws to fill in the border. The left and
        // right sides will span the full height of the dst rect. The top and bottom will span
        // the just the width of the fast interior. Strict border draws with zero width/height
        // are skipped.
        auto drawBorder = [&](const SkIRect& r) {
            if (!r.isEmpty()) {
                device->drawRect(SkRect::Make(r), paint);
            }
        };

        drawBorder({dstRect.left(),      dstRect.top(),
                    fastDstRect.left(),  dstRect.bottom()});   // Left, spanning full height
        drawBorder({fastDstRect.right(), dstRect.top(),
                    dstRect.right(),     dstRect.bottom()});   // Right, spanning full height
        drawBorder({fastDstRect.left(),  dstRect.top(),
                    fastDstRect.right(), fastDstRect.top()});  // Top, spanning inner width
        drawBorder({fastDstRect.left(),  fastDstRect.bottom(),
                    fastDstRect.right(), dstRect.bottom()});   // Bottom, spanning inner width
    }

    return device->snapSpecial(subset);
}

sk_sp<SkSpecialImage> SkShaderBlurAlgorithm::evalBlur2D(SkSize sigma,
                                                        SkISize radii,
                                                        sk_sp<SkSpecialImage> input,
                                                        const SkIRect& srcRect,
                                                        SkTileMode tileMode,
                                                        const SkIRect& dstRect) const {
    std::array<SkV4, kMaxSamples/4> kernel;
    std::array<SkV4, kMaxSamples/2> offsets;
    Compute2DBlurKernel(sigma, radii, kernel);
    Compute2DBlurOffsets(radii, offsets);

    SkRuntimeShaderBuilder builder{sk_ref_sp(GetBlur2DEffect(radii))};
    builder.uniform("kernel") = kernel;
    builder.uniform("offsets") = offsets;
    // NOTE: renderBlur() will configure the "child" shader as needed. The 2D blur effect only
    // requires nearest-neighbor filtering.
    return this->renderBlur(&builder, SkFilterMode::kNearest, radii,
                            std::move(input), srcRect, tileMode, dstRect);
}

sk_sp<SkSpecialImage> SkShaderBlurAlgorithm::evalBlur1D(float sigma,
                                                        int radius,
                                                        SkV2 dir,
                                                        sk_sp<SkSpecialImage> input,
                                                        SkIRect srcRect,
                                                        SkTileMode tileMode,
                                                        SkIRect dstRect) const {
    std::array<SkV4, kMaxSamples/2> offsetsAndKernel;
    Compute1DBlurLinearKernel(sigma, radius, offsetsAndKernel);

    SkRuntimeShaderBuilder builder{sk_ref_sp(GetLinearBlur1DEffect(radius))};
    builder.uniform("offsetsAndKernel") = offsetsAndKernel;
    builder.uniform("dir") = dir;
    // NOTE: renderBlur() will configure the "child" shader as needed. The 1D blur effect requires
    // linear filtering. Reconstruct the appropriate "2D" radii inset value from 'dir'.
    SkISize radii{dir.x ? radius : 0, dir.y ? radius : 0};
    return this->renderBlur(&builder, SkFilterMode::kLinear, radii,
                            std::move(input), srcRect, tileMode, dstRect);
}

sk_sp<SkSpecialImage> SkShaderBlurAlgorithm::blur(SkSize sigma,
                                                  sk_sp<SkSpecialImage> src,
                                                  const SkIRect& srcRect,
                                                  SkTileMode tileMode,
                                                  const SkIRect& dstRect) const {
    SkASSERT(sigma.width() <= kMaxLinearSigma &&  sigma.height() <= kMaxLinearSigma);

    int radiusX = SkBlurEngine::SigmaToRadius(sigma.width());
    int radiusY = SkBlurEngine::SigmaToRadius(sigma.height());
    const int kernelArea = KernelWidth(radiusX) * KernelWidth(radiusY);
    if (kernelArea <= kMaxSamples && radiusX > 0 && radiusY > 0) {
        // Use a single-pass 2D kernel if it fits and isn't just 1D already
        return this->evalBlur2D(sigma,
                                {radiusX, radiusY},
                                std::move(src),
                                srcRect,
                                tileMode,
                                dstRect);
    } else {
        // Use two passes of a 1D kernel (one per axis).
        SkIRect intermediateSrcRect = srcRect;
        SkIRect intermediateDstRect = dstRect;
        if (radiusX > 0) {
            if (radiusY > 0) {
                // May need to maintain extra rows above and below 'dstRect' for the follow-up pass.
                if (tileMode == SkTileMode::kRepeat || tileMode == SkTileMode::kMirror) {
                    // If the srcRect and dstRect are aligned, then we don't need extra rows since
                    // the periodic tiling on srcRect is the same for the intermediate. If they
                    // are not aligned, then outset by the Y radius.
                    const int period = srcRect.height() * (tileMode == SkTileMode::kMirror ? 2 : 1);
                    if (std::abs(dstRect.fTop - srcRect.fTop) % period != 0 ||
                        dstRect.height() != srcRect.height()) {
                        intermediateDstRect.outset(0, radiusY);
                    }
                } else {
                    // For clamp and decal tiling, we outset by the Y radius up to what's available
                    // from the srcRect. Anything beyond that is identical to tiling the
                    // intermediate dst image directly.
                    intermediateDstRect.outset(0, radiusY);
                    intermediateDstRect.fTop = std::max(intermediateDstRect.fTop, srcRect.fTop);
                    intermediateDstRect.fBottom =
                            std::min(intermediateDstRect.fBottom, srcRect.fBottom);
                    if (intermediateDstRect.fTop >= intermediateDstRect.fBottom) {
                        return nullptr;
                    }
                }
            }

            src = this->evalBlur1D(sigma.width(),
                                   radiusX,
                                   /*dir=*/{1.f, 0.f},
                                   std::move(src),
                                   srcRect,
                                   tileMode,
                                   intermediateDstRect);
            if (!src) {
                return nullptr;
            }
            intermediateSrcRect = SkIRect::MakeWH(src->width(), src->height());
            intermediateDstRect = dstRect.makeOffset(-intermediateDstRect.left(),
                                                     -intermediateDstRect.top());
        }

        if (radiusY > 0) {
            src = this->evalBlur1D(sigma.height(),
                                   radiusY,
                                   /*dir=*/{0.f, 1.f},
                                   std::move(src),
                                   intermediateSrcRect,
                                   tileMode,
                                   intermediateDstRect);
        }

        return src;
    }
}

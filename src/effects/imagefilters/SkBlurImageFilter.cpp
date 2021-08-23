/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>

#include "include/core/SkBitmap.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTPin.h"
#include "include/private/SkVx.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkGpuBlurUtils.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkOpts.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#if SK_GPU_V1
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#endif // SK_GPU_V1
#endif // SK_SUPPORT_GPU

namespace {

class SkBlurImageFilter final : public SkImageFilter_Base {
public:
    SkBlurImageFilter(SkScalar sigmaX, SkScalar sigmaY,  SkTileMode tileMode,
                      sk_sp<SkImageFilter> input, const SkRect* cropRect)
            : INHERITED(&input, 1, cropRect)
            , fSigma{sigmaX, sigmaY}
            , fTileMode(tileMode) {}

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    friend void ::SkRegisterBlurImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlurImageFilter)

#if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> gpuFilter(
            const Context& ctx, SkVector sigma,
            const sk_sp<SkSpecialImage> &input,
            SkIRect inputBounds, SkIRect dstBounds, SkIPoint inputOffset, SkIPoint* offset) const;
#endif

    SkSize     fSigma;
    SkTileMode fTileMode;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Blur(
        SkScalar sigmaX, SkScalar sigmaY, SkTileMode tileMode, sk_sp<SkImageFilter> input,
        const CropRect& cropRect) {
    if (sigmaX < SK_ScalarNearlyZero && sigmaY < SK_ScalarNearlyZero && !cropRect) {
        return input;
    }
    return sk_sp<SkImageFilter>(
          new SkBlurImageFilter(sigmaX, sigmaY, tileMode, input, cropRect));
}

void SkRegisterBlurImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkBlurImageFilter);
    SkFlattenable::Register("SkBlurImageFilterImpl", SkBlurImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkBlurImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkTileMode tileMode = buffer.read32LE(SkTileMode::kLastTileMode);
    return SkImageFilters::Blur(
          sigmaX, sigmaY, tileMode, common.getInput(0), common.cropRect());
}

void SkBlurImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSigma.fWidth);
    buffer.writeScalar(fSigma.fHeight);

    SkASSERT(fTileMode <= SkTileMode::kLastTileMode);
    buffer.writeInt(static_cast<int>(fTileMode));
}

///////////////////////////////////////////////////////////////////////////////

namespace {
// This is defined by the SVG spec:
// https://drafts.fxtf.org/filter-effects/#feGaussianBlurElement
int calculate_window(double sigma) {
    auto possibleWindow = static_cast<int>(floor(sigma * 3 * sqrt(2 * SK_DoublePI) / 4 + 0.5));
    return std::max(1, possibleWindow);
}

class Pass {
public:
    virtual ~Pass() = default;

    virtual void blur(int srcLeft, int srcRight, int dstRight,
                      const uint32_t* src, int srcXStride,
                      uint32_t* dst, int dstXStride) = 0;
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
    static PassMaker* MakeMaker(double sigma, SkArenaAlloc* alloc) {
        SkASSERT(0 <= sigma);
        int window = calculate_window(sigma);
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

        // NB the sums in the blur code use the following technique to avoid
        // adding 1/2 to round the divide.
        //
        //   Sum/d + 1/2 == (Sum + h) / d
        //   Sum + d(1/2) ==  Sum + h
        //     h == (1/2)d
        //
        // But the d/2 it self should be rounded.
        //    h == d/2 + 1/2 == (d + 1) / 2
        //
        // divisorFactor = (1 / d) * 2 ^ 32
        auto divisorFactor = static_cast<uint32_t>(round((1.0 / divisor) * (1ull << 32)));
        auto half = static_cast<uint32_t>((divisor + 1) / 2);
        return alloc->make<GaussPass>(
                buffer0, buffer1, buffer2, buffersEnd, border, divisorFactor, half);
    }

    GaussPass(skvx::Vec<4, uint32_t>* buffer0,
              skvx::Vec<4, uint32_t>* buffer1,
              skvx::Vec<4, uint32_t>* buffer2,
              skvx::Vec<4, uint32_t>* buffersEnd,
              int border,
              uint32_t divisorFactor,
              uint32_t half)
        : fBuffer0{buffer0}
        , fBuffer1{buffer1}
        , fBuffer2{buffer2}
        , fBuffersEnd{buffersEnd}
        , fBorder{border}
        , fDivisorFactor{divisorFactor}
        , fHalf{half} {}

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
    void blur(int srcLeft, int srcRight, int dstRight,
              const uint32_t* src, int srcXStride,
              uint32_t* dst, int dstXStride) override {
        skvx::Vec<4, uint32_t> sum0{0u, 0u, 0u, 0u};
        skvx::Vec<4, uint32_t> sum1{0u, 0u, 0u, 0u};
        skvx::Vec<4, uint32_t> sum2{fHalf, fHalf, fHalf, fHalf};
        sk_bzero(fBuffer0, (fBuffersEnd - fBuffer0) * sizeof(skvx::Vec<4, uint32_t>));

        skvx::Vec<4, uint32_t>* buffer0Cursor = fBuffer0;
        skvx::Vec<4, uint32_t>* buffer1Cursor = fBuffer1;
        skvx::Vec<4, uint32_t>* buffer2Cursor = fBuffer2;

        // Given an expanded input pixel, move the window ahead using the leadingEdge value.
        auto processValue = [&](const skvx::Vec<4, uint32_t>& leadingEdge) {
            sum0 += leadingEdge;
            sum1 += sum0;
            sum2 += sum1;

            skvx::Vec<4, uint64_t> w = skvx::cast<uint64_t>(sum2) * fDivisorFactor;
            skvx::Vec<4, uint32_t> value = skvx::cast<uint32_t>(w >> 32);

            sum2 -= *buffer2Cursor;
            *buffer2Cursor = sum1;
            buffer2Cursor = (buffer2Cursor + 1) < fBuffersEnd ? buffer2Cursor + 1 : fBuffer2;
            sum1 -= *buffer1Cursor;
            *buffer1Cursor = sum0;
            buffer1Cursor = (buffer1Cursor + 1) < fBuffer2 ? buffer1Cursor + 1 : fBuffer1;
            sum0 -= *buffer0Cursor;
            *buffer0Cursor = leadingEdge;
            buffer0Cursor = (buffer0Cursor + 1) < fBuffer1 ? buffer0Cursor + 1 : fBuffer0;

            return value;
        };

        auto srcStart = srcLeft - fBorder,
             srcEnd   = srcRight - fBorder,
             dstEnd   = dstRight,
             srcIdx   = srcStart,
             dstIdx   = 0;

        const uint32_t* srcCursor = src;
        uint32_t* dstCursor = dst;

        // The destination pixels are not effected by the src pixels,
        // change to zero as per the spec.
        // https://drafts.fxtf.org/filter-effects/#FilterPrimitivesOverviewIntro
        while (dstIdx < srcIdx) {
            *dstCursor = 0;
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }

        // The edge of the source is before the edge of the destination. Calculate the sums for
        // the pixels before the start of the destination.
        while (dstIdx > srcIdx) {
            skvx::Vec<4, uint32_t> leadingEdge =
                    srcIdx < srcEnd ? skvx::cast<uint32_t>(skvx::Vec<4, uint8_t>::Load(srcCursor))
                                    : 0;
            (void) processValue(leadingEdge);
            srcCursor += srcXStride;
            srcIdx++;
        }

        // The dstIdx and srcIdx are in sync now; the code just uses the dstIdx for both now.
        // Consume the source generating pixels to dst.
        auto loopEnd = std::min(dstEnd, srcEnd);
        while (dstIdx < loopEnd) {
            skvx::Vec<4, uint32_t> leadingEdge =
                    skvx::cast<uint32_t>(skvx::Vec<4, uint8_t>::Load(srcCursor));
            skvx::cast<uint8_t>(processValue(leadingEdge)).store(dstCursor);
            srcCursor += srcXStride;
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }

        // The leading edge is beyond the end of the source. Assume that the pixels
        // are now 0x0000 until the end of the destination.
        loopEnd = dstEnd;
        while (dstIdx < loopEnd) {
            skvx::cast<uint8_t>(processValue(0u)).store(dstCursor);
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }
    }

private:
    skvx::Vec<4, uint32_t>* const fBuffer0;
    skvx::Vec<4, uint32_t>* const fBuffer1;
    skvx::Vec<4, uint32_t>* const fBuffer2;
    skvx::Vec<4, uint32_t>* const fBuffersEnd;
    const int fBorder;
    const uint32_t fDivisorFactor;
    const uint32_t fHalf;
};

// Implement a scanline processor that uses a two-box filter to approximate a Tent filter.
// The TentPass is limit to processing sigmas < 2183.
class TentPass final : public Pass {
public:
    // NB 136 is the largest sigma that will not cause a buffer full of 255 mask values to overflow
    // using the Gauss filter. It also limits the size of buffers used hold intermediate values.
    // Explanation of maximums:
    //   sum0 = window * 255
    //   sum1 = window * sum0 -> window * window * 255
    //
    //   The value window^2 * 255 must fit in a uint32_t. So,
    //      window^2 < 2^32. window = 4104.
    //
    //   window = floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5)
    //   For window <= 4104, the largest value for sigma is 2183.
    static PassMaker* MakeMaker(double sigma, SkArenaAlloc* alloc) {
        SkASSERT(0 <= sigma);
        int gaussianWindow = calculate_window(sigma);
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

        // NB the sums in the blur code use the following technique to avoid
        // adding 1/2 to round the divide.
        //
        //   Sum/d + 1/2 == (Sum + h) / d
        //   Sum + d(1/2) ==  Sum + h
        //     h == (1/2)d
        //
        // But the d/2 it self should be rounded.
        //    h == d/2 + 1/2 == (d + 1) / 2
        //
        // divisorFactor = (1 / d) * 2 ^ 32
        auto divisorFactor = static_cast<uint32_t>(round((1.0 / divisor) * (1ull << 32)));
        auto half = static_cast<uint32_t>((divisor + 1) / 2);
        return alloc->make<TentPass>(buffer0, buffer1, buffersEnd, border, divisorFactor, half);
    }

    TentPass(skvx::Vec<4, uint32_t>* buffer0,
             skvx::Vec<4, uint32_t>* buffer1,
             skvx::Vec<4, uint32_t>* buffersEnd,
             int border,
             uint32_t divisorFactor,
             uint32_t half)
         : fBuffer0{buffer0}
         , fBuffer1{buffer1}
         , fBuffersEnd{buffersEnd}
         , fBorder{border}
         , fDivisorFactor{divisorFactor}
         , fHalf{half} {}

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
    void blur(int srcLeft, int srcRight, int dstRight,
              const uint32_t* src, int srcXStride,
              uint32_t* dst, int dstXStride) override {
        skvx::Vec<4, uint32_t> sum0{0u, 0u, 0u, 0u};
        skvx::Vec<4, uint32_t> sum1{fHalf, fHalf, fHalf, fHalf};
        sk_bzero(fBuffer0, (fBuffersEnd - fBuffer0) * sizeof(skvx::Vec<4, uint32_t>));

        skvx::Vec<4, uint32_t>* buffer0Cursor = fBuffer0;
        skvx::Vec<4, uint32_t>* buffer1Cursor = fBuffer1;

        // Given an expanded input pixel, move the window ahead using the leadingEdge value.
        auto processValue = [&](const skvx::Vec<4, uint32_t>& leadingEdge) {
            sum0 += leadingEdge;
            sum1 += sum0;

            skvx::Vec<4, uint64_t> w = skvx::cast<uint64_t>(sum1) * fDivisorFactor;
            skvx::Vec<4, uint32_t> value = skvx::cast<uint32_t>(w >> 32);

            sum1 -= *buffer1Cursor;
            *buffer1Cursor = sum0;
            buffer1Cursor = (buffer1Cursor + 1) < fBuffersEnd ? buffer1Cursor + 1 : fBuffer1;
            sum0 -= *buffer0Cursor;
            *buffer0Cursor = leadingEdge;
            buffer0Cursor = (buffer0Cursor + 1) < fBuffer1 ? buffer0Cursor + 1 : fBuffer0;

            return value;
        };

        auto srcStart = srcLeft - fBorder,
                srcEnd   = srcRight - fBorder,
                dstEnd   = dstRight,
                srcIdx   = srcStart,
                dstIdx   = 0;

        const uint32_t* srcCursor = src;
        uint32_t* dstCursor = dst;

        // The destination pixels are not effected by the src pixels,
        // change to zero as per the spec.
        // https://drafts.fxtf.org/filter-effects/#FilterPrimitivesOverviewIntro
        while (dstIdx < srcIdx) {
            *dstCursor = 0;
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }

        // The edge of the source is before the edge of the destination. Calculate the sums for
        // the pixels before the start of the destination.
        while (dstIdx > srcIdx) {
            skvx::Vec<4, uint32_t> leadingEdge =
                    srcIdx < srcEnd ? skvx::cast<uint32_t>(skvx::Vec<4, uint8_t>::Load(srcCursor))
                                    : 0;
            (void) processValue(leadingEdge);
            srcCursor += srcXStride;
            srcIdx++;
        }

        // The dstIdx and srcIdx are in sync now; the code just uses the dstIdx for both now.
        // Consume the source generating pixels to dst.
        auto loopEnd = std::min(dstEnd, srcEnd);
        while (dstIdx < loopEnd) {
            skvx::Vec<4, uint32_t> leadingEdge =
                    skvx::cast<uint32_t>(skvx::Vec<4, uint8_t>::Load(srcCursor));
            skvx::cast<uint8_t>(processValue(leadingEdge)).store(dstCursor);
            srcCursor += srcXStride;
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }

        // The leading edge is beyond the end of the source. Assume that the pixels
        // are now 0x0000 until the end of the destination.
        loopEnd = dstEnd;
        while (dstIdx < loopEnd) {
            skvx::cast<uint8_t>(processValue(0u)).store(dstCursor);
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }
    }

private:
    skvx::Vec<4, uint32_t>* const fBuffer0;
    skvx::Vec<4, uint32_t>* const fBuffer1;
    skvx::Vec<4, uint32_t>* const fBuffersEnd;
    const int fBorder;
    const uint32_t fDivisorFactor;
    const uint32_t fHalf;
};

sk_sp<SkSpecialImage> copy_image_with_bounds(
        const SkImageFilter_Base::Context& ctx, const sk_sp<SkSpecialImage> &input,
        SkIRect srcBounds, SkIRect dstBounds) {
    SkBitmap inputBM;
    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    SkBitmap src;
    inputBM.extractSubset(&src, srcBounds);

    // Make everything relative to the destination bounds.
    srcBounds.offset(-dstBounds.x(), -dstBounds.y());
    dstBounds.offset(-dstBounds.x(), -dstBounds.y());

    auto srcW = srcBounds.width(),
         dstW = dstBounds.width(),
         dstH = dstBounds.height();

    SkImageInfo dstInfo = SkImageInfo::Make(dstW, dstH, inputBM.colorType(), inputBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(dstInfo)) {
        return nullptr;
    }

    // There is no blurring to do, but we still need to copy the source while accounting for the
    // dstBounds. Remember that the src was intersected with the dst.
    int y = 0;
    size_t dstWBytes = dstW * sizeof(uint32_t);
    for (;y < srcBounds.top(); y++) {
        sk_bzero(dst.getAddr32(0, y), dstWBytes);
    }

    for (;y < srcBounds.bottom(); y++) {
        int x = 0;
        uint32_t* dstPtr = dst.getAddr32(0, y);
        for (;x < srcBounds.left(); x++) {
            *dstPtr++ = 0;
        }

        memcpy(dstPtr, src.getAddr32(x - srcBounds.left(), y - srcBounds.top()),
               srcW * sizeof(uint32_t));

        dstPtr += srcW;
        x += srcW;

        for (;x < dstBounds.right(); x++) {
            *dstPtr++ = 0;
        }
    }

    for (;y < dstBounds.bottom(); y++) {
        sk_bzero(dst.getAddr32(0, y), dstWBytes);
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(dstBounds.width(),
                                                          dstBounds.height()),
                                          dst, ctx.surfaceProps());
}

// TODO: Implement CPU backend for different fTileMode.
sk_sp<SkSpecialImage> cpu_blur(
        const SkImageFilter_Base::Context& ctx,
        SkVector sigma, const sk_sp<SkSpecialImage> &input,
        SkIRect srcBounds, SkIRect dstBounds) {
    SkVector limitedSigma = {SkTPin(sigma.x(), 0.0f, 2183.0f), SkTPin(sigma.y(), 0.0f, 2183.0f)};

    SkSTArenaAlloc<1024> alloc;
    auto makeMaker = [&](double sigma) -> PassMaker* {
        SkASSERT(0 <= sigma && sigma <= 2183);
        if (PassMaker* maker = GaussPass::MakeMaker(sigma, &alloc)) {
            return maker;
        }
        if (PassMaker* maker = TentPass::MakeMaker(sigma, &alloc)) {
            return maker;
        }
        SK_ABORT("Sigma is out of range.");
    };

    PassMaker* makerX = makeMaker(limitedSigma.x());
    PassMaker* makerY = makeMaker(limitedSigma.y());

    if (makerX->window() <= 1 && makerY->window() <= 1) {
        return copy_image_with_bounds(ctx, input, srcBounds, dstBounds);
    }

    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    SkBitmap src;
    inputBM.extractSubset(&src, srcBounds);

    // Make everything relative to the destination bounds.
    srcBounds.offset(-dstBounds.x(), -dstBounds.y());
    dstBounds.offset(-dstBounds.x(), -dstBounds.y());

    auto srcW = srcBounds.width(),
         srcH = srcBounds.height(),
         dstW = dstBounds.width(),
         dstH = dstBounds.height();

    SkImageInfo dstInfo = inputBM.info().makeWH(dstW, dstH);

    SkBitmap dst;
    if (!dst.tryAllocPixels(dstInfo)) {
        return nullptr;
    }

    size_t bufferSizeBytes = std::max(makerX->bufferSizeBytes(), makerY->bufferSizeBytes());
    auto buffer = alloc.makeBytesAlignedTo(bufferSizeBytes, alignof(skvx::Vec<4, uint32_t>));

    // Basic Plan: The three cases to handle
    // * Horizontal and Vertical - blur horizontally while copying values from the source to
    //     the destination. Then, do an in-place vertical blur.
    // * Horizontal only - blur horizontally copying values from the source to the destination.
    // * Vertical only - blur vertically copying values from the source to the destination.

    // Default to vertical only blur case. If a horizontal blur is needed, then these values
    // will be adjusted while doing the horizontal blur.
    auto intermediateSrc = static_cast<uint32_t *>(src.getPixels());
    auto intermediateRowBytesAsPixels = src.rowBytesAsPixels();
    auto intermediateWidth = srcW;

    // Because the border is calculated before the fork of the GPU/CPU path. The border is
    // the maximum of the two rendering methods. In the case where sigma is zero, then the
    // src and dst left values are the same. If sigma is small resulting in a window size of
    // 1, then border calculations add some pixels which will always be zero. Inset the
    // destination by those zero pixels. This case is very rare.
    auto intermediateDst = dst.getAddr32(srcBounds.left(), 0);

    // The following code is executed very rarely, I have never seen it in a real web
    // page. If sigma is small but not zero then shared GPU/CPU border calculation
    // code adds extra pixels for the border. Just clear everything to clear those pixels.
    // This solution is overkill, but very simple.
    if (makerX->window() == 1 || makerY->window() == 1) {
        dst.eraseColor(0);
    }

    if (makerX->window() > 1) {
        Pass* pass = makerX->makePass(buffer, &alloc);
        // Make int64 to avoid overflow in multiplication below.
        int64_t shift = srcBounds.top() - dstBounds.top();

        // For the horizontal blur, starts part way down in anticipation of the vertical blur.
        // For a vertical sigma of zero shift should be zero. But, for small sigma,
        // shift may be > 0 but the vertical window could be 1.
        intermediateSrc = static_cast<uint32_t *>(dst.getPixels())
                          + (shift > 0 ? shift * dst.rowBytesAsPixels() : 0);
        intermediateRowBytesAsPixels = dst.rowBytesAsPixels();
        intermediateWidth = dstW;
        intermediateDst = static_cast<uint32_t *>(dst.getPixels());

        const uint32_t* srcCursor = static_cast<uint32_t*>(src.getPixels());
        uint32_t* dstCursor = intermediateSrc;
        for (auto y = 0; y < srcH; y++) {
            pass->blur(srcBounds.left(), srcBounds.right(), dstBounds.right(),
                      srcCursor, 1, dstCursor, 1);
            srcCursor += src.rowBytesAsPixels();
            dstCursor += intermediateRowBytesAsPixels;
        }
    }

    if (makerY->window() > 1) {
        Pass* pass = makerY->makePass(buffer, &alloc);
        const uint32_t* srcCursor = intermediateSrc;
        uint32_t* dstCursor = intermediateDst;
        for (auto x = 0; x < intermediateWidth; x++) {
            pass->blur(srcBounds.top(), srcBounds.bottom(), dstBounds.bottom(),
                       srcCursor, intermediateRowBytesAsPixels,
                       dstCursor, dst.rowBytesAsPixels());
            srcCursor += 1;
            dstCursor += 1;
        }
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(dstBounds.width(),
                                                          dstBounds.height()),
                                          dst, ctx.surfaceProps());
}
}  // namespace

// This rather arbitrary-looking value results in a maximum box blur kernel size
// of 1000 pixels on the raster path, which matches the WebKit and Firefox
// implementations. Since the GPU path does not compute a box blur, putting
// the limit on sigma ensures consistent behaviour between the GPU and
// raster paths.
#define MAX_SIGMA SkIntToScalar(532)

static SkVector map_sigma(const SkSize& localSigma, const SkMatrix& ctm) {
    SkVector sigma = SkVector::Make(localSigma.width(), localSigma.height());
    ctm.mapVectors(&sigma, 1);
    sigma.fX = std::min(SkScalarAbs(sigma.fX), MAX_SIGMA);
    sigma.fY = std::min(SkScalarAbs(sigma.fY), MAX_SIGMA);
    return sigma;
}

sk_sp<SkSpecialImage> SkBlurImageFilter::onFilterImage(const Context& ctx,
                                                       SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);

    sk_sp<SkSpecialImage> input(this->filterInput(0, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    SkIRect inputBounds = SkIRect::MakeXYWH(inputOffset.fX, inputOffset.fY,
                                            input->width(), input->height());

    // Calculate the destination bounds.
    SkIRect dstBounds;
    if (!this->applyCropRect(this->mapContext(ctx), inputBounds, &dstBounds)) {
        return nullptr;
    }
    if (!inputBounds.intersect(dstBounds)) {
        return nullptr;
    }

    // Save the offset in preparation to make all rectangles relative to the inputOffset.
    SkIPoint resultOffset = SkIPoint::Make(dstBounds.fLeft, dstBounds.fTop);

    // Make all bounds relative to the inputOffset.
    inputBounds.offset(-inputOffset);
    dstBounds.offset(-inputOffset);

    SkVector sigma = map_sigma(fSigma, ctx.ctm());
    if (sigma.x() < 0 || sigma.y() < 0) {
        return nullptr;
    }

    sk_sp<SkSpecialImage> result;
#if SK_SUPPORT_GPU
    if (ctx.gpuBacked()) {
        // Ensure the input is in the destination's gamut. This saves us from having to do the
        // xform during the filter itself.
        input = ImageToColorSpace(input.get(), ctx.colorType(), ctx.colorSpace(),
                                  ctx.surfaceProps());
        result = this->gpuFilter(ctx, sigma, input, inputBounds, dstBounds, inputOffset,
                                 &resultOffset);
    } else
#endif
    {
        // Please see the comment on TentPass::MakeMaker for how the limit of 2183 for sigma is
        // calculated. The effective limit of blur is 532 which is set by the GPU above in
        // map_sigma.
        sigma.fX = SkTPin(sigma.fX, 0.0f, 2183.0f);
        sigma.fY = SkTPin(sigma.fY, 0.0f, 2183.0f);

        result = cpu_blur(ctx, sigma, input, inputBounds, dstBounds);
    }

    // Return the resultOffset if the blur succeeded.
    if (result != nullptr) {
        *offset = resultOffset;
    }
    return result;
}

#if SK_SUPPORT_GPU
sk_sp<SkSpecialImage> SkBlurImageFilter::gpuFilter(
        const Context& ctx, SkVector sigma, const sk_sp<SkSpecialImage> &input, SkIRect inputBounds,
        SkIRect dstBounds, SkIPoint inputOffset, SkIPoint* offset) const {
#if SK_GPU_V1
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(sigma.x()) &&
        SkGpuBlurUtils::IsEffectivelyZeroSigma(sigma.y())) {
        offset->fX = inputBounds.x() + inputOffset.fX;
        offset->fY = inputBounds.y() + inputOffset.fY;
        return input->makeSubset(inputBounds);
    }

    auto context = ctx.getContext();

    GrSurfaceProxyView inputView = input->view(context);
    if (!inputView.proxy()) {
        return nullptr;
    }
    SkASSERT(inputView.asTextureProxy());

    // TODO (michaelludwig) - The color space choice is odd, should it just be ctx.refColorSpace()?
    dstBounds.offset(input->subset().topLeft());
    inputBounds.offset(input->subset().topLeft());
    auto sdc = SkGpuBlurUtils::GaussianBlur(
            context,
            std::move(inputView),
            SkColorTypeToGrColorType(input->colorType()),
            input->alphaType(),
            ctx.colorSpace() ? sk_ref_sp(input->getColorSpace()) : nullptr,
            dstBounds,
            inputBounds,
            sigma.x(),
            sigma.y(),
            fTileMode);
    if (!sdc) {
        return nullptr;
    }

    return SkSpecialImage::MakeDeferredFromGpu(context,
                                               SkIRect::MakeSize(dstBounds.size()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               sdc->readSurfaceView(),
                                               sdc->colorInfo().colorType(),
                                               sk_ref_sp(input->getColorSpace()),
                                               ctx.surfaceProps());
#else // SK_GPU_V1
    return nullptr;
#endif // SK_GPU_V1
}
#endif

SkRect SkBlurImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    bounds.outset(fSigma.width() * 3, fSigma.height() * 3);
    return bounds;
}

SkIRect SkBlurImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                              MapDirection, const SkIRect* inputRect) const {
    SkVector sigma = map_sigma(fSigma, ctm);
    return src.makeOutset(SkScalarCeilToInt(sigma.x() * 3), SkScalarCeilToInt(sigma.y() * 3));
}

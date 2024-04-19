/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkVx.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>

struct SkIPoint;

#if defined(SK_GANESH) || defined(SK_GRAPHITE)
#include "src/gpu/BlurUtils.h"
#endif

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    #include <xmmintrin.h>
    #define SK_PREFETCH(ptr) _mm_prefetch(reinterpret_cast<const char*>(ptr), _MM_HINT_T0)
#elif defined(__GNUC__)
    #define SK_PREFETCH(ptr) __builtin_prefetch(ptr)
#else
    #define SK_PREFETCH(ptr)
#endif

namespace {

class SkBlurImageFilter final : public SkImageFilter_Base {
public:
    SkBlurImageFilter(SkSize sigma, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fSigma{sigma} {}

    SkBlurImageFilter(SkSize sigma, SkTileMode legacyTileMode, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fSigma(sigma)
            , fLegacyTileMode(legacyTileMode) {}

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterBlurImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlurImageFilter)

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    skif::LayerSpace<SkSize> mapSigma(const skif::Mapping& mapping, bool gpuBacked) const;

    skif::LayerSpace<SkIRect> kernelBounds(const skif::Mapping& mapping,
                                           skif::LayerSpace<SkIRect> bounds,
                                           bool gpuBacked) const {
        skif::LayerSpace<SkSize> sigma = this->mapSigma(mapping, gpuBacked);
        bounds.outset(skif::LayerSpace<SkSize>({3 * sigma.width(), 3 * sigma.height()}).ceil());
        return bounds;
    }

    skif::ParameterSpace<SkSize> fSigma;
    // kDecal means no legacy tiling, it will be handled by SkCropImageFilter instead. Legacy
    // tiling occurs when there's no provided crop rect, and should be deleted once clients create
    // their filters with defined tiling geometry.
    SkTileMode fLegacyTileMode = SkTileMode::kDecal;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Blur(
        SkScalar sigmaX, SkScalar sigmaY, SkTileMode tileMode, sk_sp<SkImageFilter> input,
        const CropRect& cropRect) {
    if (!SkIsFinite(sigmaX, sigmaY) || sigmaX < 0.f || sigmaY < 0.f) {
        // Non-finite or negative sigmas are error conditions. We allow 0 sigma for X and/or Y
        // for 1D blurs; onFilterImage() will detect when no visible blurring would occur based on
        // the Context mapping.
        return nullptr;
    }

    // Temporarily allow tiling with no crop rect
    if (tileMode != SkTileMode::kDecal && !cropRect) {
        return sk_make_sp<SkBlurImageFilter>(SkSize{sigmaX, sigmaY}, tileMode, std::move(input));
    }

    // The 'tileMode' behavior is not well-defined if there is no crop. We only apply it if
    // there is a provided 'cropRect'.
    sk_sp<SkImageFilter> filter = std::move(input);
    if (tileMode != SkTileMode::kDecal && cropRect) {
        // Historically the input image was restricted to the cropRect when tiling was not
        // kDecal, so that the kernel evaluated the tiled edge conditions, while a kDecal crop
        // only affected the output.
        filter = SkImageFilters::Crop(*cropRect, tileMode, std::move(filter));
    }

    filter = sk_make_sp<SkBlurImageFilter>(SkSize{sigmaX, sigmaY}, std::move(filter));
    if (cropRect) {
        // But regardless of the tileMode, the output is always decal cropped
        filter = SkImageFilters::Crop(*cropRect, SkTileMode::kDecal, std::move(filter));
    }
    return filter;
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

    // NOTE: For new SKPs, 'tileMode' holds the "legacy" tile mode; any originally specified tile
    // mode with valid tiling geometry is handled in the SkCropImageFilters that wrap the blur.
    // In a new SKP, when 'tileMode' is not kDecal, common.cropRect() will be null and the blur
    // will automatically emulate the legacy tiling.
    //
    // In old SKPs, the 'tileMode' and common.cropRect() may not be null. ::Blur() automatically
    // detects when this is a legacy or valid tiling and constructs the DAG appropriately.
    return SkImageFilters::Blur(
          sigmaX, sigmaY, tileMode, common.getInput(0), common.cropRect());
}

void SkBlurImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);

    buffer.writeScalar(SkSize(fSigma).fWidth);
    buffer.writeScalar(SkSize(fSigma).fHeight);
    buffer.writeInt(static_cast<int>(fLegacyTileMode));
}

///////////////////////////////////////////////////////////////////////////////

namespace {

// TODO: Move these functions into a CPU, 8888-only blur engine implementation; ideally share logic
// with the similar techniques in SkMaskBlurFilter on 4x A8 data.

// TODO(b/294575803): Provide a more accurate CPU implementation at s<2, at which point the notion
// of an identity sigma can be consolidated between the different functions.
// This is defined by the SVG spec:
// https://drafts.fxtf.org/filter-effects/#feGaussianBlurElement
int calculate_window(double sigma) {
    auto possibleWindow = static_cast<int>(floor(sigma * 3 * sqrt(2 * SK_DoublePI) / 4 + 0.5));
    return std::max(1, possibleWindow);
}

// This rather arbitrary-looking value results in a maximum box blur kernel size
// of 1000 pixels on the raster path, which matches the WebKit and Firefox
// implementations. Since the GPU path does not compute a box blur, putting
// the limit on sigma ensures consistent behaviour between the GPU and
// raster paths.
static constexpr SkScalar kMaxSigma = 532.f;

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

// TODO: Implement CPU backend for different fTileMode. This is still worth doing inline with the
// blur; at the moment the tiling is applied via the CropImageFilter and carried as metadata on
// the FilterResult. This is forcefully applied in onFilterImage() to get a simple SkSpecialImage to
// pass to cpu_blur or gpu_blur, which evaluates the tile mode into a kernel-outset buffer that is
// then processed by these functions. If the tilemode is the only thing being applied, it would be
// ideal to tile from the input image directly instead of inserting a new temporary image. For CPU
// blurs this temporary image now creates the appearance of correctness; for GPU blurs that could
// tile already it may create a regression.
sk_sp<SkSpecialImage> cpu_blur(const skif::Context& ctx,
                               skif::LayerSpace<SkSize> sigma,
                               const sk_sp<SkSpecialImage>& input,
                               skif::LayerSpace<SkIRect> srcBounds,
                               skif::LayerSpace<SkIRect> dstBounds) {
    // map_sigma limits sigma to 532 to match 1000px box filter limit of WebKit and Firefox.
    // Since this does not exceed the limits of the TentPass (2183), there won't be overflow when
    // computing a kernel over a pixel window filled with 255.
    static_assert(kMaxSigma <= 2183.0f);

    // The input image should fill the srcBounds
    SkASSERT(input->width() == srcBounds.width() && input->height() == srcBounds.height());

    SkSTArenaAlloc<1024> alloc;
    auto makeMaker = [&](double sigma) -> PassMaker* {
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
    // A no-op blur should have been caught earlier in onFilterImage().
    SkASSERT(makerX->window() > 1 || makerY->window() > 1);

    SkBitmap src;
    if (!SkSpecialImages::AsBitmap(input.get(), &src)) {
        return nullptr;
    }
    if (src.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    auto originalDstBounds = dstBounds;
    if (makerX->window() > 1) {
        // Inflate the dst by the window required for the Y pass so that the X pass can prepare it.
        // The Y pass will be offset to only write to the original rows in dstBounds, but its window
        // will access these extra rows calculated by the X pass. The SpecialImage factory will
        // then subset the bitmap so it appears to match 'originalDstBounds' tightly. We make one
        // slightly larger image to hold this extra data instead of two separate images sized
        // exactly to each pass because the CPU blur can write in place.
        const auto yPadding = skif::LayerSpace<SkSize>({0.f, 3 * sigma.height()}).ceil();
        dstBounds.outset(yPadding);
    }

    SkBitmap dst;
    const skif::LayerSpace<SkIPoint> dstOrigin = dstBounds.topLeft();
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
        // First an X-only blur from src into dst, including the extra rows that will become input
        // for the second Y pass, which will then be performed in place.
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
        // The new 'dst' is equal to dst.extractSubset(originalDstBounds.offset(-dstOrigin)), but
        // by construction only the Y offset has an interesting value so this is a little more
        // efficient.
        dstYOffset = originalDstBounds.top() - dstBounds.top();

        srcBounds = dstBounds;
        dstBounds = originalDstBounds;
    }

    // Iterate over each column to calculate 1D blur along Y. This is either blurring from src into
    // dst for a 1D blur; or it's blurring from dst into dst for the second pass of a 2D blur.
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

    originalDstBounds.offset(-dstOrigin); // Make relative to dst's pixels
    return SkSpecialImages::MakeFromRaster(SkIRect(originalDstBounds),
                                           dst,
                                           ctx.backend()->surfaceProps());
}

}  // namespace

skif::FilterResult SkBlurImageFilter::onFilterImage(const skif::Context& ctx) const {
    const bool gpuBacked = SkToBool(ctx.backend()->getBlurEngine());

    skif::Context inputCtx = ctx.withNewDesiredOutput(
            this->kernelBounds(ctx.mapping(), ctx.desiredOutput(), gpuBacked));

    skif::FilterResult childOutput = this->getChildOutput(0, inputCtx);
    skif::LayerSpace<SkSize> sigma = this->mapSigma(ctx.mapping(), gpuBacked);
    if (sigma.width() == 0.f && sigma.height() == 0.f) {
        // No actual blur, so just return the input unmodified
        return childOutput;
    }

    SkASSERT(sigma.width() >= 0.f && sigma.width() <= kMaxSigma &&
             sigma.height() >= 0.f && sigma.height() <= kMaxSigma);

    // TODO: This is equivalent to what Builder::blur() calculates under the hood, but is calculated
    // *before* we apply any legacy tile mode since the legacy tiling did not actually cause the
    // output to extend fully.
    skif::LayerSpace<SkIRect> maxOutput =
            this->kernelBounds(ctx.mapping(), childOutput.layerBounds(), gpuBacked);
    if (!maxOutput.intersect(ctx.desiredOutput())) {
        return {};
    }

    if (fLegacyTileMode != SkTileMode::kDecal) {
        // Legacy tiling applied to the input image when there was no explicit crop rect. Use the
        // child's output image's layer bounds as the crop rectangle to adjust the edge tile mode
        // without restricting the image.
        childOutput = childOutput.applyCrop(inputCtx,
                                            childOutput.layerBounds(),
                                            fLegacyTileMode);
    }

    // TODO(b/40039877): Once the CPU blur functions can handle tile modes and color types beyond
    // N32, there won't be any need to branch on how to apply the blur to the filter result.
    if (gpuBacked) {
        // For non-legacy tiling, 'maxOutput' is equal to the desired output. For decal's it matches
        // what Builder::blur() calculates internally. For legacy tiling, however, it's dependent on
        // the original child output's bounds ignoring the tile mode's effect.
        skif::Context croppedOutput = ctx.withNewDesiredOutput(maxOutput);
        skif::FilterResult::Builder builder{croppedOutput};
        builder.add(childOutput);
        return builder.blur(sigma);
    }

    // The CPU blur does not yet support tile modes so explicitly resolve it to a special image that
    // has the tiling rendered into the pixels.

    auto [resolvedChildOutput, origin] = childOutput.imageAndOffset(inputCtx);
    if (!resolvedChildOutput) {
        return {};
    }
    skif::LayerSpace<SkIRect> srcBounds{SkIRect::MakeXYWH(origin.x(),
                                                          origin.y(),
                                                          resolvedChildOutput->width(),
                                                          resolvedChildOutput->height())};

    return skif::FilterResult{cpu_blur(ctx, sigma, std::move(resolvedChildOutput),
                                       srcBounds, maxOutput),
                              maxOutput.topLeft()};
}

skif::LayerSpace<SkSize> SkBlurImageFilter::mapSigma(const skif::Mapping& mapping,
                                                     bool gpuBacked) const {
    skif::LayerSpace<SkSize> sigma = mapping.paramToLayer(fSigma);
    // Clamp to the maximum sigma
    sigma = skif::LayerSpace<SkSize>({std::min(sigma.width(), kMaxSigma),
                                      std::min(sigma.height(), kMaxSigma)});

    // TODO(b/294575803) - The CPU and GPU implementations have different requirements for
    // "identity", with the GPU able to handle smaller sigmas. calculate_window() returns <= 1 once
    // sigma is below ~0.8. Ideally we should work out the sigma threshold such that the max
    // contribution from adjacent pixels is less than 0.5/255 and use that for both backends.
    // NOTE: For convenience with builds, and the flux that is about to occur with the blur utils,
    // this GPU logic is just copied from GrBlurUtils

    // Disable bluring on axes that are not finite, or that are small enough that the blur is
    // effectively an identity.
    if (!SkIsFinite(sigma.width()) || (!gpuBacked && calculate_window(sigma.width()) <= 1)
#if defined(SK_GANESH) || defined(SK_GRAPHITE)
        || (gpuBacked && skgpu::BlurIsEffectivelyIdentity(sigma.width()))
#endif
    ) {
        sigma = skif::LayerSpace<SkSize>({0.f, sigma.height()});
    }

    if (!SkIsFinite(sigma.height()) || (!gpuBacked && calculate_window(sigma.height()) <= 1)
#if defined(SK_GANESH) || defined(SK_GRAPHITE)
        || (gpuBacked && skgpu::BlurIsEffectivelyIdentity(sigma.height()))
#endif
    ) {
        sigma = skif::LayerSpace<SkSize>({sigma.width(), 0.f});
    }

    return sigma;
}

skif::LayerSpace<SkIRect> SkBlurImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // Use gpuBacked=true since that has a more sensitive kernel, ensuring any layer input bounds
    // will be sufficient for both GPU and CPU evaluations.
    skif::LayerSpace<SkIRect> requiredInput =
            this->kernelBounds(mapping, desiredOutput, /*gpuBacked=*/true);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkBlurImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    auto childOutput = this->getChildOutputLayerBounds(0, mapping, contentBounds);
    if (childOutput) {
        // Use gpuBacked=true since it will ensure output bounds are conservative; CPU-based blurs
        // may produce 1px inset from this for very small sigmas.
        return this->kernelBounds(mapping, *childOutput, /*gpuBacked=*/true);
    } else {
        return skif::LayerSpace<SkIRect>::Unbounded();
    }
}

SkRect SkBlurImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    bounds.outset(SkSize(fSigma).width() * 3, SkSize(fSigma).height() * 3);
    return bounds;
}

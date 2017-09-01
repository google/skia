/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMaskBlurFilter.h"

#include <cmath>

#include "SkArenaAlloc.h"
#include "SkMakeUnique.h"
#include "SkSafeMath.h"

static const double kPi = 3.14159265358979323846264338327950288;

static uint64_t weight_from_diameter(uint32_t d) {
    uint64_t d2 = d * d;
    uint64_t d3 = d2 * d;
    if ((d&1) == 0) {
        // d * d * (d + 1);
        return d3 + d2;
    }

    return d3;
}

#if defined(SK_SUPPORT_LEGACY_USE_GAUSS_FOR_SMALL_RADII)
    static constexpr double kSmallSigma = 0.0;
#else
    static constexpr double kSmallSigma = 2.0;
#endif

//

static size_t filter_window(double sigma) {
    if (sigma < kSmallSigma) {
        auto radius = static_cast<size_t>(ceil(1.5 * sigma - 0.5));
        return 2 * radius + 1;
    }
    auto possibleWindow = static_cast<size_t>(floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5));
    return std::max(1u, possibleWindow);
}

static std::tuple<uint64_t, uint64_t> interp_factors(double sigma) {
    double radius = 1.5 * sigma - 0.5;
    double outerRadius = ceil(radius);
    double outerWindow = 2*outerRadius + 1;
    double innerRadius = outerRadius - 1;
    double innerWindow = 2*innerRadius + 1;

    // The inner and outer sums are divided by the window size to get the average over the window.
    // Then the two averages are weighted by how close the radius is to the inner or outer radius.
    double outerFactor = (radius - innerRadius) / outerWindow;
    double innerFactor = (outerRadius - radius) / innerWindow;

    return std::make_tuple(static_cast<uint64_t>(round(outerFactor * (1ull << 32))),
                           static_cast<uint64_t>(round(innerFactor * (1ull << 32))));
}

SkMaskBlurFilter::FilterInfo::FilterInfo(double sigma)
    : fIsSmall{sigma < kSmallSigma}
    , fFilterWindow{filter_window(sigma)}
    , fWeight{fIsSmall ? fFilterWindow : weight_from_diameter(fFilterWindow)}
    , fScaledWeight{(static_cast<uint64_t>(1) << 32) / fWeight}
    , fInterpFactors{interp_factors(sigma)}
{
    SkASSERT(sigma >= 0);
}

uint64_t SkMaskBlurFilter::FilterInfo::weight() const {
    return fWeight;

}
uint32_t SkMaskBlurFilter::FilterInfo::borderSize() const {
    if (this->isSmall()) {
        return (fFilterWindow - 1) / 2;
    }

    if ((fFilterWindow&1) == 0) {
        return 3 * (fFilterWindow / 2) - 1;
    }

    return 3 * (fFilterWindow / 2);
}

size_t SkMaskBlurFilter::FilterInfo::diameter(uint8_t pass) const {
    SkASSERT(pass <= 2);

    if ((fFilterWindow&1) == 0) {
        // Handle even case.
        switch (pass) {
            case 0: return fFilterWindow;
            case 1: return fFilterWindow;
            case 2: return fFilterWindow+1;
        }
    }

    return fFilterWindow;
}

uint64_t SkMaskBlurFilter::FilterInfo::scaledWeight() const {
    return fScaledWeight;
}

bool SkMaskBlurFilter::FilterInfo::isSmall() const {
    return fIsSmall;
}

std::tuple<uint64_t, uint64_t> SkMaskBlurFilter::FilterInfo::interpFactors() const {
    return fInterpFactors;
};

//==================================================================================================
class BlurScanInterface {
public:
    virtual ~BlurScanInterface() = default;
    virtual void Blur(const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                      uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const = 0;
};

class PlanningInterface {
public:
    virtual ~PlanningInterface() = default;
    virtual size_t bufferSize() const = 0;
    virtual size_t border() const = 0;
    virtual BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, size_t width, uint32_t* buffer) const = 0;
};


// This class is deprecated, and will be replaced by Box.
class PlanBoxInteger final : public PlanningInterface {
public:
    PlanBoxInteger(double sigma) {
        // Calculate the radius from sigma. Taken from the old code until something better is
        // figured out.
        auto possibleRadius = 1.5 * sigma - 0.5;
        auto radius = std::max(1.0, std::ceil(possibleRadius));
        auto window = 2 * radius + 1;

        fWindow = static_cast<size_t>(window);
    }

    size_t bufferSize() const override { return fWindow - 1;       }

    size_t border()     const override { return (fWindow - 1) / 2; }

    BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, size_t width, uint32_t* buffer) const override
    {
        auto weight = static_cast<uint64_t>(std::round(1.0 / fWindow * (1ull << 32)));
        auto bufferEnd = buffer + this->bufferSize();
        size_t noChangeCount = fWindow > width ? fWindow - width : 0;

        return alloc->make<BoxInteger>(weight, buffer, bufferEnd, noChangeCount);
    }

private:
    class BoxInteger final : public BlurScanInterface {
    public:
        BoxInteger(uint64_t weight, uint32_t* buffer, uint32_t* bufferEnd, size_t noChangeCount)
            : fWeight{weight}
            , fBuffer0{buffer}
            , fBuffer0End{bufferEnd}
            , fNoChangeCount{noChangeCount} { }

        void Blur(const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                  uint8_t* dst, size_t dstStride, uint8_t* dstEnd) const override {
            auto buffer0Cursor = fBuffer0;
            std::memset(&fBuffer0[0], 0, (fBuffer0End - fBuffer0) * sizeof(*fBuffer0));
            uint32_t sum0 = 0;

            // Consume the source generating pixels.
            for (auto srcCursor = src;
                 srcCursor < srcEnd; dst += dstStride, srcCursor += srcStride) {
                uint32_t s = *srcCursor;
                sum0 += s;

                *dst = this->finalScale(sum0);

                sum0 -= *buffer0Cursor;
                *buffer0Cursor = s;
                buffer0Cursor =
                    (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : &fBuffer0[0];
            }

            // This handles the case when both ends of the box are not between [src, srcEnd), and
            // both are zero at that point.
            for (auto i = 0; i < fNoChangeCount; i++) {
                uint32_t s = 0;
                sum0 += s;

                *dst = this->finalScale(sum0);

                sum0 -= *buffer0Cursor;
                *buffer0Cursor = s;
                buffer0Cursor =
                    (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : &fBuffer0[0];
                dst += dstStride;
            }

            // Starting from the right, fill in the rest of the buffer.
            std::memset(&fBuffer0[0], 0, (fBuffer0End - &fBuffer0[0]) * sizeof(fBuffer0[0]));

            sum0 = 0;

            uint8_t* dstCursor = dstEnd;
            const uint8_t* srcCursor = srcEnd;
            do {
                dstCursor -= dstStride;
                srcCursor -= srcStride;
                uint32_t s = *srcCursor;
                sum0 += s;

                *dstCursor = this->finalScale(sum0);

                sum0 -= *buffer0Cursor;
                *buffer0Cursor = s;
                buffer0Cursor =
                    (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : &fBuffer0[0];
            } while (dstCursor > dst);
        }

        static constexpr uint64_t kHalf = static_cast<uint64_t>(1) << 31;

        uint8_t finalScale(uint32_t sum) const {
            return SkTo<uint8_t>((fWeight * sum + kHalf) >> 32);
        }

        const uint64_t  fWeight;
        uint32_t* const fBuffer0;
        uint32_t* const fBuffer0End;
        const size_t    fNoChangeCount;
    };

    size_t fWindow;
};

class PlanBox final : public PlanningInterface {
public:
    PlanBox(double sigma) {
        // Calculate the radius from sigma. Taken from the old code until something better is
        // figured out.
        auto possibleRadius = 1.5 * sigma - 0.5;
        auto radius = std::max(1.0, possibleRadius);
        auto outerRadius = std::ceil(radius);
        auto outerWindow = 2 * outerRadius + 1;
        auto outerFactor = (1 - (outerRadius - radius)) / outerWindow;
        fOuterWeight = static_cast<uint64_t>(std::round(outerFactor * (1ull << 32)));

        auto innerRadius = outerRadius - 1;
        auto innerWindow = 2 * innerRadius + 1;
        auto innerFactor = (1 - (radius - innerRadius)) / innerWindow;
        fInnerWeight = static_cast<uint64_t>(std::round(innerFactor * (1ull << 32)));

        // Sliding window is defined by the relationship between the outer and inner widows.
        // In the single window case, you add the element on the right, and subtract the element on
        // the left. But, because two windows are used, this relationship is more complicated; an
        // element is added from the right of the outer window, subtracted from the left of the
        // inner window. Because innerWindow = outerWindow - 2, the distance between
        // the left and right in the two window case is outerWindow - 1.
        fSlidingWindow = static_cast<size_t>(outerWindow - 1);
    }

    size_t bufferSize() const override { return 0; }

    // Remember that sliding window = window - 1. Therefore, radius = sliding window / 2.
    size_t border()     const override { return fSlidingWindow / 2; }

    BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, size_t width, uint32_t* buffer) const override
    {
        size_t noChangeCount;
        size_t trailingEdgeZeroCount;

        // The relation between the slidingWindow and the width dictates two operating modes.
        // * width >= slidingWindow - both sides of the window are contained in the image while
        // scanning. Therefore, we assume that slidingWindow zeros are consumed on the trailing
        // edge of the window. After this count, the both edges are traversing the image.
        // * slidingWindow > width - both sides of the window are off the image while scanning
        // the middle. The front edge of the window can only travel width until it falls off the
        // image. At this point, both edges of the window are off the image consuming zeros
        // and therefore, the destination value does not change. The scan produces unchanged
        // values until the trailing edge of the window enters the image. This count is
        // slidingWindow - width.
        if (width >= fSlidingWindow) {
            noChangeCount = 0;
            trailingEdgeZeroCount = fSlidingWindow;
        } else {
            noChangeCount = fSlidingWindow - width;
            trailingEdgeZeroCount = width;
        }
        return alloc->make<Box>(fOuterWeight, fInnerWeight, noChangeCount, trailingEdgeZeroCount);

    }

private:
    class Box final : public BlurScanInterface {
    public:
        Box(uint64_t outerWeight, uint64_t innerWeight,
            size_t noChangeCount, size_t trailingEdgeZeroCount)
            : fOuterWeight{outerWeight}
            , fInnerWeight{innerWeight}
            , fNoChangeCount{noChangeCount}
            , fTrailingEdgeZeroCount{trailingEdgeZeroCount} { }

        void Blur(const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                  uint8_t* dst, size_t dstStride, uint8_t* dstEnd) const override {
            auto rightOuter = src;
            auto dstCursor = dst;

            uint32_t outerSum = 0;
            uint32_t innerSum = 0;
            for (size_t i = 0; i < fTrailingEdgeZeroCount; i++) {
                innerSum = outerSum;
                outerSum += *rightOuter;
                *dstCursor = this->interpolateSums(outerSum, innerSum);

                rightOuter += srcStride;
                dstCursor += dstStride;
            }

            // slidingWindow > width
            for (auto i = 0; i < fNoChangeCount; i++) {
                *dstCursor = this->interpolateSums(outerSum, innerSum);;
                dstCursor += dstStride;
            }

            // width > slidingWindow
            auto leftInner = src;
            while (rightOuter < srcEnd) {
                innerSum = outerSum - *leftInner;
                outerSum += *rightOuter;
                *dstCursor = this->interpolateSums(outerSum, innerSum);
                outerSum -= *leftInner;

                rightOuter += srcStride;
                leftInner += srcStride;
                dstCursor += dstStride;
            }

            auto leftOuter = srcEnd;
            dstCursor = dstEnd;
            outerSum = 0;
            for (size_t i = 0; i < fTrailingEdgeZeroCount; i++) {
                leftOuter -= srcStride;
                dstCursor -= dstStride;

                innerSum = outerSum;
                outerSum += *leftOuter;
                *dstCursor = this->interpolateSums(outerSum, innerSum);
            }
        }

    private:
        static constexpr uint64_t kHalf = static_cast<uint64_t>(1) << 31;

        uint8_t interpolateSums(uint32_t outerSum, uint32_t innerSum) const {
            return SkTo<uint8_t>((fOuterWeight * outerSum + fInnerWeight * innerSum + kHalf) >> 32);
        };
        uint64_t fOuterWeight;
        uint64_t fInnerWeight;
        size_t fNoChangeCount;
        size_t fTrailingEdgeZeroCount;
    };
private:
    uint64_t fOuterWeight;
    uint64_t fInnerWeight;
    size_t   fSlidingWindow;
};

class PlanGauss final : public PlanningInterface {
    PlanGauss(double sigma) {
        auto possibleWindow = static_cast<size_t>(floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5));
        auto window = std::max(static_cast<size_t>(1), possibleWindow);

        fPass0Size = window - 1;
        fPass1Size = window - 1;
        fPass2Size = (window & 1) == 1 ? window - 1 : window;

        fBorder = (window & 1) == 1 ? 3 * ((window - 1) / 2) : 3 * (window / 2) - 1;
        fSlidingWindow = 2 * fBorder + 1;

        // If the window is odd then the divisor is just window ^ 3 otherwise,
        // it is window * window * (window + 1) = window ^ 2 + window ^ 3;
        auto window2 = window * window;
        auto window3 = window2 * window;
        auto divisor = (window & 1) == 1 ? window3 : window3 + window2;
        fWeight = static_cast<uint64_t>(std::round(1.0 / divisor * (1ull << 32)));
    }

    size_t bufferSize() const override { return fPass0Size + fPass1Size + fPass2Size; }

    size_t border()     const override { return fBorder; }

    BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, size_t width, uint32_t* buffer) const override
    {
        uint32_t* buffer0, *buffer0End, *buffer1, *buffer1End, *buffer2, *buffer2End;
        buffer0 = buffer;
        buffer0End = buffer1 = buffer0 + fPass0Size;
        buffer1End = buffer2 = buffer1 + fPass1Size;
        buffer2End = buffer2 + fPass2Size;
        size_t noChangeCount = fSlidingWindow - width;

        return alloc->make<Gauss>(
            fWeight, noChangeCount,
            buffer0, buffer0End,
            buffer1, buffer1End,
            buffer2, buffer2End);
    }

public:
    class Gauss final : public BlurScanInterface {
    public:
        Gauss(uint64_t weight, size_t noChangeCount,
              uint32_t* buffer0, uint32_t* buffer0End,
              uint32_t* buffer1, uint32_t* buffer1End,
              uint32_t* buffer2, uint32_t* buffer2End)
            : fWeight{weight}
            , fBuffer0{buffer0}
            , fBuffer0End{buffer0End}
            , fBuffer1{buffer1}
            , fBuffer1End{buffer1End}
            , fBuffer2{buffer2}
            , fBuffer2End{buffer2End}
        { }

        void Blur(const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                  uint8_t* dst, size_t dstStride, uint8_t* dstEnd) const override {
            auto buffer0Cursor = fBuffer0;
            auto buffer1Cursor = fBuffer1;
            auto buffer2Cursor = fBuffer2;

            std::memset(fBuffer0, 0, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

            uint32_t sum0 = 0;
            uint32_t sum1 = 0;
            uint32_t sum2 = 0;

            // Consume the source generating pixels.
            for (auto srcCursor = src;
                 srcCursor < srcEnd; dst += dstStride, srcCursor += srcStride) {
                uint32_t s = *srcCursor;
                sum0 += s;
                sum1 += sum0;
                sum2 += sum1;

                *dst = this->finalScale(sum2);

                sum2 -= *buffer2Cursor;
                *buffer2Cursor = sum1;
                buffer2Cursor = (buffer2Cursor + 1) < fBuffer2End ? buffer2Cursor + 1 : fBuffer2;

                sum1 -= *buffer1Cursor;
                *buffer1Cursor = sum0;
                buffer1Cursor = (buffer1Cursor + 1) < fBuffer1End ? buffer1Cursor + 1 : fBuffer1;

                sum0 -= *buffer0Cursor;
                *buffer0Cursor = s;
                buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;
            }

            for (auto i = 0; i < fNoChangeCount; i++) {
                uint32_t s = 0;
                sum0 += s;
                sum1 += sum0;
                sum2 += sum1;

                *dst = this->finalScale(sum2);

                sum2 -= *buffer2Cursor;
                *buffer2Cursor = sum1;
                buffer2Cursor = (buffer2Cursor + 1) < fBuffer2End ? buffer2Cursor + 1 : fBuffer2;

                sum1 -= *buffer1Cursor;
                *buffer1Cursor = sum0;
                buffer1Cursor = (buffer1Cursor + 1) < fBuffer1End ? buffer1Cursor + 1 : fBuffer1;

                sum0 -= *buffer0Cursor;
                *buffer0Cursor = s;
                buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;
                dst += dstStride;
            }

            // Starting from the right, fill in the rest of the buffer.
            std::memset(fBuffer0, 0, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

            sum0 = sum1 = sum2 = 0;

            uint8_t* dstCursor = dstEnd;
            const uint8_t* srcCursor = srcEnd;
            do {
                dstCursor -= dstStride;
                srcCursor -= srcStride;
                uint32_t s = *srcCursor;
                sum0 += s;
                sum1 += sum0;
                sum2 += sum1;

                *dstCursor = this->finalScale(sum2);

                sum2 -= *buffer2Cursor;
                *buffer2Cursor = sum1;
                buffer2Cursor = (buffer2Cursor + 1) < fBuffer2End ? buffer2Cursor + 1 : fBuffer2;

                sum1 -= *buffer1Cursor;
                *buffer1Cursor = sum0;
                buffer1Cursor = (buffer1Cursor + 1) < fBuffer1End ? buffer1Cursor + 1 : fBuffer1;

                sum0 -= *buffer0Cursor;
                *buffer0Cursor = s;
                buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;
            } while (dstCursor > dst);
        }

    private:
        static constexpr uint64_t kHalf = static_cast<uint64_t>(1) << 31;

        uint8_t finalScale(uint32_t sum) const {
            return SkTo<uint8_t>((fWeight * sum + kHalf) >> 32);
        }

        uint32_t* fBuffer0;
        uint32_t* fBuffer0End;
        uint32_t* fBuffer1;
        uint32_t* fBuffer1End;
        uint32_t* fBuffer2;
        uint32_t* fBuffer2End;
        uint64_t fWeight;
        size_t fNoChangeCount;
    };

    uint64_t fWeight;
    size_t   fBorder;
    size_t   fSlidingWindow;
    size_t   fPass0Size;
    size_t   fPass1Size;
    size_t   fPass2Size;
};

static PlanningInterface* make_plan(SkArenaAlloc* alloc, double sigma) {
    PlanningInterface* plan = nullptr;

    if (3 * sigma > 1 && sigma < 2) {
        plan = alloc->make<PlanBox>(sigma);
    } else {
        plan = alloc->make<PlanGauss>(sigma);
    }

    return plan;
};

//==================================================================================================

SkMaskBlurFilter::SkMaskBlurFilter(double sigmaW, double sigmaH)
    : fInfoW{sigmaW}, fInfoH{sigmaH}
    , fBuffer0{skstd::make_unique_default<uint32_t[]>(bufferSize(0))}
    , fBuffer1{skstd::make_unique_default<uint32_t[]>(bufferSize(1))}
    , fBuffer2{skstd::make_unique_default<uint32_t[]>(bufferSize(2))} {
}

bool SkMaskBlurFilter::hasNoBlur() const {
    return fInfoW.weight() <= 1 && fInfoH.weight() <= 1;
}

SkIPoint SkMaskBlurFilter::blur(const SkMask& src, SkMask* dst) const {

    uint64_t weightW = fInfoW.weight();
    uint64_t weightH = fInfoH.weight();

    size_t borderW = fInfoW.borderSize();
    size_t borderH = fInfoH.borderSize();

    size_t srcW = src.fBounds.width();
    size_t srcH = src.fBounds.height();

    SkSafeMath safe;

    // size_t dstW = srcW + 2 * borderW;
    size_t dstW = safe.add(srcW, safe.add(borderW, borderW));
    //size_t dstH = srcH + 2 * borderH;
    size_t dstH = safe.add(srcH, safe.add(borderH, borderH));

    dst->fBounds.set(0, 0, dstW, dstH);
    dst->fBounds.offset(src.fBounds.x(), src.fBounds.y());
    dst->fBounds.offset(-SkTo<int32_t>(borderW), -SkTo<int32_t>(borderH));

    dst->fImage = nullptr;
    dst->fRowBytes = dstW;
    dst->fFormat = SkMask::kA8_Format;

    if (src.fImage == nullptr) {
        return {SkTo<int32_t>(borderW), SkTo<int32_t>(borderH)};
    }

    size_t toAlloc = safe.mul(dstW, dstH);
    if (!safe) {
        dst->fBounds = SkIRect::MakeEmpty();
        // There is no border offset because we are not drawing.
        return {0, 0};
    }
    dst->fImage = SkMask::AllocImage(toAlloc);

    SkArenaAlloc alloc;

    PlanningInterface* planW = make_plan(&alloc, fSigmaW);
    PlanningInterface* planH = make_plan(&alloc, fSigmaH);

    if (weightW > 1 && weightH > 1) {
        // Blur both directions.
        size_t tmpW = srcH;
        size_t tmpH = dstW;
        auto tmp = skstd::make_unique_default<uint8_t[]>(tmpW * tmpH);

        // Blur horizontally, and transpose.
        for (size_t y = 0; y < srcH; y++) {
            auto srcStart = &src.fImage[y * src.fRowBytes];
            auto tmpStart = &tmp[y];
            this->blurOneScan(fInfoW, srcW,
                              srcStart, 1, srcStart + srcW,
                              tmpStart, tmpW, tmpStart + tmpW * tmpH);
        }

        // Blur vertically (scan in memory order because of the transposition),
        // and transpose back to the original orientation.
        for (size_t y = 0; y < tmpH; y++) {
            auto tmpStart = &tmp[y * tmpW];
            auto dstStart = &dst->fImage[y];
            this->blurOneScan(fInfoH, srcH,
                              tmpStart, 1, tmpStart + tmpW,
                              dstStart, dst->fRowBytes, dstStart + dst->fRowBytes * dstH);
        }
    } else if (weightW > 1) {
        // Blur only horizontally.

        for (size_t y = 0; y < srcH; y++) {
            auto srcStart = &src.fImage[y * src.fRowBytes];
            auto dstStart = &dst->fImage[y * dst->fRowBytes];
            this->blurOneScan(fInfoW, srcW,
                              srcStart, 1, srcStart + srcW,
                              dstStart, 1, dstStart + dstW);
        }
    } else if (weightH > 1) {
        // Blur only vertically.

        for (size_t x = 0; x < srcW; x++) {
            auto srcStart = &src.fImage[x];
            auto srcEnd   = &src.fImage[src.fRowBytes * srcH];
            auto dstStart = &dst->fImage[x];
            auto dstEnd   = &dst->fImage[dst->fRowBytes * dstH];
            this->blurOneScan(fInfoH, srcH,
                              srcStart, src.fRowBytes, srcEnd,
                              dstStart, dst->fRowBytes, dstEnd);
        }
    } else {
        // Copy to dst. No Blur.
        SkASSERT(false);    // should not get here
        for (size_t y = 0; y < srcH; y++) {
            std::memcpy(&dst->fImage[y * dst->fRowBytes], &src.fImage[y * src.fRowBytes], dstW);
        }
    }

    return {SkTo<int32_t>(borderW), SkTo<int32_t>(borderH)};
}

size_t SkMaskBlurFilter::bufferSize(uint8_t bufferPass) const {
    return std::max(fInfoW.diameter(bufferPass), fInfoH.diameter(bufferPass)) - 1;
}

// Blur one horizontal scan into the dst.
void SkMaskBlurFilter::blurOneScan(
    const FilterInfo& info, size_t width,
    const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
          uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const {
    // We don't think this is good for quality. It is good for compatibility
    // with previous expectations...
    if (info.isSmall()) {
        #if defined(SK_LEGACY_SUPPORT_INTEGER_SMALL_RADII)
            this->blurOneScanBox(info, src, srcStride, srcEnd, dst, dstStride, dstEnd);
        #else
            this->blurOneScanBoxInterp(info, width, src, srcStride, srcEnd, dst, dstStride, dstEnd);
        #endif

    } else {
        this->blurOneScanGauss(info, src, srcStride, srcEnd, dst, dstStride, dstEnd);
    }

}


static uint8_t final_scale(uint64_t weight, uint32_t sum) {
    return SkTo<uint8_t>((weight * sum + half) >> 32);
}

static uint8_t interp_final_scale(uint64_t outerWeight, uint32_t outerSum,
                                  uint64_t innerWeight, uint32_t innerSum) {
    return SkTo<uint8_t>((outerWeight * outerSum + innerWeight * innerSum + half) >> 32);
}


// Blur one horizontal scan into the dst.
void SkMaskBlurFilter::blurOneScanBox(
    const FilterInfo& info,
    const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
    uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const {

    auto buffer0Begin = &fBuffer0[0];
    auto buffer0Cursor = buffer0Begin;
    auto buffer0End = &fBuffer0[0] + info.diameter(0) - 1;
    std::memset(&fBuffer0[0], 0, (buffer0End - buffer0Begin) * sizeof(fBuffer0[0]));
    uint32_t sum0 = 0;
    const uint64_t half = static_cast<uint64_t>(1) << 31;

    // Consume the source generating pixels.
    for (auto srcCursor = src; srcCursor < srcEnd; dst += dstStride, srcCursor += srcStride) {
        uint32_t s = *srcCursor;
        sum0 += s;

        *dst = SkTo<uint8_t>((info.scaledWeight() * sum0 + half) >> 32);


        sum0 -= *buffer0Cursor;
        *buffer0Cursor = s;
        buffer0Cursor = (buffer0Cursor + 1) < buffer0End ? buffer0Cursor + 1 : &fBuffer0[0];
    }

    // This handles the case when both ends of the box are not between [src, srcEnd), and both
    // are zero at that point.
    for (auto i = 0; i < static_cast<ptrdiff_t>(2 * info.borderSize()) - (srcEnd - src); i++) {
        uint32_t s = 0;
        sum0 += s;

        *dst = SkTo<uint8_t>((info.scaledWeight() * sum0 + half) >> 32);

        sum0 -= *buffer0Cursor;
        *buffer0Cursor = s;
        buffer0Cursor = (buffer0Cursor + 1) < buffer0End ? buffer0Cursor + 1 : &fBuffer0[0];
        dst += dstStride;
    }

    // Starting from the right, fill in the rest of the buffer.
    std::memset(&fBuffer0[0], 0, (buffer0End - &fBuffer0[0]) * sizeof(fBuffer0[0]));

    sum0 = 0;

    uint8_t* dstCursor = dstEnd;
    const uint8_t* srcCursor = srcEnd;
    do {
        dstCursor -= dstStride;
        srcCursor -= srcStride;
        uint32_t s = *srcCursor;
        sum0 += s;

        *dstCursor = SkTo<uint8_t>((info.scaledWeight() * sum0 + half) >> 32);

        sum0 -= *buffer0Cursor;
        *buffer0Cursor = s;
        buffer0Cursor = (buffer0Cursor + 1) < buffer0End ? buffer0Cursor + 1 : &fBuffer0[0];
    } while (dstCursor > dst);

}

// Blur one horizontal scan into the dst.
void SkMaskBlurFilter::blurOneScanBoxInterp(
    const FilterInfo& info, size_t width,
    const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
          uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const
{
    auto interp = [](uint64_t outerWeight, uint32_t outerSum,
                     uint64_t innerWeight, uint32_t innerSum) -> uint8_t {
        return SkTo<uint8_t>((outerWeight * outerSum + innerWeight * innerSum + half) >> 32);
    };

    uint64_t outerFactor, innerFactor;
    std::tie(outerFactor, innerFactor) = info.interpFactors();

    // There are sever windows used in thinking about this algorithm. There is the outer window
    // that is generated by the ceiling of the radius. There is the inner window that is two
    // smaller than the outer window, and they are centered on each other. Then there is the
    // scanning window which is from the right edge of the outer window to the left edge of the
    // inner window.
    auto outerWindow = info.diameter(2);
    auto slidingWindow = outerWindow - 1;
    auto border = std::min(width, slidingWindow);

    auto rightOuter = src;
    auto dstCursor = dst;

    uint32_t outerSum = 0;
    uint32_t innerSum = 0;
    for (size_t i = 0; i < border; i++) {
        innerSum = outerSum;
        outerSum += *rightOuter;
        *dstCursor = interp(outerFactor, outerSum, innerFactor, innerSum);
        rightOuter += srcStride;
        dstCursor += dstStride;
    }

    // Consider the two filter cases:
    // * slidingWindow > width - in this case the right edge of the window reaches the end of the
    //                           source data before left edge starts consuming source data. This
    //                           means that the will be adding and subtracting zero as it advances,
    //                           the sum never changing.
    // * width < slidingWindow - in this case the right edge of the window will continue consuming
    //                           source data after the left edge of the window starts consuming
    //                           source data.

    if (slidingWindow > width) {
        auto v = interp(outerFactor, outerSum, innerFactor, innerSum);
        for (auto i = width; i < slidingWindow; i++) {
            *dstCursor = v;
            dstCursor += dstStride;
        }
    } else if (slidingWindow < width) {
        auto leftInner = src;
        while (rightOuter < srcEnd) {
            innerSum = outerSum - *leftInner;
            outerSum += *rightOuter;
            *dstCursor = interp(outerFactor, outerSum, innerFactor, innerSum);
            outerSum -= *leftInner;
            rightOuter += srcStride;
            leftInner += srcStride;
            dstCursor += dstStride;
        }
    }

    auto leftOuter = srcEnd;
    dstCursor = dstEnd;
    outerSum = 0;
    for (size_t i = 0; i < border; i++) {
        leftOuter -= srcStride;
        dstCursor -= dstStride;
        innerSum = outerSum;
        outerSum += *leftOuter;
        *dstCursor = interp(outerFactor, outerSum, innerFactor, innerSum);
    }
}

// Blur one horizontal scan into the dst.
void SkMaskBlurFilter::blurOneScanGauss(
    const FilterInfo& info,
    const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
          uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const {

    auto buffer0Begin = &fBuffer0[0];
    auto buffer1Begin = &fBuffer1[0];
    auto buffer2Begin = &fBuffer2[0];

    auto buffer0Cursor = buffer0Begin;
    auto buffer1Cursor = buffer1Begin;
    auto buffer2Cursor = buffer2Begin;

    auto buffer0End = &fBuffer0[0] + info.diameter(0) - 1;
    auto buffer1End = &fBuffer1[0] + info.diameter(1) - 1;
    auto buffer2End = &fBuffer2[0] + info.diameter(2) - 1;

    std::memset(&fBuffer0[0], 0, (buffer0End - buffer0Begin) * sizeof(fBuffer0[0]));
    std::memset(&fBuffer1[0], 0, (buffer1End - buffer1Begin) * sizeof(fBuffer1[0]));
    std::memset(&fBuffer2[0], 0, (buffer2End - buffer2Begin) * sizeof(fBuffer2[0]));

    uint32_t sum0 = 0;
    uint32_t sum1 = 0;
    uint32_t sum2 = 0;

    // Consume the source generating pixels.
    for (auto srcCursor = src; srcCursor < srcEnd; dst += dstStride, srcCursor += srcStride) {
        uint32_t s = *srcCursor;
        sum0 += s;
        sum1 += sum0;
        sum2 += sum1;

        *dst = final_scale(info.scaledWeight(), sum2);

        sum2 -= *buffer2Cursor;
        *buffer2Cursor = sum1;
        buffer2Cursor = (buffer2Cursor + 1) < buffer2End ? buffer2Cursor + 1 : &fBuffer2[0];

        sum1 -= *buffer1Cursor;
        *buffer1Cursor = sum0;
        buffer1Cursor = (buffer1Cursor + 1) < buffer1End ? buffer1Cursor + 1 : &fBuffer1[0];

        sum0 -= *buffer0Cursor;
        *buffer0Cursor = s;
        buffer0Cursor = (buffer0Cursor + 1) < buffer0End ? buffer0Cursor + 1 : &fBuffer0[0];
    }

    // This handles the case when both ends of the box are not between [src, srcEnd), and both
    // are zero at that point.
    for (auto i = 0; i < static_cast<ptrdiff_t>(2 * info.borderSize()) - (srcEnd - src); i++) {
        uint32_t s = 0;
        sum0 += s;
        sum1 += sum0;
        sum2 += sum1;

        *dst = final_scale(info.scaledWeight(), sum2);

        sum2 -= *buffer2Cursor;
        *buffer2Cursor = sum1;
        buffer2Cursor = (buffer2Cursor + 1) < buffer2End ? buffer2Cursor + 1 : &fBuffer2[0];

        sum1 -= *buffer1Cursor;
        *buffer1Cursor = sum0;
        buffer1Cursor = (buffer1Cursor + 1) < buffer1End ? buffer1Cursor + 1 : &fBuffer1[0];

        sum0 -= *buffer0Cursor;
        *buffer0Cursor = s;
        buffer0Cursor = (buffer0Cursor + 1) < buffer0End ? buffer0Cursor + 1 : &fBuffer0[0];
        dst += dstStride;
    }

    // Starting from the right, fill in the rest of the buffer.
    std::memset(&fBuffer0[0], 0, (buffer0End - &fBuffer0[0]) * sizeof(fBuffer0[0]));
    std::memset(&fBuffer1[0], 0, (buffer1End - &fBuffer1[0]) * sizeof(fBuffer1[0]));
    std::memset(&fBuffer2[0], 0, (buffer2End - &fBuffer2[0]) * sizeof(fBuffer2[0]));

    sum0 = sum1 = sum2 = 0;

    uint8_t* dstCursor = dstEnd;
    const uint8_t* srcCursor = srcEnd;
    do {
        dstCursor -= dstStride;
        srcCursor -= srcStride;
        uint32_t s = *srcCursor;
        sum0 += s;
        sum1 += sum0;
        sum2 += sum1;

        *dstCursor = final_scale(info.scaledWeight(), sum2);

        sum2 -= *buffer2Cursor;
        *buffer2Cursor = sum1;
        buffer2Cursor = (buffer2Cursor + 1) < buffer2End ? buffer2Cursor + 1 : &fBuffer2[0];

        sum1 -= *buffer1Cursor;
        *buffer1Cursor = sum0;
        buffer1Cursor = (buffer1Cursor + 1) < buffer1End ? buffer1Cursor + 1 : &fBuffer1[0];

        sum0 -= *buffer0Cursor;
        *buffer0Cursor = s;
        buffer0Cursor = (buffer0Cursor + 1) < buffer0End ? buffer0Cursor + 1 : &fBuffer0[0];
    } while (dstCursor > dst);
}

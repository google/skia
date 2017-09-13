/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMaskBlurFilter.h"

#include <cmath>
#include <climits>

#include "SkArenaAlloc.h"
#include "SkNx.h"
#include "SkSafeMath.h"

static const double kPi = 3.14159265358979323846264338327950288;

#if defined(SK_SUPPORT_LEGACY_USE_GAUSS_FOR_SMALL_RADII)
    static constexpr double kSmallSigma = 0.0;
#else
    static constexpr double kSmallSigma = 2.0;
#endif

class BlurScanInterface {
public:
    virtual ~BlurScanInterface() = default;
    virtual void blur(const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                            uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const = 0;
    virtual bool canBlur4() { return false; }
    virtual void blur4Transpose(
        const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
              uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const {
        SK_ABORT("This should not be called.");
    }
};

class PlanningInterface {
public:
    virtual ~PlanningInterface() = default;
    virtual size_t bufferSize() const = 0;
    virtual size_t border() const = 0;
    virtual bool   needsBlur() const = 0;
    virtual BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, size_t width, uint32_t* buffer) const = 0;
};

class None final : public PlanningInterface {
public:
    None() = default;
    size_t bufferSize() const override { return 0; }
    size_t border()     const override { return 0; }
    bool   needsBlur()  const override { return false; }
    BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, size_t width, uint32_t* buffer) const override {
        SK_ABORT("Should never be called.");
        return nullptr;
    }
};

// Old slower version of Box which uses 64 bit multiply instead of 32 bit multiple.
// Controlled by SK_SUPPORT_LEGACY_SLOW_SMALL_BLUR
class PlanBox32 final : public PlanningInterface {
public:
    explicit PlanBox32(double sigma) {
        // Calculate the radius from sigma. Taken from the old code until something better is
        // figured out.
        auto possibleRadius = 1.5 * sigma - 0.5;
        auto radius = std::max(std::numeric_limits<double>::epsilon(), possibleRadius);
        auto outerRadius = std::ceil(radius);
        auto outerWindow = 2 * outerRadius + 1;
        auto outerFactor = (1 - (outerRadius - radius)) / outerWindow;
        fOuterWeight = static_cast<uint64_t>(round(outerFactor * (1ull << 32)));

        auto innerRadius = outerRadius - 1;
        auto innerWindow = 2 * innerRadius + 1;
        auto innerFactor = (1 - (radius - innerRadius)) / innerWindow;
        fInnerWeight = static_cast<uint64_t>(round(innerFactor * (1ull << 32)));

        // Sliding window is defined by the relationship between the outer and inner widows.
        // In the single window case, you add the element on the right, and subtract the element on
        // the left. But, because two windows are used, this relationship is more complicated; an
        // element is added from the right of the outer window, and subtracted from the left of the
        // inner window. Because innerWindow = outerWindow - 2, the distance between
        // the left and right in the two window case is outerWindow - 1.
        fSlidingWindow = static_cast<size_t>(outerWindow - 1);
    }

    size_t bufferSize() const override { return 0; }

    // Remember that sliding window = window - 1. Therefore, radius = sliding window / 2.
    size_t border()     const override { return fSlidingWindow / 2; }

    bool needsBlur()    const override { return true; }

    BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, size_t width, uint32_t* buffer) const override
    {
        size_t noChangeCount;
        size_t trailingEdgeZeroCount;

        // The relation between the slidingWindow and the width dictates two operating modes.
        // * width >= slidingWindow - both sides of the window are contained in the image while
        // scanning. Therefore, we assume that slidingWindow zeros are consumed on the trailing
        // edge of the window. After this count, then both edges are traversing the image.
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

        void blur(const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
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
            for (size_t i = 0; i < fNoChangeCount; i++) {
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
        }
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


class PlanBox final : public PlanningInterface {
public:
    explicit PlanBox(double sigma) {
        // Calculate the radius from sigma. Taken from the old code until something better is
        // figured out.
        auto possibleRadius = 1.5 * sigma - 0.5;
        auto radius = std::max(std::numeric_limits<double>::epsilon(), possibleRadius);
        auto outerRadius = std::ceil(radius);
        auto outerWindow = 2 * outerRadius + 1;
        auto outerFactor = (1 - (outerRadius - radius)) / outerWindow;
        fOuterWeight = static_cast<uint32_t>(round(outerFactor * (1ull << 24)));

        auto innerRadius = outerRadius - 1;
        auto innerWindow = 2 * innerRadius + 1;
        auto innerFactor = (1 - (radius - innerRadius)) / innerWindow;
        fInnerWeight = static_cast<uint32_t>(round(innerFactor * (1ull << 24)));

        // Sliding window is defined by the relationship between the outer and inner widows.
        // In the single window case, you add the element on the right, and subtract the element on
        // the left. But, because two windows are used, this relationship is more complicated; an
        // element is added from the right of the outer window, and subtracted from the left of the
        // inner window. Because innerWindow = outerWindow - 2, the distance between
        // the left and right in the two window case is outerWindow - 1.
        fSlidingWindow = static_cast<size_t>(outerWindow - 1);
    }

    size_t bufferSize() const override {
        return fSlidingWindow * (sizeof(Sk4u) / sizeof(uint32_t));
    }

    // Remember that sliding window = window - 1. Therefore, radius = sliding window / 2.
    size_t border()     const override { return fSlidingWindow / 2; }

    bool needsBlur()    const override { return true; }

    BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, size_t width, uint32_t* buffer) const override
    {
        size_t noChangeCount;
        size_t trailingEdgeZeroCount;

        // The relation between the slidingWindow and the width dictates two operating modes.
        // * width >= slidingWindow - both sides of the window are contained in the image while
        // scanning. Therefore, we assume that slidingWindow zeros are consumed on the trailing
        // edge of the window. After this count, then both edges are traversing the image.
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

        Sk4u* sk4uBuffer = reinterpret_cast<Sk4u*>(buffer);
        return alloc->make<Box>(fOuterWeight, fInnerWeight, noChangeCount, trailingEdgeZeroCount,
                                sk4uBuffer, sk4uBuffer + fSlidingWindow);
    }

private:
    class Box final : public BlurScanInterface {
    public:
        Box(uint32_t outerWeight, uint32_t innerWeight,
            size_t noChangeCount, size_t trailingEdgeZeroCount,
            Sk4u* buffer, Sk4u* bufferEnd)
            : fOuterWeight{outerWeight}
            , fInnerWeight{innerWeight}
            , fNoChangeCount{noChangeCount}
            , fTrailingEdgeZeroCount{trailingEdgeZeroCount}
            , fBuffer{buffer}
            , fBufferEnd{bufferEnd} { }

        void blur(const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                        uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const override {
            auto rightOuter = src;
            auto dstCursor = dst;

            auto interpolateSums = [this](uint32_t outerSum, uint32_t innerSum) {
                return SkTo<uint8_t>(
                    (fOuterWeight * outerSum + fInnerWeight * innerSum + kHalf) >> 24);
            };

            uint32_t outerSum = 0;
            uint32_t innerSum = 0;
            for (size_t i = 0; i < fTrailingEdgeZeroCount; i++) {
                innerSum = outerSum;
                outerSum += *rightOuter;
                *dstCursor = interpolateSums(outerSum, innerSum);

                rightOuter += srcStride;
                dstCursor += dstStride;
            }

            // slidingWindow > width
            for (size_t i = 0; i < fNoChangeCount; i++) {
                *dstCursor = interpolateSums(outerSum, innerSum);;
                dstCursor += dstStride;
            }

            // width > slidingWindow
            auto leftInner = src;
            while (rightOuter < srcEnd) {
                innerSum = outerSum - *leftInner;
                outerSum += *rightOuter;
                *dstCursor = interpolateSums(outerSum, innerSum);
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
                *dstCursor = interpolateSums(outerSum, innerSum);
            }
        }

        bool canBlur4() override { return true; }

        // NB this is a transposing scan. The next src is src+1, and the next down is
        // src+srcStride.
        void blur4Transpose(
            const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                  uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const override {
            auto rightOuter = src;
            auto dstCursor = dst;

            Sk4u* const bufferStart = fBuffer;
            Sk4u* bufferCursor = bufferStart;
            Sk4u* const bufferEnd = fBufferEnd;

            const Sk4u outerWeight(SkTo<uint32_t>(fOuterWeight));
            const Sk4u innerWeight(SkTo<uint32_t>(fInnerWeight));

            auto load = [](const uint8_t* cursor, size_t stride) -> Sk4u {
                return Sk4u(cursor[0*stride], cursor[1*stride], cursor[2*stride], cursor[3*stride]);
            };

            auto interpolateSums = [&] (const Sk4u& outerSum,  const Sk4u& innerSum) {
                return
                    SkNx_cast<uint8_t>(
                        (outerSum * outerWeight + innerSum * innerWeight + kHalf) >> 24);
            };

            Sk4u outerSum = 0;
            Sk4u innerSum = 0;
            for (size_t i = 0; i < fTrailingEdgeZeroCount; i++) {
                innerSum = outerSum;

                Sk4u leadingEdge = load(rightOuter, srcStride);
                outerSum += leadingEdge;
                Sk4b blurred = interpolateSums(outerSum, innerSum);
                blurred.store(dstCursor);

                leadingEdge.store(bufferCursor);
                bufferCursor = (bufferCursor + 1) < bufferEnd ? bufferCursor + 1 : bufferStart;

                rightOuter += 1;
                dstCursor += dstStride;
            }

            // slidingWindow > width
            for (size_t i = 0; i < fNoChangeCount; i++) {
                Sk4b blurred = interpolateSums(outerSum, innerSum);
                blurred.store(dstCursor);
                dstCursor += dstStride;
            }

            // width > slidingWindow
            auto leftInner = src;
            while (rightOuter < srcEnd) {
                Sk4u trailEdge = Sk4u::Load(bufferCursor);
                Sk4u leadingEdge = load(rightOuter, srcStride);
                innerSum = outerSum - trailEdge;
                outerSum += leadingEdge;

                Sk4b blurred = interpolateSums(outerSum, innerSum);
                blurred.store(dstCursor);

                outerSum -= trailEdge;
                leadingEdge.store(bufferCursor);
                bufferCursor = (bufferCursor + 1) < bufferEnd ? bufferCursor + 1 : bufferStart;

                rightOuter += 1;
                leftInner += 1;
                dstCursor += dstStride;
            }

            auto leftOuter = srcEnd;
            dstCursor = dstEnd;
            outerSum = 0;
            for (size_t i = 0; i < fTrailingEdgeZeroCount; i++) {
                leftOuter -= 1;
                dstCursor -= dstStride;

                innerSum = outerSum;
                outerSum += load(leftOuter, srcStride);
                Sk4b blurred = interpolateSums(outerSum, innerSum);
                blurred.store(dstCursor);
            }
        }

    private:
        static constexpr uint32_t kHalf = static_cast<uint32_t>(1) << 23;

        const uint32_t fOuterWeight;
        const uint32_t fInnerWeight;
        const size_t   fNoChangeCount;
        const size_t   fTrailingEdgeZeroCount;
        Sk4u* const    fBuffer;
        Sk4u* const    fBufferEnd;
    };
private:
    uint32_t fOuterWeight;
    uint32_t fInnerWeight;
    size_t   fSlidingWindow;
};

class PlanGauss final : public PlanningInterface {
public:
    explicit PlanGauss(double sigma) {
        auto possibleWindow = static_cast<size_t>(floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5));
        auto window = std::max(static_cast<size_t>(1), possibleWindow);

        fPass0Size = window - 1;
        fPass1Size = window - 1;
        fPass2Size = (window & 1) == 1 ? window - 1 : window;

        // Calculating the border is tricky. I will go through the odd case which is simpler, and
        // then through the even case. Given a stack of filters seven wide for the odd case of
        // three passes.
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
        //  The A pixel is calculated using the value S, the B uses A, and the C uses B, and
        // finally D is C. So, with a window size of seven the border is nine. In general, the
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
        // For a window of size, the border value is seven. In general the border is 3 *
        // (window/2) -1.
        fBorder = (window & 1) == 1 ? 3 * ((window - 1) / 2) : 3 * (window / 2) - 1;
        fSlidingWindow = 2 * fBorder + 1;

        // If the window is odd then the divisor is just window ^ 3 otherwise,
        // it is window * window * (window + 1) = window ^ 2 + window ^ 3;
        auto window2 = window * window;
        auto window3 = window2 * window;
        auto divisor = (window & 1) == 1 ? window3 : window3 + window2;

        #if defined(SK_LEGACY_SUPPORT_INTEGER_SMALL_RADII)
            fWeight = (static_cast<uint64_t>(1) << 32) / divisor;
        #else
            fWeight = static_cast<uint64_t>(round(1.0 / divisor * (1ull << 32)));
        #endif
    }

    size_t bufferSize() const override { return fPass0Size + fPass1Size + fPass2Size; }

    size_t border()     const override { return fBorder; }

    bool needsBlur()    const override { return true; }

    BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, size_t width, uint32_t* buffer) const override
    {
        uint32_t* buffer0, *buffer0End, *buffer1, *buffer1End, *buffer2, *buffer2End;
        buffer0 = buffer;
        buffer0End = buffer1 = buffer0 + fPass0Size;
        buffer1End = buffer2 = buffer1 + fPass1Size;
        buffer2End = buffer2 + fPass2Size;
        size_t noChangeCount = fSlidingWindow > width ? fSlidingWindow - width : 0;

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
            , fNoChangeCount{noChangeCount}
            , fBuffer0{buffer0}
            , fBuffer0End{buffer0End}
            , fBuffer1{buffer1}
            , fBuffer1End{buffer1End}
            , fBuffer2{buffer2}
            , fBuffer2End{buffer2End}
        { }

        void blur(const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                  uint8_t* dst, size_t dstStride, uint8_t* dstEnd) const override {
            auto buffer0Cursor = fBuffer0;
            auto buffer1Cursor = fBuffer1;
            auto buffer2Cursor = fBuffer2;

            std::memset(fBuffer0, 0x00, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

            uint32_t sum0 = 0;
            uint32_t sum1 = 0;
            uint32_t sum2 = 0;

            // Consume the source generating pixels.
            for (auto srcCursor = src;
                 srcCursor < srcEnd; dst += dstStride, srcCursor += srcStride) {
                uint32_t leadingEdge = *srcCursor;
                sum0 += leadingEdge;
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
                *buffer0Cursor = leadingEdge;
                buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;
            }

            // The leading edge is off the right side of the mask.
            for (size_t i = 0; i < fNoChangeCount; i++) {
                uint32_t leadingEdge = 0;
                sum0 += leadingEdge;
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
                *buffer0Cursor = leadingEdge;
                buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;

                dst += dstStride;
            }

            // Starting from the right, fill in the rest of the buffer.
            std::memset(fBuffer0, 0, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

            sum0 = sum1 = sum2 = 0;

            uint8_t* dstCursor = dstEnd;
            const uint8_t* srcCursor = srcEnd;
            while (dstCursor > dst) {
                dstCursor -= dstStride;
                srcCursor -= srcStride;
                uint32_t leadingEdge = *srcCursor;
                sum0 += leadingEdge;
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
                *buffer0Cursor = leadingEdge;
                buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;
            }
        }

    private:
        static constexpr uint64_t kHalf = static_cast<uint64_t>(1) << 31;

        uint8_t finalScale(uint32_t sum) const {
            return SkTo<uint8_t>((fWeight * sum + kHalf) >> 32);
        }

        uint64_t  fWeight;
        size_t    fNoChangeCount;
        uint32_t* fBuffer0;
        uint32_t* fBuffer0End;
        uint32_t* fBuffer1;
        uint32_t* fBuffer1End;
        uint32_t* fBuffer2;
        uint32_t* fBuffer2End;
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

    if (3 * sigma <= 1) {
        plan = alloc->make<None>();
    } else if (sigma < kSmallSigma) {
        #if defined(SK_SUPPORT_LEGACY_SLOW_SMALL_BLUR)
            plan = alloc->make<PlanBox32>(sigma);
        #else
            plan = alloc->make<PlanBox>(sigma);
        #endif
    } else {
        plan = alloc->make<PlanGauss>(sigma);
    }

    return plan;
};

SkMaskBlurFilter::SkMaskBlurFilter(double sigmaW, double sigmaH)
    : fSigmaW{std::max(sigmaW, 0.0)}
    , fSigmaH{std::max(sigmaH, 0.0)}
{
    SkASSERT(sigmaW >= 0);
    SkASSERT(sigmaH >= 0);
}

bool SkMaskBlurFilter::hasNoBlur() const {
    return (3 * fSigmaW <= 1) && (3 * fSigmaH <= 1);
}

SkIPoint SkMaskBlurFilter::blur(const SkMask& src, SkMask* dst) const {

    // 1024 is a place holder guess until more analysis can be done.
    SkSTArenaAlloc<1024> alloc;

    PlanningInterface* planW = make_plan(&alloc, fSigmaW);
    PlanningInterface* planH = make_plan(&alloc, fSigmaH);

    size_t borderW = planW->border();
    size_t borderH = planH->border();

    auto srcW = SkTo<size_t>(src.fBounds.width());
    auto srcH = SkTo<size_t>(src.fBounds.height());

    SkSafeMath safe;

    // size_t dstW = srcW + 2 * borderW;
    size_t dstW = safe.add(srcW, safe.add(borderW, borderW));
    //size_t dstH = srcH + 2 * borderH;
    size_t dstH = safe.add(srcH, safe.add(borderH, borderH));

    dst->fBounds.set(0, 0, dstW, dstH);
    dst->fBounds.offset(src.fBounds.x(), src.fBounds.y());
    dst->fBounds.offset(-SkTo<int32_t>(borderW), -SkTo<int32_t>(borderH));

    dst->fImage = nullptr;
    dst->fRowBytes = SkTo<uint32_t>(dstW);
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

    auto bufferSize = std::max(planW->bufferSize(), planH->bufferSize());
    auto buffer = alloc.makeArrayDefault<uint32_t>(bufferSize);

    if (planW->needsBlur() && planH->needsBlur()) {
        // Blur both directions.
        size_t tmpW = srcH;
        size_t tmpH = dstW;

        auto tmp = alloc.makeArrayDefault<uint8_t>(tmpW * tmpH);

        // Blur horizontally, and transpose.
        auto scanW = planW->makeBlurScan(&alloc, srcW, buffer);
        size_t y = 0;
        if (scanW->canBlur4() && srcH > 4) {
            for (;y + 4 <= srcH; y += 4) {
                auto srcStart = &src.fImage[y * src.fRowBytes];
                auto tmpStart = &tmp[y];
                scanW->blur4Transpose(srcStart, src.fRowBytes, srcStart + srcW,
                                      tmpStart, tmpW, tmpStart + tmpW * tmpH);
            }
        }

        for (;y < srcH; y++) {
            auto srcStart = &src.fImage[y * src.fRowBytes];
            auto tmpStart = &tmp[y];
            scanW->blur(srcStart,    1, srcStart + srcW,
                        tmpStart, tmpW, tmpStart + tmpW * tmpH);
        }


        // Blur vertically (scan in memory order because of the transposition),
        // and transpose back to the original orientation.
        auto scanH = planH->makeBlurScan(&alloc, tmpW, buffer);
        y = 0;
        if (scanH->canBlur4() && tmpH > 4) {
            for (;y + 4 <= tmpH; y += 4) {
                auto tmpStart = &tmp[y * tmpW];
                auto dstStart = &dst->fImage[y];

                scanH->blur4Transpose(
                    tmpStart, tmpW, tmpStart + tmpW,
                    dstStart, dst->fRowBytes, dstStart + dst->fRowBytes * dstH);
            }
        }
        for (;y < tmpH; y++) {
            auto tmpStart = &tmp[y * tmpW];
            auto dstStart = &dst->fImage[y];

            scanH->blur(tmpStart, 1, tmpStart + tmpW,
                        dstStart, dst->fRowBytes, dstStart + dst->fRowBytes * dstH);
        }
    } else if (planW->needsBlur()) {
        // Blur only horizontally.

        auto scanW = planW->makeBlurScan(&alloc, srcW, buffer);
        for (size_t y = 0; y < srcH; y++) {
            auto srcStart = &src.fImage[y * src.fRowBytes];
            auto dstStart = &dst->fImage[y * dst->fRowBytes];
            scanW->blur(srcStart, 1, srcStart + srcW,
                        dstStart, 1, dstStart + dstW);

        }
    } else if (planH->needsBlur()) {
        // Blur only vertically.

        auto srcEnd   = &src.fImage[src.fRowBytes * srcH];
        auto dstEnd   = &dst->fImage[dst->fRowBytes * dstH];
        auto scanH = planH->makeBlurScan(&alloc, srcH, buffer);
        for (size_t x = 0; x < srcW; x++) {
            auto srcStart = &src.fImage[x];
            auto dstStart = &dst->fImage[x];
            scanH->blur(srcStart, src.fRowBytes, srcEnd,
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



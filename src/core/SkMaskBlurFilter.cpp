/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkGaussFilter.h"
#include "SkMalloc.h"
#include "SkMaskBlurFilter.h"
#include "SkNx.h"
#include "SkSafeMath.h"

#include <cmath>
#include <climits>

static const double kPi = 3.14159265358979323846264338327950288;

class BlurScanInterface {
public:
    virtual ~BlurScanInterface() = default;
    virtual void blur(const uint8_t* src, int srcStride, const uint8_t* srcEnd,
                            uint8_t* dst, int dstStride,       uint8_t* dstEnd) const = 0;
    virtual bool canBlur4() { return false; }
    virtual void blur4Transpose(
        const uint8_t* src, int srcStride, const uint8_t* srcEnd,
              uint8_t* dst, int dstStride,       uint8_t* dstEnd) const {
        SK_ABORT("This should not be called.");
    }
};

class PlanningInterface {
public:
    virtual ~PlanningInterface() = default;
    virtual size_t bufferSize() const = 0;
    virtual int    border() const = 0;
    virtual bool   needsBlur() const = 0;
    virtual BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, int width, uint32_t* buffer) const = 0;
};

class None final : public PlanningInterface {
public:
    None() = default;
    size_t bufferSize() const override { return 0; }
    int    border()     const override { return 0; }
    bool   needsBlur()  const override { return false; }
    BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, int width, uint32_t* buffer) const override {
        SK_ABORT("Should never be called.");
        return nullptr;
    }
};

class PlanGauss final : public PlanningInterface {
public:
    explicit PlanGauss(double sigma) {
        auto possibleWindow = static_cast<int>(floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5));
        auto window = std::max(1, possibleWindow);

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

        fWeight = static_cast<uint64_t>(round(1.0 / divisor * (1ull << 32)));
    }

    size_t bufferSize() const override { return fPass0Size + fPass1Size + fPass2Size; }

    int    border()     const override { return fBorder; }

    bool needsBlur()    const override { return true; }

    BlurScanInterface* makeBlurScan(
        SkArenaAlloc* alloc, int width, uint32_t* buffer) const override
    {
        uint32_t* buffer0, *buffer0End, *buffer1, *buffer1End, *buffer2, *buffer2End;
        buffer0 = buffer;
        buffer0End = buffer1 = buffer0 + fPass0Size;
        buffer1End = buffer2 = buffer1 + fPass1Size;
        buffer2End = buffer2 + fPass2Size;
        int noChangeCount = fSlidingWindow > width ? fSlidingWindow - width : 0;

        return alloc->make<Gauss>(
            fWeight, noChangeCount,
            buffer0, buffer0End,
            buffer1, buffer1End,
            buffer2, buffer2End);
    }

public:
    class Gauss final : public BlurScanInterface {
    public:
        Gauss(uint64_t weight, int noChangeCount,
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

        void blur(const uint8_t* src, int srcStride, const uint8_t* srcEnd,
                        uint8_t* dst, int dstStride, uint8_t* dstEnd) const override {
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
            for (int i = 0; i < fNoChangeCount; i++) {
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
        int       fNoChangeCount;
        uint32_t* fBuffer0;
        uint32_t* fBuffer0End;
        uint32_t* fBuffer1;
        uint32_t* fBuffer1End;
        uint32_t* fBuffer2;
        uint32_t* fBuffer2End;
    };

    uint64_t fWeight;
    int      fBorder;
    int      fSlidingWindow;
    int      fPass0Size;
    int      fPass1Size;
    int      fPass2Size;
};

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
SkMaskBlurFilter::SkMaskBlurFilter(double sigmaW, double sigmaH)
    : fSigmaW{SkTPin(sigmaW, 0.0, 136.0)}
    , fSigmaH{SkTPin(sigmaH, 0.0, 136.0)}
{
    SkASSERT(sigmaW >= 0);
    SkASSERT(sigmaH >= 0);
}

bool SkMaskBlurFilter::hasNoBlur() const {
    return (3 * fSigmaW <= 1) && (3 * fSigmaH <= 1);
}

static SkMask prepare_destination(int radiusX, int radiusY, const SkMask& src) {
    SkSafeMath safe;

    SkMask dst;
    // dstW = srcW + 2 * radiusX;
    size_t dstW = safe.add(src.fBounds.width(), safe.add(radiusX, radiusX));
    // dstH = srcH + 2 * radiusY;
    size_t dstH = safe.add(src.fBounds.height(), safe.add(radiusY, radiusY));

    dst.fBounds.set(0, 0, SkTo<int>(dstW), SkTo<int>(dstH));
    dst.fBounds.offset(src.fBounds.x(), src.fBounds.y());
    dst.fBounds.offset(-radiusX, -radiusY);

    dst.fImage = nullptr;
    dst.fRowBytes = SkTo<uint32_t>(dstW);
    dst.fFormat = SkMask::kA8_Format;

    size_t toAlloc = safe.mul(dstW, dstH);

    if (safe && src.fImage != nullptr) {
        dst.fImage = SkMask::AllocImage(toAlloc);
    }

    return dst;
}

static constexpr uint16_t _____ = 0u;
static constexpr uint16_t kHalf = 0x80u;

static SK_ALWAYS_INLINE Sk8h load(const uint8_t* from, int width) {
    uint8_t buffer[8];
    if (width < 8) {
        sk_bzero(buffer, sizeof(buffer));
        for (int i = 0; i < width; i++) {
            buffer[i] = from[i];
        }
        from = buffer;
    }
    auto v = SkNx_cast<uint16_t>(Sk8b::Load(from));
    // Convert from 0-255 to 8.8 encoding.
    return v << 8;
};

static SK_ALWAYS_INLINE void store(uint8_t* to, const Sk8h& v, int width) {
    Sk8b b = SkNx_cast<uint8_t>(v >> 8);
    if (width == 8) {
        b.store(to);
    } else {
        uint8_t buffer[8];
        b.store(buffer);
        for (int i = 0; i < width; i++) {
            to[i] = buffer[i];
        }
    }
};

// In all the blur_x_radius_N and blur_y_radius_N functions the gaussian values are encoded
// in 0.16 format, none of the values is greater than one. The incoming mask values are in 8.8
// format. The resulting multiply has a 8.24 format, by the mulhi truncates the lower 16 bits
// resulting in a 8.8 format.
//
// The blur_x_radius_N function below blur along a row of pixels using a kernel with radius N. This
// system is setup to minimize the number of multiplies needed.
//
// Explanation:
//    Blurring a specific mask value is given by the following equation where D_n is the resulting
// mask value and S_n is the source value. The example below is for a filter with a radius of 1
// and a width of 3 (radius == (width-1)/2). The indexes for the source and destination are
// aligned. The filter is given by G_n where n is the symmetric filter value.
//
//   D[n] = S[n-1]*G[1] + S[n]*G[0] + S[n+1]*G[1].
//
// We can start the source index at an offset relative to the destination separated by the
// radius. This results in a non-traditional restating of the above filter.
//
//  D[n] = S[n]*G[1] + S[n+1]*G[0] + S[n+2]*G[1]
//
// If we look at three specific consecutive destinations the following equations result:
//
//   D[5] = S[5]*G[1] + S[6]*G[0] + S[7]*G[1]
//   D[7] = S[6]*G[1] + S[7]*G[0] + S[8]*G[1]
//   D[8] = S[7]*G[1] + S[8]*G[0] + S[9]*G[1].
//
// In the above equations, notice that S[7] is used in all three. In particular, two values are
// used: S[7]*G[0] and S[7]*G[1]. So, S[7] is only multiplied twice, but used in D[5], D[6] and
// D[7].
//
// From the point of view of a source value we end up with the following three equations.
//
// Given S[7]:
//   D[5] += S[7]*G[1]
//   D[6] += S[7]*G[0]
//   D[7] += S[7]*G[1]
//
// In General:
//   D[n]   += S[n]*G[1]
//   D[n+1] += S[n]*G[0]
//   D[n+2] += S[n]*G[1]
//
// Now these equations can be ganged using SIMD to form:
//   D[n..n+7]   += S[n..n+7]*G[1]
//   D[n+1..n+8] += S[n..n+7]*G[0]
//   D[n+2..n+9] += S[n..n+7]*G[1]
// The next set of values becomes.
//   D[n+8..n+15]  += S[n+8..n+15]*G[1]
//   D[n+9..n+16]  += S[n+8..n+15]*G[0]
//   D[n+10..n+17] += S[n+8..n+15]*G[1]
// You can see that the D[n+8] and D[n+9] values overlap the two sets, using parts of both
// S[n..7] and S[n+8..n+15].
//
// Just one more transformation allows the code to maintain all working values in
// registers. I introduce the notation {0, S[n..n+7] * G[k]} to mean that the value where 0 is
// prepended to the array of values to form {0, S[n] * G[k], ..., S[n+7]*G[k]}.
//
//   D[n..n+7]  += S[n..n+7] * G[1]
//   D[n..n+8]  += {0, S[n..n+7] * G[0]}
//   D[n..n+9]  += {0, 0, S[n..n+7] * G[1]}
//
// Now we can encode D[n..n+7] in a single Sk8h register called d0, and D[n+8..n+15] in a
// register d8. In addition, S[0..n+7] becomes s0.
//
// The translation of the {0, S[n..n+7] * G[k]} is translated in the following way below.
//
// Sk8h v0 = s0*G[0]
// Sk8h v1 = s0*G[1]
// /* D[n..n+7]  += S[n..n+7] * G[1] */
// d0 += v1;
// /* D[n..n+8]  += {0, S[n..n+7] * G[0]} */
// d0 += {_____, v0[0], v0[1], v0[2], v0[3], v0[4], v0[5], v0[6]}
// d1 += {v0[7], _____, _____, _____, _____, _____, _____, _____}
// /* D[n..n+9]  += {0, 0, S[n..n+7] * G[1]} */
// d0 += {_____, _____, v1[0], v1[1], v1[2], v1[3], v1[4], v1[5]}
// d1 += {v1[6], v1[7], _____, _____, _____, _____, _____, _____}
// Where we rely on the compiler to generate efficient code for the {____, n, ....} notation.

static SK_ALWAYS_INLINE void blur_x_radius_1(
        const Sk8h& s0,
        const Sk8h& g0, const Sk8h& g1, const Sk8h&, const Sk8h&, const Sk8h&,
        Sk8h* d0, Sk8h* d8) {

    auto v1 = s0.mulHi(g1);
    auto v0 = s0.mulHi(g0);

    // D[n..n+7]  += S[n..n+7] * G[1]
    *d0 += v1;

    //D[n..n+8]  += {0, S[n..n+7] * G[0]}
    *d0 += Sk8h{_____, v0[0], v0[1], v0[2], v0[3], v0[4], v0[5], v0[6]};
    *d8 += Sk8h{v0[7], _____, _____, _____, _____, _____, _____, _____};

    // D[n..n+9]  += {0, 0, S[n..n+7] * G[1]}
    *d0 += Sk8h{_____, _____, v1[0], v1[1], v1[2], v1[3], v1[4], v1[5]};
    *d8 += Sk8h{v1[6], v1[7], _____, _____, _____, _____, _____, _____};

}

static SK_ALWAYS_INLINE void blur_x_radius_2(
        const Sk8h& s0,
        const Sk8h& g0, const Sk8h& g1, const Sk8h& g2, const Sk8h&, const Sk8h&,
        Sk8h* d0, Sk8h* d8) {
    auto v0 = s0.mulHi(g0);
    auto v1 = s0.mulHi(g1);
    auto v2 = s0.mulHi(g2);

    // D[n..n+7]  += S[n..n+7] * G[2]
    *d0 += v2;

    // D[n..n+8]  += {0, S[n..n+7] * G[1]}
    *d0 += Sk8h{_____, v1[0], v1[1], v1[2], v1[3], v1[4], v1[5], v1[6]};
    *d8 += Sk8h{v1[7], _____, _____, _____, _____, _____, _____, _____};

    // D[n..n+9]  += {0, 0, S[n..n+7] * G[0]}
    *d0 += Sk8h{_____, _____, v0[0], v0[1], v0[2], v0[3], v0[4], v0[5]};
    *d8 += Sk8h{v0[6], v0[7], _____, _____, _____, _____, _____, _____};

    // D[n..n+10]  += {0, 0, 0, S[n..n+7] * G[1]}
    *d0 += Sk8h{_____, _____, _____, v1[0], v1[1], v1[2], v1[3], v1[4]};
    *d8 += Sk8h{v1[5], v1[6], v1[7], _____, _____, _____, _____, _____};

    // D[n..n+11]  += {0, 0, 0, 0, S[n..n+7] * G[2]}
    *d0 += Sk8h{_____, _____, _____, _____, v2[0], v2[1], v2[2], v2[3]};
    *d8 += Sk8h{v2[4], v2[5], v2[6], v2[7], _____, _____, _____, _____};
}

static SK_ALWAYS_INLINE void blur_x_radius_3(
        const Sk8h& s0,
        const Sk8h& gauss0, const Sk8h& gauss1, const Sk8h& gauss2, const Sk8h& gauss3, const Sk8h&,
        Sk8h* d0, Sk8h* d8) {
    auto v0 = s0.mulHi(gauss0);
    auto v1 = s0.mulHi(gauss1);
    auto v2 = s0.mulHi(gauss2);
    auto v3 = s0.mulHi(gauss3);

    // D[n..n+7]  += S[n..n+7] * G[3]
    *d0 += v3;

    // D[n..n+8]  += {0, S[n..n+7] * G[2]}
    *d0 += Sk8h{_____, v2[0], v2[1], v2[2], v2[3], v2[4], v2[5], v2[6]};
    *d8 += Sk8h{v2[7], _____, _____, _____, _____, _____, _____, _____};

    // D[n..n+9]  += {0, 0, S[n..n+7] * G[1]}
    *d0 += Sk8h{_____, _____, v1[0], v1[1], v1[2], v1[3], v1[4], v1[5]};
    *d8 += Sk8h{v1[6], v1[7], _____, _____, _____, _____, _____, _____};

    // D[n..n+10]  += {0, 0, 0, S[n..n+7] * G[0]}
    *d0 += Sk8h{_____, _____, _____, v0[0], v0[1], v0[2], v0[3], v0[4]};
    *d8 += Sk8h{v0[5], v0[6], v0[7], _____, _____, _____, _____, _____};

    // D[n..n+11]  += {0, 0, 0, 0, S[n..n+7] * G[1]}
    *d0 += Sk8h{_____, _____, _____, _____, v1[0], v1[1], v1[2], v1[3]};
    *d8 += Sk8h{v1[4], v1[5], v1[6], v1[7], _____, _____, _____, _____};

    // D[n..n+12]  += {0, 0, 0, 0, 0, S[n..n+7] * G[2]}
    *d0 += Sk8h{_____, _____, _____, _____, _____, v2[0], v2[1], v2[2]};
    *d8 += Sk8h{v2[3], v2[4], v2[5], v2[6], v2[7], _____, _____, _____};

    // D[n..n+13]  += {0, 0, 0, 0, 0, 0, S[n..n+7] * G[3]}
    *d0 += Sk8h{_____, _____, _____, _____, _____, _____, v3[0], v3[1]};
    *d8 += Sk8h{v3[2], v3[3], v3[4], v3[5], v3[6], v3[7], _____, _____};
}

static SK_ALWAYS_INLINE void blur_x_radius_4(
        const Sk8h& s0,
        const Sk8h& gauss0,
        const Sk8h& gauss1,
        const Sk8h& gauss2,
        const Sk8h& gauss3,
        const Sk8h& gauss4,
        Sk8h* d0, Sk8h* d8) {
    auto v0 = s0.mulHi(gauss0);
    auto v1 = s0.mulHi(gauss1);
    auto v2 = s0.mulHi(gauss2);
    auto v3 = s0.mulHi(gauss3);
    auto v4 = s0.mulHi(gauss4);

    // D[n..n+7]  += S[n..n+7] * G[4]
    *d0 += v4;

    // D[n..n+8]  += {0, S[n..n+7] * G[3]}
    *d0 += Sk8h{_____, v3[0], v3[1], v3[2], v3[3], v3[4], v3[5], v3[6]};
    *d8 += Sk8h{v3[7], _____, _____, _____, _____, _____, _____, _____};

    // D[n..n+9]  += {0, 0, S[n..n+7] * G[2]}
    *d0 += Sk8h{_____, _____, v2[0], v2[1], v2[2], v2[3], v2[4], v2[5]};
    *d8 += Sk8h{v2[6], v2[7], _____, _____, _____, _____, _____, _____};

    // D[n..n+10]  += {0, 0, 0, S[n..n+7] * G[1]}
    *d0 += Sk8h{_____, _____, _____, v1[0], v1[1], v1[2], v1[3], v1[4]};
    *d8 += Sk8h{v1[5], v1[6], v1[7], _____, _____, _____, _____, _____};

    // D[n..n+11]  += {0, 0, 0, 0, S[n..n+7] * G[0]}
    *d0 += Sk8h{_____, _____, _____, _____, v0[0], v0[1], v0[2], v0[3]};
    *d8 += Sk8h{v0[4], v0[5], v0[6], v0[7], _____, _____, _____, _____};

    // D[n..n+12]  += {0, 0, 0, 0, 0, S[n..n+7] * G[1]}
    *d0 += Sk8h{_____, _____, _____, _____, _____, v1[0], v1[1], v1[2]};
    *d8 += Sk8h{v1[3], v1[4], v1[5], v1[6], v1[7], _____, _____, _____};

    // D[n..n+13]  += {0, 0, 0, 0, 0, 0, S[n..n+7] * G[2]}
    *d0 += Sk8h{_____, _____, _____, _____, _____, _____, v2[0], v2[1]};
    *d8 += Sk8h{v2[2], v2[3], v2[4], v2[5], v2[6], v2[7], _____, _____};

    // D[n..n+14]  += {0, 0, 0, 0, 0, 0, 0, S[n..n+7] * G[3]}
    *d0 += Sk8h{_____, _____, _____, _____, _____, _____, _____, v3[0]};
    *d8 += Sk8h{v3[1], v3[2], v3[3], v3[4], v3[5], v3[6], v3[7], _____};

    // D[n..n+15]  += {0, 0, 0, 0, 0, 0, 0, 0, S[n..n+7] * G[4]}
    *d8 += v4;
}

using BlurX = decltype(blur_x_radius_1);

// BlurX will only be one of the functions blur_x_radius_(1|2|3|4).
static SK_ALWAYS_INLINE void blur_row(
        BlurX blur,
        const Sk8h& g0, const Sk8h& g1, const Sk8h& g2, const Sk8h& g3, const Sk8h& g4,
        const uint8_t* src, int srcW,
              uint8_t* dst, int dstW) {
    // Clear the buffer to handle summing wider than source.
    Sk8h d0{kHalf}, d8{kHalf};

    // Go by multiples of 8 in src.
    int x = 0;
    for (; x <= srcW - 8; x += 8) {
        blur(load(src, 8), g0, g1, g2, g3, g4, &d0, &d8);

        store(dst, d0, 8);

        d0 = d8;
        d8 = Sk8h{kHalf};

        src += 8;
        dst += 8;
    }

    // There are src values left, but the remainder of src values is not a multiple of 8.
    int srcTail = srcW - x;
    if (srcTail > 0) {

        blur(load(src, srcTail), g0, g1, g2, g3, g4, &d0, &d8);

        int dstTail = std::min(8, dstW - x);
        store(dst, d0, dstTail);

        d0 = d8;
        dst += dstTail;
        x += dstTail;
    }

    // There are dst mask values to complete.
    int dstTail = dstW - x;
    if (dstTail > 0) {
        store(dst, d0, dstTail);
    }
}

// BlurX will only be one of the functions blur_x_radius_(1|2|3|4).
static SK_ALWAYS_INLINE void blur_x_rect(
        BlurX blur,
        uint16_t* gauss,
        const uint8_t* src, size_t srcStride, int srcW,
              uint8_t* dst, size_t dstStride, int dstW, int dstH) {

    Sk8h g0{gauss[0]},
         g1{gauss[1]},
         g2{gauss[2]},
         g3{gauss[3]},
         g4{gauss[4]};

    // Blur *ALL* the rows.
    for (int y = 0; y < dstH; y++) {
        blur_row(blur, g0, g1, g2, g3, g4, src, srcW, dst, dstW);
        src += srcStride;
        dst += dstStride;
    }
}

SK_ATTRIBUTE(noinline) static void direct_blur_x(
    int radius, uint16_t* gauss,
    const uint8_t* src, size_t srcStride, int srcW,
          uint8_t* dst, size_t dstStride, int dstW, int dstH) {

    switch (radius) {
        case 1:
            blur_x_rect(blur_x_radius_1, gauss, src, srcStride, srcW, dst, dstStride, dstW, dstH);
            break;

        case 2:
            blur_x_rect(blur_x_radius_2, gauss, src, srcStride, srcW, dst, dstStride, dstW, dstH);
            break;

        case 3:
            blur_x_rect(blur_x_radius_3, gauss, src, srcStride, srcW, dst, dstStride, dstW, dstH);
            break;

        case 4:
            blur_x_rect(blur_x_radius_4, gauss, src, srcStride, srcW, dst, dstStride, dstW, dstH);
            break;

        default:
            SkASSERTF(false, "The radius %d is not handled\n", radius);
    }
}

// The operations of the blur_y_radius_N functions work on a theme similar to the blur_x_radius_N
// functions, but end up being simpler because there is no complicated shift of registers. We
// start with the non-traditional form of the gaussian filter. In the following r is the value
// when added generates the next value in the column.
//
//   D[n+0r] = S[n+0r]*G[1]
//           + S[n+1r]*G[0]
//           + S[n+2r]*G[1]
//
// Expanding out in a way similar to blur_x_radius_N for specific values of n.
//
//   D[n+0r] = S[n-2r]*G[1] + S[n-1r]*G[0] + S[n+0r]*G[1]
//   D[n+1r] = S[n-1r]*G[1] + S[n+0r]*G[0] + S[n+1r]*G[1]
//   D[n+2r] = S[n+0r]*G[1] + S[n+1r]*G[0] + S[n+2r]*G[1]
//
// We can see that S[n+0r] is in all three D[] equations, but is only multiplied twice. Now we
// can look at the calculation form the point of view of a source value.
//
//   Given S[n+0r]:
//   D[n+0r] += S[n+0r]*G[1];
//   /* D[n+0r] is done and can be stored now. */
//   D[n+1r] += S[n+0r]*G[0];
//   D[n+2r]  = S[n+0r]*G[1];
//
// Remember, by induction, that D[n+0r] == S[n-2r]*G[1] + S[n-1r]*G[0] before adding in
// S[n+0r]*G[1]. So, after the addition D[n+0r] has finished calculation and can be stored. Also,
// notice that D[n+2r] is receiving its first value from S[n+0r]*G[1] and is not added in. Notice
// how values flow in the following two iterations in source.
//
//   D[n+0r] += S[n+0r]*G[1]
//   D[n+1r] += S[n+0r]*G[0]
//   D[n+2r]  = S[n+0r]*G[1]
//   /* ------- */
//   D[n+1r] += S[n+1r]*G[1]
//   D[n+2r] += S[n+1r]*G[0]
//   D[n+3r]  = S[n+1r]*G[1]
//
// Instead of using memory we can introduce temporaries d01 and d12. The update step changes
// to the following.
//
//   answer = d01 + S[n+0r]*G[1]
//   d01    = d12 + S[n+0r]*G[0]
//   d12    =       S[n+0r]*G[1]
//   return answer
//
// Finally, this can be ganged into SIMD style.
//   answer[0..7] = d01[0..7] + S[n+0r..n+0r+7]*G[1]
//   d01[0..7]    = d12[0..7] + S[n+0r..n+0r+7]*G[0]
//   d12[0..7]    =             S[n+0r..n+0r+7]*G[1]
//   return answer[0..7]
static SK_ALWAYS_INLINE Sk8h blur_y_radius_1(
        const Sk8h& s0,
        const Sk8h& g0, const Sk8h& g1, const Sk8h&, const Sk8h&, const Sk8h&,
        Sk8h* d01, Sk8h* d12, Sk8h*, Sk8h*, Sk8h*, Sk8h*, Sk8h*, Sk8h*) {
    auto v0 = s0.mulHi(g0);
    auto v1 = s0.mulHi(g1);

    Sk8h answer = *d01 + v1;
           *d01 = *d12 + v0;
           *d12 =        v1 + kHalf;

    return answer;
}

static SK_ALWAYS_INLINE Sk8h blur_y_radius_2(
        const Sk8h& s0,
        const Sk8h& g0, const Sk8h& g1, const Sk8h& g2, const Sk8h&, const Sk8h&,
        Sk8h* d01, Sk8h* d12, Sk8h* d23, Sk8h* d34, Sk8h*, Sk8h*, Sk8h*, Sk8h*) {
    auto v0 = s0.mulHi(g0);
    auto v1 = s0.mulHi(g1);
    auto v2 = s0.mulHi(g2);

    Sk8h answer = *d01 + v2;
           *d01 = *d12 + v1;
           *d12 = *d23 + v0;
           *d23 = *d34 + v1;
           *d34 =        v2 + kHalf;

    return answer;
}

static SK_ALWAYS_INLINE Sk8h blur_y_radius_3(
        const Sk8h& s0,
        const Sk8h& g0, const Sk8h& g1, const Sk8h& g2, const Sk8h& g3, const Sk8h&,
        Sk8h* d01, Sk8h* d12, Sk8h* d23, Sk8h* d34, Sk8h* d45, Sk8h* d56, Sk8h*, Sk8h*) {
    auto v0 = s0.mulHi(g0);
    auto v1 = s0.mulHi(g1);
    auto v2 = s0.mulHi(g2);
    auto v3 = s0.mulHi(g3);

    Sk8h answer = *d01 + v3;
           *d01 = *d12 + v2;
           *d12 = *d23 + v1;
           *d23 = *d34 + v0;
           *d34 = *d45 + v1;
           *d45 = *d56 + v2;
           *d56 =        v3 + kHalf;

    return answer;
}

static SK_ALWAYS_INLINE Sk8h blur_y_radius_4(
    const Sk8h& s0,
    const Sk8h& g0, const Sk8h& g1, const Sk8h& g2, const Sk8h& g3, const Sk8h& g4,
    Sk8h* d01, Sk8h* d12, Sk8h* d23, Sk8h* d34, Sk8h* d45, Sk8h* d56, Sk8h* d67, Sk8h* d78) {
    auto v0 = s0.mulHi(g0);
    auto v1 = s0.mulHi(g1);
    auto v2 = s0.mulHi(g2);
    auto v3 = s0.mulHi(g3);
    auto v4 = s0.mulHi(g4);

    Sk8h answer = *d01 + v4;
           *d01 = *d12 + v3;
           *d12 = *d23 + v2;
           *d23 = *d34 + v1;
           *d34 = *d45 + v0;
           *d45 = *d56 + v1;
           *d56 = *d67 + v2;
           *d67 = *d78 + v3;
           *d78 =        v4 + kHalf;

    return answer;
}

using BlurY = decltype(blur_y_radius_1);

// BlurY will be one of blur_y_radius_(1|2|3|4).
static SK_ALWAYS_INLINE void blur_column(
        BlurY blur, int radius, int width,
        const Sk8h& g0, const Sk8h& g1, const Sk8h& g2, const Sk8h& g3, const Sk8h& g4,
        const uint8_t* src, size_t srcStride, int srcH,
        uint8_t* dst, size_t dstStride) {
    Sk8h d01{kHalf}, d12{kHalf}, d23{kHalf}, d34{kHalf},
         d45{kHalf}, d56{kHalf}, d67{kHalf}, d78{kHalf};

    auto flush = [&](uint8_t* to, const Sk8h& v0, const Sk8h& v1) {
        store(to, v0, width);
        to += dstStride;
        store(to, v1, width);
        return to + dstStride;
    };

    for (int y = 0; y < srcH; y += 1) {
        auto s = load(src, width);
        auto b = blur(s,
                      g0, g1, g2, g3, g4,
                      &d01, &d12, &d23, &d34, &d45, &d56, &d67, &d78);
        store(dst, b, width);
        src += srcStride;
        dst += dstStride;
    }

    if (radius >= 1) {
        dst = flush(dst, d01, d12);
    }
    if (radius >= 2) {
        dst = flush(dst, d23, d34);
    }
    if (radius >= 3) {
        dst = flush(dst, d45, d56);
    }
    if (radius >= 4) {
              flush(dst, d67, d78);
    }
}

// BlurY will be one of blur_y_radius_(1|2|3|4).
static SK_ALWAYS_INLINE void blur_y_rect(
        BlurY blur, int radius, uint16_t *gauss,
        const uint8_t *src, size_t srcStride, int srcW, int srcH,
        uint8_t *dst, size_t dstStride) {

    Sk8h g0{gauss[0]},
         g1{gauss[1]},
         g2{gauss[2]},
         g3{gauss[3]},
         g4{gauss[4]};

    int x = 0;
    for (; x <= srcW - 8; x += 8) {
        blur_column(blur, radius, 8,
                    g0, g1, g2, g3, g4,
                    src, srcStride, srcH,
                    dst, dstStride);
        src += 8;
        dst += 8;
    }

    int xTail = srcW - x;
    if (xTail > 0) {
        blur_column(blur, radius, xTail,
                    g0, g1, g2, g3, g4,
                    src, srcStride, srcH,
                    dst, dstStride);
    }
}

SK_ATTRIBUTE(noinline) static void direct_blur_y(
        int radius, uint16_t* gauss,
        const uint8_t* src, size_t srcStride, int srcW, int srcH,
              uint8_t* dst, size_t dstStride) {

    switch (radius) {
        case 1:
            blur_y_rect(blur_y_radius_1, 1, gauss,
                        src, srcStride, srcW, srcH,
                        dst, dstStride);
            break;

        case 2:
            blur_y_rect(blur_y_radius_2, 2, gauss,
                        src, srcStride, srcW, srcH,
                        dst, dstStride);
            break;

        case 3:
            blur_y_rect(blur_y_radius_3, 3, gauss,
                        src, srcStride, srcW, srcH,
                        dst, dstStride);
            break;

        case 4:
            blur_y_rect(blur_y_radius_4, 4, gauss,
                        src, srcStride, srcW, srcH,
                        dst, dstStride);
            break;

        default:
            SkASSERTF(false, "The radius %d is not handled\n", radius);
    }
}

static SkIPoint small_blur(double sigmaX, double sigmaY, const SkMask& src, SkMask* dst) {
    SkASSERT(0 <= sigmaX && sigmaX < 2);
    SkASSERT(0 <= sigmaY && sigmaY < 2);

    SkGaussFilter filterX{sigmaX, SkGaussFilter::Type::Bessel},
                  filterY{sigmaY, SkGaussFilter::Type::Bessel};

    int radiusX = filterX.radius(),
        radiusY = filterY.radius();

    SkASSERT(radiusX <= 4 && radiusY <= 4);

    auto prepareGauss = [](const SkGaussFilter& filter, uint16_t* factors) {
        int i = 0;
        for (double d : filter) {
            factors[i++] = static_cast<uint16_t>(round(d * (1 << 16)));
        }
    };

    uint16_t gaussFactorsX[SkGaussFilter::kGaussArrayMax],
             gaussFactorsY[SkGaussFilter::kGaussArrayMax];

    prepareGauss(filterX, gaussFactorsX);
    prepareGauss(filterY, gaussFactorsY);

    *dst = prepare_destination(radiusX, radiusY, src);
    if (src.fImage == nullptr) {
        return {SkTo<int32_t>(radiusX), SkTo<int32_t>(radiusY)};
    }
    if (dst->fImage == nullptr) {
        dst->fBounds.setEmpty();
        return {0, 0};
    }

    int srcW = src.fBounds.width(),
        srcH = src.fBounds.height();

    int dstW = dst->fBounds.width(),
        dstH = dst->fBounds.height();

    size_t srcStride = src.fRowBytes,
           dstStride = dst->fRowBytes;

    //TODO: handle bluring in only one direction.

    // Blur vertically and copy to destination.
    direct_blur_y(radiusY, gaussFactorsY,
                  src.fImage,  srcStride, srcW, srcH,
                  dst->fImage + radiusX, dstStride);

    // Blur horizontally in place.
    direct_blur_x(radiusX, gaussFactorsX,
                  dst->fImage + radiusX,  dstStride, srcW,
                  dst->fImage,            dstStride, dstW, dstH);

    return {radiusX, radiusY};
}

// TODO: assuming sigmaW = sigmaH. Allow different sigmas. Right now the
// API forces the sigmas to be the same.
SkIPoint SkMaskBlurFilter::blur(const SkMask& src, SkMask* dst) const {

    if (fSigmaW < 2.0 && fSigmaH < 2.0) {
        return small_blur(fSigmaW, fSigmaH, src, dst);
    }

    // 1024 is a place holder guess until more analysis can be done.
    SkSTArenaAlloc<1024> alloc;

    PlanningInterface* planW = alloc.make<PlanGauss>(fSigmaW);
    PlanningInterface* planH = alloc.make<PlanGauss>(fSigmaH);

    int borderW = planW->border(),
        borderH = planH->border();
    SkASSERT(borderH >= 0 && borderW >= 0);

    *dst = prepare_destination(borderW, borderH, src);
    if (src.fImage == nullptr) {
        return {SkTo<int32_t>(borderW), SkTo<int32_t>(borderH)};
    }
    if (dst->fImage == nullptr) {
        dst->fBounds.setEmpty();
        return {0, 0};
    }

    int srcW = src.fBounds.width(),
        srcH = src.fBounds.height(),
        dstW = dst->fBounds.width(),
        dstH = dst->fBounds.height();
    SkASSERT(srcW >= 0 && srcH >= 0 && dstW >= 0 && dstH >= 0);

    auto bufferSize = std::max(planW->bufferSize(), planH->bufferSize());
    auto buffer = alloc.makeArrayDefault<uint32_t>(bufferSize);

    if (planW->needsBlur() && planH->needsBlur()) {
        // Blur both directions.
        int tmpW = srcH,
            tmpH = dstW;

        auto tmp = alloc.makeArrayDefault<uint8_t>(tmpW * tmpH);

        // Blur horizontally, and transpose.
        auto scanW = planW->makeBlurScan(&alloc, srcW, buffer);
        for (int y = 0; y < srcH; y++) {
            auto srcStart = &src.fImage[y * src.fRowBytes];
            auto tmpStart = &tmp[y];
            scanW->blur(srcStart,    1, srcStart + srcW,
                        tmpStart, tmpW, tmpStart + tmpW * tmpH);
        }

        // Blur vertically (scan in memory order because of the transposition),
        // and transpose back to the original orientation.
        auto scanH = planH->makeBlurScan(&alloc, tmpW, buffer);
        for (int y = 0; y < tmpH; y++) {
            auto tmpStart = &tmp[y * tmpW];
            auto dstStart = &dst->fImage[y];

            scanH->blur(tmpStart, 1, tmpStart + tmpW,
                        dstStart, dst->fRowBytes, dstStart + dst->fRowBytes * dstH);
        }
    } else {
        // Copy to dst. No Blur.
        SkASSERT(false);    // should not get here
        for (int y = 0; y < srcH; y++) {
            std::memcpy(&dst->fImage[y * dst->fRowBytes], &src.fImage[y * src.fRowBytes], dstW);
        }
    }

    return {SkTo<int32_t>(borderW), SkTo<int32_t>(borderH)};
}

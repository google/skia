/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMaskBlurFilter.h"

#include "include/core/SkColorPriv.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkNx.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkGaussFilter.h"

#include <cmath>
#include <climits>

namespace {
static const double kPi = 3.14159265358979323846264338327950288;

class PlanGauss final {
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

    size_t bufferSize() const { return fPass0Size + fPass1Size + fPass2Size; }

    int    border()     const { return fBorder; }

public:
    class Scan {
    public:
        Scan(uint64_t weight, int noChangeCount,
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

        template <typename AlphaIter> void blur(const AlphaIter srcBegin, const AlphaIter srcEnd,
                    uint8_t* dst, int dstStride, uint8_t* dstEnd) const {
            auto buffer0Cursor = fBuffer0;
            auto buffer1Cursor = fBuffer1;
            auto buffer2Cursor = fBuffer2;

            std::memset(fBuffer0, 0x00, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

            uint32_t sum0 = 0;
            uint32_t sum1 = 0;
            uint32_t sum2 = 0;

            // Consume the source generating pixels.
            for (AlphaIter src = srcBegin; src < srcEnd; ++src, dst += dstStride) {
                uint32_t leadingEdge = *src;
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
            AlphaIter src = srcEnd;
            while (dstCursor > dst) {
                dstCursor -= dstStride;
                uint32_t leadingEdge = *(--src);
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

    Scan makeBlurScan(int width, uint32_t* buffer) const {
        uint32_t* buffer0, *buffer0End, *buffer1, *buffer1End, *buffer2, *buffer2End;
        buffer0 = buffer;
        buffer0End = buffer1 = buffer0 + fPass0Size;
        buffer1End = buffer2 = buffer1 + fPass1Size;
        buffer2End = buffer2 + fPass2Size;
        int noChangeCount = fSlidingWindow > width ? fSlidingWindow - width : 0;

        return Scan(
            fWeight, noChangeCount,
            buffer0, buffer0End,
            buffer1, buffer1End,
            buffer2, buffer2End);
    }

    uint64_t fWeight;
    int      fBorder;
    int      fSlidingWindow;
    int      fPass0Size;
    int      fPass1Size;
    int      fPass2Size;
};

} // namespace

// NB 135 is the largest sigma that will not cause a buffer full of 255 mask values to overflow
// using the Gauss filter. It also limits the size of buffers used hold intermediate values. The
// additional + 1 added to window represents adding one more leading element before subtracting the
// trailing element.
// Explanation of maximums:
//   sum0 = (window + 1) * 255
//   sum1 = (window + 1) * sum0 -> (window + 1) * (window + 1) * 255
//   sum2 = (window + 1) * sum1 -> (window + 1) * (window + 1) * (window + 1) * 255 -> window^3 * 255
//
//   The value (window + 1)^3 * 255 must fit in a uint32_t. So,
//      (window + 1)^3 * 255 < 2^32. window = 255.
//
//   window = floor(sigma * 3 * sqrt(2 * kPi) / 4)
//   For window <= 255, the largest value for sigma is 135.
SkMaskBlurFilter::SkMaskBlurFilter(double sigmaW, double sigmaH)
    : fSigmaW{SkTPin(sigmaW, 0.0, 135.0)}
    , fSigmaH{SkTPin(sigmaH, 0.0, 135.0)}
{
    SkASSERT(sigmaW >= 0);
    SkASSERT(sigmaH >= 0);
}

bool SkMaskBlurFilter::hasNoBlur() const {
    return (3 * fSigmaW <= 1) && (3 * fSigmaH <= 1);
}

// We favor A8 masks, and if we need to work with another format, we'll convert to A8 first.
// Each of these converts width (up to 8) mask values to A8.
static void bw_to_a8(uint8_t* a8, const uint8_t* from, int width) {
    SkASSERT(0 < width && width <= 8);

    uint8_t masks = *from;
    for (int i = 0; i < width; ++i) {
        a8[i] = (masks >> (7 - i)) & 1 ? 0xFF
                                       : 0x00;
    }
}
static void lcd_to_a8(uint8_t* a8, const uint8_t* from, int width) {
    SkASSERT(0 < width && width <= 8);

    for (int i = 0; i < width; ++i) {
        unsigned rgb = reinterpret_cast<const uint16_t*>(from)[i],
                   r = SkPacked16ToR32(rgb),
                   g = SkPacked16ToG32(rgb),
                   b = SkPacked16ToB32(rgb);
        a8[i] = (r + g + b) / 3;
    }
}
static void argb32_to_a8(uint8_t* a8, const uint8_t* from, int width) {
    SkASSERT(0 < width && width <= 8);
    for (int i = 0; i < width; ++i) {
        uint32_t rgba = reinterpret_cast<const uint32_t*>(from)[i];
        a8[i] = SkGetPackedA32(rgba);
    }
}
using ToA8 = decltype(bw_to_a8);

static Sk8h load(const uint8_t* from, int width, ToA8* toA8) {
    // Our fast path is a full 8-byte load of A8.
    // So we'll conditionally handle the two slow paths using tmp:
    //    - if we have a function to convert another mask to A8, use it;
    //    - if not but we have less than 8 bytes to load, load them one at a time.
    uint8_t tmp[8] = {0,0,0,0, 0,0,0,0};
    if (toA8) {
        toA8(tmp, from, width);
        from = tmp;
    } else if (width < 8) {
        for (int i = 0; i < width; ++i) {
            tmp[i] = from[i];
        }
        from = tmp;
    }

    // Load A8 and convert to 8.8 fixed-point.
    return SkNx_cast<uint16_t>(Sk8b::Load(from)) << 8;
}

static void store(uint8_t* to, const Sk8h& v, int width) {
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

static constexpr uint16_t _____ = 0u;
static constexpr uint16_t kHalf = 0x80u;

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

static void blur_x_radius_1(
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

static void blur_x_radius_2(
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

static void blur_x_radius_3(
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

static void blur_x_radius_4(
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
static void blur_row(
        BlurX blur,
        const Sk8h& g0, const Sk8h& g1, const Sk8h& g2, const Sk8h& g3, const Sk8h& g4,
        const uint8_t* src, int srcW,
              uint8_t* dst, int dstW) {
    // Clear the buffer to handle summing wider than source.
    Sk8h d0{kHalf}, d8{kHalf};

    // Go by multiples of 8 in src.
    int x = 0;
    for (; x <= srcW - 8; x += 8) {
        blur(load(src, 8, nullptr), g0, g1, g2, g3, g4, &d0, &d8);

        store(dst, d0, 8);

        d0 = d8;
        d8 = Sk8h{kHalf};

        src += 8;
        dst += 8;
    }

    // There are src values left, but the remainder of src values is not a multiple of 8.
    int srcTail = srcW - x;
    if (srcTail > 0) {

        blur(load(src, srcTail, nullptr), g0, g1, g2, g3, g4, &d0, &d8);

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
static void blur_x_rect(BlurX blur,
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

static void direct_blur_x(int radius, uint16_t* gauss,
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
static Sk8h blur_y_radius_1(
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

static Sk8h blur_y_radius_2(
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

static Sk8h blur_y_radius_3(
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

static Sk8h blur_y_radius_4(
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
static void blur_column(
        ToA8 toA8,
        BlurY blur, int radius, int width,
        const Sk8h& g0, const Sk8h& g1, const Sk8h& g2, const Sk8h& g3, const Sk8h& g4,
        const uint8_t* src, size_t srcRB, int srcH,
        uint8_t* dst, size_t dstRB) {
    Sk8h d01{kHalf}, d12{kHalf}, d23{kHalf}, d34{kHalf},
         d45{kHalf}, d56{kHalf}, d67{kHalf}, d78{kHalf};

    auto flush = [&](uint8_t* to, const Sk8h& v0, const Sk8h& v1) {
        store(to, v0, width);
        to += dstRB;
        store(to, v1, width);
        return to + dstRB;
    };

    for (int y = 0; y < srcH; y += 1) {
        auto s = load(src, width, toA8);
        auto b = blur(s,
                      g0, g1, g2, g3, g4,
                      &d01, &d12, &d23, &d34, &d45, &d56, &d67, &d78);
        store(dst, b, width);
        src += srcRB;
        dst += dstRB;
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
static void blur_y_rect(ToA8 toA8, const int strideOf8,
                        BlurY blur, int radius, uint16_t *gauss,
                        const uint8_t *src, size_t srcRB, int srcW, int srcH,
                        uint8_t *dst, size_t dstRB) {

    Sk8h g0{gauss[0]},
         g1{gauss[1]},
         g2{gauss[2]},
         g3{gauss[3]},
         g4{gauss[4]};

    int x = 0;
    for (; x <= srcW - 8; x += 8) {
        blur_column(toA8, blur, radius, 8,
                    g0, g1, g2, g3, g4,
                    src, srcRB, srcH,
                    dst, dstRB);
        src += strideOf8;
        dst += 8;
    }

    int xTail = srcW - x;
    if (xTail > 0) {
        blur_column(toA8, blur, radius, xTail,
                    g0, g1, g2, g3, g4,
                    src, srcRB, srcH,
                    dst, dstRB);
    }
}

static void direct_blur_y(ToA8 toA8, const int strideOf8,
                          int radius, uint16_t* gauss,
                          const uint8_t* src, size_t srcRB, int srcW, int srcH,
                          uint8_t* dst, size_t dstRB) {

    switch (radius) {
        case 1:
            blur_y_rect(toA8, strideOf8, blur_y_radius_1, 1, gauss,
                        src, srcRB, srcW, srcH,
                        dst, dstRB);
            break;

        case 2:
            blur_y_rect(toA8, strideOf8, blur_y_radius_2, 2, gauss,
                        src, srcRB, srcW, srcH,
                        dst, dstRB);
            break;

        case 3:
            blur_y_rect(toA8, strideOf8, blur_y_radius_3, 3, gauss,
                        src, srcRB, srcW, srcH,
                        dst, dstRB);
            break;

        case 4:
            blur_y_rect(toA8, strideOf8, blur_y_radius_4, 4, gauss,
                        src, srcRB, srcW, srcH,
                        dst, dstRB);
            break;

        default:
            SkASSERTF(false, "The radius %d is not handled\n", radius);
    }
}

static SkIPoint small_blur(double sigmaX, double sigmaY, const SkMask& src, SkMask* dst) {
    SkASSERT(sigmaX == sigmaY); // TODO
    SkASSERT(0.01 <= sigmaX && sigmaX < 2);
    SkASSERT(0.01 <= sigmaY && sigmaY < 2);

    SkGaussFilter filterX{sigmaX},
                  filterY{sigmaY};

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

    *dst = SkMask::PrepareDestination(radiusX, radiusY, src);
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

    size_t srcRB = src.fRowBytes,
           dstRB = dst->fRowBytes;

    //TODO: handle bluring in only one direction.

    // Blur vertically and copy to destination.
    switch (src.fFormat) {
        case SkMask::kBW_Format:
            direct_blur_y(bw_to_a8, 1,
                          radiusY, gaussFactorsY,
                          src.fImage, srcRB, srcW, srcH,
                          dst->fImage + radiusX, dstRB);
            break;
        case SkMask::kA8_Format:
            direct_blur_y(nullptr, 8,
                          radiusY, gaussFactorsY,
                          src.fImage, srcRB, srcW, srcH,
                          dst->fImage + radiusX, dstRB);
            break;
        case SkMask::kARGB32_Format:
            direct_blur_y(argb32_to_a8, 32,
                          radiusY, gaussFactorsY,
                          src.fImage, srcRB, srcW, srcH,
                          dst->fImage + radiusX, dstRB);
            break;
        case SkMask::kLCD16_Format:
            direct_blur_y(lcd_to_a8, 16, radiusY, gaussFactorsY,
                          src.fImage, srcRB, srcW, srcH,
                          dst->fImage + radiusX, dstRB);
            break;
        default:
            SK_ABORT("Unhandled format.");
    }

    // Blur horizontally in place.
    direct_blur_x(radiusX, gaussFactorsX,
                  dst->fImage + radiusX,  dstRB, srcW,
                  dst->fImage,            dstRB, dstW, dstH);

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

    PlanGauss planW(fSigmaW);
    PlanGauss planH(fSigmaH);

    int borderW = planW.border(),
        borderH = planH.border();
    SkASSERT(borderH >= 0 && borderW >= 0);

    *dst = SkMask::PrepareDestination(borderW, borderH, src);
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

    auto bufferSize = std::max(planW.bufferSize(), planH.bufferSize());
    auto buffer = alloc.makeArrayDefault<uint32_t>(bufferSize);

    // Blur both directions.
    int tmpW = srcH,
        tmpH = dstW;

    auto tmp = alloc.makeArrayDefault<uint8_t>(tmpW * tmpH);

    // Blur horizontally, and transpose.
    const PlanGauss::Scan& scanW = planW.makeBlurScan(srcW, buffer);
    switch (src.fFormat) {
        case SkMask::kBW_Format: {
            const uint8_t* bwStart = src.fImage;
            auto start = SkMask::AlphaIter<SkMask::kBW_Format>(bwStart, 0);
            auto end = SkMask::AlphaIter<SkMask::kBW_Format>(bwStart + (srcW / 8), srcW % 8);
            for (int y = 0; y < srcH; ++y, start >>= src.fRowBytes, end >>= src.fRowBytes) {
                auto tmpStart = &tmp[y];
                scanW.blur(start, end, tmpStart, tmpW, tmpStart + tmpW * tmpH);
            }
        } break;
        case SkMask::kA8_Format: {
            const uint8_t* a8Start = src.fImage;
            auto start = SkMask::AlphaIter<SkMask::kA8_Format>(a8Start);
            auto end = SkMask::AlphaIter<SkMask::kA8_Format>(a8Start + srcW);
            for (int y = 0; y < srcH; ++y, start >>= src.fRowBytes, end >>= src.fRowBytes) {
                auto tmpStart = &tmp[y];
                scanW.blur(start, end, tmpStart, tmpW, tmpStart + tmpW * tmpH);
            }
        } break;
        case SkMask::kARGB32_Format: {
            const uint32_t* argbStart = reinterpret_cast<const uint32_t*>(src.fImage);
            auto start = SkMask::AlphaIter<SkMask::kARGB32_Format>(argbStart);
            auto end = SkMask::AlphaIter<SkMask::kARGB32_Format>(argbStart + srcW);
            for (int y = 0; y < srcH; ++y, start >>= src.fRowBytes, end >>= src.fRowBytes) {
                auto tmpStart = &tmp[y];
                scanW.blur(start, end, tmpStart, tmpW, tmpStart + tmpW * tmpH);
            }
        } break;
        case SkMask::kLCD16_Format: {
            const uint16_t* lcdStart = reinterpret_cast<const uint16_t*>(src.fImage);
            auto start = SkMask::AlphaIter<SkMask::kLCD16_Format>(lcdStart);
            auto end = SkMask::AlphaIter<SkMask::kLCD16_Format>(lcdStart + srcW);
            for (int y = 0; y < srcH; ++y, start >>= src.fRowBytes, end >>= src.fRowBytes) {
                auto tmpStart = &tmp[y];
                scanW.blur(start, end, tmpStart, tmpW, tmpStart + tmpW * tmpH);
            }
        } break;
        default:
            SK_ABORT("Unhandled format.");
    }

    // Blur vertically (scan in memory order because of the transposition),
    // and transpose back to the original orientation.
    const PlanGauss::Scan& scanH = planH.makeBlurScan(tmpW, buffer);
    for (int y = 0; y < tmpH; y++) {
        auto tmpStart = &tmp[y * tmpW];
        auto dstStart = &dst->fImage[y];

        scanH.blur(tmpStart, tmpStart + tmpW,
                   dstStart, dst->fRowBytes, dstStart + dst->fRowBytes * dstH);
    }

    return {SkTo<int32_t>(borderW), SkTo<int32_t>(borderH)};
}

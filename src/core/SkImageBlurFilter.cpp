/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageBlurFilter.h"

#include <algorithm>

#include "SkArenaAlloc.h"
#include "SkBitmap.h"
#include "SkNx.h"

static const double kPi = 3.14159265358979323846264338327950288;

class GaussBlur {
    using Pass0And1 = Sk4u[2];

public:
    static GaussBlur Make(Sk4u* buffer, uint32_t window,
                          int32_t sLeft, int32_t sRight,
                                         int32_t dRight);

    static uint32_t CalculateWindow(double sigma);

    // Calculate the size in number of Sk4u for the three sum buffers.
    static size_t CalculateBufferSize(uint32_t window);

    // Given a window size returned from CalculateWindow, return the pixels needed to create a
    // border; the distance between first src pixel and the first dst pixel which uses
    // the src pixel in its calculation.
    static uint32_t CalculateBorder(uint32_t window);

    // Blur a single row of pixels.
    void blur(const uint32_t* src, size_t srcXStride, size_t srcYStride, size_t srcH,
                    uint32_t* dst, size_t dstXStride, size_t dstYStride) const;

private:
    GaussBlur(Pass0And1* buffer01Start, Sk4u* buffer2Start, Sk4u* buffer2End,
              int32_t srcStart, int32_t srcEnd, int32_t dstEnd,
              uint32_t weight, uint32_t half)
            : fBuffer01Start{buffer01Start}
            , fBuffer2Start{buffer2Start}
            , fBuffer2End{buffer2End}
            , fSrcStart{srcStart}
            , fSrcEnd{srcEnd}
            , fDstEnd{dstEnd}
            , fWeight{weight}
            , fHalf{half} {}

    Pass0And1* const fBuffer01Start;
    Sk4u* const      fBuffer2Start;
    Sk4u* const      fBuffer2End;
    const int32_t    fSrcStart,
                     fSrcEnd,
                     fDstEnd;
    const uint32_t   fWeight,
                     fHalf;
};

// The would be dLeft parameter is assumed to be 0.
GaussBlur GaussBlur::Make(Sk4u* buffer, uint32_t window,
                          int32_t sLeft, int32_t sRight,
                                         int32_t dRight) {

    // The circular buffers are one less than the window.
    size_t pass0Count = window - 1,
           pass1Count = window - 1,
           pass2Count = (window & 1) == 1 ? window - 1 : window;

    Pass0And1* buffer01Start = (Pass0And1*)(buffer);
    Sk4u*      buffer2Start  = buffer + pass0Count + pass1Count;
    Sk4u*      buffer2End    = buffer2Start + pass2Count;

    // If the window is odd then the divisor is just window ^ 3 otherwise,
    // it is window * window * (window + 1) = window ^ 3 + window ^ 2;
    auto window2 = window * window;
    auto window3 = window2 * window;
    auto divisor = (window & 1) == 1 ? window3 : window3 + window2;

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
    // weight = 1 / d * 2 ^ 32
    auto weight = static_cast<uint32_t>(round(1.0 / divisor * (1ull << 32)));
    auto half   = static_cast<uint32_t>((divisor + 1) / 2);

    int32_t border = CalculateBorder(window);

    // Calculate the start and end of the source pixels with respect to the destination start.
    int32_t srcStart = sLeft - border,
            srcEnd   = sRight - border;

    return GaussBlur{buffer01Start,  buffer2Start, buffer2End,
                     srcStart, srcEnd, dRight, weight, half};
}

size_t GaussBlur::CalculateBufferSize(uint32_t window) {
    size_t bufferSize = window - 1;
    return (window & 1) == 1 ? 3 * bufferSize : 3 * bufferSize + 1;
}

// This is defined by the SVG spec:
// https://drafts.fxtf.org/filter-effects/#feGaussianBlurElement
uint32_t GaussBlur::CalculateWindow(double sigma) {
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
    sigma = SkTPin(sigma, 0.0, 136.0);
    auto possibleWindow = static_cast<uint32_t>(floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5));
    return std::max(static_cast<uint32_t>(1), possibleWindow);
}

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
uint32_t GaussBlur::CalculateBorder(uint32_t window) {
    return (window & 1) == 1 ? 3 * ((window - 1) / 2) : 3 * (window / 2) - 1;
}

void GaussBlur::blur(const uint32_t* src, size_t srcXStride, size_t srcYStride, size_t srcH,
                           uint32_t* dst, size_t dstXStride, size_t dstYStride) const {
    auto buffer01Start = fBuffer01Start;
    auto buffer01End   = (Pass0And1*)(fBuffer2Start);
    auto buffer2Start  = fBuffer2Start;
    auto buffer2End    = fBuffer2End;

    Sk4u weight{fWeight};

    for (size_t y = 0; y < srcH; y++) {
        auto buffer01Cursor = buffer01Start;
        auto buffer2Cursor  = buffer2Start;

        Sk4u sum0{0u};
        Sk4u sum1{0u};
        Sk4u sum2{fHalf};

        std::memset(buffer01Start, 0x00,
                    (buffer2End - (Sk4u *) (buffer01Start)) * sizeof(*buffer2Start));

        // Given an expanded input pixel, move the window ahead using the leadingEdge value.
        auto processValue = [&](const Sk4u &leadingEdge) -> Sk4u {
            sum0 += leadingEdge;
            sum1 += sum0;
            sum2 += sum1;

            Sk4u value = sum2.mulHi(weight);

            sum2 -= *buffer2Cursor;
            *buffer2Cursor = sum1;
            buffer2Cursor = (buffer2Cursor + 1) < buffer2End ? buffer2Cursor + 1 : buffer2Start;

            sum1 -= (*buffer01Cursor)[1];
            (*buffer01Cursor)[1] = sum0;
            sum0 -= (*buffer01Cursor)[0];
            (*buffer01Cursor)[0] = leadingEdge;
            buffer01Cursor =
                    (buffer01Cursor + 1) < buffer01End ? buffer01Cursor + 1 : buffer01Start;

            return value;
        };

        int32_t srcIdx = fSrcStart;
        int32_t dstIdx = 0;
        const uint32_t *srcCursor = src;
        uint32_t *dstCursor = dst;

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
            Sk4u leadingEdge = srcIdx < fSrcEnd ? SkNx_cast<uint32_t>(Sk4b::Load(srcCursor)) : 0;
            (void) processValue(leadingEdge);
            srcCursor += srcXStride;
            srcIdx++;
        }

        // The dstIdx and srcIdx are in sync now; the code just uses the dstIdx for both now.
        // Consume the source generating pixels to dst.
        auto loopEnd = std::min(fDstEnd, fSrcEnd);
        while (dstIdx < loopEnd) {
            Sk4u leadingEdge = SkNx_cast<uint32_t>(Sk4b::Load(srcCursor));
            SkNx_cast<uint8_t>(processValue(leadingEdge)).store(dstCursor);
            srcCursor += srcXStride;
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }

        // The leading edge is beyond the end of the source. Assume that the pixels
        // are now 0x0000 until the end of the destination.
        loopEnd = fDstEnd;
        while (dstIdx < loopEnd) {
            SkNx_cast<uint8_t>(processValue(0u)).store(dstCursor);
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }

        src += srcYStride;
        dst += dstYStride;
    }
}

SkImageBlurFilter::SkImageBlurFilter(SkVector sigma)
    : fSigma{sigma.x(), sigma.y()} {}

sk_sp<SkSpecialImage> SkImageBlurFilter::blur(SkSpecialImage* source,
                                              const sk_sp<SkSpecialImage>& input,
                                              SkIRect inputBounds, SkIRect dstBounds) const {
    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    SkBitmap src;
    inputBM.extractSubset(&src, inputBounds);

    // Make everything relative to the destination bounds.
    inputBounds.offset(-dstBounds.x(), -dstBounds.y());
    dstBounds.offset(  -dstBounds.x(), -dstBounds.y());

    uint32_t windowW = GaussBlur::CalculateWindow(fSigma.x()),
             windowH = GaussBlur::CalculateWindow(fSigma.y());

    auto srcW = SkTo<uint32_t>(inputBounds.width() ),
         srcH = SkTo<uint32_t>(inputBounds.height());

    auto dstW = SkTo<uint32_t>(dstBounds.width() ),
         dstH = SkTo<uint32_t>(dstBounds.height());

    SkImageInfo dstInfo = SkImageInfo::Make(dstW, dstH, inputBM.colorType(), inputBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(dstInfo)) {
        return nullptr;
    }

    if (windowW > 1 && windowH > 1) {
        // Blur both directions.

        size_t bufferSizeW = GaussBlur::CalculateBufferSize(windowW),
               bufferSizeH = GaussBlur::CalculateBufferSize(windowH);

        // The amount 1024 is enough for buffers up to 10 sigma. The tmp bitmap will be
        // allocated on the heap.
        SkSTArenaAlloc<1024> alloc;
        Sk4u* buffer = alloc.makeArrayDefault<Sk4u>(std::max(bufferSizeW, bufferSizeH));

        GaussBlur blurW = GaussBlur::Make(buffer, windowW,
                                          inputBounds.left(), inputBounds.right(),
                                          dstBounds.right()),
                  blurH = GaussBlur::Make(buffer, windowH,
                                          inputBounds.top(), inputBounds.bottom(),
                                          dstBounds.bottom());

        size_t tmpW = srcH;
        size_t tmpH = dstW;

        auto tmp = alloc.makeArrayDefault<uint32_t>(tmpW * tmpH);

        // Blur horizontally, and transpose.
        blurW.blur(static_cast<uint32_t*>(src.getPixels()), 1, src.rowBytesAsPixels(), srcH,
                   tmp, tmpW, 1);

        // Blur vertically (scan in memory order because of the transposition),
        // and transpose back to the original orientation.
        blurH.blur(tmp, 1, tmpW, tmpH,
                   static_cast<uint32_t*>(dst.getPixels()), dst.rowBytesAsPixels(), 1);
    } else if (windowW > 1) {
        // Blur only horizontally.

        size_t bufferSizeW = GaussBlur::CalculateBufferSize(windowW);

        // The amount 512 is enough for buffers up to 10 sigma.
        SkSTArenaAlloc<512> alloc;
        Sk4u* buffer = alloc.makeArrayDefault<Sk4u>(bufferSizeW);

        GaussBlur blurW = GaussBlur::Make(buffer, windowW,
                                          inputBounds.left(), inputBounds.right(),
                                          dstBounds.right());

            blurW.blur(static_cast<uint32_t*>(src.getPixels()), 1, src.rowBytesAsPixels(), srcH,
                       static_cast<uint32_t*>(dst.getPixels()), 1, dst.rowBytesAsPixels());
    } else if (windowH > 1) {
        // Blur only vertically.

        size_t bufferSizeH = GaussBlur::CalculateBufferSize(windowH);

        // The amount 512 is enough for buffers up to 10 sigma.
        SkSTArenaAlloc<512> alloc;
        Sk4u* buffer = alloc.makeArrayDefault<Sk4u>(bufferSizeH);

        GaussBlur blurH = GaussBlur::Make(buffer, windowH,
                                          inputBounds.top(), inputBounds.bottom(),
                                          dstBounds.bottom());

        blurH.blur(static_cast<uint32_t*>(src.getPixels()), src.rowBytesAsPixels(), 1, srcW,
                   static_cast<uint32_t*>(dst.getPixels()), dst.rowBytesAsPixels(), 1);
    } else {
        return input->makeSubset(inputBounds);
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(dstBounds.width(),
                                                          dstBounds.height()),
                                          dst, &source->props());
}

SkRect SkImageBlurFilter::OutsetRect(const SkRect& src, SkVector sigma) {
    uint32_t borderW = GaussBlur::CalculateBorder(GaussBlur::CalculateWindow(sigma.x())),
             borderH = GaussBlur::CalculateBorder(GaussBlur::CalculateWindow(sigma.y()));
    SkRect bounds = src;
    bounds.outset(borderW, borderH);
    return bounds;
}

SkIRect SkImageBlurFilter::OutsetIRect(const SkIRect& src, SkVector sigma) {
    uint32_t borderW = GaussBlur::CalculateBorder(GaussBlur::CalculateWindow(sigma.x())),
             borderH = GaussBlur::CalculateBorder(GaussBlur::CalculateWindow(sigma.y()));
    SkIRect bounds = src;
    bounds.outset(borderW, borderH);
    return bounds;
}

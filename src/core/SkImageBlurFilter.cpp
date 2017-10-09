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
public:
    // dstBounds is assumed to have a (left, top) == (0, 0). The inputBounds is relative to that point.
    static GaussBlur Make(SkArenaAlloc* alloc, double sigma,
                          int32_t sLeft, int32_t sRight,
                          int32_t dLeft, int32_t dRight) {
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

        // Shift rectangles from input frame of reference to dst frame of reference.
        auto window = CalculateWindow(sigma);
        //auto border = CalculateBorder(sigma);

        // The circular buffers are one less than the window.
        size_t pass0Count = window - 1,
               pass1Count = window - 1,
               pass2Count = (window & 1) == 1 ? window - 1 : window;

        Sk4u*  buffer = alloc->makeArrayDefault<Sk4u>(pass0Count + pass1Count + pass2Count);

        Sk4u   *buffer0    = buffer,
               *buffer01   = buffer0 + pass0Count,
               *buffer12   = buffer01 + pass1Count,
               *buffer2End = buffer12 + pass2Count;

        int32_t border = CalculateBorder(sigma);

        // If the window is odd then the divisor is just window ^ 3 otherwise,
        // it is window * window * (window + 1) = window ^ 3 + window ^ 2;
        auto window2 = window * window;
        auto window3 = window2 * window;
        auto divisor = (window & 1) == 1 ? window3 : window3 + window2;
        uint32_t weight = static_cast<uint64_t>(round(1.0 / divisor * (1ull << 32)));
        uint32_t half = static_cast<uint32_t>((divisor + 1) / 2);

        return GaussBlur{
                buffer0,  buffer01,
                buffer01, buffer12,
                buffer12, buffer2End,
                sLeft - border, sRight - border, dRight,
                weight, half};
    }

    GaussBlur(Sk4u* buffer0, Sk4u* buffer0End,
              Sk4u* buffer1, Sk4u* buffer1End,
              Sk4u* buffer2, Sk4u* buffer2End,
              int32_t srcStart, int32_t srcEnd, int32_t dstEnd,
              uint32_t weight, uint32_t half)
        : fBuffer0{buffer0}
        , fBuffer0End{buffer0End}
        , fBuffer1{buffer1}
        , fBuffer1End{buffer1End}
        , fBuffer2{buffer2}
        , fBuffer2End{buffer2End}
        , fSrcStart{srcStart}
        , fSrcEnd{srcEnd}
        , fDstEnd{dstEnd}
        , fWeight{weight}
        , fHalf{half} {}

    void blur(const uint32_t* src, size_t srcStride,
                    uint32_t* dst, size_t dstStride) const;

    static uint32_t CalculateWindow(double sigma) {
        auto possibleWindow = static_cast<uint32_t>(floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5));
        return std::max(static_cast<uint32_t>(1), possibleWindow);
    }

    static uint32_t CalculateBorder(double sigma) {
        auto window = CalculateWindow(sigma);
        return (window & 1) == 1 ? 3 * ((window - 1) / 2) : 3 * (window / 2) - 1;
    }

    Sk4u* const    fBuffer0;
    Sk4u* const    fBuffer0End;
    Sk4u* const    fBuffer1;
    Sk4u* const    fBuffer1End;
    Sk4u* const    fBuffer2;
    Sk4u* const    fBuffer2End;
    int32_t        fSrcStart,
                   fSrcEnd,
                   fDstEnd;
    uint32_t       fWeight,
                   fHalf;
};

void GaussBlur::blur(const uint32_t* src, size_t srcStride, uint32_t* dst, size_t dstStride) const {
    auto buffer0Start = fBuffer0;
    auto buffer1Start = fBuffer1;
    auto buffer2Start = fBuffer2;

    auto buffer0Cursor = fBuffer0;
    auto buffer1Cursor = fBuffer1;
    auto buffer2Cursor = fBuffer2;

    auto buffer0End = fBuffer0End;
    auto buffer1End = fBuffer1End;
    auto buffer2End = fBuffer2End;

    Sk4u sum0{0u};
    Sk4u sum1{0u};
    Sk4u sum2{fHalf};

    std::memset(fBuffer0, 0x00, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

    Sk4u weight{fWeight};

    // Given an expanded input pixel, move the window ahead using the leadingEdge value.
    auto processValue = [&](const Sk4u leadingEdge) -> Sk4u {
        sum0 += leadingEdge;
        sum1 += sum0;
        sum2 += sum1;

        Sk4u value = sum2.mulHi(weight);

        sum2 -= *buffer2Cursor;
        *buffer2Cursor = sum1;
        buffer2Cursor = (buffer2Cursor + 1) < buffer2End ? buffer2Cursor + 1 : buffer2Start;

        sum1 -= *buffer1Cursor;
        *buffer1Cursor = sum0;
        buffer1Cursor = (buffer1Cursor + 1) < buffer1End ? buffer1Cursor + 1 : buffer1Start;

        sum0 -= *buffer0Cursor;
        *buffer0Cursor = leadingEdge;
        buffer0Cursor = (buffer0Cursor + 1) < buffer0End ? buffer0Cursor + 1 : buffer0Start;
        return value;
    };

    int32_t srcIdx = fSrcStart;
    int32_t dstIdx = 0;
    const uint32_t* srcCursor = src;
          uint32_t* dstCursor = dst;

    while (dstIdx < srcIdx) {
        *dstCursor = 0;
        dstCursor += dstStride;
        dstIdx++;
    }

    while (dstIdx > srcIdx) {
        Sk4u leadingEdge = srcIdx < fSrcEnd ? SkNx_cast<uint32_t>(Sk4b::Load(srcCursor)) : 0;
        (void)processValue(leadingEdge);
        srcCursor += srcStride;
        srcIdx++;
    }

    // The srcCursor and dstCursor are in sync now.
    // The window edge starts beyond the left edge of the image.

    // Consume the source generating pixels to dst.
    while (dstIdx < fDstEnd && srcIdx < fSrcEnd) {
        Sk4u leadingEdge = SkNx_cast<uint32_t>(Sk4b::Load(srcCursor));
        SkNx_cast<uint8_t>(processValue(leadingEdge)).store(dstCursor);
        dstCursor += dstStride;
        srcCursor += srcStride;
        dstIdx++;
        srcIdx++;
    }

    while (dstIdx < fDstEnd) {
        SkNx_cast<uint8_t>(processValue(0u)).store(dstCursor);
        dstCursor += dstStride;
        dstIdx++;
    }
}

SkImageBlurFilter::SkImageBlurFilter(SkVector sigma)
    : fSigma{sigma} {}

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

    SkBitmap srcBM;
    inputBM.extractSubset(&srcBM, inputBounds);

    inputBounds.offset(-dstBounds.x(), -dstBounds.y());
    dstBounds.offset(-dstBounds.x(), -dstBounds.y());

    // 1024 is a place holder guess until more analysis can be done.
    SkSTArenaAlloc<1024> alloc;
    GaussBlur blurW = GaussBlur::Make(&alloc, fSigma.x(),
                                      inputBounds.left(), inputBounds.right(),
                                      dstBounds.left(), dstBounds.right()),
              blurH = GaussBlur::Make(&alloc, fSigma.y(),
                                      inputBounds.top(), inputBounds.bottom(),
                                      dstBounds.top(), dstBounds.bottom());

    uint32_t windowW = GaussBlur::CalculateWindow(fSigma.x()),
             windowH = GaussBlur::CalculateWindow(fSigma.y());

    uint32_t srcW = SkTo<uint32_t>(inputBounds.width() ),
             srcH = SkTo<uint32_t>(inputBounds.height());

    uint32_t dstW = SkTo<uint32_t>(dstBounds.width() ),
             dstH = SkTo<uint32_t>(dstBounds.height());

    SkImageInfo dstInfo = SkImageInfo::Make(dstW, dstH, inputBM.colorType(), inputBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(dstInfo)) {
        return nullptr;
    }

    if (windowW > 1 && windowH > 1) {
        // Make srcBM based on input.

        // Blur both directions.
        size_t tmpW = srcH;
        size_t tmpH = dstW;

        auto tmp = alloc.makeArrayDefault<uint32_t>(tmpW * tmpH);

        // Blur horizontally, and transpose.
        for (size_t y = 0; y < srcH; y++) {
            auto srcStart = srcBM.getAddr32(0, y);
            auto tmpStart = &tmp[y];
            blurW.blur(srcStart, 1, tmpStart, tmpW);
        }

        // Blur vertically (scan in memory order because of the transposition),
        // and transpose back to the original orientation.
        for (size_t y = 0; y < tmpH; y++) {
            auto tmpStart = &tmp[y * tmpW];
            // N.B. Use y as x parameter to transpose.
            auto dstStart = dst.getAddr32(y, 0);

            blurH.blur(tmpStart, 1, dstStart, dst.rowBytesAsPixels());
        }
    } else if (windowW > 1) {
        // Blur only horizontally.

        for (size_t y = 0; y < srcH; y++) {
            auto srcStart = srcBM.getAddr32(0, y);
            auto dstStart = dst.getAddr32(0, y);
            blurW.blur(srcStart, 1, dstStart, 1);

        }
    } else if (windowH > 1) {
        // Blur only vertically.

        for (size_t x = 0; x < srcW; x++) {
            auto srcStart = srcBM.getAddr32(x, 0);
            auto dstStart = dst.getAddr32(x, 0);

            blurH.blur(srcStart, inputBM.rowBytesAsPixels(),
                       dstStart, dst.rowBytesAsPixels());
        }
    } else {
        return input->makeSubset(inputBounds);
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(dstBounds.width(),
                                                          dstBounds.height()),
                                          dst, &source->props());
}

SkRect SkImageBlurFilter::OutsetRect(const SkRect& src, SkVector sigma) {
    uint32_t borderW = GaussBlur::CalculateBorder(sigma.x()),
             borderH = GaussBlur::CalculateBorder(sigma.y());
    SkRect bounds = src;
    bounds.outset(borderW, borderH);
    return bounds;
}

SkIRect SkImageBlurFilter::OutsetIRect(const SkIRect& src, SkVector sigma) {
    uint32_t borderW = GaussBlur::CalculateBorder(sigma.x()),
             borderH = GaussBlur::CalculateBorder(sigma.y());
    SkIRect bounds = src;
    bounds.outset(borderW, borderH);
    return bounds;
}

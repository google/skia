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
    static GaussBlur Make(SkArenaAlloc* alloc, double sigma, SkIRect inputBounds, SkIRect dstBounds) {
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

        size_t initializeLeft = 0,
               noChangeCount = 0,
               initializeRight = 0;

        uint32_t weight = 0;

        return GaussBlur{
                buffer0,  buffer01,
                buffer01, buffer12,
                buffer12, buffer2End,
                initializeLeft, noChangeCount, initializeRight,
                weight};
    }

    GaussBlur(Sk4u* buffer0, Sk4u* buffer0End,
              Sk4u* buffer1, Sk4u* buffer1End,
              Sk4u* buffer2, Sk4u* buffer2End,
              size_t initializeLeft, size_t noChangeCount, size_t initializeRight,
              uint32_t weight)
        : fBuffer0{buffer0}
        , fBuffer0End{buffer0End}
        , fBuffer1{buffer1}
        , fBuffer1End{buffer1End}
        , fBuffer2{buffer2}
        , fBuffer2End{buffer2End}
        , fInitializeLeft{initializeLeft}
        , fNoChangeCount{noChangeCount}
        , fInitializeRight{initializeRight}
        , fWeight{weight} {}

    void blur(const uint32_t* src, size_t srcStride, const uint32_t* srcEnd,
                    uint32_t* dst, size_t dstStride,       uint32_t* dstEnd) const;

    static uint32_t CalculateWindow(double sigma) {
        auto possibleWindow = static_cast<uint32_t>(floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5));
        return std::max(static_cast<uint32_t>(1), possibleWindow);
    }

    static uint32_t CalculateBorder(double sigma) {
        auto window = CalculateWindow(sigma);
        return (window & 1) == 1 ? 3 * ((window - 1) / 2) : 3 * (window / 2) - 1;
    }

    Sk4u finalScale(const Sk4u& sum) const {
        return sum.scaleRound(fWeight);
    }

    Sk4u* const    fBuffer0;
    Sk4u* const    fBuffer0End;
    Sk4u* const    fBuffer1;
    Sk4u* const    fBuffer1End;
    Sk4u* const    fBuffer2;
    Sk4u* const    fBuffer2End;
    size_t         fInitializeLeft;
    size_t         fNoChangeCount;
    size_t         fInitializeRight;
    uint32_t       fWeight;
};


void GaussBlur::blur(const uint32_t* src, size_t srcStride, const uint32_t* srcEnd,
                           uint32_t* dst, size_t dstStride,       uint32_t* dstEnd) const {
    auto buffer0Cursor = fBuffer0;
    auto buffer1Cursor = fBuffer1;
    auto buffer2Cursor = fBuffer2;

    Sk4u sum0{0u};
    Sk4u sum1{0u};
    Sk4u sum2{0u};

    // Given an expanded input pixel, move the window ahead using the leadingEdge value.
    // NB, this can be use traversing to the left or to the right.
    auto processValue = [&](const Sk4u leadingEdge) -> Sk4u {
        sum0 += leadingEdge;
        sum1 += sum0;
        sum2 += sum1;

        Sk4u value= sum2;

        sum2 -= *buffer2Cursor;
        *buffer2Cursor = sum1;
        buffer2Cursor = (buffer2Cursor + 1) < fBuffer2End ? buffer2Cursor + 1 : fBuffer2;

        sum1 -= *buffer1Cursor;
        *buffer1Cursor = sum0;
        buffer1Cursor = (buffer1Cursor + 1) < fBuffer1End ? buffer1Cursor + 1 : fBuffer1;

        sum0 -= *buffer0Cursor;
        *buffer0Cursor = leadingEdge;
        buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;
        return value;
    };

    std::memset(fBuffer0, 0x00, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

    // The window edge starts beyond the left edge of the image.
    const uint32_t* srcCursor = src;
    for (size_t i = 0; i < fInitializeLeft; i++) {
        Sk4u leadingEdge = srcCursor < srcEnd ? SkNx_cast<uint32_t>(Sk4b::Load(srcCursor)) : 0;
        (void)processValue(leadingEdge);
        srcCursor += srcStride;
    }

    // Consume the source generating pixels to dst.
    while (srcCursor < srcEnd) {
        Sk4u leadingEdge = SkNx_cast<uint32_t>(Sk4b::Load(srcCursor));
        SkNx_cast<uint8_t>(this->finalScale(processValue(leadingEdge))).store(dst);
        dst       += dstStride;
        srcCursor += srcStride;
    }

    // The leading edge and trailing edges are off the image.
    for (size_t i = 0; i < fNoChangeCount; i++) {
        SkNx_cast<uint8_t>(this->finalScale(processValue(0u))).store(dst);
        dst += dstStride;
    }

    // Starting from the right, fill in the rest of the buffer.
    std::memset(fBuffer0, 0, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

    sum0 = sum1 = sum2 = 0;
    uint32_t* dstCursor = dstEnd;
    srcCursor = srcEnd;

    for (size_t i = 0; i < fInitializeRight; i++) {
        srcCursor -= srcStride;
        Sk4u leadingEdge = srcCursor >= src ? SkNx_cast<uint32_t>(Sk4b::Load(srcCursor)) : 0;
        (void)processValue(leadingEdge);
    }

    while (dstCursor > dst) {
        srcCursor -= srcStride;
        dstCursor -= dstStride;
        Sk4u leadingEdge = SkNx_cast<uint32_t>(Sk4b::Load(srcCursor));
        SkNx_cast<uint8_t>(this->finalScale(processValue(leadingEdge))).store(dstCursor);
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

    // 1024 is a place holder guess until more analysis can be done.
    SkSTArenaAlloc<1024> alloc;
    GaussBlur blurW = GaussBlur::Make(&alloc, fSigma.y(), inputBounds, dstBounds),
              blurH = GaussBlur::Make(&alloc, fSigma.y(), inputBounds, dstBounds);

    uint32_t windowW = GaussBlur::CalculateWindow(fSigma.x()),
             windowH = GaussBlur::CalculateWindow(fSigma.y());

    uint32_t srcW = SkTo<uint32_t>(inputBounds.width() ),
             srcH = SkTo<uint32_t>(inputBounds.height());

    uint32_t dstW = SkTo<uint32_t>(dstBounds.width() ),
             dstH = SkTo<uint32_t>(dstBounds.height());

    SkImageInfo dstInfo = SkImageInfo::Make(dstW, dstH, inputBM.colorType(), inputBM.alphaType());

    inputBounds.offset(-dstBounds.x(), -dstBounds.y());
    dstBounds.offset(-dstBounds.x(), -dstBounds.y());

    SkBitmap dst;
    if (!dst.tryAllocPixels(dstInfo)) {
        return nullptr;
    }

    if (windowW > 1 && windowH > 1) {
        // Make srcBM based on input.

        sk_sp<SkSpecialImage> srcBM = input

        // Blur both directions.
        size_t tmpW = srcH;
        size_t tmpH = dstW;

        auto tmp = alloc.makeArrayDefault<uint32_t>(tmpW * tmpH);

        // Blur horizontally, and transpose.
        size_t y = inputBounds.top();
        for (;y < inputBounds.bottom(); y++) {
            auto srcStart = inputBM.getAddr32(inputBounds.left(), y);
            auto tmpStart = &tmp[y];
            blurW.blur(srcStart,    1, srcStart + inputBounds.width(),
                       tmpStart, tmpW, tmpStart + tmpW * tmpH);
        }

        // Blur vertically (scan in memory order because of the transposition),
        // and transpose back to the original orientation.
        y = 0;
        for (;y < tmpH; y++) {
            auto tmpStart = &tmp[y * tmpW];
            // N.B. Use y as x parameter to transpose.
            auto dstStart = dst.getAddr32(y, 0);

            blurH.blur(tmpStart, 1, tmpStart + tmpW,
                       dstStart, dst.rowBytesAsPixels(), dstStart + dst.width() * dstH);
        }
    } else if (windowW > 1) {
        // Blur only horizontally.

        for (size_t y = 0; y < srcH; y++) {
            auto srcStart = inputBM.getAddr32(0, y);
            auto dstStart = dst.getAddr32(0, y);
            blurW.blur(srcStart, 1, srcStart + srcW,
                       dstStart, 1, dstStart + dstW);

        }
    } else if (windowH > 1) {
        // Blur only vertically.

        auto srcEnd   = inputBM.getAddr32(0, srcH);
        auto dstEnd   = dst.getAddr32(0, dstH);
        for (size_t x = 0; x < srcW; x++) {
            auto srcStart = inputBM.getAddr32(x, 0);
            auto dstStart = dst.getAddr32(x, 0);
            blurH.blur(srcStart, inputBM.rowBytesAsPixels(), srcEnd,
                       dstStart, dst.rowBytesAsPixels(), dstEnd);
        }
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(dstBounds.width(),
                                                          dstBounds.height()),
                                          dst, &source->props());
}

SkRect SkImageBlurFilter::OutsetRect(const SkRect& src, SkVector sigma) {
    uint32_t borderW = GaussBlur::CalculateBorder(sigma.x()),
             borderH = GaussBlur::CalculateBorder(sigma.y());
    SkRect bounds = src;
    bounds.offset(borderW, borderH);
    return bounds;
}

SkIRect SkImageBlurFilter::OutsetIRect(const SkIRect& src, SkVector sigma) {
    uint32_t borderW = GaussBlur::CalculateBorder(sigma.x()),
             borderH = GaussBlur::CalculateBorder(sigma.y());
    SkIRect bounds = src;
    bounds.outset(borderW, borderH);
    return bounds;
}

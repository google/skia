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
    explicit GaussBlur(double sigma, SkArenaAlloc* alloc)
        : fBuffer0{nullptr}
        , fBuffer0End{nullptr}
        , fBuffer1{nullptr}
        , fBuffer1End{nullptr}
        , fBuffer2{nullptr}
        , fBuffer2End{nullptr}
        , fSigma{sigma}
        , fWindow{CalculateWindow(sigma)}
        , fBorder{CalculateBorder(sigma)}
    {
        SkASSERT(sigma >= 0);
    }

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

    static constexpr uint64_t kHalf = static_cast<uint64_t>(1) << 31;

    Sk4u finalScale(const Sk4u& sum) const {
        return sum.scaleRound(fWeight);
    }

    Sk4u* const    fBuffer0;
    Sk4u* const    fBuffer0End;
    Sk4u* const    fBuffer1;
    Sk4u* const    fBuffer1End;
    Sk4u* const    fBuffer2;
    Sk4u* const    fBuffer2End;
    const double   fSigma;
    const uint32_t fWindow;
    const uint32_t fBorder;
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
        Sk4u leadingEdge = SkNx_cast<uint32_t>(Sk4b::Load(srcCursor));
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
        Sk4u leadingEdge = SkNx_cast<uint32_t>(Sk4b::Load(srcCursor));
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
                                              SkIRect inputBounds, SkIRect dstBounds,
                                              SkIPoint inputOffset, SkIPoint* offset) const {
    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    // 1024 is a place holder guess until more analysis can be done.
    SkSTArenaAlloc<1024> alloc;
    GaussBlur blurW{fSigma.x(), &alloc},
              blurH{fSigma.y(), &alloc};

    uint32_t windowW = blurW.fWindow,
             windowH = blurH.fWindow;

    uint32_t borderW = blurW.fBorder,
             borderH = blurH.fBorder;

    uint32_t srcW = SkTo<uint32_t>(inputBounds.width() ),
             srcH = SkTo<uint32_t>(inputBounds.height());

    uint32_t dstW = borderW + srcW + borderW,
             dstH = borderH + srcH + borderH;

    SkImageInfo dstInfo = SkImageInfo::Make(dstW, dstH, inputBM.colorType(), inputBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(dstInfo)) {
        return nullptr;
    }

    if (windowW > 1 && windowH > 1) {
        // Blur both directions.
        size_t tmpW = srcH;
        size_t tmpH = dstW;

        auto tmp = alloc.makeArrayDefault<uint32_t>(tmpW * tmpH);

        // Blur horizontally, and transpose.
        size_t y = 0;
        for (;y < srcH; y++) {
            auto srcStart = inputBM.getAddr32(0, y);
            auto tmpStart = &tmp[y];
            blurW.blur(srcStart,    1, srcStart + srcW,
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
    } else {
        offset->fX = inputBounds.x();
        offset->fY = inputBounds.y();
        return input->makeSubset(inputBounds.makeOffset(-inputOffset.x(),
                                                        -inputOffset.y()));
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

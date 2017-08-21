/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMaskBlurFilter.h"

#include <cmath>

#include "SkMakeUnique.h"

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

static uint32_t filter_window(double sigma) {
    if (sigma < kSmallSigma) {
        auto radius = static_cast<uint32_t>(ceil(1.5 * sigma - 0.5));
        return 2 * radius + 1;
    }
    auto possibleWindow = static_cast<uint32_t>(floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5));
    return std::max(1u, possibleWindow);
}

SkMaskBlurFilter::FilterInfo::FilterInfo(double sigma)
    : fIsSmall{sigma < kSmallSigma}
    , fFilterWindow{filter_window(sigma)}
    , fWeight{fIsSmall ? fFilterWindow : weight_from_diameter(fFilterWindow)}
    , fScaledWeight{(static_cast<uint64_t>(1) << 32) / fWeight}
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

SkMaskBlurFilter::SkMaskBlurFilter(double sigmaW, double sigmaH)
    : fInfoW{sigmaW}, fInfoH{sigmaH}
    , fBuffer0{skstd::make_unique_default<uint32_t[]>(bufferSize(0))}
    , fBuffer1{skstd::make_unique_default<uint32_t[]>(bufferSize(1))}
    , fBuffer2{skstd::make_unique_default<uint32_t[]>(bufferSize(2))} {
}

SkIPoint SkMaskBlurFilter::blur(const SkMask& src, SkMask* dst) const {

    uint64_t weightW = fInfoW.weight();
    uint64_t weightH = fInfoH.weight();

    size_t borderW = fInfoW.borderSize();
    size_t borderH = fInfoH.borderSize();

    size_t srcW = src.fBounds.width();
    size_t srcH = src.fBounds.height();

    size_t dstW = srcW + 2 * borderW;
    size_t dstH = srcH + 2 * borderH;

    dst->fBounds.set(0, 0, dstW, dstH);
    dst->fBounds.offset(src.fBounds.x(), src.fBounds.y());
    dst->fBounds.offset(-SkTo<int32_t>(borderW), -SkTo<int32_t>(borderH));

    dst->fImage = nullptr;
    dst->fRowBytes = dstW;
    dst->fFormat = SkMask::kA8_Format;

    if (src.fImage == nullptr) {
        return {SkTo<int32_t>(borderW), SkTo<int32_t>(borderH)};
    }

    dst->fImage = SkMask::AllocImage(dstW * dstH);

    if (weightW > 1 && weightH > 1) {
        // Blur both directions.
        size_t tmpW = srcH;
        size_t tmpH = dstW;
        auto tmp = skstd::make_unique_default<uint8_t[]>(tmpW * tmpH);

        // Blur horizontally, and transpose.
        for (size_t y = 0; y < srcH; y++) {
            auto srcStart = &src.fImage[y * src.fRowBytes];
            auto tmpStart = &tmp[y];
            this->blurOneScan(fInfoW,
                              srcStart, 1, srcStart + srcW,
                              tmpStart, tmpW, tmpStart + tmpW * tmpH);
        }

        // Blur vertically (scan in memory order because of the transposition),
        // and transpose back to the original orientation.
        for (size_t y = 0; y < tmpH; y++) {
            auto tmpStart = &tmp[y * tmpW];
            auto dstStart = &dst->fImage[y];
            this->blurOneScan(fInfoH,
                              tmpStart, 1, tmpStart + tmpW,
                              dstStart, dst->fRowBytes, dstStart + dst->fRowBytes * dstH);
        }
    } else if (weightW > 1) {
        // Blur only horizontally.

        for (size_t y = 0; y < srcH; y++) {
            auto srcStart = &src.fImage[y * src.fRowBytes];
            auto dstStart = &dst->fImage[y * dst->fRowBytes];
            this->blurOneScan(fInfoW,
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
            this->blurOneScan(fInfoH,
                              srcStart, src.fRowBytes, srcEnd,
                              dstStart, dst->fRowBytes, dstEnd);
        }
    } else {
        // Copy to dst. No Blur.

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
    FilterInfo info,
    const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
          uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const {
    // We don't think this is good for quality. It is good for compatibility
    // with previous expectations...
    if (info.isSmall()) {
        this->blurOneScanBox(info, src, srcStride, srcEnd, dst, dstStride, dstEnd);
    } else {
        this->blurOneScanGauss(info, src, srcStride, srcEnd, dst, dstStride, dstEnd);
    }

}

// Blur one horizontal scan into the dst.
void SkMaskBlurFilter::blurOneScanBox(
    FilterInfo info,
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
void SkMaskBlurFilter::blurOneScanGauss(
    FilterInfo info,
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

    const uint64_t half = static_cast<uint64_t>(1) << 31;

    // Consume the source generating pixels.
    for (auto srcCursor = src; srcCursor < srcEnd; dst += dstStride, srcCursor += srcStride) {
        uint32_t s = *srcCursor;
        sum0 += s;
        sum1 += sum0;
        sum2 += sum1;

        *dst = SkTo<uint8_t>((info.scaledWeight() * sum2 + half) >> 32);

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

        *dst = SkTo<uint8_t>((info.scaledWeight() * sum2 + half) >> 32);

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

        *dstCursor = SkTo<uint8_t>((info.scaledWeight() * sum2 + half) >> 32);

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

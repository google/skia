/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkAndroidCodecAdapter.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkSampledCodec.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <utility>

class SkPngChunkReader;

static bool is_valid_sample_size(int sampleSize) {
    // FIXME: As Leon has mentioned elsewhere, surely there is also a maximum sampleSize?
    return sampleSize > 0;
}

SkAndroidCodec::SkAndroidCodec(SkCodec* codec)
    : fInfo(codec->getInfo())
    , fCodec(codec)
{}

SkAndroidCodec::~SkAndroidCodec() {}

std::unique_ptr<SkAndroidCodec> SkAndroidCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               SkPngChunkReader* chunkReader) {
    auto codec = SkCodec::MakeFromStream(std::move(stream), nullptr, chunkReader);
    return MakeFromCodec(std::move(codec));
}

std::unique_ptr<SkAndroidCodec> SkAndroidCodec::MakeFromCodec(std::unique_ptr<SkCodec> codec) {
    if (nullptr == codec) {
        return nullptr;
    }

    switch (codec->getEncodedFormat()) {
        case SkEncodedImageFormat::kPNG:
        case SkEncodedImageFormat::kICO:
        case SkEncodedImageFormat::kJPEG:
        case SkEncodedImageFormat::kBMP:
        case SkEncodedImageFormat::kWBMP:
            return std::make_unique<SkSampledCodec>(codec.release());
        case SkEncodedImageFormat::kGIF:
        case SkEncodedImageFormat::kWEBP:
        case SkEncodedImageFormat::kDNG:
        // On the Android framework, both HEIF and AVIF are handled by
        // SkCrabbyAvifCodec. It can handle scaling internally. So we can use
        // SkAndroidCodecAdapter for both these formats.
        case SkEncodedImageFormat::kAVIF:
        case SkEncodedImageFormat::kHEIF:
            return std::make_unique<SkAndroidCodecAdapter>(codec.release());
        case SkEncodedImageFormat::kPKM:
        case SkEncodedImageFormat::kKTX:
        case SkEncodedImageFormat::kASTC:
        case SkEncodedImageFormat::kJPEGXL:
            return nullptr;
    }
    SkUNREACHABLE;
}

std::unique_ptr<SkAndroidCodec> SkAndroidCodec::MakeFromData(sk_sp<const SkData> data,
                                                             SkPngChunkReader* chunkReader) {
    if (!data) {
        return nullptr;
    }

    return MakeFromStream(SkMemoryStream::Make(std::move(data)), chunkReader);
}

SkColorType SkAndroidCodec::computeOutputColorType(SkColorType requestedColorType) {
    bool highPrecision = fCodec->getEncodedInfo().bitsPerComponent() > 8;
    uint8_t colorDepth = fCodec->getEncodedInfo().getColorDepth();
    switch (requestedColorType) {
        case kARGB_4444_SkColorType:
            return kN32_SkColorType;
        case kN32_SkColorType:
            break;
        case kAlpha_8_SkColorType:
            // Fall through to kGray_8.  Before kGray_8_SkColorType existed,
            // we allowed clients to request kAlpha_8 when they wanted a
            // grayscale decode.
        case kGray_8_SkColorType:
            if (kGray_8_SkColorType == this->getInfo().colorType()) {
                return kGray_8_SkColorType;
            }
            break;
        case kRGB_565_SkColorType:
            if (kOpaque_SkAlphaType == this->getInfo().alphaType()) {
                return kRGB_565_SkColorType;
            }
            break;
        case kRGBA_1010102_SkColorType:
            if (colorDepth == 10) {
              return kRGBA_1010102_SkColorType;
            }
            break;
        case kRGBA_F16_SkColorType:
            return kRGBA_F16_SkColorType;
        default:
            break;
    }

    // F16 is the Android default for high precision images.
    return highPrecision ? kRGBA_F16_SkColorType :
        (colorDepth == 10 ? kRGBA_1010102_SkColorType : kN32_SkColorType);
}

SkAlphaType SkAndroidCodec::computeOutputAlphaType(bool requestedUnpremul) {
    if (kOpaque_SkAlphaType == this->getInfo().alphaType()) {
        return kOpaque_SkAlphaType;
    }
    return requestedUnpremul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType;
}

sk_sp<SkColorSpace> SkAndroidCodec::computeOutputColorSpace(SkColorType outputColorType,
                                                            sk_sp<SkColorSpace> prefColorSpace) {
    switch (outputColorType) {
        case kRGBA_F16_SkColorType:
        case kRGB_565_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_1010102_SkColorType: {
            // If |prefColorSpace| is supplied, choose it.
            if (prefColorSpace) {
                return prefColorSpace;
            }
            if (const auto* colorProfile = fCodec->getEncodedInfo().colorProfile()) {
                return colorProfile->getAndroidOutputColorSpace();
            }
            return SkColorSpace::MakeSRGB();
        }
        default:
            // Color correction not supported for kGray.
            return nullptr;
    }
}

static bool is_webp(const SkCodec* codec) {
    return codec->getEncodedFormat() == SkEncodedImageFormat::kWEBP;
}

// There are a variety of ways two SkISizes could be compared. This method
// returns true if either dimensions of a is < that of b.
// computeSampleSize also uses the opposite, which means that both
// dimensions of a >= b.
static inline bool smaller_than(const SkISize& a, const SkISize& b) {
    return a.width() < b.width() || a.height() < b.height();
}

// Both dimensions of a > that of b.
static inline bool strictly_bigger_than(const SkISize& a, const SkISize& b) {
    return a.width() > b.width() && a.height() > b.height();
}

int SkAndroidCodec::computeSampleSize(SkISize* desiredSize) const {
    SkASSERT(desiredSize);

    const auto origDims = fCodec->dimensions();
    if (!desiredSize || *desiredSize == origDims) {
        return 1;
    }

    if (smaller_than(origDims, *desiredSize)) {
        *desiredSize = origDims;
        return 1;
    }

    // Handle bad input:
    if (desiredSize->width() < 1 || desiredSize->height() < 1) {
        *desiredSize = SkISize::Make(std::max(1, desiredSize->width()),
                                     std::max(1, desiredSize->height()));
    }

    if (is_webp(fCodec.get())) {
        if (fCodec->getFrameCount() > 1) {
           // Cannot downscale animated webp
           *desiredSize = origDims;
        }
        return 1;
    }

    int sampleX = origDims.width()  / desiredSize->width();
    int sampleY = origDims.height() / desiredSize->height();
    int sampleSize = std::min(sampleX, sampleY);
    auto computedSize = this->getSampledDimensions(sampleSize);
    if (computedSize == *desiredSize) {
        return sampleSize;
    }

    if (computedSize == origDims || sampleSize == 1) {
        // Cannot downscale
        *desiredSize = computedSize;
        return 1;
    }

    if (strictly_bigger_than(computedSize, *desiredSize)) {
        // See if there is a tighter fit.
        while (true) {
            auto smaller = this->getSampledDimensions(sampleSize + 1);
            if (smaller == *desiredSize) {
                return sampleSize + 1;
            }
            if (smaller == computedSize || smaller_than(smaller, *desiredSize)) {
                // Cannot get any smaller without being smaller than desired.
                *desiredSize = computedSize;
                return sampleSize;
            }

            sampleSize++;
            computedSize = smaller;
        }

        SkASSERT(false);
    }

    if (!smaller_than(computedSize, *desiredSize)) {
        // This means one of the computed dimensions is equal to desired, and
        // the other is bigger. This is as close as we can get.
        *desiredSize = computedSize;
        return sampleSize;
    }

    // computedSize is too small. Make it larger.
    while (sampleSize > 2) {
        auto bigger = this->getSampledDimensions(sampleSize - 1);
        if (bigger == *desiredSize || !smaller_than(bigger, *desiredSize)) {
            *desiredSize = bigger;
            return sampleSize - 1;
        }
        sampleSize--;
    }

    *desiredSize = origDims;
    return 1;
}

SkISize SkAndroidCodec::getSampledDimensions(int sampleSize) const {
    if (!is_valid_sample_size(sampleSize)) {
        return {0, 0};
    }

    // Fast path for when we are not scaling.
    if (1 == sampleSize) {
        return fCodec->dimensions();
    }

    return this->onGetSampledDimensions(sampleSize);
}

bool SkAndroidCodec::getSupportedSubset(SkIRect* desiredSubset) const {
    if (!desiredSubset || !SkCodecPriv::IsValidSubset(*desiredSubset, fCodec->dimensions())) {
        return false;
    }

    return this->onGetSupportedSubset(desiredSubset);
}

SkISize SkAndroidCodec::getSampledSubsetDimensions(int sampleSize, const SkIRect& subset) const {
    if (!is_valid_sample_size(sampleSize)) {
        return {0, 0};
    }

    // We require that the input subset is a subset that is supported by SkAndroidCodec.
    // We test this by calling getSupportedSubset() and verifying that no modifications
    // are made to the subset.
    SkIRect copySubset = subset;
    if (!this->getSupportedSubset(&copySubset) || copySubset != subset) {
        return {0, 0};
    }

    // If the subset is the entire image, for consistency, use getSampledDimensions().
    if (fCodec->dimensions() == subset.size()) {
        return this->getSampledDimensions(sampleSize);
    }

    // This should perhaps call a virtual function, but currently both of our subclasses
    // want the same implementation.
    return {SkCodecPriv::GetSampledDimension(subset.width(), sampleSize),
            SkCodecPriv::GetSampledDimension(subset.height(), sampleSize)};
}

SkCodec::Result SkAndroidCodec::getAndroidPixels(const SkImageInfo& requestInfo,
        void* requestPixels, size_t requestRowBytes, const AndroidOptions* options) {
    if (!requestPixels) {
        return SkCodec::kInvalidParameters;
    }
    if (requestRowBytes < requestInfo.minRowBytes()) {
        return SkCodec::kInvalidParameters;
    }

    AndroidOptions defaultOptions;
    if (!options) {
        options = &defaultOptions;
    } else {
        if (options->fSubset) {
            if (!SkCodecPriv::IsValidSubset(*options->fSubset, fCodec->dimensions())) {
                return SkCodec::kInvalidParameters;
            }

            if (SkIRect::MakeSize(fCodec->dimensions()) == *options->fSubset) {
                // The caller wants the whole thing, rather than a subset. Modify
                // the AndroidOptions passed to onGetAndroidPixels to not specify
                // a subset.
                defaultOptions = *options;
                defaultOptions.fSubset = nullptr;
                options = &defaultOptions;
            }
        }
    }

    // We may need to have handleFrameIndex recursively call this method
    // to resolve one frame depending on another. The recursion stops
    // when we find a frame which does not require an earlier frame
    // e.g. frame->getRequiredFrame() returns kNoFrame
    auto getPixelsFn = [&](const SkImageInfo& info, void* pixels, size_t rowBytes,
                           const SkCodec::Options& opts, int requiredFrame
                           ) -> SkCodec::Result {
        SkAndroidCodec::AndroidOptions prevFrameOptions(
                        reinterpret_cast<const SkAndroidCodec::AndroidOptions&>(opts));
        prevFrameOptions.fFrameIndex = requiredFrame;
        return this->getAndroidPixels(info, pixels, rowBytes, &prevFrameOptions);
    };
    if (auto result = fCodec->handleFrameIndex(requestInfo, requestPixels, requestRowBytes,
            *options, getPixelsFn); result != SkCodec::kSuccess) {
        return result;
    }

    return this->onGetAndroidPixels(requestInfo, requestPixels, requestRowBytes, *options);
}

SkCodec::Result SkAndroidCodec::getAndroidPixels(const SkImageInfo& info, void* pixels,
        size_t rowBytes) {
    return this->getAndroidPixels(info, pixels, rowBytes, nullptr);
}

bool SkAndroidCodec::getGainmapAndroidCodec(SkGainmapInfo* info,
                                            std::unique_ptr<SkAndroidCodec>* outCodec) {
    if (outCodec) {
        std::unique_ptr<SkCodec> gainmapCodec;
        if (!fCodec->onGetGainmapCodec(info, &gainmapCodec)) {
            return false;
        }
        *outCodec = MakeFromCodec(std::move(gainmapCodec));
        return true;
    }
    return fCodec->onGetGainmapCodec(info, nullptr);
}

bool SkAndroidCodec::getAndroidGainmap(SkGainmapInfo* info,
                                       std::unique_ptr<SkStream>* outGainmapImageStream) {
    return fCodec->onGetGainmapInfo(info, outGainmapImageStream);
}

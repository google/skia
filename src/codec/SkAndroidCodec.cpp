/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/base/SkFloatingPoint.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkSampledCodec.h"

#if defined(SK_CODEC_DECODES_WEBP) || defined(SK_CODEC_DECODES_RAW) || \
        defined(SK_HAS_WUFFS_LIBRARY) || defined(SK_CODEC_DECODES_AVIF)
#include "src/codec/SkAndroidCodecAdapter.h"
#endif

#include <algorithm>
#include <cstdint>
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

    switch ((SkEncodedImageFormat)codec->getEncodedFormat()) {
        case SkEncodedImageFormat::kPNG:
        case SkEncodedImageFormat::kICO:
        case SkEncodedImageFormat::kJPEG:
#ifndef SK_HAS_WUFFS_LIBRARY
        case SkEncodedImageFormat::kGIF:
#endif
        case SkEncodedImageFormat::kBMP:
        case SkEncodedImageFormat::kWBMP:
        case SkEncodedImageFormat::kHEIF:
#ifndef SK_CODEC_DECODES_AVIF
        case SkEncodedImageFormat::kAVIF:
#endif
            return std::make_unique<SkSampledCodec>(codec.release());
#ifdef SK_HAS_WUFFS_LIBRARY
        case SkEncodedImageFormat::kGIF:
#endif
#ifdef SK_CODEC_DECODES_WEBP
        case SkEncodedImageFormat::kWEBP:
#endif
#ifdef SK_CODEC_DECODES_RAW
        case SkEncodedImageFormat::kDNG:
#endif
#ifdef SK_CODEC_DECODES_AVIF
        case SkEncodedImageFormat::kAVIF:
#endif
#if defined(SK_CODEC_DECODES_WEBP) || defined(SK_CODEC_DECODES_RAW) || \
        defined(SK_HAS_WUFFS_LIBRARY) || defined(SK_CODEC_DECODES_AVIF)
            return std::make_unique<SkAndroidCodecAdapter>(codec.release());
#endif

        default:
            return nullptr;
    }
}

std::unique_ptr<SkAndroidCodec> SkAndroidCodec::MakeFromData(sk_sp<SkData> data,
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

            const skcms_ICCProfile* encodedProfile = fCodec->getEncodedInfo().profile();
            if (encodedProfile) {
                if (auto encodedSpace = SkColorSpace::Make(*encodedProfile)) {
                    // Leave the pixels in the encoded color space.  Color space conversion
                    // will be handled after decode time.
                    return encodedSpace;
                }

                if (encodedProfile->has_toXYZD50) {
                    return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                 encodedProfile->toXYZD50);
                }
            }

            return SkColorSpace::MakeSRGB();
        }
        default:
            // Color correction not supported for kGray.
            return nullptr;
    }
}

static bool supports_any_down_scale(const SkCodec* codec) {
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

    if (supports_any_down_scale(fCodec.get())) {
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
    if (!desiredSubset || !is_valid_subset(*desiredSubset, fCodec->dimensions())) {
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
    return {get_scaled_dimension(subset.width(), sampleSize),
            get_scaled_dimension(subset.height(), sampleSize)};
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
            if (!is_valid_subset(*options->fSubset, fCodec->dimensions())) {
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

    if (auto result = fCodec->handleFrameIndex(requestInfo, requestPixels, requestRowBytes,
            *options, this); result != SkCodec::kSuccess) {
        return result;
    }

    return this->onGetAndroidPixels(requestInfo, requestPixels, requestRowBytes, *options);
}

SkCodec::Result SkAndroidCodec::getAndroidPixels(const SkImageInfo& info, void* pixels,
        size_t rowBytes) {
    return this->getAndroidPixels(info, pixels, rowBytes, nullptr);
}

bool SkAndroidCodec::getAndroidGainmap(SkGainmapInfo* info,
                                       std::unique_ptr<SkStream>* outGainmapImageStream) {
    if (!fCodec->onGetGainmapInfo(info, outGainmapImageStream)) {
        return false;
    }
    // Convert old parameter names to new parameter names.
    // TODO(ccameron): Remove these parameters.
    info->fLogRatioMin.fR = sk_float_log(info->fGainmapRatioMin.fR);
    info->fLogRatioMin.fG = sk_float_log(info->fGainmapRatioMin.fG);
    info->fLogRatioMin.fB = sk_float_log(info->fGainmapRatioMin.fB);
    info->fLogRatioMax.fR = sk_float_log(info->fGainmapRatioMax.fR);
    info->fLogRatioMax.fG = sk_float_log(info->fGainmapRatioMax.fG);
    info->fLogRatioMax.fB = sk_float_log(info->fGainmapRatioMax.fB);
    info->fHdrRatioMin = info->fDisplayRatioSdr;
    info->fHdrRatioMax = info->fDisplayRatioHdr;
    return true;
}

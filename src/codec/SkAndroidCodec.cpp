/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAndroidCodec.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkRawAdapterCodec.h"
#include "SkSampledCodec.h"
#include "SkWebpAdapterCodec.h"

static bool is_valid_sample_size(int sampleSize) {
    // FIXME: As Leon has mentioned elsewhere, surely there is also a maximum sampleSize?
    return sampleSize > 0;
}

SkAndroidCodec::SkAndroidCodec(SkCodec* codec)
    : fInfo(codec->getInfo())
    , fCodec(codec)
{}

SkAndroidCodec* SkAndroidCodec::NewFromStream(SkStream* stream, SkPngChunkReader* chunkReader) {
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream, chunkReader));
    if (nullptr == codec) {
        return nullptr;
    }

    switch (codec->getEncodedFormat()) {
#ifdef SK_CODEC_DECODES_PNG
        case kPNG_SkEncodedFormat:
        case kICO_SkEncodedFormat:
#endif
#ifdef SK_CODEC_DECODES_JPEG
        case kJPEG_SkEncodedFormat:
#endif
#ifdef SK_CODEC_DECODES_GIF
        case kGIF_SkEncodedFormat:
#endif
        case kBMP_SkEncodedFormat:
        case kWBMP_SkEncodedFormat:
            return new SkSampledCodec(codec.release());
#ifdef SK_CODEC_DECODES_WEBP
        case kWEBP_SkEncodedFormat:
            return new SkWebpAdapterCodec((SkWebpCodec*) codec.release());
#endif
#ifdef SK_CODEC_DECODES_RAW
        case kDNG_SkEncodedFormat:
            return new SkRawAdapterCodec((SkRawCodec*)codec.release());
#endif
        default:
            return nullptr;
    }
}

SkAndroidCodec* SkAndroidCodec::NewFromData(SkData* data, SkPngChunkReader* chunkReader) {
    if (!data) {
        return nullptr;
    }

    return NewFromStream(new SkMemoryStream(data), chunkReader);
}

SkColorType SkAndroidCodec::computeOutputColorType(SkColorType requestedColorType) {
    // The legacy GIF and WBMP decoders always decode to kIndex_8_SkColorType.
    // We will maintain this behavior.
    SkEncodedFormat format = this->getEncodedFormat();
    if (kGIF_SkEncodedFormat == format || kWBMP_SkEncodedFormat == format) {
        return kIndex_8_SkColorType;
    }

    SkColorType suggestedColorType = this->getInfo().colorType();
    switch (requestedColorType) {
        case kARGB_4444_SkColorType:
        case kN32_SkColorType:
            return kN32_SkColorType;
        case kIndex_8_SkColorType:
            if (kIndex_8_SkColorType == suggestedColorType) {
                return kIndex_8_SkColorType;
            }
            break;
        case kAlpha_8_SkColorType:
            // Fall through to kGray_8.  Before kGray_8_SkColorType existed,
            // we allowed clients to request kAlpha_8 when they wanted a
            // grayscale decode.
        case kGray_8_SkColorType:
            if (kGray_8_SkColorType == suggestedColorType) {
                return kGray_8_SkColorType;
            }
            break;
        case kRGB_565_SkColorType:
            if (kOpaque_SkAlphaType == this->getInfo().alphaType()) {
                return kRGB_565_SkColorType;
            }
            break;
        default:
            break;
    }

    // Android has limited support for kGray_8 (using kAlpha_8).  We will not
    // use kGray_8 for Android unless they specifically ask for it.
    if (kGray_8_SkColorType == suggestedColorType) {
        return kN32_SkColorType;
    }

    // This may be kN32_SkColorType or kIndex_8_SkColorType.
    return suggestedColorType;
}

SkAlphaType SkAndroidCodec::computeOutputAlphaType(bool requestedUnpremul) {
    if (kOpaque_SkAlphaType == this->getInfo().alphaType()) {
        return kOpaque_SkAlphaType;
    }
    return requestedUnpremul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType;
}

SkISize SkAndroidCodec::getSampledDimensions(int sampleSize) const {
    if (!is_valid_sample_size(sampleSize)) {
        return SkISize::Make(0, 0);
    }

    // Fast path for when we are not scaling.
    if (1 == sampleSize) {
        return fInfo.dimensions();
    }

    return this->onGetSampledDimensions(sampleSize);
}

bool SkAndroidCodec::getSupportedSubset(SkIRect* desiredSubset) const {
    if (!desiredSubset || !is_valid_subset(*desiredSubset, fInfo.dimensions())) {
        return false;
    }

    return this->onGetSupportedSubset(desiredSubset);
}

SkISize SkAndroidCodec::getSampledSubsetDimensions(int sampleSize, const SkIRect& subset) const {
    if (!is_valid_sample_size(sampleSize)) {
        return SkISize::Make(0, 0);
    }

    // We require that the input subset is a subset that is supported by SkAndroidCodec.
    // We test this by calling getSupportedSubset() and verifying that no modifications
    // are made to the subset.
    SkIRect copySubset = subset;
    if (!this->getSupportedSubset(&copySubset) || copySubset != subset) {
        return SkISize::Make(0, 0);
    }

    // If the subset is the entire image, for consistency, use getSampledDimensions().
    if (fInfo.dimensions() == subset.size()) {
        return this->getSampledDimensions(sampleSize);
    }

    // This should perhaps call a virtual function, but currently both of our subclasses
    // want the same implementation.
    return SkISize::Make(get_scaled_dimension(subset.width(), sampleSize),
                get_scaled_dimension(subset.height(), sampleSize));
}

SkCodec::Result SkAndroidCodec::getAndroidPixels(const SkImageInfo& info, void* pixels,
        size_t rowBytes, const AndroidOptions* options) {
    if (!pixels) {
        return SkCodec::kInvalidParameters;
    }
    if (rowBytes < info.minRowBytes()) {
        return SkCodec::kInvalidParameters;
    }

    AndroidOptions defaultOptions;
    if (!options) {
        options = &defaultOptions;
    } else if (options->fSubset) {
        if (!is_valid_subset(*options->fSubset, fInfo.dimensions())) {
            return SkCodec::kInvalidParameters;
        }

        if (SkIRect::MakeSize(fInfo.dimensions()) == *options->fSubset) {
            // The caller wants the whole thing, rather than a subset. Modify
            // the AndroidOptions passed to onGetAndroidPixels to not specify
            // a subset.
            defaultOptions = *options;
            defaultOptions.fSubset = nullptr;
            options = &defaultOptions;
        }
    }

    return this->onGetAndroidPixels(info, pixels, rowBytes, *options);
}

SkCodec::Result SkAndroidCodec::getAndroidPixels(const SkImageInfo& info, void* pixels,
        size_t rowBytes) {
    return this->getAndroidPixels(info, pixels, rowBytes, nullptr);
}

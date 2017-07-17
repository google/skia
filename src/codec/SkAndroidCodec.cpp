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

/**
 *  Loads the gamut as a set of three points (triangle).
 */
static void load_gamut(SkPoint rgb[], const SkMatrix44& xyz) {
    // rx = rX / (rX + rY + rZ)
    // ry = rY / (rX + rY + rZ)
    // gx, gy, bx, and gy are calulcated similarly.
    float rSum = xyz.get(0, 0) + xyz.get(1, 0) + xyz.get(2, 0);
    float gSum = xyz.get(0, 1) + xyz.get(1, 1) + xyz.get(2, 1);
    float bSum = xyz.get(0, 2) + xyz.get(1, 2) + xyz.get(2, 2);
    rgb[0].fX = xyz.get(0, 0) / rSum;
    rgb[0].fY = xyz.get(1, 0) / rSum;
    rgb[1].fX = xyz.get(0, 1) / gSum;
    rgb[1].fY = xyz.get(1, 1) / gSum;
    rgb[2].fX = xyz.get(0, 2) / bSum;
    rgb[2].fY = xyz.get(1, 2) / bSum;
}

/**
 *  Calculates the area of the triangular gamut.
 */
static float calculate_area(SkPoint abc[]) {
    SkPoint a = abc[0];
    SkPoint b = abc[1];
    SkPoint c = abc[2];
    return 0.5f * SkTAbs(a.fX*b.fY + b.fX*c.fY - a.fX*c.fY - c.fX*b.fY - b.fX*a.fY);
}

static const float kSRGB_D50_GamutArea = 0.084f;

static bool is_wide_gamut(const SkColorSpace* colorSpace) {
    // Determine if the source image has a gamut that is wider than sRGB.  If so, we
    // will use P3 as the output color space to avoid clipping the gamut.
    const SkMatrix44* toXYZD50 = as_CSB(colorSpace)->toXYZD50();
    if (toXYZD50) {
        SkPoint rgb[3];
        load_gamut(rgb, *toXYZD50);
        return calculate_area(rgb) > kSRGB_D50_GamutArea;
    }

    return false;
}

SkAndroidCodec::SkAndroidCodec(SkCodec* codec)
    : fInfo(codec->getInfo())
    , fCodec(codec)
{}

SkAndroidCodec* SkAndroidCodec::NewFromStream(SkStream* stream, SkPngChunkReader* chunkReader) {
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromStream(stream, nullptr, chunkReader));
    if (nullptr == codec) {
        return nullptr;
    }

    switch ((SkEncodedImageFormat)codec->getEncodedFormat()) {
#ifdef SK_HAS_PNG_LIBRARY
        case SkEncodedImageFormat::kPNG:
        case SkEncodedImageFormat::kICO:
#endif
#ifdef SK_HAS_JPEG_LIBRARY
        case SkEncodedImageFormat::kJPEG:
#endif
        case SkEncodedImageFormat::kGIF:
        case SkEncodedImageFormat::kBMP:
        case SkEncodedImageFormat::kWBMP:
            return new SkSampledCodec(codec.release());
#ifdef SK_HAS_WEBP_LIBRARY
        case SkEncodedImageFormat::kWEBP:
            return new SkWebpAdapterCodec((SkWebpCodec*) codec.release());
#endif
#ifdef SK_CODEC_DECODES_RAW
        case SkEncodedImageFormat::kDNG:
            return new SkRawAdapterCodec((SkRawCodec*)codec.release());
#endif
        default:
            return nullptr;
    }
}

SkAndroidCodec* SkAndroidCodec::NewFromData(sk_sp<SkData> data, SkPngChunkReader* chunkReader) {
    if (!data) {
        return nullptr;
    }

    return NewFromStream(new SkMemoryStream(data), chunkReader);
}

SkColorType SkAndroidCodec::computeOutputColorType(SkColorType requestedColorType) {
    bool highPrecision = fCodec->getEncodedInfo().bitsPerComponent() > 8;
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
        case kRGBA_F16_SkColorType:
            return kRGBA_F16_SkColorType;
        default:
            break;
    }

    // F16 is the Android default for high precision images.
    return highPrecision ? kRGBA_F16_SkColorType : kN32_SkColorType;
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
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType: {
            // If |prefColorSpace| is supported, choose it.
            SkColorSpaceTransferFn fn;
            if (prefColorSpace && prefColorSpace->isNumericalTransferFn(&fn)) {
                return prefColorSpace;
            }

            SkColorSpace* encodedSpace = fCodec->getInfo().colorSpace();
            if (encodedSpace->isNumericalTransferFn(&fn)) {
                // Leave the pixels in the encoded color space.  Color space conversion
                // will be handled after decode time.
                return sk_ref_sp(encodedSpace);
            }

            if (is_wide_gamut(encodedSpace)) {
                return SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                             SkColorSpace::kDCIP3_D65_Gamut);
            }

            return SkColorSpace::MakeSRGB();
        }
        case kRGBA_F16_SkColorType:
            // Note that |prefColorSpace| is ignored, F16 is always linear sRGB.
            return SkColorSpace::MakeSRGBLinear();
        case kRGB_565_SkColorType:
            // Note that |prefColorSpace| is ignored, 565 is always sRGB.
            return SkColorSpace::MakeSRGB();
        default:
            // Color correction not supported for kGray.
            return nullptr;
    }
}

SkISize SkAndroidCodec::getSampledDimensions(int sampleSize) const {
    if (!is_valid_sample_size(sampleSize)) {
        return {0, 0};
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
    if (fInfo.dimensions() == subset.size()) {
        return this->getSampledDimensions(sampleSize);
    }

    // This should perhaps call a virtual function, but currently both of our subclasses
    // want the same implementation.
    return {get_scaled_dimension(subset.width(), sampleSize),
            get_scaled_dimension(subset.height(), sampleSize)};
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

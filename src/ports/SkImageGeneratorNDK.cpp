/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/ports/SkImageGeneratorNDK.h"
#include "src/ports/SkNDKConversions.h"

#include <android/bitmap.h>
#include <android/data_space.h>
#include <android/imagedecoder.h>

namespace {
class ImageGeneratorNDK : public SkImageGenerator {
public:
    ImageGeneratorNDK(const SkImageInfo&, sk_sp<SkData>, AImageDecoder*);
    ~ImageGeneratorNDK() override;

protected:
    sk_sp<SkData> onRefEncodedData() override;

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     const Options& opts) override;

private:
    sk_sp<SkData>  fData;
    AImageDecoder* fDecoder;
    // Setting the ADataSpace is sticky - it is set for all future decodes
    // until it is set again. But as of R there is no way to reset it to
    // ADATASPACE_UNKNOWN to skip color correction. If the client requests
    // skipping correction after having set it to something else, we need
    // to recreate the AImageDecoder.
    bool           fPreviouslySetADataSpace;

    using INHERITED = SkImageGenerator;
};

} // anonymous namespace

static bool ok(int result) {
    return result == ANDROID_IMAGE_DECODER_SUCCESS;
}

static bool set_android_bitmap_format(AImageDecoder* decoder, SkColorType colorType) {
    auto format = SkNDKConversions::toAndroidBitmapFormat(colorType);
    return ok(AImageDecoder_setAndroidBitmapFormat(decoder, format));
}

static SkColorType colorType(AImageDecoder* decoder, const AImageDecoderHeaderInfo* headerInfo) {
    // AImageDecoder never defaults to gray, but allows setting it if the image is 8 bit gray.
    if (set_android_bitmap_format(decoder, kGray_8_SkColorType)) {
        return kGray_8_SkColorType;
    }

    auto format = static_cast<AndroidBitmapFormat>(
            AImageDecoderHeaderInfo_getAndroidBitmapFormat(headerInfo));
    return SkNDKConversions::toColorType(format);
}

static sk_sp<SkColorSpace> get_default_colorSpace(const AImageDecoderHeaderInfo* headerInfo) {
    auto dataSpace = static_cast<ADataSpace>(AImageDecoderHeaderInfo_getDataSpace(headerInfo));
    if (auto cs = SkNDKConversions::toColorSpace(dataSpace)) {
        return cs;
    }

    return SkColorSpace::MakeSRGB();
}

std::unique_ptr<SkImageGenerator> SkImageGeneratorNDK::MakeFromEncodedNDK(sk_sp<SkData> data) {
    if (!data) return nullptr;

    AImageDecoder* rawDecoder;
    if (!ok(AImageDecoder_createFromBuffer(data->data(), data->size(), &rawDecoder))) {
        return nullptr;
    }

    const AImageDecoderHeaderInfo* headerInfo = AImageDecoder_getHeaderInfo(rawDecoder);
    int32_t width  = AImageDecoderHeaderInfo_getWidth(headerInfo);
    int32_t height = AImageDecoderHeaderInfo_getHeight(headerInfo);
    SkColorType ct = colorType(rawDecoder, headerInfo);

    // Although the encoded data stores unpremultiplied pixels, AImageDecoder defaults to premul
    // (if the image may have alpha).
    SkAlphaType at = AImageDecoderHeaderInfo_getAlphaFlags(headerInfo)
            == ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    auto imageInfo = SkImageInfo::Make(width, height, ct, at, get_default_colorSpace(headerInfo));
    return std::unique_ptr<SkImageGenerator>(
            new ImageGeneratorNDK(imageInfo, std::move(data), rawDecoder));
}

ImageGeneratorNDK::ImageGeneratorNDK(const SkImageInfo& info, sk_sp<SkData> data,
                                     AImageDecoder* decoder)
    : INHERITED(info)
    , fData(std::move(data))
    , fDecoder(decoder)
    , fPreviouslySetADataSpace(false)
{
    SkASSERT(fDecoder);
}

ImageGeneratorNDK::~ImageGeneratorNDK() {
    AImageDecoder_delete(fDecoder);
}

static bool set_target_size(AImageDecoder* decoder, const SkISize& size, const SkISize targetSize) {
    if (size != targetSize) {
        // AImageDecoder will scale to arbitrary sizes. Only support a size if it's supported by the
        // underlying library.
        const AImageDecoderHeaderInfo* headerInfo = AImageDecoder_getHeaderInfo(decoder);
        const char* mimeType = AImageDecoderHeaderInfo_getMimeType(headerInfo);
        if (0 == strcmp(mimeType, "image/jpeg")) {
            bool supported = false;
            for (int sampleSize : { 2, 4, 8 }) {
                int32_t width;
                int32_t height;
                if (ok(AImageDecoder_computeSampledSize(decoder, sampleSize, &width, &height))
                        && targetSize == SkISize::Make(width, height)) {
                    supported = true;
                    break;
                }
            }
            if (!supported) return false;
        } else if (0 == strcmp(mimeType, "image/webp")) {
            // libwebp supports arbitrary downscaling.
            if (targetSize.width() > size.width() || targetSize.height() > size.height()) {
                return false;
            }
        } else {
            return false;
        }
    }
    return ok(AImageDecoder_setTargetSize(decoder, targetSize.width(), targetSize.height()));
}

bool ImageGeneratorNDK::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                    const Options& opts) {
    if (auto* cs = info.colorSpace()) {
        if (!ok(AImageDecoder_setDataSpace(fDecoder, SkNDKConversions::toDataSpace(cs)))) {
            return false;
        }
        fPreviouslySetADataSpace = true;
    } else {
        // If the requested SkColorSpace is null, the client wants the "raw" colors, without color
        // space transformations applied. (This is primarily useful for a client that wants to do
        // their own color transformations.) This is AImageDecoder's default, but if a previous call
        // set an ADataSpace, AImageDecoder is no longer using its default, so we need to set it
        // back.
        if (fPreviouslySetADataSpace) {
            // AImageDecoderHeaderInfo_getDataSpace always returns the same value for the same
            // image, regardless of prior calls to AImageDecoder_setDataSpace. Check if it's
            // ADATASPACE_UNKNOWN, which needs to be handled specially.
            const AImageDecoderHeaderInfo* headerInfo = AImageDecoder_getHeaderInfo(fDecoder);
            const auto defaultDataSpace = AImageDecoderHeaderInfo_getDataSpace(headerInfo);
            if (defaultDataSpace == ADATASPACE_UNKNOWN) {
                // As of R, there's no way to reset AImageDecoder to ADATASPACE_UNKNOWN, so
                // create a new one.
                AImageDecoder* decoder;
                if (!ok(AImageDecoder_createFromBuffer(fData->data(), fData->size(), &decoder))) {
                    return false;
                }
                AImageDecoder_delete(fDecoder);
                fDecoder = decoder;
            } else {
                if (!ok(AImageDecoder_setDataSpace(fDecoder, defaultDataSpace))) {
                    return false;
                }
            }

            // Whether by recreating AImageDecoder or calling AImageDecoder_setDataSpace, the
            // AImageDecoder is back to its default, so if the next call has a null SkColorSpace, it
            // does not need to reset it again.
            fPreviouslySetADataSpace = false;
        }
    }

    if (!set_android_bitmap_format(fDecoder, info.colorType())) {
        return false;
    }

    switch (info.alphaType()) {
        case kUnknown_SkAlphaType:
            return false;
        case kOpaque_SkAlphaType:
            if (this->getInfo().alphaType() != kOpaque_SkAlphaType) {
                return false;
            }
            break;
        case kUnpremul_SkAlphaType:
            if (!ok(AImageDecoder_setUnpremultipliedRequired(fDecoder, true))) {
                return false;
            }
            break;
        case kPremul_SkAlphaType:
            break;
    }

    if (!set_target_size(fDecoder, getInfo().dimensions(), info.dimensions())) {
        return false;
    }

    auto byteSize = info.computeByteSize(rowBytes);
    switch (AImageDecoder_decodeImage(fDecoder, pixels, rowBytes, byteSize)) {
        case ANDROID_IMAGE_DECODER_INCOMPLETE:
            // The image was partially decoded, but the input was truncated. The client may be
            // happy with the partial image.
        case ANDROID_IMAGE_DECODER_ERROR:
            // Similarly, the image was partially decoded, but the input had an error. The client
            // may be happy with the partial image.
        case ANDROID_IMAGE_DECODER_SUCCESS:
            return true;
        default:
            return false;
    }
}

sk_sp<SkData> ImageGeneratorNDK::onRefEncodedData() {
    return fData;
}

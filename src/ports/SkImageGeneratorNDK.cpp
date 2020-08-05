/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/ports/SkImageGeneratorNDK.h"

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

    typedef SkImageGenerator INHERITED;
};

} // anonymous namespace

static bool ok(int result) {
    return result == ANDROID_IMAGE_DECODER_SUCCESS;
}

namespace {
static const struct {
    SkColorType         colorType;
    AndroidBitmapFormat format;
} gColorTypeTable[] = {
    { kRGBA_8888_SkColorType, ANDROID_BITMAP_FORMAT_RGBA_8888 },
    { kRGBA_F16_SkColorType,  ANDROID_BITMAP_FORMAT_RGBA_F16 },
    { kRGB_565_SkColorType,   ANDROID_BITMAP_FORMAT_RGB_565 },
    // Android allows using its alpha 8 format to get 8 bit gray pixels.
    { kGray_8_SkColorType,    ANDROID_BITMAP_FORMAT_A_8 },
};

} // anonymous namespace

static bool set_android_bitmap_format(AImageDecoder* decoder, SkColorType colorType) {
    for (const auto& entry : gColorTypeTable) {
        if (entry.colorType == colorType) {
            return ok(AImageDecoder_setAndroidBitmapFormat(decoder, entry.format));
        }
    }
    return false;
}

static SkColorType colorType(AImageDecoder* decoder, const AImageDecoderHeaderInfo* headerInfo) {
    // AImageDecoder never defaults to gray, but allows setting it if the image is 8 bit gray.
    if (set_android_bitmap_format(decoder, kGray_8_SkColorType)) {
        return kGray_8_SkColorType;
    }

    const auto format = AImageDecoderHeaderInfo_getAndroidBitmapFormat(headerInfo);
    for (const auto& entry : gColorTypeTable) {
        if (format == entry.format) {
            return entry.colorType;
        }
    }

    SkUNREACHABLE;
}

static constexpr skcms_TransferFunction k2Dot6 = {2.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

static constexpr skcms_Matrix3x3 kDCIP3 = {{
        {0.486143, 0.323835, 0.154234},
        {0.226676, 0.710327, 0.0629966},
        {0.000800549, 0.0432385, 0.78275},
}};

namespace {
static const struct {
    ADataSpace             dataSpace;
    skcms_TransferFunction transferFunction;
    skcms_Matrix3x3        gamut;
} gColorSpaceTable[] = {
    // Note: ADATASPACE_SCRGB would look the same as ADATASPACE_SRGB. Leaving it out of the table is
    // fine, since users of the table will will still use SRGB.
    { ADATASPACE_SRGB,         SkNamedTransferFn::kSRGB,    SkNamedGamut::kSRGB },
    { ADATASPACE_SCRGB_LINEAR, SkNamedTransferFn::kLinear,  SkNamedGamut::kSRGB },
    { ADATASPACE_SRGB_LINEAR,  SkNamedTransferFn::kLinear,  SkNamedGamut::kSRGB },
    { ADATASPACE_ADOBE_RGB,    SkNamedTransferFn::k2Dot2,   SkNamedGamut::kAdobeRGB },
    { ADATASPACE_DISPLAY_P3,   SkNamedTransferFn::kSRGB,    SkNamedGamut::kDisplayP3 },
    { ADATASPACE_BT2020,       SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020 },
    { ADATASPACE_BT709,        SkNamedTransferFn::kRec2020, SkNamedGamut::kSRGB },
    { ADATASPACE_DCI_P3,       k2Dot6,                      kDCIP3 },
};
} // anonymous namespace

static sk_sp<SkColorSpace> get_default_colorSpace(const AImageDecoderHeaderInfo* headerInfo) {
    auto dataSpace = AImageDecoderHeaderInfo_getDataSpace(headerInfo);
    for (const auto& entry : gColorSpaceTable) {
        if (entry.dataSpace == dataSpace) {
            return SkColorSpace::MakeRGB(entry.transferFunction, entry.gamut);
        }
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

static bool nearly_equal(float a, float b) {
    return fabs(a - b) < .002f;
}

static bool nearly_equal(const skcms_TransferFunction& x, const skcms_TransferFunction& y) {
    return nearly_equal(x.g, y.g)
        && nearly_equal(x.a, y.a)
        && nearly_equal(x.b, y.b)
        && nearly_equal(x.c, y.c)
        && nearly_equal(x.d, y.d)
        && nearly_equal(x.e, y.e)
        && nearly_equal(x.f, y.f);
}

static bool nearly_equal(const skcms_Matrix3x3& a, const skcms_Matrix3x3& b) {
    for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
        if (!nearly_equal(a.vals[i][j], b.vals[i][j])) return false;
    }
    return true;
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
        skcms_TransferFunction fn;
        skcms_Matrix3x3 gamut;
        if (!cs->isNumericalTransferFn(&fn) || !cs->toXYZD50(&gamut)) {
            return false;
        }

        ADataSpace dataSpace = ADATASPACE_UNKNOWN;
        for (const auto& entry : gColorSpaceTable) {
            if (nearly_equal(gamut, entry.gamut) && nearly_equal(fn, entry.transferFunction)) {
                dataSpace = entry.dataSpace;
            }
        }
        if (!ok(AImageDecoder_setDataSpace(fDecoder, dataSpace))) {
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

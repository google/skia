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

static constexpr skcms_TransferFunction k2Dot6 = {2.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

static constexpr skcms_Matrix3x3 kDCIP3 = {{
        {0.486143, 0.323835, 0.154234},
        {0.226676, 0.710327, 0.0629966},
        {0.000800549, 0.0432385, 0.78275},
}};

static sk_sp<SkColorSpace> get_default_colorSpace(const AImageDecoderHeaderInfo* headerInfo) {
    switch (AImageDecoderHeaderInfo_getDataSpace(headerInfo)) {
        case ADATASPACE_SCRGB_LINEAR:
        case ADATASPACE_SRGB_LINEAR:
            return SkColorSpace::MakeSRGBLinear();
        case ADATASPACE_ADOBE_RGB:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB);
        case ADATASPACE_DISPLAY_P3:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
        case ADATASPACE_BT2020:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020);
        case ADATASPACE_BT709:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kSRGB);
        case ADATASPACE_DCI_P3:
            return SkColorSpace::MakeRGB(k2Dot6, kDCIP3);
        default:
            return SkColorSpace::MakeSRGB();
    }
}

static bool not_(int result) {
    return result != ANDROID_IMAGE_DECODER_SUCCESS;
}

static SkColorType colorType(AImageDecoder* rawDecoder, const AImageDecoderHeaderInfo* headerInfo){
    if (AImageDecoder_setAndroidBitmapFormat(rawDecoder, ANDROID_BITMAP_FORMAT_A_8)
            == ANDROID_IMAGE_DECODER_SUCCESS) {
        return kGray_8_SkColorType;
    }
    if (AImageDecoderHeaderInfo_getAndroidBitmapFormat(headerInfo)
             == ANDROID_BITMAP_FORMAT_RGBA_F16) {
        return kRGBA_F16_SkColorType;
    }
    return kRGBA_8888_SkColorType;
}

} // anonymous namespace

std::unique_ptr<SkImageGenerator> SkImageGeneratorNDK::MakeFromEncodedNDK(sk_sp<SkData> data) {
    if (!data) return nullptr;

    AImageDecoder* rawDecoder;
    if (not_(AImageDecoder_createFromBuffer(data->data(), data->size(), &rawDecoder))) {
        return nullptr;
    }

    const AImageDecoderHeaderInfo* headerInfo = AImageDecoder_getHeaderInfo(rawDecoder);
    int32_t width  = AImageDecoderHeaderInfo_getWidth(headerInfo);
    int32_t height = AImageDecoderHeaderInfo_getHeight(headerInfo);
    SkColorType ct = colorType(rawDecoder, headerInfo);
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

namespace {
bool nearly_equal(float a, float b) {
    return fabs(a - b) < .002f;
}

bool operator==(const skcms_TransferFunction& a, const skcms_TransferFunction& b) {
    return nearly_equal(a.g, b.g)
        && nearly_equal(a.a, b.a)
        && nearly_equal(a.b, b.b)
        && nearly_equal(a.c, b.c)
        && nearly_equal(a.d, b.d)
        && nearly_equal(a.e, b.e)
        && nearly_equal(a.f, b.f);
}

bool operator==(const skcms_Matrix3x3& a, const skcms_Matrix3x3& b) {
    for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
        if (!nearly_equal(a.vals[i][j], b.vals[i][j])) return false;
    }
    return true;
}

bool set_android_bitmap_format(AImageDecoder* decoder, SkColorType colorType) {
    auto format = ANDROID_BITMAP_FORMAT_NONE;
    switch (colorType) {
        case kRGBA_8888_SkColorType:
            format = ANDROID_BITMAP_FORMAT_RGBA_8888;
            break;
        case kRGBA_F16_SkColorType:
            format = ANDROID_BITMAP_FORMAT_RGBA_F16;
            break;
        case kRGB_565_SkColorType:
            format = ANDROID_BITMAP_FORMAT_RGB_565;
            break;
        case kARGB_4444_SkColorType:
            format = ANDROID_BITMAP_FORMAT_RGBA_4444;
            break;
        case kGray_8_SkColorType:
            format = ANDROID_BITMAP_FORMAT_A_8;
            break;
        default:
            return false;
    }

    return AImageDecoder_setAndroidBitmapFormat(decoder, format) == ANDROID_IMAGE_DECODER_SUCCESS;
}

bool set_target_size(AImageDecoder* decoder, const SkISize& size, const SkISize targetSize) {
    if (size != targetSize) {
        // AImageDecoder will scale to arbitrary sizes. Only support a size if it's supported by the
        // underlying library.
        const AImageDecoderHeaderInfo* headerInfo = AImageDecoder_getHeaderInfo(decoder);
        const char* mimeType = AImageDecoderHeaderInfo_getMimeType(headerInfo);
        if (strcmp(mimeType, "image/jpeg") == 0) {
            bool supported = false;
            for (int sampleSize : { 2, 4, 8 }) {
                int32_t width;
                int32_t height;
                if (AImageDecoder_computeSampledSize(decoder, sampleSize, &width, &height)
                        == ANDROID_IMAGE_DECODER_SUCCESS
                        && targetSize == SkISize::Make(width, height)) {
                    supported = true;
                    break;
                }
            }
            if (!supported) return false;
        } else if (strcmp(mimeType, "image/webp") == 0) {
            // libwebp supports arbitrary downscaling.
            if (targetSize.width() > size.width() || targetSize.height() > size.height()) {
                return false;
            }
        } else {
            return false;
        }
    }
    return AImageDecoder_setTargetSize(decoder, targetSize.width(), targetSize.height())
            == ANDROID_IMAGE_DECODER_SUCCESS;
}

} // anonymous namespace

bool ImageGeneratorNDK::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                    const Options& opts) {
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
            if (not_(AImageDecoder_setUnpremultipliedRequired(fDecoder, true))) {
                return false;
            }
            break;
        case kPremul_SkAlphaType:
            break;
    }

    if (!set_target_size(fDecoder, getInfo().dimensions(), info.dimensions())) {
        return false;
    }

    if (auto* cs = info.colorSpace()) {
        ADataSpace dataSpace = ADATASPACE_UNKNOWN;
        if (cs->isSRGB()) {
            dataSpace = ADATASPACE_SRGB;
        } else if (cs == SkColorSpace::MakeSRGBLinear().get()) {
            dataSpace = ADATASPACE_SRGB_LINEAR;
        } else {
            skcms_TransferFunction fn;
            skcms_Matrix3x3 gamut;
            if (!cs->isNumericalTransferFn(&fn) || !cs->toXYZD50(&gamut)) {
                return false;
            }

            if (gamut == SkNamedGamut::kSRGB && fn == SkNamedTransferFn::kRec2020) {
                dataSpace = ADATASPACE_BT709;
            } else if (gamut == SkNamedGamut::kDisplayP3 && fn == SkNamedTransferFn::kSRGB) {
                dataSpace = ADATASPACE_DISPLAY_P3;
            } else if (gamut == SkNamedGamut::kAdobeRGB && fn == SkNamedTransferFn::k2Dot2) {
                dataSpace = ADATASPACE_ADOBE_RGB;
            } else if (gamut == SkNamedGamut::kRec2020 && fn == SkNamedTransferFn::kRec2020) {
                dataSpace = ADATASPACE_BT2020;
            } else if (gamut == kDCIP3 && fn == k2Dot6) {
                dataSpace = ADATASPACE_DCI_P3;
            }
        }
        if (not_(AImageDecoder_setDataSpace(fDecoder, dataSpace))) {
            return false;
        }
        fPreviouslySetADataSpace = true;
    } else {
        // If the requested color space is null, no color correction is desired, which is the
        // default. That said, if we previously set an ADataSpace, we need to set it back.
        if (fPreviouslySetADataSpace) {
            const AImageDecoderHeaderInfo* headerInfo = AImageDecoder_getHeaderInfo(fDecoder);
            const auto defaultDataSpace = AImageDecoderHeaderInfo_getDataSpace(headerInfo);
            if (defaultDataSpace == ADATASPACE_UNKNOWN) {
                // As of R, there's no way to reset AImageDecoder to ADATASPACE_UNKNOWN, so
                // create a new one.
                AImageDecoder* decoder;
                if (not_(AImageDecoder_createFromBuffer(fData->data(), fData->size(), &decoder))) {
                    return false;
                }
                // FIXME: This block needs to be first - update the test to verify that...
                AImageDecoder_delete(fDecoder);
                fDecoder = decoder;
            } else {
                if (not_(AImageDecoder_setDataSpace(fDecoder, defaultDataSpace))) {
                    return false;
                }
            }
        }
    }

    auto byteSize = info.computeByteSize(rowBytes);
    switch (AImageDecoder_decodeImage(fDecoder, pixels, rowBytes, byteSize)) {
        case ANDROID_IMAGE_DECODER_SUCCESS:
        case ANDROID_IMAGE_DECODER_INCOMPLETE:
        case ANDROID_IMAGE_DECODER_ERROR:
            return true;
        default:
            return false;
    }
}

sk_sp<SkData> ImageGeneratorNDK::onRefEncodedData() {
    return fData;
}

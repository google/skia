/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkCrabbyAvifCodec.h"

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkAvifDecoder.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkGainmapInfo.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkStreamPriv.h"

#include <cstdint>
#include <cstring>
#include <utility>

#include "avif/avif.h"
#include "avif/libavif_compat.h"

namespace {

template <typename NumeratorType>
float FractionToFloat(NumeratorType numerator, uint32_t denominator) {
    // First cast to double and not float because uint32_t->float conversion can
    // cause precision loss.
    return static_cast<double>(numerator) / denominator;
}

sk_sp<SkColorSpace> AltImageColorSpace(const crabbyavif::avifGainMap& gain_map,
                                       const crabbyavif::avifImage& image) {
    sk_sp<SkColorSpace> color_space = nullptr;
    if (!gain_map.altICC.size) {
        return nullptr;
    }
    if (image.icc.size == gain_map.altICC.size &&
        memcmp(gain_map.altICC.data, image.icc.data, gain_map.altICC.size) == 0) {
        // Same ICC as the base image, no need to specify it.
        return nullptr;
    }
    skcms_ICCProfile icc_profile;
    if (!skcms_Parse(gain_map.altICC.data, gain_map.altICC.size, &icc_profile)) {
        return nullptr;
    }
    return SkColorSpace::Make(icc_profile);
}

bool PopulateGainmapInfo(const crabbyavif::avifGainMap& gain_map,
                         const crabbyavif::avifImage& image,
                         SkGainmapInfo* info) {
    if (gain_map.baseHdrHeadroom.d == 0 || gain_map.alternateHdrHeadroom.d == 0) {
        return false;
    }
    const float base_headroom =
            std::exp2(FractionToFloat(gain_map.baseHdrHeadroom.n, gain_map.baseHdrHeadroom.d));
    const float alternate_headroom = std::exp2(
            FractionToFloat(gain_map.alternateHdrHeadroom.n, gain_map.alternateHdrHeadroom.d));
    const bool base_is_hdr = base_headroom > alternate_headroom;
    info->fDisplayRatioSdr = base_is_hdr ? alternate_headroom : base_headroom;
    info->fDisplayRatioHdr = base_is_hdr ? base_headroom : alternate_headroom;
    info->fBaseImageType =
            base_is_hdr ? SkGainmapInfo::BaseImageType::kHDR : SkGainmapInfo::BaseImageType::kSDR;
    for (int i = 0; i < 3; ++i) {
        if (gain_map.gainMapMin[i].d == 0 || gain_map.gainMapMax[i].d == 0 ||
            gain_map.gainMapGamma[i].d == 0 || gain_map.baseOffset[i].d == 0 ||
            gain_map.alternateOffset[i].d == 0 || gain_map.gainMapGamma[i].n == 0) {
            return false;
        }
        const float min_log2 = FractionToFloat(gain_map.gainMapMin[i].n, gain_map.gainMapMin[i].d);
        const float max_log2 = FractionToFloat(gain_map.gainMapMax[i].n, gain_map.gainMapMax[i].d);
        info->fGainmapRatioMin[i] = std::exp2(min_log2);
        info->fGainmapRatioMax[i] = std::exp2(max_log2);
        // Numerator and denominator intentionally swapped to get 1.0/gamma.
        info->fGainmapGamma[i] =
                FractionToFloat(gain_map.gainMapGamma[i].d, gain_map.gainMapGamma[i].n);
        const float base_offset =
                FractionToFloat(gain_map.baseOffset[i].n, gain_map.baseOffset[i].d);
        const float alternate_offset =
                FractionToFloat(gain_map.alternateOffset[i].n, gain_map.alternateOffset[i].d);
        info->fEpsilonSdr[i] = base_is_hdr ? alternate_offset : base_offset;
        info->fEpsilonHdr[i] = base_is_hdr ? base_offset : alternate_offset;
    }
    if (!gain_map.useBaseColorSpace) {
        info->fGainmapMathColorSpace = AltImageColorSpace(gain_map, image);
    }
    return true;
}

SkEncodedOrigin ComputeSkEncodedOrigin(const crabbyavif::avifImage& image) {
    // |angle| * 90 specifies the angle of anti-clockwise rotation in degrees.
    // Legal values: [0-3].
    const int angle =
            ((image.transformFlags & crabbyavif::AVIF_TRANSFORM_IROT) && image.irot.angle <= 3)
                    ? image.irot.angle
                    : 0;
    // |axis| specifies how the mirroring is performed.
    //   -1: No mirroring.
    //    0: The top and bottom parts of the image are exchanged.
    //    1: The left and right parts of the image are exchanged.
    const int axis =
            ((image.transformFlags & crabbyavif::AVIF_TRANSFORM_IMIR) && image.imir.axis <= 1)
                    ? image.imir.axis
                    : -1;
    // The first dimension is axis (with an offset of 1). The second dimension
    // is angle.
    const SkEncodedOrigin kAxisAngleToSkEncodedOrigin[3][4] = {
            // No mirroring.
            {kTopLeft_SkEncodedOrigin,
             kLeftBottom_SkEncodedOrigin,
             kBottomRight_SkEncodedOrigin,
             kRightTop_SkEncodedOrigin},
            // Top-to-bottom mirroring. Change Top<->Bottom in the first row.
            {kBottomLeft_SkEncodedOrigin,
             kLeftTop_SkEncodedOrigin,
             kTopRight_SkEncodedOrigin,
             kRightBottom_SkEncodedOrigin},
            // Left-to-right mirroring. Change Left<->Right in the first row.
            {kTopRight_SkEncodedOrigin,
             kRightBottom_SkEncodedOrigin,
             kBottomLeft_SkEncodedOrigin,
             kLeftTop_SkEncodedOrigin},
    };
    return kAxisAngleToSkEncodedOrigin[axis + 1][angle];
}

}  // namespace

void AvifDecoderDeleter::operator()(crabbyavif::avifDecoder* decoder) const {
    if (decoder != nullptr) {
        crabbyavif::avifDecoderDestroy(decoder);
    }
}

bool SkCrabbyAvifCodec::IsAvif(const void* buffer, size_t bytesRead) {
    crabbyavif::avifROData avifData = {static_cast<const uint8_t*>(buffer), bytesRead};
    return crabbyavif::avifPeekCompatibleFileType(&avifData) == crabbyavif::CRABBY_AVIF_TRUE;
}

std::unique_ptr<SkCodec> SkCrabbyAvifCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                           Result* result,
                                                           bool gainmapOnly /*=false*/) {
    SkASSERT(result);
    if (!stream) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }

    // CrabbyAvif needs a contiguous data buffer.
    sk_sp<SkData> data = nullptr;
    if (stream->getMemoryBase()) {
        // It is safe to make without copy because we'll hold onto the stream.
        data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        data = SkCopyStreamToData(stream.get());
        // If we are forced to copy the stream to a data, we can go ahead and
        // delete the stream.
        stream.reset(nullptr);
    }
    return SkCrabbyAvifCodec::MakeFromData(std::move(stream), std::move(data), result, gainmapOnly);
}

std::unique_ptr<SkCodec> SkCrabbyAvifCodec::MakeFromData(std::unique_ptr<SkStream> stream,
                                                         sk_sp<SkData> data,
                                                         Result* result,
                                                         bool gainmapOnly /*=false*/) {
    SkASSERT(result);

    AvifDecoder avifDecoder(crabbyavif::avifDecoderCreate());
    if (avifDecoder == nullptr) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    // Ignore XMP and Exif to ensure that avifDecoderParse() isn't waiting for
    // some tiny Exif payload hiding at the end of a file.
    avifDecoder->ignoreXMP = crabbyavif::CRABBY_AVIF_TRUE;
    avifDecoder->ignoreExif = crabbyavif::CRABBY_AVIF_TRUE;

    // Disable strict mode. This allows some AVIF files in the wild that are
    // technically invalid according to the specification because they were
    // created with older tools but can be decoded and rendered without any
    // issues.
    avifDecoder->strictFlags = crabbyavif::AVIF_STRICT_DISABLED;

    // Android uses MediaCodec for decoding the underlying image. So there is no
    // need to set maxThreads since MediaCodec doesn't allow explicit setting
    // of threads.

    if (gainmapOnly) {
        avifDecoder->imageContentToDecode = crabbyavif::AVIF_IMAGE_CONTENT_GAIN_MAP;
    }

    crabbyavif::avifResult res =
            crabbyavif::avifDecoderSetIOMemory(avifDecoder.get(), data->bytes(), data->size());
    if (res != crabbyavif::AVIF_RESULT_OK) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    res = crabbyavif::avifDecoderParse(avifDecoder.get());
    if (res != crabbyavif::AVIF_RESULT_OK) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }

    // CrabbyAvif uses MediaCodec, which always sets bitsPerComponent to 8.
    const int bitsPerComponent = 8;
    SkEncodedInfo::Color color;
    SkEncodedInfo::Alpha alpha;
    if (avifDecoder->alphaPresent && !gainmapOnly) {
        color = SkEncodedInfo::kRGBA_Color;
        alpha = SkEncodedInfo::kUnpremul_Alpha;
    } else {
        color = SkEncodedInfo::kRGB_Color;
        alpha = SkEncodedInfo::kOpaque_Alpha;
    }
    if (gainmapOnly && !avifDecoder->image->gainMap) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }
    crabbyavif::avifImage* image =
            gainmapOnly ? avifDecoder->image->gainMap->image : avifDecoder->image;
    auto width = image->width;
    auto height = image->height;
    if (image->transformFlags & crabbyavif::AVIF_TRANSFORM_CLAP) {
        crabbyavif::avifCropRect rect;
        if (crabbyavif::crabby_avifCropRectConvertCleanApertureBox(
                    &rect, &image->clap, width, height, image->yuvFormat, nullptr)) {
            width = rect.width;
            height = rect.height;
        }
    }

    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = nullptr;
    if (image->icc.size > 0) {
        auto icc = SkData::MakeWithCopy(image->icc.data, image->icc.size);
        profile = SkEncodedInfo::ICCProfile::Make(std::move(icc));
    } else if (image->transferCharacteristics == crabbyavif::AVIF_TRANSFER_CHARACTERISTICS_PQ ||
               image->transferCharacteristics == crabbyavif::AVIF_TRANSFER_CHARACTERISTICS_HLG) {
        // TODO(https://issues.skia.org/issues/432721733): Create a version of
        // SkEncodedInfo::ICCProfile::Make that directly takes CICP values.
        skcms_ICCProfile skcmsProfile;
        skcms_Init(&skcmsProfile);
        skcmsProfile.CICP.color_primaries = image->colorPrimaries;
        skcmsProfile.CICP.transfer_characteristics = image->transferCharacteristics;
        // Do not set matrix_coefficients here because cicp_get_sk_color_space in SkAndroidCodec.cpp
        // fails if matrix_coeffieicnts is not zero:
        // https://skia.googlesource.com/skia/+/33b2d3333755ac5ce21495959c2d4bb11f299f8b/src/codec/SkAndroidCodec.cpp#186
        skcmsProfile.CICP.video_full_range_flag = image->yuvRange == crabbyavif::AVIF_RANGE_FULL;
        skcmsProfile.has_CICP = true;
        profile = SkEncodedInfo::ICCProfile::Make(skcmsProfile);
    }
    if (profile && profile->profile()->data_color_space != skcms_Signature_RGB) {
        profile = nullptr;
    }

    SkEncodedInfo info = SkEncodedInfo::Make(
            width, height, color, alpha, bitsPerComponent, std::move(profile), image->depth);
    bool animation = avifDecoder->imageCount > 1;
    *result = kSuccess;
    SkEncodedImageFormat format =
            avifDecoder->compressionFormat == crabbyavif::COMPRESSION_FORMAT_AVIF
                    ? SkEncodedImageFormat::kAVIF
                    : SkEncodedImageFormat::kHEIF;
    const SkEncodedOrigin origin = ComputeSkEncodedOrigin(*image);
    return std::unique_ptr<SkCodec>(new SkCrabbyAvifCodec(std::move(info),
                                                          std::move(stream),
                                                          std::move(data),
                                                          std::move(avifDecoder),
                                                          origin,
                                                          animation,
                                                          gainmapOnly,
                                                          format));
}

SkCrabbyAvifCodec::SkCrabbyAvifCodec(SkEncodedInfo&& info,
                                     std::unique_ptr<SkStream> stream,
                                     sk_sp<SkData> data,
                                     AvifDecoder avifDecoder,
                                     SkEncodedOrigin origin,
                                     bool useAnimation,
                                     bool gainmapOnly,
                                     SkEncodedImageFormat format)
        : SkScalingCodec(std::move(info), skcms_PixelFormat_RGBA_8888, std::move(stream), origin)
        , fData(std::move(data))
        , fAvifDecoder(std::move(avifDecoder))
        , fUseAnimation(useAnimation)
        , fGainmapOnly(gainmapOnly)
        , fFormat(format) {}

int SkCrabbyAvifCodec::onGetFrameCount() {
    if (!fUseAnimation) {
        return 1;
    }

    if (fFrameHolder.size() == 0) {
        if (fAvifDecoder->imageCount <= 1) {
            fUseAnimation = false;
            return 1;
        }
        fFrameHolder.reserve(fAvifDecoder->imageCount);
        for (int i = 0; i < fAvifDecoder->imageCount; i++) {
            Frame* frame = fFrameHolder.appendNewFrame(fAvifDecoder->alphaPresent ==
                                                       crabbyavif::CRABBY_AVIF_TRUE);
            frame->setXYWH(0, 0, fAvifDecoder->image->width, fAvifDecoder->image->height);
            frame->setDisposalMethod(SkCodecAnimation::DisposalMethod::kKeep);
            crabbyavif::avifImageTiming timing;
            avifDecoderNthImageTiming(fAvifDecoder.get(), i, &timing);
            frame->setDuration(timing.duration * 1000);
            frame->setRequiredFrame(SkCodec::kNoFrame);
            frame->setHasAlpha(fAvifDecoder->alphaPresent == crabbyavif::CRABBY_AVIF_TRUE);
        }
    }

    return fFrameHolder.size();
}

const SkFrame* SkCrabbyAvifCodec::FrameHolder::onGetFrame(int i) const {
    return static_cast<const SkFrame*>(this->frame(i));
}

SkCrabbyAvifCodec::Frame* SkCrabbyAvifCodec::FrameHolder::appendNewFrame(bool hasAlpha) {
    const int i = this->size();
    fFrames.emplace_back(i,
                         hasAlpha ? SkEncodedInfo::kUnpremul_Alpha : SkEncodedInfo::kOpaque_Alpha);
    return &fFrames[i];
}

const SkCrabbyAvifCodec::Frame* SkCrabbyAvifCodec::FrameHolder::frame(int i) const {
    SkASSERT(i >= 0 && i < this->size());
    return &fFrames[i];
}

bool SkCrabbyAvifCodec::onGetFrameInfo(int i, FrameInfo* frameInfo) const {
    if (i >= fFrameHolder.size()) {
        return false;
    }

    const Frame* frame = fFrameHolder.frame(i);
    if (!frame) {
        return false;
    }

    if (frameInfo) {
        frame->fillIn(frameInfo, true);
    }

    return true;
}

int SkCrabbyAvifCodec::onGetRepetitionCount() {
    return (fAvifDecoder->repetitionCount < 0) ? kRepetitionCountInfinite
                                               : fAvifDecoder->repetitionCount;
}

SkCodec::IsAnimated SkCrabbyAvifCodec::onIsAnimated() {
    if (!fUseAnimation || fAvifDecoder->imageCount <= 1) {
        return IsAnimated::kNo;
    }
    return IsAnimated::kYes;
}

bool SkCrabbyAvifCodec::conversionSupported(const SkImageInfo& dstInfo,
                                            bool srcIsOpaque,
                                            bool needsColorXform) {
    return dstInfo.colorType() == kRGBA_8888_SkColorType ||
           dstInfo.colorType() == kBGRA_8888_SkColorType ||
           dstInfo.colorType() == kRGBA_1010102_SkColorType ||
           dstInfo.colorType() == kRGBA_F16_SkColorType ||
           dstInfo.colorType() == kRGB_565_SkColorType;
}

SkCodec::Result SkCrabbyAvifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                               void* dst,
                                               size_t dstRowBytes,
                                               const Options& options,
                                               int* rowsDecoded) {
    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGB_565_SkColorType:
            fAvifDecoder->androidMediaCodecOutputColorFormat =
                    crabbyavif::ANDROID_MEDIA_CODEC_OUTPUT_COLOR_FORMAT_YUV420_FLEXIBLE;
            break;
        case kRGBA_F16_SkColorType:
        case kRGBA_1010102_SkColorType:
            fAvifDecoder->androidMediaCodecOutputColorFormat =
                    crabbyavif::ANDROID_MEDIA_CODEC_OUTPUT_COLOR_FORMAT_P010;
            break;
        default:
            return kUnimplemented;
    }

    crabbyavif::avifResult result =
            crabbyavif::avifDecoderNthImage(fAvifDecoder.get(), options.fFrameIndex);
    if (result != crabbyavif::AVIF_RESULT_OK) {
        return kInvalidInput;
    }
    if (fGainmapOnly && !fAvifDecoder->image->gainMap) {
        return kInvalidInput;
    }

    // At this point we have the decoded image. Now we have to perform cropping, subset computation
    // and scaling. The right order of these operations is:
    // 1) Cropping (as described by the CleanAperture property). This has to be the first step to
    //    ensure that we don't accidentally expose the non-cropped portions of the image to the
    //    subsequent operations.
    // 2) Subset computation (as requested by options.fSubset).
    // 3) Scaling (to match dstInfo.dimensions() if necessary). This has to be the last step to
    //    ensure that we never fill in more pixels than what is requested by dstInfo.dimensions().

    crabbyavif::avifImage* image =
            fGainmapOnly ? fAvifDecoder->image->gainMap->image : fAvifDecoder->image;
    using AvifImagePtr =
            std::unique_ptr<crabbyavif::avifImage, decltype(&crabbyavif::crabby_avifImageDestroy)>;

    // cropped_image is a view into the underlying image. It can be safely deleted once the pixels
    // are converted into RGB (or when it goes out of scope in one of the error paths).
    AvifImagePtr cropped_image{nullptr, crabbyavif::crabby_avifImageDestroy};
    if (image->transformFlags & crabbyavif::AVIF_TRANSFORM_CLAP) {
        crabbyavif::avifCropRect rect;
        if (crabbyavif::crabby_avifCropRectConvertCleanApertureBox(
                    &rect, &image->clap, image->width, image->height, image->yuvFormat, nullptr)) {
            cropped_image.reset(crabbyavif::crabby_avifImageCreateEmpty());
            result = crabbyavif::crabby_avifImageSetViewRect(cropped_image.get(), image, &rect);
            if (result != crabbyavif::AVIF_RESULT_OK) {
                return kInvalidInput;
            }
            image = cropped_image.get();
        }
    }

    AvifImagePtr subset_image{nullptr, crabbyavif::crabby_avifImageDestroy};
    if (options.fSubset) {
        const crabbyavif::avifCropRect rect{
                .x = static_cast<uint32_t>(options.fSubset->x()),
                .y = static_cast<uint32_t>(options.fSubset->y()),
                .width = static_cast<uint32_t>(options.fSubset->width()),
                .height = static_cast<uint32_t>(options.fSubset->height())};
        subset_image.reset(crabbyavif::crabby_avifImageCreateEmpty());
        result = crabbyavif::crabby_avifImageSetViewRect(subset_image.get(), image, &rect);
        if (result != crabbyavif::AVIF_RESULT_OK) {
            return kInvalidInput;
        }
        image = subset_image.get();
    }

    AvifImagePtr scaled_image{nullptr, crabbyavif::crabby_avifImageDestroy};
    if (dstInfo.width() != image->width || dstInfo.height() != image->height) {
        // |image| contains plane pointers which point to Android MediaCodec's buffers. Those
        // buffers are read-only and hence we cannot scale in place. Make a copy of the image and
        // scale the copied image.
        scaled_image.reset(crabbyavif::crabby_avifImageCreateEmpty());
        result = crabbyavif::crabby_avifImageCopy(
                scaled_image.get(), image, crabbyavif::AVIF_PLANES_ALL);
        if (result != crabbyavif::AVIF_RESULT_OK) {
            return kInvalidInput;
        }
        image = scaled_image.get();
        result = crabbyavif::avifImageScale(
                image, dstInfo.width(), dstInfo.height(), &fAvifDecoder->diag);
        if (result != crabbyavif::AVIF_RESULT_OK) {
            return kInvalidInput;
        }
    }

    crabbyavif::avifRGBImage rgbImage;
    crabbyavif::avifRGBImageSetDefaults(&rgbImage, image);

    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            rgbImage.depth = 8;
            rgbImage.format = crabbyavif::AVIF_RGB_FORMAT_RGBA;
            break;
        case kBGRA_8888_SkColorType:
            rgbImage.depth = 8;
            rgbImage.format = crabbyavif::AVIF_RGB_FORMAT_BGRA;
            break;
        case kRGBA_F16_SkColorType:
            rgbImage.depth = 16;
            rgbImage.format = crabbyavif::AVIF_RGB_FORMAT_RGBA;
            rgbImage.isFloat = crabbyavif::CRABBY_AVIF_TRUE;
            break;
        case kRGBA_1010102_SkColorType:
            rgbImage.depth = 10;
            rgbImage.format = crabbyavif::AVIF_RGB_FORMAT_RGBA1010102;
            break;
        case kRGB_565_SkColorType:
            rgbImage.depth = 8;
            rgbImage.format = crabbyavif::AVIF_RGB_FORMAT_RGB565;
            break;
        default:
            // Not reached because of the checks in conversionSupported().
            return kUnimplemented;
    }

    rgbImage.pixels = static_cast<uint8_t*>(dst);
    rgbImage.rowBytes = dstRowBytes;
    rgbImage.chromaUpsampling = crabbyavif::AVIF_CHROMA_UPSAMPLING_FASTEST;

    result = crabbyavif::avifImageYUVToRGB(image, &rgbImage);
    if (result != crabbyavif::AVIF_RESULT_OK) {
        return kInvalidInput;
    }

    *rowsDecoded = image->height;
    return kSuccess;
}

bool SkCrabbyAvifCodec::onGetGainmapCodec(SkGainmapInfo* info,
                                          std::unique_ptr<SkCodec>* gainmapCodec) {
    if (!gainmapCodec || !info || !fAvifDecoder->image || !fAvifDecoder->image->gainMap ||
        !PopulateGainmapInfo(*fAvifDecoder->image->gainMap, *fAvifDecoder->image, info)) {
        return false;
    }
    Result result;
    *gainmapCodec = SkCrabbyAvifCodec::MakeFromData(
            /*stream=*/nullptr, fData, &result, /*gainmapOnly=*/true);
    return static_cast<bool>(*gainmapCodec);
}

namespace SkAvifDecoder {
namespace CrabbyAvif {

bool IsAvif(const void* data, size_t len) { return SkCrabbyAvifCodec::IsAvif(data, len); }

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext) {
    SkCodec::Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }
    return SkCrabbyAvifCodec::MakeFromStream(std::move(stream), outResult);
}

std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext) {
    if (!data) {
        if (outResult) {
            *outResult = SkCodec::kInvalidInput;
        }
        return nullptr;
    }
    return Decode(SkMemoryStream::Make(std::move(data)), outResult, nullptr);
}

}  // namespace CrabbyAvif
}  // namespace SkAvifDecoder

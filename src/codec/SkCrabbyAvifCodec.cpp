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

bool PopulateGainmapInfo(const crabbyavif::avifGainMap& gain_map, SkGainmapInfo* info) {
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

        if (!gain_map.useBaseColorSpace) {
            // TODO(vigneshv): Compute fGainmapMathColorSpace.
        }
    }
    return true;
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

    // TODO(vigneshv): Enable threading based on number of CPU cores available.
    avifDecoder->maxThreads = 1;

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

    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = nullptr;
    // TODO(vigneshv): Get ICC Profile from the avif decoder.

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
    SkEncodedInfo info = SkEncodedInfo::Make(
            width, height, color, alpha, bitsPerComponent, std::move(profile), image->depth);
    bool animation = avifDecoder->imageCount > 1;
    *result = kSuccess;
    return std::unique_ptr<SkCodec>(new SkCrabbyAvifCodec(std::move(info),
                                                          std::move(stream),
                                                          std::move(data),
                                                          std::move(avifDecoder),
                                                          kDefault_SkEncodedOrigin,
                                                          animation,
                                                          gainmapOnly));
}

SkCrabbyAvifCodec::SkCrabbyAvifCodec(SkEncodedInfo&& info,
                                     std::unique_ptr<SkStream> stream,
                                     sk_sp<SkData> data,
                                     AvifDecoder avifDecoder,
                                     SkEncodedOrigin origin,
                                     bool useAnimation,
                                     bool gainmapOnly)
        : SkScalingCodec(std::move(info), skcms_PixelFormat_RGBA_8888, std::move(stream), origin)
        , fData(std::move(data))
        , fAvifDecoder(std::move(avifDecoder))
        , fUseAnimation(useAnimation)
        , fGainmapOnly(gainmapOnly) {}

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

int SkCrabbyAvifCodec::onGetRepetitionCount() { return kRepetitionCountInfinite; }

bool SkCrabbyAvifCodec::conversionSupported(const SkImageInfo& dstInfo,
                                            bool srcIsOpaque,
                                            bool needsColorXform) {
    return dstInfo.colorType() == kRGBA_8888_SkColorType ||
           dstInfo.colorType() == kRGBA_1010102_SkColorType ||
           dstInfo.colorType() == kRGBA_F16_SkColorType;
}

SkCodec::Result SkCrabbyAvifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                               void* dst,
                                               size_t dstRowBytes,
                                               const Options& options,
                                               int* rowsDecoded) {
    if (options.fSubset) {
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
    crabbyavif::avifImage* image =
            fGainmapOnly ? fAvifDecoder->image->gainMap->image : fAvifDecoder->image;
    if (this->dimensions() != dstInfo.dimensions()) {
        result = crabbyavif::avifImageScale(
                image, dstInfo.width(), dstInfo.height(), &fAvifDecoder->diag);
        if (result != crabbyavif::AVIF_RESULT_OK) {
            return kInvalidInput;
        }
    }

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

    crabbyavif::avifRGBImage rgbImage;
    crabbyavif::avifRGBImageSetDefaults(&rgbImage, image);

    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            rgbImage.depth = 8;
            break;
        case kRGBA_F16_SkColorType:
            rgbImage.depth = 16;
            rgbImage.isFloat = crabbyavif::CRABBY_AVIF_TRUE;
            break;
        case kRGBA_1010102_SkColorType:
            rgbImage.depth = 10;
            rgbImage.format = crabbyavif::AVIF_RGB_FORMAT_RGBA1010102;
            break;
        default:
            // TODO(vigneshv): Check if more color types need to be supported.
            // Currently android supports at least RGB565 and BGRA8888 which is
            // not supported here.
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
        !PopulateGainmapInfo(*fAvifDecoder->image->gainMap, info)) {
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

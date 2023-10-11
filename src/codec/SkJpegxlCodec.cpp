/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegxlCodec.h"

#include "include/codec/SkCodec.h"
#include "include/codec/SkJpegxlDecoder.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkFrameHolder.h"
#include "src/core/SkStreamPriv.h"
#include "src/core/SkSwizzlePriv.h"

#include "jxl/codestream_header.h"  // NO_G3_REWRITE
#include "jxl/decode.h"  // NO_G3_REWRITE
#include "jxl/decode_cxx.h"  // NO_G3_REWRITE
#include "jxl/types.h"  // NO_G3_REWRITE

#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

namespace {

class Frame : public SkFrame {
public:
    explicit Frame(int i, SkEncodedInfo::Alpha alpha) : INHERITED(i), fReportedAlpha(alpha) {}
    SkEncodedInfo::Alpha onReportedAlpha() const override { return fReportedAlpha; }

private:
    const SkEncodedInfo::Alpha fReportedAlpha;

    using INHERITED = SkFrame;
};

}  // namespace

bool SkJpegxlCodec::IsJpegxl(const void* buffer, size_t bytesRead) {
    JxlSignature result = JxlSignatureCheck(reinterpret_cast<const uint8_t*>(buffer), bytesRead);
    return (result == JXL_SIG_CODESTREAM) || (result == JXL_SIG_CONTAINER);
}

class SkJpegxlCodecPriv : public SkFrameHolder {
public:
    SkJpegxlCodecPriv() : fDecoder(JxlDecoderMake(/* memory_manager= */ nullptr)) {}
    JxlDecoderPtr fDecoder;  // unique_ptr with custom destructor
    JxlBasicInfo fInfo;
    bool fSeenAllFrames = false;
    std::vector<Frame> fFrames;
    int fLastProcessedFrame = SkCodec::kNoFrame;
    void* fDst;
    size_t fPixelShift;
    size_t fRowBytes;
    SkColorType fDstColorType;

protected:
    const SkFrame* onGetFrame(int i) const override {
        SkASSERT(i >= 0 && static_cast<size_t>(i) < fFrames.size());
        return static_cast<const SkFrame*>(&fFrames[i]);
    }
};

SkJpegxlCodec::SkJpegxlCodec(std::unique_ptr<SkJpegxlCodecPriv> codec,
                             SkEncodedInfo&& info,
                             std::unique_ptr<SkStream> stream,
                             sk_sp<SkData> data)
        : INHERITED(std::move(info), skcms_PixelFormat_RGBA_16161616LE, std::move(stream))
        , fCodec(std::move(codec))
        , fData(std::move(data)) {}

std::unique_ptr<SkCodec> SkJpegxlCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                       Result* result) {
    SkASSERT(result);
    if (!stream) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }
    *result = kInternalError;
    // Either wrap or copy stream data.
    sk_sp<SkData> data = nullptr;
    if (stream->getMemoryBase()) {
        data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        data = SkCopyStreamToData(stream.get());
        // Data is copied; stream can be released now.
        stream.reset(nullptr);
    }

    auto priv = std::make_unique<SkJpegxlCodecPriv>();
    JxlDecoder* dec = priv->fDecoder.get();

    // Only query metadata this time.
    auto status = JxlDecoderSubscribeEvents(dec, JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING);
    if (status != JXL_DEC_SUCCESS) {
        // Fresh instance must accept request for subscription.
        SkDEBUGFAIL("libjxl returned unexpected status");
        return nullptr;
    }

    status = JxlDecoderSetInput(dec, data->bytes(), data->size());
    if (status != JXL_DEC_SUCCESS) {
        // Fresh instance must accept first chunk of input.
        SkDEBUGFAIL("libjxl returned unexpected status");
        return nullptr;
    }

    status = JxlDecoderProcessInput(dec);
    if (status == JXL_DEC_NEED_MORE_INPUT) {
        *result = kIncompleteInput;
        return nullptr;
    }
    if (status != JXL_DEC_BASIC_INFO) {
        *result = kInvalidInput;
        return nullptr;
    }
    JxlBasicInfo& info = priv->fInfo;
    status = JxlDecoderGetBasicInfo(dec, &info);
    if (status != JXL_DEC_SUCCESS) {
        // Current event is "JXL_DEC_BASIC_INFO" -> can't fail.
        SkDEBUGFAIL("libjxl returned unexpected status");
        return nullptr;
    }

    // Check that image dimensions are not too large.
    if (!SkTFitsIn<int32_t>(info.xsize) || !SkTFitsIn<int32_t>(info.ysize)) {
        *result = kInvalidInput;
        return nullptr;
    }
    int32_t width = SkTo<int32_t>(info.xsize);
    int32_t height = SkTo<int32_t>(info.ysize);

    bool hasAlpha = (info.alpha_bits != 0);
    bool isGray = (info.num_color_channels == 1);
    SkEncodedInfo::Alpha alpha =
            hasAlpha ? SkEncodedInfo::kUnpremul_Alpha : SkEncodedInfo::kOpaque_Alpha;
    SkEncodedInfo::Color color;
    if (hasAlpha) {
        color = isGray ? SkEncodedInfo::kGrayAlpha_Color : SkEncodedInfo::kRGBA_Color;
    } else {
        color = isGray ? SkEncodedInfo::kGray_Color : SkEncodedInfo::kRGB_Color;
    }

    status = JxlDecoderProcessInput(dec);
    if (status != JXL_DEC_COLOR_ENCODING) {
        *result = kInvalidInput;
        return nullptr;
    }

    size_t iccSize = 0;
    // TODO(eustas): format field is currently ignored by decoder.
    status = JxlDecoderGetICCProfileSize(
        dec, /* format = */ nullptr, JXL_COLOR_PROFILE_TARGET_DATA, &iccSize);
    if (status != JXL_DEC_SUCCESS) {
        // Likely incompatible colorspace.
        iccSize = 0;
    }
    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = nullptr;
    if (iccSize) {
        auto icc = SkData::MakeUninitialized(iccSize);
        // TODO(eustas): format field is currently ignored by decoder.
        status = JxlDecoderGetColorAsICCProfile(dec,
                                                /* format = */ nullptr,
                                                JXL_COLOR_PROFILE_TARGET_DATA,
                                                reinterpret_cast<uint8_t*>(icc->writable_data()),
                                                iccSize);
        if (status != JXL_DEC_SUCCESS) {
            // Current event is JXL_DEC_COLOR_ENCODING -> can't fail.
            SkDEBUGFAIL("libjxl returned unexpected status");
            return nullptr;
        }
        profile = SkEncodedInfo::ICCProfile::Make(std::move(icc));
    }

    int bitsPerChannel = 16;

    *result = kSuccess;
    SkEncodedInfo encodedInfo =
            SkEncodedInfo::Make(width, height, color, alpha, bitsPerChannel, std::move(profile));

    return std::unique_ptr<SkCodec>(new SkJpegxlCodec(
            std::move(priv), std::move(encodedInfo), std::move(stream), std::move(data)));
}

SkCodec::Result SkJpegxlCodec::onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
                                           const Options& options, int* rowsDecodedPtr) {
    // TODO(eustas): implement
    if (options.fSubset) {
        return kUnimplemented;
    }
    auto& codec = *fCodec.get();
    const int index = options.fFrameIndex;
    SkASSERT(0 == index || static_cast<size_t>(index) < codec.fFrames.size());
    auto* dec = codec.fDecoder.get();
    JxlDecoderStatus status;

    if ((codec.fLastProcessedFrame >= index) || (codec.fLastProcessedFrame = SkCodec::kNoFrame)) {
        codec.fLastProcessedFrame = SkCodec::kNoFrame;
        JxlDecoderRewind(dec);
        status = JxlDecoderSubscribeEvents(dec, JXL_DEC_FRAME | JXL_DEC_FULL_IMAGE);
        if (status != JXL_DEC_SUCCESS) {
            // Fresh decoder instance (after rewind) must accept subscription request.
            SkDEBUGFAIL("libjxl returned unexpected status");
            return kInternalError;
        }
        status = JxlDecoderSetInput(dec, fData->bytes(), fData->size());
        if (status != JXL_DEC_SUCCESS) {
            // Fresh decoder instance (after rewind) must accept first data chunk.
            SkDEBUGFAIL("libjxl returned unexpected status");
            return kInternalError;
        }
        SkASSERT(codec.fLastProcessedFrame + 1 == 0);
    }

    int nextFrame = codec.fLastProcessedFrame + 1;
    if (nextFrame < index) {
        JxlDecoderSkipFrames(dec, index - nextFrame);
    }

    // Decode till the frame start.
    status = JxlDecoderProcessInput(dec);
    // TODO(eustas): actually, frame is not completely processed; for streaming / partial decoding
    //               we should also add a flag that "last processed frame" is still incomplete, and
    //               flip that flag when frame decoding is over.
    codec.fLastProcessedFrame = index;
    if (status != JXL_DEC_FRAME) {
        // TODO(eustas): check status: it might be either corrupted or incomplete input.
        return kInternalError;
    }

    codec.fDst = dst;
    codec.fRowBytes = rowBytes;

    // TODO(eustas): consider grayscale.
    uint32_t numColorChannels = 3;
    // TODO(eustas): consider no-alpha.
    uint32_t numAlphaChannels = 1;
    // NB: SKIA works with little-endian F16s.
    auto endianness = JXL_LITTLE_ENDIAN;

    // Internally JXL does most processing in floats. By "default" we request
    // output data type to be U8; it takes less memory, but results in some precision loss.
    //  We request F16 in two cases:
    //  - destination type is F16
    //  - color transformation is required; in this case values are remapped,
    //    and with 8-bit precision it is likely that visual artefact will appear
    //    (like banding, etc.)
    bool halfFloatOutput = false;
    if (fCodec->fDstColorType == kRGBA_F16_SkColorType) halfFloatOutput = true;
    if (colorXform()) halfFloatOutput = true;
    auto dataType = halfFloatOutput ? JXL_TYPE_FLOAT16 : JXL_TYPE_UINT8;

    JxlPixelFormat format =
        {numColorChannels + numAlphaChannels, dataType, endianness, /* align = */ 0};
    status = JxlDecoderSetImageOutCallback(dec, &format, SkJpegxlCodec::imageOutCallback, this);
    if (status != JXL_DEC_SUCCESS) {
        // Current event is JXL_DEC_FRAME -> decoder must accept callback.
        SkDEBUGFAIL("libjxl returned unexpected status");
        return kInternalError;
    }

    // Decode till the frame start.
    status = JxlDecoderProcessInput(dec);
    if (status != JXL_DEC_FULL_IMAGE) {
        // TODO(eustas): check status: it might be either corrupted or incomplete input.
        return kInternalError;
    }
    // TODO(eustas): currently it is supposed that complete input is accessible;
    //               when streaming support is added JXL_DEC_NEED_MORE_INPUT would also
    //               become a legal outcome; amount of decoded scanlines should be calculated
    //               based on callback invocations / render-pipeline API.
    *rowsDecodedPtr = dstInfo.height();

    return kSuccess;
}

bool SkJpegxlCodec::onRewind() {
    JxlDecoderRewind(fCodec->fDecoder.get());
    return true;
}

bool SkJpegxlCodec::conversionSupported(const SkImageInfo& dstInfo, bool srcIsOpaque,
                                        bool needsColorXform) {
    fCodec->fDstColorType = dstInfo.colorType();
    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            return true;  // memcpy
        case kBGRA_8888_SkColorType:
            return true;  // rgba->bgra

        case kRGBA_F16_SkColorType:
            SkASSERT(needsColorXform);  // TODO(eustas): not necessary for JXL.
            return true;  // memcpy

        // TODO(eustas): implement
        case kRGB_565_SkColorType:
            return false;
        case kGray_8_SkColorType:
            return false;
        case kAlpha_8_SkColorType:
            return false;

        default:
            return false;
    }
    return true;
}

void SkJpegxlCodec::imageOutCallback(void* opaque, size_t x, size_t y,
                                     size_t num_pixels, const void* pixels) {
    SkJpegxlCodec* instance = reinterpret_cast<SkJpegxlCodec*>(opaque);
    auto& codec = *instance->fCodec.get();
    size_t offset = y * codec.fRowBytes + (x << codec.fPixelShift);
    void* dst = SkTAddOffset<void>(codec.fDst, offset);
    if (instance->colorXform()) {
        instance->applyColorXform(dst, pixels, num_pixels);
        return;
    }
    switch (codec.fDstColorType) {
        case kRGBA_8888_SkColorType:
            memcpy(dst, pixels, 4 * num_pixels);
            return;
        case kBGRA_8888_SkColorType:
            SkOpts::RGBA_to_bgrA((uint32_t*) dst, (const uint32_t*)(pixels), num_pixels);
            return;
        case kRGBA_F16_SkColorType:
            memcpy(dst, pixels, 8 * num_pixels);
            return;
        default:
            SK_ABORT("Selected output format is not supported yet");
            return;
    }
}

bool SkJpegxlCodec::scanFrames() {
    auto decoder = JxlDecoderMake(/* memory_manager = */ nullptr);
    JxlDecoder* dec = decoder.get();
    auto* frameHolder = fCodec.get();
    auto& frames = frameHolder->fFrames;
    const auto& info = fCodec->fInfo;
    frames.clear();

    auto alpha = (info.alpha_bits != 0) ? SkEncodedInfo::Alpha::kUnpremul_Alpha
                                        : SkEncodedInfo::Alpha::kOpaque_Alpha;

    auto status = JxlDecoderSubscribeEvents(dec, JXL_DEC_FRAME);
    if (status != JXL_DEC_SUCCESS) {
        // Fresh instance must accept request for subscription.
        SkDEBUGFAIL("libjxl returned unexpected status");
        return true;
    }

    status = JxlDecoderSetInput(dec, fData->bytes(), fData->size());
    if (status != JXL_DEC_SUCCESS) {
        // Fresh instance must accept first input chunk.
        SkDEBUGFAIL("libjxl returned unexpected status");
        return true;
    }

    while (true) {
        status = JxlDecoderProcessInput(dec);
        switch (status) {
            case JXL_DEC_FRAME: {
                size_t frameId = frames.size();
                JxlFrameHeader frameHeader;
                if (JxlDecoderGetFrameHeader(dec, &frameHeader) != JXL_DEC_SUCCESS) {
                    return true;
                }
                frames.emplace_back(static_cast<int>(frameId), alpha);
                auto& frame = frames.back();
                // TODO(eustas): for better consistency we need to track total duration and report
                //               frame duration as delta to previous frame.
                int duration = (1000 * frameHeader.duration * info.animation.tps_denominator) /
                               info.animation.tps_numerator;
                frame.setDuration(duration);
                frameHolder->setAlphaAndRequiredFrame(&frame);
                break;
            }
            case JXL_DEC_SUCCESS: {
                return true;
            }
            default: {
                return false;
            }
        }
    }
}

int SkJpegxlCodec::onGetFrameCount() {
    if (!fCodec->fInfo.have_animation) {
        return 1;
    }

    if (!fCodec->fSeenAllFrames) {
        fCodec->fSeenAllFrames = scanFrames();
    }

    return fCodec->fFrames.size();
}

bool SkJpegxlCodec::onGetFrameInfo(int index, FrameInfo* frameInfo) const {
    if (index < 0) {
        return false;
    }
    if (static_cast<size_t>(index) >= fCodec->fFrames.size()) {
        return false;
    }
    fCodec->fFrames[index].fillIn(frameInfo, true);
    return true;
}

int SkJpegxlCodec::onGetRepetitionCount() {
    JxlBasicInfo& info = fCodec->fInfo;
    if (!info.have_animation) {
        return 0;
    }

    if (info.animation.num_loops == 0) {
        return kRepetitionCountInfinite;
    }

    if (SkTFitsIn<int>(info.animation.num_loops)) {
        return info.animation.num_loops - 1;
    }

    // Largest "non-infinite" value.
    return std::numeric_limits<int>::max();
}

const SkFrameHolder* SkJpegxlCodec::getFrameHolder() const {
    return fCodec.get();
}

// TODO(eustas): implement
// SkCodec::Result SkJpegxlCodec::onStartScanlineDecode(
//     const SkImageInfo& /*dstInfo*/, const Options& /*options*/) { return kUnimplemented; }

// TODO(eustas): implement
// SkCodec::Result SkJpegxlCodec::onStartIncrementalDecode(
//     const SkImageInfo& /*dstInfo*/, void*, size_t, const Options&) { return kUnimplemented; }

// TODO(eustas): implement
// SkCodec::Result SkJpegxlCodec::onIncrementalDecode(int*) { return kUnimplemented; }

// TODO(eustas): implement
// bool SkJpegxlCodec::onSkipScanlines(int /*countLines*/) { return false; }

// TODO(eustas): implement
// int SkJpegxlCodec::onGetScanlines(
//     void* /*dst*/, int /*countLines*/, size_t /*rowBytes*/) { return 0; }

// TODO(eustas): implement
// SkSampler* SkJpegxlCodec::getSampler(bool /*createIfNecessary*/) { return nullptr; }

namespace SkJpegxlDecoder {
bool IsJpegxl(const void* data, size_t len) {
    return SkJpegxlCodec::IsJpegxl(data, len);
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext) {
    SkCodec::Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }
    return SkJpegxlCodec::MakeFromStream(std::move(stream), outResult);
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
}  // namespace SkJpegDecoder

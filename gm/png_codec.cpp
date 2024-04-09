/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkSwizzlePriv.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/flags/CommonFlags.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

DEFINE_string(pngCodecGMImages,
              "",
              "Zero or more images or directories where to find PNG images to test with "
              "PNGCodecGM. Directories are scanned non-recursively. All files are assumed to be "
              "PNG images.");
DEFINE_string(pngCodecDecodeMode,
              "",
              "One of \"get-all-pixels\", \"incremental\" or \"zero-init\".");
DEFINE_string(pngCodecDstColorType,
              "",
              "One of \"force-grayscale\", "
              "\"force-nonnative-premul-color\" or \"get-from-canvas\".");
DEFINE_string(pngCodecDstAlphaType, "", "One of \"premul\" or \"unpremul\".");

static constexpr const char* sk_color_type_to_str(SkColorType colorType) {
    switch (colorType) {
        case kUnknown_SkColorType:
            return "kUnknown_SkColorType";
        case kAlpha_8_SkColorType:
            return "kAlpha_8_SkColorType";
        case kRGB_565_SkColorType:
            return "kRGB_565_SkColorType";
        case kARGB_4444_SkColorType:
            return "kARGB_4444_SkColorType";
        case kRGBA_8888_SkColorType:
            return "kRGBA_8888_SkColorType";
        case kRGB_888x_SkColorType:
            return "kRGB_888x_SkColorType";
        case kBGRA_8888_SkColorType:
            return "kBGRA_8888_SkColorType";
        case kRGBA_1010102_SkColorType:
            return "kRGBA_1010102_SkColorType";
        case kBGRA_1010102_SkColorType:
            return "kBGRA_1010102_SkColorType";
        case kRGB_101010x_SkColorType:
            return "kRGB_101010x_SkColorType";
        case kBGR_101010x_SkColorType:
            return "kBGR_101010x_SkColorType";
        case kBGR_101010x_XR_SkColorType:
            return "kBGR_101010x_XR_SkColorType";
        case kGray_8_SkColorType:
            return "kGray_8_SkColorType";
        case kRGBA_F16Norm_SkColorType:
            return "kRGBA_F16Norm_SkColorType";
        case kRGBA_F16_SkColorType:
            return "kRGBA_F16_SkColorType";
        case kRGBA_F32_SkColorType:
            return "kRGBA_F32_SkColorType";
        case kR8G8_unorm_SkColorType:
            return "kR8G8_unorm_SkColorType";
        case kA16_float_SkColorType:
            return "kA16_float_SkColorType";
        case kR16G16_float_SkColorType:
            return "kR16G16_float_SkColorType";
        case kA16_unorm_SkColorType:
            return "kA16_unorm_SkColorType";
        case kR16G16_unorm_SkColorType:
            return "kR16G16_unorm_SkColorType";
        case kR16G16B16A16_unorm_SkColorType:
            return "kR16G16B16A16_unorm_SkColorType";
        case kSRGBA_8888_SkColorType:
            return "kSRGBA_8888_SkColorType";
        case kR8_unorm_SkColorType:
            return "kR8_unorm_SkColorType";
        case kRGBA_10x6_SkColorType:
            return "kRGBA_10x6_SkColorType";
        case kBGRA_10101010_XR_SkColorType:
            return "kBGRA_10101010_XR_SkColorType";
    }
    SkUNREACHABLE;
}

static constexpr const char* sk_alpha_type_to_str(SkAlphaType alphaType) {
    switch (alphaType) {
        case kUnknown_SkAlphaType:
            return "kUnknown_SkAlphaType";
        case kOpaque_SkAlphaType:
            return "kOpaque_SkAlphaType";
        case kPremul_SkAlphaType:
            return "kPremul_SkAlphaType";
        case kUnpremul_SkAlphaType:
            return "kUnpremul_SkAlphaType";
    }
    SkUNREACHABLE;
}

struct DecodeResult {
    std::unique_ptr<SkCodec> codec;
    std::string errorMsg;
};

static DecodeResult decode(std::string path) {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
    if (!encoded) {
        return {.errorMsg = SkStringPrintf("Could not read \"%s\".", path.c_str()).c_str()};
    }
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkPngDecoder::Decode(SkMemoryStream::Make(encoded), &result);
    if (result != SkCodec::Result::kSuccess) {
        return {.errorMsg = SkStringPrintf("Could not create codec for \"%s\": %s.",
                                           path.c_str(),
                                           SkCodec::ResultToString(result))
                                    .c_str()};
    }
    return {.codec = std::move(codec)};
}

// This GM implements the PNG-related behaviors found in DM's CodecSrc class. It takes a single
// image as an argument and applies the same logic as CodecSrc.
//
// See the CodecSrc class here:
// https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.h#158.
class PNGCodecGM : public skiagm::GM {
public:
    // Based on CodecSrc::Mode.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.h#160
    enum class DecodeMode {
        kGetAllPixels,
        kIncremental,
        kZeroInit,
    };

    // Based on CodecSrc::DstColorType.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.h#172
    enum class DstColorType {
        kForceGrayscale,
        kForceNonNativePremulColor,
        kGetFromCanvas,
    };

    static constexpr const char* DecodeModeToString(DecodeMode decodeMode) {
        switch (decodeMode) {
            case DecodeMode::kGetAllPixels:
                return "kGetAllPixels";
            case DecodeMode::kIncremental:
                return "kIncremental";
            case DecodeMode::kZeroInit:
                return "kZeroInit";
        }
        SkUNREACHABLE;
    }

    static constexpr const char* DstColorTypeToString(DstColorType dstColorType) {
        switch (dstColorType) {
            case DstColorType::kForceGrayscale:
                return "kForceGrayscale";
            case DstColorType::kForceNonNativePremulColor:
                return "kForceNonNativePremulColor";
            case DstColorType::kGetFromCanvas:
                return "kGetFromCanvas";
        }
        SkUNREACHABLE;
    }

    // Based on DM's CodecSrc::CodecSrc().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#371
    PNGCodecGM(std::string path,
               DecodeMode decodeMode,
               DstColorType dstColorType,
               SkAlphaType dstAlphaType)
            : skiagm::GM()
            , fPath(path)
            , fDecodeMode(decodeMode)
            , fDstColorType(dstColorType)
            , fDstAlphaType(dstAlphaType) {}

    bool isBazelOnly() const override {
        // This GM class overlaps with DM's CodecSrc and related sources.
        return true;
    }

    std::map<std::string, std::string> getGoldKeys() const override {
        return std::map<std::string, std::string>{
                {"name", getName().c_str()},
                {"source_type", "image"},
                {"decode_mode", DecodeModeToString(fDecodeMode)},
                {"dst_color_type", DstColorTypeToString(fDstColorType)},
                {"dst_alpha_type", sk_alpha_type_to_str(fDstAlphaType)},
        };
    }

protected:
    // Based on CodecSrc::name().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#828
    SkString getName() const override {
        SkString name = SkOSPath::Basename(fPath.c_str());
        return name;
    }

    // Based on CodecSrc::size().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#803
    SkISize getISize() override {
        DecodeResult decodeResult = decode(fPath);
        if (decodeResult.errorMsg != "") {
            return {0, 0};
        }
        return decodeResult.codec->dimensions();
    }

    // Based on CodecSrc::draw().
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#450
    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        DecodeResult decodeResult = decode(fPath);
        if (decodeResult.errorMsg != "") {
            *errorMsg = decodeResult.errorMsg.c_str();
            return DrawResult::kFail;
        }
        std::unique_ptr<SkCodec> codec = std::move(decodeResult.codec);

        SkImageInfo decodeInfo = codec->getInfo();
        if (*errorMsg = validateCanvasColorTypeAndGetDecodeInfo(&decodeInfo,
                                                                canvas->imageInfo().colorType());
            *errorMsg != SkString()) {
            return DrawResult::kFail;
        }

        SkISize size = codec->dimensions();
        decodeInfo = decodeInfo.makeDimensions(size);

        const int bpp = decodeInfo.bytesPerPixel();
        const size_t rowBytes = size.width() * bpp;
        const size_t safeSize = decodeInfo.computeByteSize(rowBytes);
        SkAutoMalloc pixels(safeSize);

        SkCodec::Options options;
        if (DecodeMode::kZeroInit == fDecodeMode) {
            memset(pixels.get(), 0, size.height() * rowBytes);
            options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;
        }

        // For codec srcs, we want the "draw" step to be a memcpy.  Any interesting color space or
        // color format conversions should be performed by the codec.  Sometimes the output of the
        // decode will be in an interesting color space.  On our srgb and f16 backends, we need to
        // "pretend" that the color space is standard sRGB to avoid triggering color conversion
        // at draw time.
        SkImageInfo bitmapInfo = decodeInfo.makeColorSpace(SkColorSpace::MakeSRGB());

        if (kRGBA_8888_SkColorType == decodeInfo.colorType() ||
            kBGRA_8888_SkColorType == decodeInfo.colorType()) {
            bitmapInfo = bitmapInfo.makeColorType(kN32_SkColorType);
        }

        switch (fDecodeMode) {
            case DecodeMode::kZeroInit:
            case DecodeMode::kGetAllPixels: {
                switch (codec->getPixels(decodeInfo, pixels.get(), rowBytes, &options)) {
                    case SkCodec::kSuccess:
                        // We consider these to be valid, since we should still decode what is
                        // available.
                    case SkCodec::kErrorInInput:
                    case SkCodec::kIncompleteInput:
                        break;
                    default:
                        // Everything else is considered a failure.
                        *errorMsg = SkStringPrintf("Couldn't getPixels %s.", fPath.c_str());
                        return DrawResult::kFail;
                }

                drawToCanvas(canvas, bitmapInfo, pixels.get(), rowBytes);
                break;
            }
            case DecodeMode::kIncremental: {
                void* dst = pixels.get();
                uint32_t height = decodeInfo.height();
                if (SkCodec::kSuccess ==
                    codec->startIncrementalDecode(decodeInfo, dst, rowBytes, &options)) {
                    int rowsDecoded;
                    auto result = codec->incrementalDecode(&rowsDecoded);
                    if (SkCodec::kIncompleteInput == result || SkCodec::kErrorInInput == result) {
                        codec->fillIncompleteImage(decodeInfo,
                                                   dst,
                                                   rowBytes,
                                                   SkCodec::kNo_ZeroInitialized,
                                                   height,
                                                   rowsDecoded);
                    }
                } else {
                    *errorMsg = "Could not start incremental decode";
                    return DrawResult::kFail;
                }
                drawToCanvas(canvas, bitmapInfo, dst, rowBytes);
                break;
            }
            default:
                SkASSERT(false);
                *errorMsg = "Invalid fDecodeMode";
                return DrawResult::kFail;
        }
        return DrawResult::kOk;
    }

private:
    // Checks that the canvas color type, destination color and alpha types and input image
    // constitute an interesting test case, and constructs the SkImageInfo to use when decoding the
    // image.
    //
    // Based on DM's get_decode_info() function.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#398
    SkString validateCanvasColorTypeAndGetDecodeInfo(SkImageInfo* decodeInfo,
                                                     SkColorType canvasColorType) {
        switch (fDstColorType) {
            case DstColorType::kForceGrayscale:
                if (kRGB_565_SkColorType == canvasColorType) {
                    return SkStringPrintf(
                            "canvas color type %s and destination color type %s are redundant",
                            sk_color_type_to_str(canvasColorType),
                            DstColorTypeToString(fDstColorType));
                }
                *decodeInfo = decodeInfo->makeColorType(kGray_8_SkColorType);
                break;

            case DstColorType::kForceNonNativePremulColor:
                if (kRGB_565_SkColorType == canvasColorType ||
                    kRGBA_F16_SkColorType == canvasColorType) {
                    return SkStringPrintf(
                            "canvas color type %s and destination color type %s are redundant",
                            sk_color_type_to_str(canvasColorType),
                            DstColorTypeToString(fDstColorType));
                }
#ifdef SK_PMCOLOR_IS_RGBA
                *decodeInfo = decodeInfo->makeColorType(kBGRA_8888_SkColorType);
#else
                *decodeInfo = decodeInfo->makeColorType(kRGBA_8888_SkColorType);
#endif
                break;

            case DstColorType::kGetFromCanvas:
                if (kRGB_565_SkColorType == canvasColorType &&
                    kOpaque_SkAlphaType != decodeInfo->alphaType()) {
                    return SkStringPrintf(
                            "image \"%s\" has alpha type %s; this is incompatible with with "
                            "canvas color type %s and destination color type %s",
                            fPath.c_str(),
                            sk_alpha_type_to_str(decodeInfo->alphaType()),
                            sk_color_type_to_str(canvasColorType),
                            DstColorTypeToString(fDstColorType));
                }
                *decodeInfo = decodeInfo->makeColorType(canvasColorType);
                break;

            default:
                SkUNREACHABLE;
        }

        *decodeInfo = decodeInfo->makeAlphaType(fDstAlphaType);
        return SkString();
    }

    // Based on DM's draw_to_canvas() function.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#432
    void drawToCanvas(SkCanvas* canvas,
                      const SkImageInfo& info,
                      void* pixels,
                      size_t rowBytes,
                      SkScalar left = 0,
                      SkScalar top = 0) {
        SkBitmap bitmap;
        bitmap.installPixels(info, pixels, rowBytes);
        swapRbIfNecessary(bitmap);
        canvas->drawImage(bitmap.asImage(), left, top);
    }

    // Allows us to test decodes to non-native 8888.
    //
    // Based on DM's swap_rb_if_necessary function.
    // https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DMSrcSink.cpp#387
    void swapRbIfNecessary(SkBitmap& bitmap) {
        if (DstColorType::kForceNonNativePremulColor != fDstColorType) {
            return;
        }

        for (int y = 0; y < bitmap.height(); y++) {
            uint32_t* row = (uint32_t*)bitmap.getAddr(0, y);
            SkOpts::RGBA_to_BGRA(row, row, bitmap.width());
        }
    }

    std::string fPath;
    DecodeMode fDecodeMode;
    DstColorType fDstColorType;
    SkAlphaType fDstAlphaType;
};

// Registers GMs with zero or more PNGCodecGM instances for the given image. Returns a non-empty,
// human-friendly error message in the case of errors.
//
// Based on DM's push_codec_srcs() function. It only covers "simple" codecs (lines 740-834).
// https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DM.cpp#740
//
// Specifically, this function does not capture any behaviors found in the following DM classes:
//
//  - AndroidCodecSrc
//  - BRDSrc
//  - ImageGenSrc
//
// TODO(lovisolo): Implement the above sources as GMs (if necessary).
static std::string registerGMsForImage(std::string path,
                                       PNGCodecGM::DecodeMode decodeMode,
                                       PNGCodecGM::DstColorType dstColorType,
                                       SkAlphaType dstAlphaType) {
    DecodeResult decodeResult = decode(path);
    if (decodeResult.errorMsg != "") {
        return decodeResult.errorMsg;
    }

    if (dstColorType == PNGCodecGM::DstColorType::kForceGrayscale &&
        decodeResult.codec->getInfo().colorType() != kGray_8_SkColorType) {
        return SkStringPrintf(
                       "image \"%s\" has color type %s; this is incompatible with the given "
                       "dstColorType argument: %s (expected image color type: %s)",
                       path.c_str(),
                       sk_color_type_to_str(decodeResult.codec->getInfo().colorType()),
                       PNGCodecGM::DstColorTypeToString(PNGCodecGM::DstColorType::kForceGrayscale),
                       sk_color_type_to_str(kGray_8_SkColorType))
                .c_str();
    }

    if (dstAlphaType == kUnpremul_SkAlphaType &&
        decodeResult.codec->getInfo().alphaType() == kOpaque_SkAlphaType) {
        return SkStringPrintf(
                       "image \"%s\" has alpha type %s; this is incompatible with the given "
                       "dstAlphaType argument: %s",
                       path.c_str(),
                       sk_alpha_type_to_str(kOpaque_SkAlphaType),
                       sk_alpha_type_to_str(kUnpremul_SkAlphaType))
                .c_str();
    }

    skiagm::Register(new PNGCodecGM(path, decodeMode, dstColorType, dstAlphaType));
    return "";
}

// Returns a non-empty message in the case of errors.
static std::string parse_and_validate_flags(PNGCodecGM::DecodeMode* decodeMode,
                                            PNGCodecGM::DstColorType* dstColorType,
                                            SkAlphaType* dstAlphaType) {
    skia_private::THashMap<SkString, PNGCodecGM::DecodeMode> decodeModeValues = {
            {SkString("get-all-pixels"), PNGCodecGM::DecodeMode::kGetAllPixels},
            {SkString("incremental"), PNGCodecGM::DecodeMode::kIncremental},
            {SkString("zero-init"), PNGCodecGM::DecodeMode::kZeroInit},
    };
    if (SkString errorMsg = FLAGS_pngCodecDecodeMode.parseAndValidate(
                "--pngCodecDecodeMode", decodeModeValues, decodeMode);
        errorMsg != SkString()) {
        return errorMsg.c_str();
    }

    skia_private::THashMap<SkString, PNGCodecGM::DstColorType> dstColorTypeValues = {
            {SkString("get-from-canvas"), PNGCodecGM::DstColorType::kGetFromCanvas},
            {SkString("force-grayscale"), PNGCodecGM::DstColorType::kForceGrayscale},
            {SkString("force-nonnative-premul-color"),
             PNGCodecGM::DstColorType::kForceNonNativePremulColor},
    };
    if (SkString errorMsg = FLAGS_pngCodecDstColorType.parseAndValidate(
                "--pngCodecDstColorType", dstColorTypeValues, dstColorType);
        errorMsg != SkString()) {
        return errorMsg.c_str();
    }

    skia_private::THashMap<SkString, SkAlphaType> dstAlphaTypeValues = {
            {SkString("premul"), kPremul_SkAlphaType},
            {SkString("unpremul"), kUnpremul_SkAlphaType},
    };
    if (SkString errorMsg = FLAGS_pngCodecDstAlphaType.parseAndValidate(
                "--pngCodecDstAlphaType", dstAlphaTypeValues, dstAlphaType);
        errorMsg != SkString()) {
        return errorMsg.c_str();
    }

    return "";
}

// Registers one PNGCodecGM instance for each image passed via the --pngCodecGMImages flag, which
// can take files and directories. Directories are scanned non-recursively.
//
// Based on DM's gather_srcs() function.
// https://skia.googlesource.com/skia/+/ce49fc71bc7cc25244020cd3e64764a6d08e54fb/dm/DM.cpp#953
DEF_GM_REGISTERER_FN([]() -> std::string {
    // Parse flags.
    PNGCodecGM::DecodeMode decodeMode;
    PNGCodecGM::DstColorType dstColorType;
    SkAlphaType dstAlphaType;
    if (std::string errorMsg = parse_and_validate_flags(&decodeMode, &dstColorType, &dstAlphaType);
        errorMsg != "") {
        return errorMsg;
    }

    // Collect images.
    skia_private::TArray<SkString> images;
    if (!CommonFlags::CollectImages(FLAGS_pngCodecGMImages, &images)) {
        return "Failed to collect images.";
    }

    // Register one GM per image.
    for (const SkString& image : images) {
        if (std::string errorMsg =
                    registerGMsForImage(image.c_str(), decodeMode, dstColorType, dstAlphaType);
            errorMsg != "") {
            return errorMsg;
        }
    }

    return "";
});

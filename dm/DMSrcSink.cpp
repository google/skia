/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMSrcSink.h"
#include "Resources.h"
#include "SkAndroidCodec.h"
#include "SkAutoMalloc.h"
#include "SkBase64.h"
#include "SkCodec.h"
#include "SkCodecImageGenerator.h"
#include "SkColorSpace.h"
#include "SkColorSpaceXform.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkColorSpace_XYZ.h"
#include "SkCommonFlags.h"
#include "SkCommonFlagsGpu.h"
#include "SkData.h"
#include "SkDebugCanvas.h"
#include "SkDeferredDisplayListRecorder.h"
#include "SkDocument.h"
#include "SkExecutor.h"
#include "SkImageGenerator.h"
#include "SkImageGeneratorCG.h"
#include "SkImageGeneratorWIC.h"
#include "SkLiteDL.h"
#include "SkLiteRecorder.h"
#include "SkMallocPixelRef.h"
#include "SkMultiPictureDocumentPriv.h"
#include "SkMultiPictureDraw.h"
#include "SkNullCanvas.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkOpts.h"
#include "SkPictureCommon.h"
#include "SkPictureData.h"
#include "SkPictureRecorder.h"
#include "SkPipe.h"
#include "SkPngEncoder.h"
#include "SkRandom.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkSurfaceCharacterization.h"
#include "SkSVGCanvas.h"
#include "SkStream.h"
#include "SkSwizzler.h"
#include "SkTaskGroup.h"
#include "SkTLogic.h"
#include <cmath>
#include <functional>
#include "../src/jumper/SkJumper.h"

#if defined(SK_BUILD_FOR_WIN)
    #include "SkAutoCoInitialize.h"
    #include "SkHRESULT.h"
    #include "SkTScopedComPtr.h"
    #include <XpsObjectModel.h>
#endif

#if !defined(SK_BUILD_FOR_GOOGLE3)
    #include "Skottie.h"
#endif

#if defined(SK_XML)
    #include "SkSVGDOM.h"
    #include "SkXMLWriter.h"
#endif

DEFINE_bool(multiPage, false, "For document-type backends, render the source"
            " into multiple pages");
DEFINE_bool(RAW_threading, true, "Allow RAW decodes to run on multiple threads?");

using sk_gpu_test::GrContextFactory;

namespace DM {

GMSrc::GMSrc(skiagm::GMRegistry::Factory factory) : fFactory(factory) {}

Error GMSrc::draw(SkCanvas* canvas) const {
    std::unique_ptr<skiagm::GM> gm(fFactory(nullptr));
    gm->draw(canvas);
    return "";
}

SkISize GMSrc::size() const {
    std::unique_ptr<skiagm::GM> gm(fFactory(nullptr));
    return gm->getISize();
}

Name GMSrc::name() const {
    std::unique_ptr<skiagm::GM> gm(fFactory(nullptr));
    return gm->getName();
}

void GMSrc::modifyGrContextOptions(GrContextOptions* options) const {
    std::unique_ptr<skiagm::GM> gm(fFactory(nullptr));
    gm->modifyGrContextOptions(options);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

BRDSrc::BRDSrc(Path path, Mode mode, CodecSrc::DstColorType dstColorType, uint32_t sampleSize)
    : fPath(path)
    , fMode(mode)
    , fDstColorType(dstColorType)
    , fSampleSize(sampleSize)
{}

bool BRDSrc::veto(SinkFlags flags) const {
    // No need to test to non-raster or indirect backends.
    return flags.type != SinkFlags::kRaster
        || flags.approach != SinkFlags::kDirect;
}

static SkBitmapRegionDecoder* create_brd(Path path) {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
    if (!encoded) {
        return nullptr;
    }
    return SkBitmapRegionDecoder::Create(encoded, SkBitmapRegionDecoder::kAndroidCodec_Strategy);
}

static inline void alpha8_to_gray8(SkBitmap* bitmap) {
    // Android requires kGray8 bitmaps to be tagged as kAlpha8.  Here we convert
    // them back to kGray8 so our test framework can draw them correctly.
    if (kAlpha_8_SkColorType == bitmap->info().colorType()) {
        SkImageInfo newInfo = bitmap->info().makeColorType(kGray_8_SkColorType)
                                            .makeAlphaType(kOpaque_SkAlphaType);
        *const_cast<SkImageInfo*>(&bitmap->info()) = newInfo;
    }
}

Error BRDSrc::draw(SkCanvas* canvas) const {
    if (canvas->imageInfo().colorSpace() &&
            kRGBA_F16_SkColorType != canvas->imageInfo().colorType()) {
        // SkAndroidCodec uses legacy premultiplication and blending.  Therefore, we only
        // run these tests on legacy canvases.
        // We allow an exception for F16, since Android uses F16.
        return Error::Nonfatal("Skip testing to color correct canvas.");
    }

    SkColorType colorType = canvas->imageInfo().colorType();
    if (kRGB_565_SkColorType == colorType &&
            CodecSrc::kGetFromCanvas_DstColorType != fDstColorType) {
        return Error::Nonfatal("Testing non-565 to 565 is uninteresting.");
    }
    switch (fDstColorType) {
        case CodecSrc::kGetFromCanvas_DstColorType:
            break;
        case CodecSrc::kGrayscale_Always_DstColorType:
            colorType = kGray_8_SkColorType;
            break;
        default:
            SkASSERT(false);
            break;
    }

    std::unique_ptr<SkBitmapRegionDecoder> brd(create_brd(fPath));
    if (nullptr == brd.get()) {
        return Error::Nonfatal(SkStringPrintf("Could not create brd for %s.", fPath.c_str()));
    }

    if (kRGB_565_SkColorType == colorType) {
        auto recommendedCT = brd->computeOutputColorType(colorType);
        if (recommendedCT != colorType) {
            return Error::Nonfatal("Skip decoding non-opaque to 565.");
        }
    }

    const uint32_t width = brd->width();
    const uint32_t height = brd->height();
    // Visually inspecting very small output images is not necessary.
    if ((width / fSampleSize <= 10 || height / fSampleSize <= 10) && 1 != fSampleSize) {
        return Error::Nonfatal("Scaling very small images is uninteresting.");
    }
    switch (fMode) {
        case kFullImage_Mode: {
            SkBitmap bitmap;
            if (!brd->decodeRegion(&bitmap, nullptr, SkIRect::MakeXYWH(0, 0, width, height),
                    fSampleSize, colorType, false, SkColorSpace::MakeSRGB())) {
                return "Cannot decode (full) region.";
            }
            alpha8_to_gray8(&bitmap);

            canvas->drawBitmap(bitmap, 0, 0);
            return "";
        }
        case kDivisor_Mode: {
            const uint32_t divisor = 2;
            if (width < divisor || height < divisor) {
                return Error::Nonfatal("Divisor is larger than image dimension.");
            }

            // Use a border to test subsets that extend outside the image.
            // We will not allow the border to be larger than the image dimensions.  Allowing
            // these large borders causes off by one errors that indicate a problem with the
            // test suite, not a problem with the implementation.
            const uint32_t maxBorder = SkTMin(width, height) / (fSampleSize * divisor);
            const uint32_t scaledBorder = SkTMin(5u, maxBorder);
            const uint32_t unscaledBorder = scaledBorder * fSampleSize;

            // We may need to clear the canvas to avoid uninitialized memory.
            // Assume we are scaling a 780x780 image with sampleSize = 8.
            // The output image should be 97x97.
            // Each subset will be 390x390.
            // Each scaled subset be 48x48.
            // Four scaled subsets will only fill a 96x96 image.
            // The bottom row and last column will not be touched.
            // This is an unfortunate result of our rounding rules when scaling.
            // Maybe we need to consider testing scaled subsets without trying to
            // combine them to match the full scaled image?  Or maybe this is the
            // best we can do?
            canvas->clear(0);

            for (uint32_t x = 0; x < divisor; x++) {
                for (uint32_t y = 0; y < divisor; y++) {
                    // Calculate the subset dimensions
                    uint32_t subsetWidth = width / divisor;
                    uint32_t subsetHeight = height / divisor;
                    const int left = x * subsetWidth;
                    const int top = y * subsetHeight;

                    // Increase the size of the last subset in each row or column, when the
                    // divisor does not divide evenly into the image dimensions
                    subsetWidth += (x + 1 == divisor) ? (width % divisor) : 0;
                    subsetHeight += (y + 1 == divisor) ? (height % divisor) : 0;

                    // Increase the size of the subset in order to have a border on each side
                    const int decodeLeft = left - unscaledBorder;
                    const int decodeTop = top - unscaledBorder;
                    const uint32_t decodeWidth = subsetWidth + unscaledBorder * 2;
                    const uint32_t decodeHeight = subsetHeight + unscaledBorder * 2;
                    SkBitmap bitmap;
                    if (!brd->decodeRegion(&bitmap, nullptr, SkIRect::MakeXYWH(decodeLeft,
                            decodeTop, decodeWidth, decodeHeight), fSampleSize, colorType, false,
                            SkColorSpace::MakeSRGB())) {
                        return "Cannot decode region.";
                    }

                    alpha8_to_gray8(&bitmap);
                    canvas->drawBitmapRect(bitmap,
                            SkRect::MakeXYWH((SkScalar) scaledBorder, (SkScalar) scaledBorder,
                                    (SkScalar) (subsetWidth / fSampleSize),
                                    (SkScalar) (subsetHeight / fSampleSize)),
                            SkRect::MakeXYWH((SkScalar) (left / fSampleSize),
                                    (SkScalar) (top / fSampleSize),
                                    (SkScalar) (subsetWidth / fSampleSize),
                                    (SkScalar) (subsetHeight / fSampleSize)),
                            nullptr);
                }
            }
            return "";
        }
        default:
            SkASSERT(false);
            return "Error: Should not be reached.";
    }
}

SkISize BRDSrc::size() const {
    std::unique_ptr<SkBitmapRegionDecoder> brd(create_brd(fPath));
    if (brd) {
        return {SkTMax(1, brd->width() / (int)fSampleSize),
                SkTMax(1, brd->height() / (int)fSampleSize)};
    }
    return {0, 0};
}

static SkString get_scaled_name(const Path& path, float scale) {
    return SkStringPrintf("%s_%.3f", SkOSPath::Basename(path.c_str()).c_str(), scale);
}

Name BRDSrc::name() const {
    // We will replicate the names used by CodecSrc so that images can
    // be compared in Gold.
    if (1 == fSampleSize) {
        return SkOSPath::Basename(fPath.c_str());
    }
    return get_scaled_name(fPath, 1.0f / (float) fSampleSize);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static bool serial_from_path_name(const SkString& path) {
    if (!FLAGS_RAW_threading) {
        static const char* const exts[] = {
            "arw", "cr2", "dng", "nef", "nrw", "orf", "raf", "rw2", "pef", "srw",
            "ARW", "CR2", "DNG", "NEF", "NRW", "ORF", "RAF", "RW2", "PEF", "SRW",
        };
        const char* actualExt = strrchr(path.c_str(), '.');
        if (actualExt) {
            actualExt++;
            for (auto* ext : exts) {
                if (0 == strcmp(ext, actualExt)) {
                    return true;
                }
            }
        }
    }
    return false;
}

CodecSrc::CodecSrc(Path path, Mode mode, DstColorType dstColorType, SkAlphaType dstAlphaType,
                   float scale)
    : fPath(path)
    , fMode(mode)
    , fDstColorType(dstColorType)
    , fDstAlphaType(dstAlphaType)
    , fScale(scale)
    , fRunSerially(serial_from_path_name(path))
{}

bool CodecSrc::veto(SinkFlags flags) const {
    // Test to direct raster backends (8888 and 565).
    return flags.type != SinkFlags::kRaster || flags.approach != SinkFlags::kDirect;
}

// Allows us to test decodes to non-native 8888.
static void swap_rb_if_necessary(SkBitmap& bitmap, CodecSrc::DstColorType dstColorType) {
    if (CodecSrc::kNonNative8888_Always_DstColorType != dstColorType) {
        return;
    }

    for (int y = 0; y < bitmap.height(); y++) {
        uint32_t* row = (uint32_t*) bitmap.getAddr(0, y);
        SkOpts::RGBA_to_BGRA(row, row, bitmap.width());
    }
}

// FIXME: Currently we cannot draw unpremultiplied sources. skbug.com/3338 and skbug.com/3339.
// This allows us to still test unpremultiplied decodes.
static void premultiply_if_necessary(SkBitmap& bitmap) {
    if (kUnpremul_SkAlphaType != bitmap.alphaType()) {
        return;
    }

    switch (bitmap.colorType()) {
        case kRGBA_F16_SkColorType: {
            SkJumper_MemoryCtx ctx = { bitmap.getAddr(0,0), bitmap.rowBytesAsPixels() };
            SkRasterPipeline_<256> p;
            p.append(SkRasterPipeline::load_f16, &ctx);
            p.append(SkRasterPipeline::premul);
            p.append(SkRasterPipeline::store_f16, &ctx);
            p.run(0,0, bitmap.width(), bitmap.height());
        }
            break;
        case kN32_SkColorType:
            for (int y = 0; y < bitmap.height(); y++) {
                uint32_t* row = (uint32_t*) bitmap.getAddr(0, y);
                SkOpts::RGBA_to_rgbA(row, row, bitmap.width());
            }
            break;
        default:
            // No need to premultiply kGray or k565 outputs.
            break;
    }

    // In the kIndex_8 case, the canvas won't even try to draw unless we mark the
    // bitmap as kPremul.
    bitmap.setAlphaType(kPremul_SkAlphaType);
}

static bool get_decode_info(SkImageInfo* decodeInfo, SkColorType canvasColorType,
                            CodecSrc::DstColorType dstColorType, SkAlphaType dstAlphaType) {
    switch (dstColorType) {
        case CodecSrc::kGrayscale_Always_DstColorType:
            if (kRGB_565_SkColorType == canvasColorType) {
                return false;
            }
            *decodeInfo = decodeInfo->makeColorType(kGray_8_SkColorType);
            break;
        case CodecSrc::kNonNative8888_Always_DstColorType:
            if (kRGB_565_SkColorType == canvasColorType
                    || kRGBA_F16_SkColorType == canvasColorType) {
                return false;
            }
#ifdef SK_PMCOLOR_IS_RGBA
            *decodeInfo = decodeInfo->makeColorType(kBGRA_8888_SkColorType);
#else
            *decodeInfo = decodeInfo->makeColorType(kRGBA_8888_SkColorType);
#endif
            break;
        default:
            if (kRGB_565_SkColorType == canvasColorType &&
                    kOpaque_SkAlphaType != decodeInfo->alphaType()) {
                return false;
            }

            if (kRGBA_F16_SkColorType == canvasColorType) {
                sk_sp<SkColorSpace> linearSpace = decodeInfo->colorSpace()->makeLinearGamma();
                *decodeInfo = decodeInfo->makeColorSpace(std::move(linearSpace));
            }

            *decodeInfo = decodeInfo->makeColorType(canvasColorType);
            break;
    }

    *decodeInfo = decodeInfo->makeAlphaType(dstAlphaType);
    return true;
}

static void draw_to_canvas(SkCanvas* canvas, const SkImageInfo& info, void* pixels, size_t rowBytes,
                           CodecSrc::DstColorType dstColorType,
                           SkScalar left = 0, SkScalar top = 0) {
    SkBitmap bitmap;
    bitmap.installPixels(info, pixels, rowBytes);
    premultiply_if_necessary(bitmap);
    swap_rb_if_necessary(bitmap, dstColorType);
    canvas->drawBitmap(bitmap, left, top);
}

// For codec srcs, we want the "draw" step to be a memcpy.  Any interesting color space or
// color format conversions should be performed by the codec.  Sometimes the output of the
// decode will be in an interesting color space.  On our srgb and f16 backends, we need to
// "pretend" that the color space is standard sRGB to avoid triggering color conversion
// at draw time.
static void set_bitmap_color_space(SkImageInfo* info) {
    if (kRGBA_F16_SkColorType == info->colorType()) {
        *info = info->makeColorSpace(SkColorSpace::MakeSRGBLinear());
    } else {
        *info = info->makeColorSpace(SkColorSpace::MakeSRGB());
    }
}

Error CodecSrc::draw(SkCanvas* canvas) const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec.get()) {
        return SkStringPrintf("Couldn't create codec for %s.", fPath.c_str());
    }

    SkImageInfo decodeInfo = codec->getInfo();
    if (!get_decode_info(&decodeInfo, canvas->imageInfo().colorType(), fDstColorType,
                         fDstAlphaType)) {
        return Error::Nonfatal("Skipping uninteresting test.");
    }

    // Try to scale the image if it is desired
    SkISize size = codec->getScaledDimensions(fScale);
    if (size == decodeInfo.dimensions() && 1.0f != fScale) {
        return Error::Nonfatal("Test without scaling is uninteresting.");
    }

    // Visually inspecting very small output images is not necessary.  We will
    // cover these cases in unit testing.
    if ((size.width() <= 10 || size.height() <= 10) && 1.0f != fScale) {
        return Error::Nonfatal("Scaling very small images is uninteresting.");
    }
    decodeInfo = decodeInfo.makeWH(size.width(), size.height());

    const int bpp = SkColorTypeBytesPerPixel(decodeInfo.colorType());
    const size_t rowBytes = size.width() * bpp;
    const size_t safeSize = decodeInfo.computeByteSize(rowBytes);
    SkAutoMalloc pixels(safeSize);

    SkCodec::Options options;
    options.fPremulBehavior = canvas->imageInfo().colorSpace() ?
            SkTransferFunctionBehavior::kRespect : SkTransferFunctionBehavior::kIgnore;
    if (kCodecZeroInit_Mode == fMode) {
        memset(pixels.get(), 0, size.height() * rowBytes);
        options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;
    }

    SkImageInfo bitmapInfo = decodeInfo;
    set_bitmap_color_space(&bitmapInfo);
    if (kRGBA_8888_SkColorType == decodeInfo.colorType() ||
            kBGRA_8888_SkColorType == decodeInfo.colorType()) {
        bitmapInfo = bitmapInfo.makeColorType(kN32_SkColorType);
    }

    switch (fMode) {
        case kAnimated_Mode: {
            std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
            if (frameInfos.size() <= 1) {
                return SkStringPrintf("%s is not an animated image.", fPath.c_str());
            }

            // As in CodecSrc::size(), compute a roughly square grid to draw the frames
            // into. "factor" is the number of frames to draw on one row. There will be
            // up to "factor" rows as well.
            const float root = sqrt((float) frameInfos.size());
            const int factor = sk_float_ceil2int(root);

            // Used to cache a frame that future frames will depend on.
            SkAutoMalloc priorFramePixels;
            int cachedFrame = SkCodec::kNone;
            for (int i = 0; static_cast<size_t>(i) < frameInfos.size(); i++) {
                options.fFrameIndex = i;
                // Check for a prior frame
                const int reqFrame = frameInfos[i].fRequiredFrame;
                if (reqFrame != SkCodec::kNone && reqFrame == cachedFrame
                        && priorFramePixels.get()) {
                    // Copy into pixels
                    memcpy(pixels.get(), priorFramePixels.get(), safeSize);
                    options.fPriorFrame = reqFrame;
                } else {
                    options.fPriorFrame = SkCodec::kNone;
                }
                SkCodec::Result result = codec->getPixels(decodeInfo, pixels.get(),
                                                          rowBytes, &options);
                if (SkCodec::kInvalidInput == result && i > 0) {
                    // Some of our test images have truncated later frames. Treat that
                    // the same as incomplete.
                    result = SkCodec::kIncompleteInput;
                }
                switch (result) {
                    case SkCodec::kSuccess:
                    case SkCodec::kErrorInInput:
                    case SkCodec::kIncompleteInput: {
                        // If the next frame depends on this one, store it in priorFrame.
                        // It is possible that we may discard a frame that future frames depend on,
                        // but the codec will simply redecode the discarded frame.
                        // Do this before calling draw_to_canvas, which premultiplies in place. If
                        // we're decoding to unpremul, we want to pass the unmodified frame to the
                        // codec for decoding the next frame.
                        if (static_cast<size_t>(i+1) < frameInfos.size()
                                && frameInfos[i+1].fRequiredFrame == i) {
                            memcpy(priorFramePixels.reset(safeSize), pixels.get(), safeSize);
                            cachedFrame = i;
                        }

                        SkAutoCanvasRestore acr(canvas, true);
                        const int xTranslate = (i % factor) * decodeInfo.width();
                        const int yTranslate = (i / factor) * decodeInfo.height();
                        canvas->translate(SkIntToScalar(xTranslate), SkIntToScalar(yTranslate));
                        draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes, fDstColorType);
                        if (result != SkCodec::kSuccess) {
                            return "";
                        }
                        break;
                    }
                    case SkCodec::kInvalidConversion:
                        if (i > 0 && (decodeInfo.colorType() == kRGB_565_SkColorType)) {
                            return Error::Nonfatal(SkStringPrintf(
                                "Cannot decode frame %i to 565 (%s).", i, fPath.c_str()));
                        }
                        // Fall through.
                    default:
                        return SkStringPrintf("Couldn't getPixels for frame %i in %s.",
                                              i, fPath.c_str());
                }
            }
            break;
        }
        case kCodecZeroInit_Mode:
        case kCodec_Mode: {
            switch (codec->getPixels(decodeInfo, pixels.get(), rowBytes, &options)) {
                case SkCodec::kSuccess:
                    // We consider these to be valid, since we should still decode what is
                    // available.
                case SkCodec::kErrorInInput:
                case SkCodec::kIncompleteInput:
                    break;
                default:
                    // Everything else is considered a failure.
                    return SkStringPrintf("Couldn't getPixels %s.", fPath.c_str());
            }

            draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes, fDstColorType);
            break;
        }
        case kScanline_Mode: {
            void* dst = pixels.get();
            uint32_t height = decodeInfo.height();
            const bool useIncremental = [this]() {
                auto exts = { "png", "PNG", "gif", "GIF" };
                for (auto ext : exts) {
                    if (fPath.endsWith(ext)) {
                        return true;
                    }
                }
                return false;
            }();
            // ico may use the old scanline method or the new one, depending on whether it
            // internally holds a bmp or a png.
            const bool ico = fPath.endsWith("ico");
            bool useOldScanlineMethod = !useIncremental && !ico;
            if (useIncremental || ico) {
                if (SkCodec::kSuccess == codec->startIncrementalDecode(decodeInfo, dst,
                        rowBytes, &options)) {
                    int rowsDecoded;
                    auto result = codec->incrementalDecode(&rowsDecoded);
                    if (SkCodec::kIncompleteInput == result || SkCodec::kErrorInInput == result) {
                        codec->fillIncompleteImage(decodeInfo, dst, rowBytes,
                                                   SkCodec::kNo_ZeroInitialized, height,
                                                   rowsDecoded);
                    }
                } else {
                    if (useIncremental) {
                        // Error: These should support incremental decode.
                        return "Could not start incremental decode";
                    }
                    // Otherwise, this is an ICO. Since incremental failed, it must contain a BMP,
                    // which should work via startScanlineDecode
                    useOldScanlineMethod = true;
                }
            }

            if (useOldScanlineMethod) {
                if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo)) {
                    return "Could not start scanline decoder";
                }

                switch (codec->getScanlineOrder()) {
                    case SkCodec::kTopDown_SkScanlineOrder:
                    case SkCodec::kBottomUp_SkScanlineOrder:
                        // We do not need to check the return value.  On an incomplete
                        // image, memory will be filled with a default value.
                        codec->getScanlines(dst, height, rowBytes);
                        break;
                }
            }

            draw_to_canvas(canvas, bitmapInfo, dst, rowBytes, fDstColorType);
            break;
        }
        case kStripe_Mode: {
            const int height = decodeInfo.height();
            // This value is chosen arbitrarily.  We exercise more cases by choosing a value that
            // does not align with image blocks.
            const int stripeHeight = 37;
            const int numStripes = (height + stripeHeight - 1) / stripeHeight;
            void* dst = pixels.get();

            // Decode odd stripes
            if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo, &options)) {
                return "Could not start scanline decoder";
            }

            // This mode was designed to test the new skip scanlines API in libjpeg-turbo.
            // Jpegs have kTopDown_SkScanlineOrder, and at this time, it is not interesting
            // to run this test for image types that do not have this scanline ordering.
            // We only run this on Jpeg, which is always kTopDown.
            SkASSERT(SkCodec::kTopDown_SkScanlineOrder == codec->getScanlineOrder());

            for (int i = 0; i < numStripes; i += 2) {
                // Skip a stripe
                const int linesToSkip = SkTMin(stripeHeight, height - i * stripeHeight);
                codec->skipScanlines(linesToSkip);

                // Read a stripe
                const int startY = (i + 1) * stripeHeight;
                const int linesToRead = SkTMin(stripeHeight, height - startY);
                if (linesToRead > 0) {
                    codec->getScanlines(SkTAddOffset<void>(dst, rowBytes * startY), linesToRead,
                                        rowBytes);
                }
            }

            // Decode even stripes
            const SkCodec::Result startResult = codec->startScanlineDecode(decodeInfo);
            if (SkCodec::kSuccess != startResult) {
                return "Failed to restart scanline decoder with same parameters.";
            }
            for (int i = 0; i < numStripes; i += 2) {
                // Read a stripe
                const int startY = i * stripeHeight;
                const int linesToRead = SkTMin(stripeHeight, height - startY);
                codec->getScanlines(SkTAddOffset<void>(dst, rowBytes * startY), linesToRead,
                                    rowBytes);

                // Skip a stripe
                const int linesToSkip = SkTMin(stripeHeight, height - (i + 1) * stripeHeight);
                if (linesToSkip > 0) {
                    codec->skipScanlines(linesToSkip);
                }
            }

            draw_to_canvas(canvas, bitmapInfo, dst, rowBytes, fDstColorType);
            break;
        }
        case kCroppedScanline_Mode: {
            const int width = decodeInfo.width();
            const int height = decodeInfo.height();
            // This value is chosen because, as we move across the image, it will sometimes
            // align with the jpeg block sizes and it will sometimes not.  This allows us
            // to test interestingly different code paths in the implementation.
            const int tileSize = 36;
            SkIRect subset;
            for (int x = 0; x < width; x += tileSize) {
                subset = SkIRect::MakeXYWH(x, 0, SkTMin(tileSize, width - x), height);
                options.fSubset = &subset;
                if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo, &options)) {
                    return "Could not start scanline decoder.";
                }

                codec->getScanlines(SkTAddOffset<void>(pixels.get(), x * bpp), height, rowBytes);
            }

            draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes, fDstColorType);
            break;
        }
        case kSubset_Mode: {
            // Arbitrarily choose a divisor.
            int divisor = 2;
            // Total width/height of the image.
            const int W = codec->getInfo().width();
            const int H = codec->getInfo().height();
            if (divisor > W || divisor > H) {
                return Error::Nonfatal(SkStringPrintf("Cannot codec subset: divisor %d is too big "
                                                      "for %s with dimensions (%d x %d)", divisor,
                                                      fPath.c_str(), W, H));
            }
            // subset dimensions
            // SkWebpCodec, the only one that supports subsets, requires even top/left boundaries.
            const int w = SkAlign2(W / divisor);
            const int h = SkAlign2(H / divisor);
            SkIRect subset;
            options.fSubset = &subset;
            SkBitmap subsetBm;
            // We will reuse pixel memory from bitmap.
            void* dst = pixels.get();
            // Keep track of left and top (for drawing subsetBm into canvas). We could use
            // fScale * x and fScale * y, but we want integers such that the next subset will start
            // where the last one ended. So we'll add decodeInfo.width() and height().
            int left = 0;
            for (int x = 0; x < W; x += w) {
                int top = 0;
                for (int y = 0; y < H; y+= h) {
                    // Do not make the subset go off the edge of the image.
                    const int preScaleW = SkTMin(w, W - x);
                    const int preScaleH = SkTMin(h, H - y);
                    subset.setXYWH(x, y, preScaleW, preScaleH);
                    // And scale
                    // FIXME: Should we have a version of getScaledDimensions that takes a subset
                    // into account?
                    const int scaledW = SkTMax(1, SkScalarRoundToInt(preScaleW * fScale));
                    const int scaledH = SkTMax(1, SkScalarRoundToInt(preScaleH * fScale));
                    decodeInfo = decodeInfo.makeWH(scaledW, scaledH);
                    SkImageInfo subsetBitmapInfo = bitmapInfo.makeWH(scaledW, scaledH);
                    size_t subsetRowBytes = subsetBitmapInfo.minRowBytes();
                    const SkCodec::Result result = codec->getPixels(decodeInfo, dst, subsetRowBytes,
                            &options);
                    switch (result) {
                        case SkCodec::kSuccess:
                        case SkCodec::kErrorInInput:
                        case SkCodec::kIncompleteInput:
                            break;
                        default:
                            return SkStringPrintf("subset codec failed to decode (%d, %d, %d, %d) "
                                                  "from %s with dimensions (%d x %d)\t error %d",
                                                  x, y, decodeInfo.width(), decodeInfo.height(),
                                                  fPath.c_str(), W, H, result);
                    }
                    draw_to_canvas(canvas, subsetBitmapInfo, dst, subsetRowBytes, fDstColorType,
                                   SkIntToScalar(left), SkIntToScalar(top));

                    // translate by the scaled height.
                    top += decodeInfo.height();
                }
                // translate by the scaled width.
                left += decodeInfo.width();
            }
            return "";
        }
        default:
            SkASSERT(false);
            return "Invalid fMode";
    }
    return "";
}

SkISize CodecSrc::size() const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return {0, 0};
    }

    auto imageSize = codec->getScaledDimensions(fScale);
    if (fMode == kAnimated_Mode) {
        // We'll draw one of each frame, so make it big enough to hold them all
        // in a grid. The grid will be roughly square, with "factor" frames per
        // row and up to "factor" rows.
        const size_t count = codec->getFrameInfo().size();
        const float root = sqrt((float) count);
        const int factor = sk_float_ceil2int(root);
        imageSize.fWidth  = imageSize.fWidth  * factor;
        imageSize.fHeight = imageSize.fHeight * sk_float_ceil2int((float) count / (float) factor);
    }
    return imageSize;
}

Name CodecSrc::name() const {
    if (1.0f == fScale) {
        Name name = SkOSPath::Basename(fPath.c_str());
        if (fMode == kAnimated_Mode) {
            name.append("_animated");
        }
        return name;
    }
    SkASSERT(fMode != kAnimated_Mode);
    return get_scaled_name(fPath, fScale);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

AndroidCodecSrc::AndroidCodecSrc(Path path, CodecSrc::DstColorType dstColorType,
        SkAlphaType dstAlphaType, int sampleSize)
    : fPath(path)
    , fDstColorType(dstColorType)
    , fDstAlphaType(dstAlphaType)
    , fSampleSize(sampleSize)
    , fRunSerially(serial_from_path_name(path))
{}

bool AndroidCodecSrc::veto(SinkFlags flags) const {
    // No need to test decoding to non-raster or indirect backend.
    return flags.type != SinkFlags::kRaster
        || flags.approach != SinkFlags::kDirect;
}

Error AndroidCodecSrc::draw(SkCanvas* canvas) const {
    if (canvas->imageInfo().colorSpace() &&
            kRGBA_F16_SkColorType != canvas->imageInfo().colorType()) {
        // SkAndroidCodec uses legacy premultiplication and blending.  Therefore, we only
        // run these tests on legacy canvases.
        // We allow an exception for F16, since Android uses F16.
        return Error::Nonfatal("Skip testing to color correct canvas.");
    }

    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }
    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return SkStringPrintf("Couldn't create android codec for %s.", fPath.c_str());
    }

    SkImageInfo decodeInfo = codec->getInfo();
    if (!get_decode_info(&decodeInfo, canvas->imageInfo().colorType(), fDstColorType,
                         fDstAlphaType)) {
        return Error::Nonfatal("Skipping uninteresting test.");
    }

    // Scale the image if it is desired.
    SkISize size = codec->getSampledDimensions(fSampleSize);

    // Visually inspecting very small output images is not necessary.  We will
    // cover these cases in unit testing.
    if ((size.width() <= 10 || size.height() <= 10) && 1 != fSampleSize) {
        return Error::Nonfatal("Scaling very small images is uninteresting.");
    }
    decodeInfo = decodeInfo.makeWH(size.width(), size.height());

    int bpp = SkColorTypeBytesPerPixel(decodeInfo.colorType());
    size_t rowBytes = size.width() * bpp;
    SkAutoMalloc pixels(size.height() * rowBytes);

    SkBitmap bitmap;
    SkImageInfo bitmapInfo = decodeInfo;
    set_bitmap_color_space(&bitmapInfo);
    if (kRGBA_8888_SkColorType == decodeInfo.colorType() ||
            kBGRA_8888_SkColorType == decodeInfo.colorType()) {
        bitmapInfo = bitmapInfo.makeColorType(kN32_SkColorType);
    }

    // Create options for the codec.
    SkAndroidCodec::AndroidOptions options;
    options.fSampleSize = fSampleSize;

    switch (codec->getAndroidPixels(decodeInfo, pixels.get(), rowBytes, &options)) {
        case SkCodec::kSuccess:
        case SkCodec::kErrorInInput:
        case SkCodec::kIncompleteInput:
            break;
        default:
            return SkStringPrintf("Couldn't getPixels %s.", fPath.c_str());
    }
    draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes, fDstColorType);
    return "";
}

SkISize AndroidCodecSrc::size() const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return {0, 0};
    }
    return codec->getSampledDimensions(fSampleSize);
}

Name AndroidCodecSrc::name() const {
    // We will replicate the names used by CodecSrc so that images can
    // be compared in Gold.
    if (1 == fSampleSize) {
        return SkOSPath::Basename(fPath.c_str());
    }
    return get_scaled_name(fPath, 1.0f / (float) fSampleSize);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ImageGenSrc::ImageGenSrc(Path path, Mode mode, SkAlphaType alphaType, bool isGpu)
    : fPath(path)
    , fMode(mode)
    , fDstAlphaType(alphaType)
    , fIsGpu(isGpu)
    , fRunSerially(serial_from_path_name(path))
{}

bool ImageGenSrc::veto(SinkFlags flags) const {
    if (fIsGpu) {
        // MSAA runs tend to run out of memory and tests the same code paths as regular gpu configs.
        return flags.type != SinkFlags::kGPU || flags.approach != SinkFlags::kDirect ||
               flags.multisampled == SinkFlags::kMultisampled;
    }

    return flags.type != SinkFlags::kRaster || flags.approach != SinkFlags::kDirect;
}

Error ImageGenSrc::draw(SkCanvas* canvas) const {
    if (kRGB_565_SkColorType == canvas->imageInfo().colorType()) {
        return Error::Nonfatal("Uninteresting to test image generator to 565.");
    }

    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }

#if defined(SK_BUILD_FOR_WIN)
    // Initialize COM in order to test with WIC.
    SkAutoCoInitialize com;
    if (!com.succeeded()) {
        return "Could not initialize COM.";
    }
#endif

    std::unique_ptr<SkImageGenerator> gen(nullptr);
    switch (fMode) {
        case kCodec_Mode:
            gen = SkCodecImageGenerator::MakeFromEncodedCodec(encoded);
            if (!gen) {
                return "Could not create codec image generator.";
            }
            break;
        case kPlatform_Mode: {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
            gen = SkImageGeneratorCG::MakeFromEncodedCG(encoded);
#elif defined(SK_BUILD_FOR_WIN)
            gen.reset(SkImageGeneratorWIC::NewFromEncodedWIC(encoded.get()));
#endif

            if (!gen) {
                return "Could not create platform image generator.";
            }
            break;
        }
        default:
            SkASSERT(false);
            return "Invalid image generator mode";
    }

    // Test deferred decoding path on GPU
    if (fIsGpu) {
        sk_sp<SkImage> image(SkImage::MakeFromGenerator(std::move(gen), nullptr));
        if (!image) {
            return "Could not create image from codec image generator.";
        }
        canvas->drawImage(image, 0, 0);
        return "";
    }

    // Test various color and alpha types on CPU
    SkImageInfo decodeInfo = gen->getInfo().makeAlphaType(fDstAlphaType);

    SkImageGenerator::Options options;
    options.fBehavior = canvas->imageInfo().colorSpace() ?
            SkTransferFunctionBehavior::kRespect : SkTransferFunctionBehavior::kIgnore;

    int bpp = SkColorTypeBytesPerPixel(decodeInfo.colorType());
    size_t rowBytes = decodeInfo.width() * bpp;
    SkAutoMalloc pixels(decodeInfo.height() * rowBytes);
    if (!gen->getPixels(decodeInfo, pixels.get(), rowBytes, &options)) {
        SkString err =
                SkStringPrintf("Image generator could not getPixels() for %s\n", fPath.c_str());

#if defined(SK_BUILD_FOR_WIN)
        if (kPlatform_Mode == fMode) {
            // Do not issue a fatal error for WIC flakiness.
            return Error::Nonfatal(err);
        }
#endif

        return err;
    }

    set_bitmap_color_space(&decodeInfo);
    draw_to_canvas(canvas, decodeInfo, pixels.get(), rowBytes,
                   CodecSrc::kGetFromCanvas_DstColorType);
    return "";
}

SkISize ImageGenSrc::size() const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return {0, 0};
    }
    return codec->getInfo().dimensions();
}

Name ImageGenSrc::name() const {
    return SkOSPath::Basename(fPath.c_str());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ColorCodecSrc::ColorCodecSrc(Path path, Mode mode, SkColorType colorType)
    : fPath(path)
    , fMode(mode)
    , fColorType(colorType)
{}

bool ColorCodecSrc::veto(SinkFlags flags) const {
    // Test to direct raster backends (8888 and 565).
    return flags.type != SinkFlags::kRaster || flags.approach != SinkFlags::kDirect;
}

void clamp_if_necessary(const SkBitmap& bitmap, SkColorType dstCT) {
    if (kRGBA_F16_SkColorType != bitmap.colorType() || kRGBA_F16_SkColorType == dstCT) {
        // No need to clamp if the dst is F16.  We will clamp when we encode to PNG.
        return;
    }

    SkJumper_MemoryCtx ptr = { bitmap.getAddr(0,0), bitmap.rowBytesAsPixels() };

    SkRasterPipeline_<256> p;
    p.append(SkRasterPipeline::load_f16, &ptr);
    p.append(SkRasterPipeline::clamp_0);
    if (kPremul_SkAlphaType == bitmap.alphaType()) {
        p.append(SkRasterPipeline::clamp_a);
    } else {
        p.append(SkRasterPipeline::clamp_1);
    }
    p.append(SkRasterPipeline::store_f16, &ptr);

    p.run(0,0, bitmap.width(), bitmap.height());
}

Error ColorCodecSrc::draw(SkCanvas* canvas) const {
    if (kRGB_565_SkColorType == canvas->imageInfo().colorType()) {
        return Error::Nonfatal("No need to test color correction to 565 backend.");
    }

    bool runInLegacyMode = kBaseline_Mode == fMode;
    if (runInLegacyMode && canvas->imageInfo().colorSpace()) {
        return Error::Nonfatal("Skipping tests that are only interesting in legacy mode.");
    } else if (!runInLegacyMode && !canvas->imageInfo().colorSpace()) {
        return Error::Nonfatal("Skipping tests that are only interesting in srgb mode.");
    }

    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return SkStringPrintf("Couldn't create codec for %s.", fPath.c_str());
    }

    // Load the dst ICC profile.  This particular dst is fairly similar to Adobe RGB.
    sk_sp<SkData> dstData = GetResourceAsData("icc_profiles/HP_ZR30w.icc");
    if (!dstData) {
        return "Cannot read monitor profile.  Is the resource path set correctly?";
    }

    sk_sp<SkColorSpace> dstSpace = nullptr;
    if (kDst_sRGB_Mode == fMode) {
        dstSpace = SkColorSpace::MakeSRGB();
    } else if (kDst_HPZR30w_Mode == fMode) {
        dstSpace = SkColorSpace::MakeICC(dstData->data(), dstData->size());
    }

    SkImageInfo decodeInfo = codec->getInfo().makeColorType(fColorType).makeColorSpace(dstSpace);
    if (kUnpremul_SkAlphaType == decodeInfo.alphaType()) {
        decodeInfo = decodeInfo.makeAlphaType(kPremul_SkAlphaType);
    }
    if (kRGBA_F16_SkColorType == fColorType) {
        decodeInfo = decodeInfo.makeColorSpace(decodeInfo.colorSpace()->makeLinearGamma());
    }

    SkImageInfo bitmapInfo = decodeInfo;
    set_bitmap_color_space(&bitmapInfo);
    if (kRGBA_8888_SkColorType == decodeInfo.colorType() ||
        kBGRA_8888_SkColorType == decodeInfo.colorType())
    {
        bitmapInfo = bitmapInfo.makeColorType(kN32_SkColorType);
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(bitmapInfo)) {
        return SkStringPrintf("Image(%s) is too large (%d x %d)", fPath.c_str(),
                              bitmapInfo.width(), bitmapInfo.height());
    }

    size_t rowBytes = bitmap.rowBytes();
    SkCodec::Result r = codec->getPixels(decodeInfo, bitmap.getPixels(), rowBytes);
    switch (r) {
        case SkCodec::kSuccess:
        case SkCodec::kErrorInInput:
        case SkCodec::kIncompleteInput:
            break;
        default:
            return SkStringPrintf("Couldn't getPixels %s. Error code %d", fPath.c_str(), r);
    }

    switch (fMode) {
        case kBaseline_Mode:
        case kDst_sRGB_Mode:
        case kDst_HPZR30w_Mode:
            // We do not support drawing unclamped F16.
            clamp_if_necessary(bitmap, canvas->imageInfo().colorType());
            canvas->drawBitmap(bitmap, 0, 0);
            break;
        default:
            SkASSERT(false);
            return "Invalid fMode";
    }
    return "";
}

SkISize ColorCodecSrc::size() const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return {0, 0};
    }
    return {codec->getInfo().width(), codec->getInfo().height()};
}

Name ColorCodecSrc::name() const {
    return SkOSPath::Basename(fPath.c_str());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static const SkRect kSKPViewport = {0, 0, 1000, 1000};

SKPSrc::SKPSrc(Path path) : fPath(path) { }

static sk_sp<SkPicture> read_skp(const char* path) {
    std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(path);
    if (!stream) {
        return nullptr;
    }
    sk_sp<SkPicture> pic(SkPicture::MakeFromStream(stream.get()));
    if (!pic) {
        return nullptr;
    }
    stream = nullptr;  // Might as well drop this when we're done with it.

    return pic;
}

Error SKPSrc::draw(SkCanvas* canvas) const {
    sk_sp<SkPicture> pic = read_skp(fPath.c_str());
    if (!pic) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }

    canvas->clipRect(kSKPViewport);
    canvas->drawPicture(pic);
    return "";
}

static SkRect get_cull_rect_for_skp(const char* path) {
    std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(path);
    if (!stream) {
        return SkRect::MakeEmpty();
    }
    SkPictInfo info;
    if (!SkPicture_StreamIsSKP(stream.get(), &info)) {
        return SkRect::MakeEmpty();
    }

    return info.fCullRect;
}

SkISize SKPSrc::size() const {
    SkRect viewport = get_cull_rect_for_skp(fPath.c_str());
    if (!viewport.intersect(kSKPViewport)) {
        return {0, 0};
    }
    return viewport.roundOut().size();
}

Name SKPSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static const int kNumDDLXTiles = 4;
static const int kNumDDLYTiles = 4;
static const int kDDLTileSize = 1024;
static const SkRect kDDLSKPViewport = { 0, 0,
                                        kNumDDLXTiles * kDDLTileSize,
                                        kNumDDLYTiles * kDDLTileSize };

DDLSKPSrc::DDLSKPSrc(Path path) : fPath(path) { }

Error DDLSKPSrc::draw(SkCanvas* canvas) const {
    class TileData {
    public:
        // Note: we could just pass in surface characterization
        TileData(sk_sp<SkSurface> surf, const SkIRect& clip)
                : fSurface(std::move(surf))
                , fClip(clip) {
            SkAssertResult(fSurface->characterize(&fCharacterization));
        }

        // This method operates in parallel
        void preprocess(SkPicture* pic) {
            SkDeferredDisplayListRecorder recorder(fCharacterization);

            SkCanvas* subCanvas = recorder.getCanvas();

            subCanvas->clipRect(SkRect::MakeWH(fClip.width(), fClip.height()));
            subCanvas->translate(-fClip.fLeft, -fClip.fTop);

            // Note: in this use case we only render a picture to the deferred canvas
            // but, more generally, clients will use arbitrary draw calls.
            subCanvas->drawPicture(pic);

            fDisplayList = recorder.detach();
        }

        // This method operates serially
        void draw() {
            fSurface->draw(fDisplayList.get());
        }

        // This method also operates serially
        void compose(SkCanvas* dst) {
            sk_sp<SkImage> img = fSurface->makeImageSnapshot();
            dst->save();
            dst->clipRect(SkRect::Make(fClip));
            dst->drawImage(std::move(img), fClip.fLeft, fClip.fTop);
            dst->restore();
        }

    private:
        sk_sp<SkSurface> fSurface;
        SkIRect          fClip;    // in the device space of the destination canvas
        std::unique_ptr<SkDeferredDisplayList> fDisplayList;
        SkSurfaceCharacterization              fCharacterization;
    };

    SkTArray<TileData> tileData;
    tileData.reserve(16);

    sk_sp<SkPicture> pic = read_skp(fPath.c_str());
    if (!pic) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }

    const SkRect cullRect = pic->cullRect();

    // All the destination tiles are the same size
    const SkImageInfo tileII = SkImageInfo::MakeN32Premul(kDDLTileSize, kDDLTileSize);

    // First, create the destination tiles
    for (int y = 0; y < kNumDDLYTiles; ++y) {
        for (int x = 0; x < kNumDDLXTiles; ++x) {
            SkRect clip = SkRect::MakeXYWH(x * kDDLTileSize, y * kDDLTileSize,
                                           kDDLTileSize, kDDLTileSize);

            if (!clip.intersect(cullRect)) {
                continue;
            }

            tileData.push_back(TileData(canvas->makeSurface(tileII), clip.roundOut()));
        }
    }

    // Second, run the cpu pre-processing in threads
    SkTaskGroup().batch(tileData.count(), [&](int i) {
        tileData[i].preprocess(pic.get());
    });

    // Third, synchronously render the display lists into the dest tiles
    // TODO: it would be cool to not wait until all the tiles are drawn to begin
    // drawing to the GPU
    for (int i = 0; i < tileData.count(); ++i) {
        tileData[i].draw();
    }

    // Finally, compose the drawn tiles into the result
    // Note: the separation between the tiles and the final composition better
    // matches Chrome but costs us a copy
    for (int i = 0; i < tileData.count(); ++i) {
        tileData[i].compose(canvas);
    }

    return "";
}

SkISize DDLSKPSrc::size() const {
    SkRect viewport = get_cull_rect_for_skp(fPath.c_str());
    if (!viewport.intersect(kDDLSKPViewport)) {
        return {0, 0};
    }
    return viewport.roundOut().size();
}

Name DDLSKPSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#if !defined(SK_BUILD_FOR_GOOGLE3)
SkottieSrc::SkottieSrc(Path path)
    : fName(SkOSPath::Basename(path.c_str())) {

    fAnimation  = skottie::Animation::MakeFromFile(path.c_str());
    if (!fAnimation) {
        return;
    }

    // Fit kTileCount x kTileCount frames to a 1000x1000 film strip.
    static constexpr SkScalar kTargetSize = 1000;
    const auto scale = kTargetSize / (kTileCount * std::max(fAnimation->size().width(),
                                                            fAnimation->size().height()));
    fTileSize = SkSize::Make(scale * fAnimation->size().width(),
                             scale * fAnimation->size().height()).toCeil();

}

Error SkottieSrc::draw(SkCanvas* canvas) const {
    if (!fAnimation) {
        return SkStringPrintf("Unable to parse file: %s", fName.c_str());
    }

    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint, clockPaint;
    paint.setColor(0xffa0a0a0);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    paint.setAntiAlias(true);

    clockPaint.setTextSize(12);
    clockPaint.setAntiAlias(true);

    const auto ip = fAnimation->inPoint() * 1000 / fAnimation->frameRate(),
               op = fAnimation->outPoint() * 1000 / fAnimation->frameRate(),
               fr = (op - ip) / (kTileCount * kTileCount - 1);

    // Shuffled order to exercise non-linear frame progression.
    static constexpr int frames[] = { 4, 0, 3, 1, 2 };
    static_assert(SK_ARRAY_COUNT(frames) == kTileCount, "");

    const auto canvas_size = this->size();
    for (int i = 0; i < kTileCount; ++i) {
        const SkScalar y = frames[i] * (fTileSize.height() + 1);

        for (int j = 0; j < kTileCount; ++j) {
            const SkScalar x = frames[j] * (fTileSize.width() + 1);
            SkRect dest = SkRect::MakeXYWH(x, y, fTileSize.width(), fTileSize.height());

            const auto t = fr * (frames[i] * kTileCount + frames[j]);
            {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->clipRect(dest, true);
                canvas->concat(SkMatrix::MakeRectToRect(SkRect::MakeSize(fAnimation->size()),
                                                        dest,
                                                        SkMatrix::kFill_ScaleToFit));

                fAnimation->animationTick(t);
                fAnimation->render(canvas);
            }

            canvas->drawLine(x + fTileSize.width() + .5f, 0,
                             x + fTileSize.width() + .5f, canvas_size.height(), paint);
            const auto label = SkStringPrintf("%.3f", t);
            canvas->drawText(label.c_str(), label.size(), dest.x(),
                             dest.bottom(), clockPaint);
        }

        canvas->drawLine(0                  , y + fTileSize.height() + .5f,
                         canvas_size.width(), y + fTileSize.height() + .5f, paint);
    }

    return "";
}

SkISize SkottieSrc::size() const {
    // Padding for grid.
    return SkISize::Make(kTileCount * (fTileSize.width()  + 1),
                         kTileCount * (fTileSize.height() + 1));
}

Name SkottieSrc::name() const { return fName; }

bool SkottieSrc::veto(SinkFlags flags) const {
    // No need to test to non-(raster||gpu||vector) or indirect backends.
    bool type_ok = flags.type == SinkFlags::kRaster
                || flags.type == SinkFlags::kGPU
                || flags.type == SinkFlags::kVector;

    return !type_ok || flags.approach != SinkFlags::kDirect;
}
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if defined(SK_XML)
// Used when the image doesn't have an intrinsic size.
static const SkSize kDefaultSVGSize = {1000, 1000};

// Used to force-scale tiny fixed-size images.
static const SkSize kMinimumSVGSize = {128, 128};

SVGSrc::SVGSrc(Path path)
    : fName(SkOSPath::Basename(path.c_str()))
    , fScale(1) {

  SkFILEStream stream(path.c_str());
  if (!stream.isValid()) {
      return;
  }
  fDom = SkSVGDOM::MakeFromStream(stream);
  if (!fDom) {
      return;
  }

  const SkSize& sz = fDom->containerSize();
  if (sz.isEmpty()) {
      // no intrinsic size
      fDom->setContainerSize(kDefaultSVGSize);
  } else {
      fScale = SkTMax(1.f, SkTMax(kMinimumSVGSize.width()  / sz.width(),
                                  kMinimumSVGSize.height() / sz.height()));
  }
}

Error SVGSrc::draw(SkCanvas* canvas) const {
    if (!fDom) {
        return SkStringPrintf("Unable to parse file: %s", fName.c_str());
    }

    SkAutoCanvasRestore acr(canvas, true);
    canvas->scale(fScale, fScale);
    fDom->render(canvas);

    return "";
}

SkISize SVGSrc::size() const {
    if (!fDom) {
        return {0, 0};
    }

    return SkSize{fDom->containerSize().width() * fScale, fDom->containerSize().height() * fScale}
            .toRound();
}

Name SVGSrc::name() const { return fName; }

bool SVGSrc::veto(SinkFlags flags) const {
    // No need to test to non-(raster||gpu||vector) or indirect backends.
    bool type_ok = flags.type == SinkFlags::kRaster
                || flags.type == SinkFlags::kGPU
                || flags.type == SinkFlags::kVector;

    return !type_ok || flags.approach != SinkFlags::kDirect;
}

#endif // defined(SK_XML)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

MSKPSrc::MSKPSrc(Path path) : fPath(path) {
    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(fPath.c_str());
    int count = SkMultiPictureDocumentReadPageCount(stream.get());
    if (count > 0) {
        fPages.reset(count);
        (void)SkMultiPictureDocumentReadPageSizes(stream.get(), &fPages[0], fPages.count());
    }
}

int MSKPSrc::pageCount() const { return fPages.count(); }

SkISize MSKPSrc::size() const { return this->size(0); }
SkISize MSKPSrc::size(int i) const {
    return i >= 0 && i < fPages.count() ? fPages[i].fSize.toCeil() : SkISize{0, 0};
}

Error MSKPSrc::draw(SkCanvas* c) const { return this->draw(0, c); }
Error MSKPSrc::draw(int i, SkCanvas* canvas) const {
    if (this->pageCount() == 0) {
        return SkStringPrintf("Unable to parse MultiPictureDocument file: %s", fPath.c_str());
    }
    if (i >= fPages.count() || i < 0) {
        return SkStringPrintf("MultiPictureDocument page number out of range: %d", i);
    }
    SkPicture* page = fPages[i].fPicture.get();
    if (!page) {
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(fPath.c_str());
        if (!stream) {
            return SkStringPrintf("Unable to open file: %s", fPath.c_str());
        }
        if (!SkMultiPictureDocumentRead(stream.get(), &fPages[0], fPages.count())) {
            return SkStringPrintf("SkMultiPictureDocument reader failed on page %d: %s", i,
                                  fPath.c_str());
        }
        page = fPages[i].fPicture.get();
    }
    canvas->drawPicture(page);
    return "";
}

Name MSKPSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Error NullSink::draw(const Src& src, SkBitmap*, SkWStream*, SkString*) const {
    return src.draw(SkMakeNullCanvas().get());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static bool encode_png_base64(const SkBitmap& bitmap, SkString* dst) {
    SkPixmap pm;
    if (!bitmap.peekPixels(&pm)) {
        dst->set("peekPixels failed");
        return false;
    }

    // We're going to embed this PNG in a data URI, so make it as small as possible
    SkPngEncoder::Options options;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kAll;
    options.fZLibLevel = 9;
    options.fUnpremulBehavior = pm.colorSpace() ? SkTransferFunctionBehavior::kRespect
                                                : SkTransferFunctionBehavior::kIgnore;

    SkDynamicMemoryWStream wStream;
    if (!SkPngEncoder::Encode(&wStream, pm, options)) {
        dst->set("SkPngEncoder::Encode failed");
        return false;
    }

    sk_sp<SkData> pngData = wStream.detachAsData();
    size_t len = SkBase64::Encode(pngData->data(), pngData->size(), nullptr);

    // The PNG can be almost arbitrarily large. We don't want to fill our logs with enormous URLs.
    // Infra says these can be pretty big, as long as we're only outputting them on failure.
    static const size_t kMaxBase64Length = 1024 * 1024;
    if (len > kMaxBase64Length) {
        dst->printf("Encoded image too large (%u bytes)", static_cast<uint32_t>(len));
        return false;
    }

    dst->resize(len);
    SkBase64::Encode(pngData->data(), pngData->size(), dst->writable_str());
    return true;
}

static Error compare_bitmaps(const SkBitmap& reference, const SkBitmap& bitmap) {
    // The dimensions are a property of the Src only, and so should be identical.
    SkASSERT(reference.computeByteSize() == bitmap.computeByteSize());
    if (reference.computeByteSize() != bitmap.computeByteSize()) {
        return "Dimensions don't match reference";
    }
    // All SkBitmaps in DM are tight, so this comparison is easy.
    if (0 != memcmp(reference.getPixels(), bitmap.getPixels(), reference.computeByteSize())) {
        SkString encoded;
        SkString errString("Pixels don't match reference");
        if (encode_png_base64(reference, &encoded)) {
            errString.append("\nExpected: data:image/png;base64,");
            errString.append(encoded);
        } else {
            errString.append("\nExpected image failed to encode: ");
            errString.append(encoded);
        }
        if (encode_png_base64(bitmap, &encoded)) {
            errString.append("\nActual: data:image/png;base64,");
            errString.append(encoded);
        } else {
            errString.append("\nActual image failed to encode: ");
            errString.append(encoded);
        }
        return errString;
    }
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

DEFINE_bool(gpuStats, false, "Append GPU stats to the log for each GPU task?");

GPUSink::GPUSink(GrContextFactory::ContextType ct,
                 GrContextFactory::ContextOverrides overrides,
                 int samples,
                 bool diText,
                 SkColorType colorType,
                 SkAlphaType alphaType,
                 sk_sp<SkColorSpace> colorSpace,
                 bool threaded,
                 const GrContextOptions& grCtxOptions)
        : fContextType(ct)
        , fContextOverrides(overrides)
        , fSampleCount(samples)
        , fUseDIText(diText)
        , fColorType(colorType)
        , fAlphaType(alphaType)
        , fColorSpace(std::move(colorSpace))
        , fThreaded(threaded)
        , fBaseContextOptions(grCtxOptions) {}

DEFINE_bool(drawOpClip, false, "Clip each GrDrawOp to its device bounds for testing.");

Error GPUSink::draw(const Src& src, SkBitmap* dst, SkWStream* dstStream, SkString* log) const {
    return this->onDraw(src, dst, dstStream, log, fBaseContextOptions);
}

Error GPUSink::onDraw(const Src& src, SkBitmap* dst, SkWStream*, SkString* log,
                      const GrContextOptions& baseOptions) const {
    GrContextOptions grOptions = baseOptions;

    src.modifyGrContextOptions(&grOptions);

    GrContextFactory factory(grOptions);
    const SkISize size = src.size();
    SkImageInfo info =
            SkImageInfo::Make(size.width(), size.height(), fColorType, fAlphaType, fColorSpace);
#if SK_SUPPORT_GPU
    GrContext* context = factory.getContextInfo(fContextType, fContextOverrides).grContext();
    const int maxDimension = context->caps()->maxTextureSize();
    if (maxDimension < SkTMax(size.width(), size.height())) {
        return Error::Nonfatal("Src too large to create a texture.\n");
    }
#endif

    auto surface(
        NewGpuSurface(&factory, fContextType, fContextOverrides, info, fSampleCount, fUseDIText));
    if (!surface) {
        return "Could not create a surface.";
    }
    if (FLAGS_preAbandonGpuContext) {
        factory.abandonContexts();
    }
    SkCanvas* canvas = surface->getCanvas();
    Error err = src.draw(canvas);
    if (!err.isEmpty()) {
        return err;
    }
    canvas->flush();
    if (FLAGS_gpuStats) {
        canvas->getGrContext()->dumpCacheStats(log);
        canvas->getGrContext()->dumpGpuStats(log);
    }
    if (info.colorType() == kRGB_565_SkColorType || info.colorType() == kARGB_4444_SkColorType) {
        // We don't currently support readbacks into these formats on the GPU backend. Convert to
        // 32 bit.
        info = SkImageInfo::Make(size.width(), size.height(), kRGBA_8888_SkColorType,
                                 kPremul_SkAlphaType, fColorSpace);
    }
    dst->allocPixels(info);
    canvas->readPixels(*dst, 0, 0);
    if (FLAGS_abandonGpuContext) {
        factory.abandonContexts();
    } else if (FLAGS_releaseAndAbandonGpuContext) {
        factory.releaseResourcesAndAbandonContexts();
    }
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GPUThreadTestingSink::GPUThreadTestingSink(GrContextFactory::ContextType ct,
                                           GrContextFactory::ContextOverrides overrides,
                                           int samples,
                                           bool diText,
                                           SkColorType colorType,
                                           SkAlphaType alphaType,
                                           sk_sp<SkColorSpace> colorSpace,
                                           bool threaded,
                                           const GrContextOptions& grCtxOptions)
        : INHERITED(ct, overrides, samples, diText, colorType, alphaType, std::move(colorSpace),
                    threaded, grCtxOptions)
#if SK_SUPPORT_GPU
        , fExecutor(SkExecutor::MakeFIFOThreadPool(FLAGS_gpuThreads)) {
#else
        , fExecutor(nullptr) {
#endif
    SkASSERT(fExecutor);
}

Error GPUThreadTestingSink::draw(const Src& src, SkBitmap* dst, SkWStream* wStream,
                                 SkString* log) const {
    // Draw twice, once with worker threads, and once without. Verify that we get the same result.
    // Also, force us to only use the software path renderer, so we really stress-test the threaded
    // version of that code.
    GrContextOptions contextOptions = this->baseContextOptions();
    contextOptions.fGpuPathRenderers = GpuPathRenderers::kNone;

    contextOptions.fExecutor = fExecutor.get();
    Error err = this->onDraw(src, dst, wStream, log, contextOptions);
    if (!err.isEmpty() || !dst) {
        return err;
    }

    SkBitmap reference;
    SkString refLog;
    SkDynamicMemoryWStream refStream;
    contextOptions.fExecutor = nullptr;
    Error refErr = this->onDraw(src, &reference, &refStream, &refLog, contextOptions);
    if (!refErr.isEmpty()) {
        return refErr;
    }

    return compare_bitmaps(reference, *dst);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static Error draw_skdocument(const Src& src, SkDocument* doc, SkWStream* dst) {
    if (src.size().isEmpty()) {
        return "Source has empty dimensions";
    }
    SkASSERT(doc);
    int pageCount = src.pageCount();
    for (int i = 0; i < pageCount; ++i) {
        int width = src.size(i).width(), height = src.size(i).height();
        SkCanvas* canvas =
                doc->beginPage(SkIntToScalar(width), SkIntToScalar(height));
        if (!canvas) {
            return "SkDocument::beginPage(w,h) returned nullptr";
        }
        Error err = src.draw(i, canvas);
        if (!err.isEmpty()) {
            return err;
        }
        doc->endPage();
    }
    doc->close();
    dst->flush();
    return "";
}

Error PDFSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    SkDocument::PDFMetadata metadata;
    metadata.fTitle = src.name();
    metadata.fSubject = "rendering correctness test";
    metadata.fCreator = "Skia/DM";
    metadata.fRasterDPI = fRasterDpi;
    metadata.fPDFA = fPDFA;
    sk_sp<SkDocument> doc = SkDocument::MakePDF(dst, metadata);
    if (!doc) {
        return "SkDocument::MakePDF() returned nullptr";
    }
    return draw_skdocument(src, doc.get(), dst);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

XPSSink::XPSSink() {}

#ifdef SK_BUILD_FOR_WIN
static SkTScopedComPtr<IXpsOMObjectFactory> make_xps_factory() {
    IXpsOMObjectFactory* factory;
    HRN(CoCreateInstance(CLSID_XpsOMObjectFactory,
                         nullptr,
                         CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&factory)));
    return SkTScopedComPtr<IXpsOMObjectFactory>(factory);
}

Error XPSSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    SkAutoCoInitialize com;
    if (!com.succeeded()) {
        return "Could not initialize COM.";
    }
    SkTScopedComPtr<IXpsOMObjectFactory> factory = make_xps_factory();
    if (!factory) {
        return "Failed to create XPS Factory.";
    }
    sk_sp<SkDocument> doc(SkDocument::MakeXPS(dst, factory.get()));
    if (!doc) {
        return "SkDocument::MakeXPS() returned nullptr";
    }
    return draw_skdocument(src, doc.get(), dst);
}
#else
Error XPSSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    return "XPS not supported on this platform.";
}
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

PipeSink::PipeSink() {}

Error PipeSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    return src.draw(SkPipeSerializer().beginWrite(SkRect::Make(src.size()), dst));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SKPSink::SKPSink() {}

Error SKPSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    SkSize size;
    size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(size.width(), size.height()));
    if (!err.isEmpty()) {
        return err;
    }
    recorder.finishRecordingAsPicture()->serialize(dst);
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Error DebugSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    SkDebugCanvas debugCanvas(src.size().width(), src.size().height());
    Error err = src.draw(&debugCanvas);
    if (!err.isEmpty()) {
        return err;
    }
    std::unique_ptr<SkCanvas> nullCanvas = SkMakeNullCanvas();
    UrlDataManager dataManager(SkString("data"));
    Json::Value json = debugCanvas.toJSON(
            dataManager, debugCanvas.getSize(), nullCanvas.get());
    std::string value = Json::StyledWriter().write(json);
    return dst->write(value.c_str(), value.size()) ? "" : "SkWStream Error";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SVGSink::SVGSink() {}

Error SVGSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
#if defined(SK_XML)
    std::unique_ptr<SkXMLWriter> xmlWriter(new SkXMLStreamWriter(dst));
    return src.draw(SkSVGCanvas::Make(SkRect::MakeWH(SkIntToScalar(src.size().width()),
                                                     SkIntToScalar(src.size().height())),
                                      xmlWriter.get()).get());
#else
    return Error("SVG sink is disabled.");
#endif // SK_XML
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

RasterSink::RasterSink(SkColorType colorType, sk_sp<SkColorSpace> colorSpace)
    : fColorType(colorType)
    , fColorSpace(std::move(colorSpace)) {}

Error RasterSink::draw(const Src& src, SkBitmap* dst, SkWStream*, SkString*) const {
    const SkISize size = src.size();
    // If there's an appropriate alpha type for this color type, use it, otherwise use premul.
    SkAlphaType alphaType = kPremul_SkAlphaType;
    (void)SkColorTypeValidateAlphaType(fColorType, alphaType, &alphaType);

    dst->allocPixelsFlags(SkImageInfo::Make(size.width(), size.height(),
                                            fColorType, alphaType, fColorSpace),
                          SkBitmap::kZeroPixels_AllocFlag);
    SkCanvas canvas(*dst);
    return src.draw(&canvas);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Handy for front-patching a Src.  Do whatever up-front work you need, then call draw_to_canvas(),
// passing the Sink draw() arguments, a size, and a function draws into an SkCanvas.
// Several examples below.

template <typename Fn>
static Error draw_to_canvas(Sink* sink, SkBitmap* bitmap, SkWStream* stream, SkString* log,
                            SkISize size, const Fn& draw) {
    class ProxySrc : public Src {
    public:
        ProxySrc(SkISize size, const Fn& draw) : fSize(size), fDraw(draw) {}
        Error   draw(SkCanvas* canvas) const override { return fDraw(canvas); }
        Name    name() const override { return "ProxySrc"; }
        SkISize size() const override { return fSize; }
    private:
        SkISize   fSize;
        const Fn& fDraw;
    };
    return sink->draw(ProxySrc(size, draw), bitmap, stream, log);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

DEFINE_bool(check, true, "If true, have most Via- modes fail if they affect the output.");

// Is *bitmap identical to what you get drawing src into sink?
static Error check_against_reference(const SkBitmap* bitmap, const Src& src, Sink* sink) {
    // We can only check raster outputs.
    // (Non-raster outputs like .pdf, .skp, .svg may differ but still draw identically.)
    if (FLAGS_check && bitmap) {
        SkBitmap reference;
        SkString log;
        SkDynamicMemoryWStream wStream;
        Error err = sink->draw(src, &reference, &wStream, &log);
        // If we can draw into this Sink via some pipeline, we should be able to draw directly.
        SkASSERT(err.isEmpty());
        if (!err.isEmpty()) {
            return err;
        }
        return compare_bitmaps(reference, *bitmap);
    }
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static SkISize auto_compute_translate(SkMatrix* matrix, int srcW, int srcH) {
    SkRect bounds = SkRect::MakeIWH(srcW, srcH);
    matrix->mapRect(&bounds);
    matrix->postTranslate(-bounds.x(), -bounds.y());
    return {SkScalarRoundToInt(bounds.width()), SkScalarRoundToInt(bounds.height())};
}

ViaMatrix::ViaMatrix(SkMatrix matrix, Sink* sink) : Via(sink), fMatrix(matrix) {}

Error ViaMatrix::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    SkMatrix matrix = fMatrix;
    SkISize size = auto_compute_translate(&matrix, src.size().width(), src.size().height());
    return draw_to_canvas(fSink.get(), bitmap, stream, log, size, [&](SkCanvas* canvas) {
        canvas->concat(matrix);
        return src.draw(canvas);
    });
}

// Undoes any flip or 90 degree rotate without changing the scale of the bitmap.
// This should be pixel-preserving.
ViaUpright::ViaUpright(SkMatrix matrix, Sink* sink) : Via(sink), fMatrix(matrix) {}

Error ViaUpright::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    Error err = fSink->draw(src, bitmap, stream, log);
    if (!err.isEmpty()) {
        return err;
    }

    SkMatrix inverse;
    if (!fMatrix.rectStaysRect() || !fMatrix.invert(&inverse)) {
        return "Cannot upright --matrix.";
    }
    SkMatrix upright = SkMatrix::I();
    upright.setScaleX(SkScalarSignAsScalar(inverse.getScaleX()));
    upright.setScaleY(SkScalarSignAsScalar(inverse.getScaleY()));
    upright.setSkewX(SkScalarSignAsScalar(inverse.getSkewX()));
    upright.setSkewY(SkScalarSignAsScalar(inverse.getSkewY()));

    SkBitmap uprighted;
    SkISize size = auto_compute_translate(&upright, bitmap->width(), bitmap->height());
    uprighted.allocPixels(bitmap->info().makeWH(size.width(), size.height()));

    SkCanvas canvas(uprighted);
    canvas.concat(upright);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas.drawBitmap(*bitmap, 0, 0, &paint);

    *bitmap = uprighted;
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Error ViaSerialization::draw(
        const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    // Record our Src into a picture.
    auto size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(SkIntToScalar(size.width()),
                                                 SkIntToScalar(size.height())));
    if (!err.isEmpty()) {
        return err;
    }
    sk_sp<SkPicture> pic(recorder.finishRecordingAsPicture());

    // Serialize it and then deserialize it.
    sk_sp<SkPicture> deserialized(SkPicture::MakeFromData(pic->serialize().get()));

    return draw_to_canvas(fSink.get(), bitmap, stream, log, size, [&](SkCanvas* canvas) {
        canvas->drawPicture(deserialized);
        return check_against_reference(bitmap, src, fSink.get());
    });
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaTiles::ViaTiles(int w, int h, SkBBHFactory* factory, Sink* sink)
    : Via(sink)
    , fW(w)
    , fH(h)
    , fFactory(factory) {}

Error ViaTiles::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    auto size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(SkIntToScalar(size.width()),
                                                 SkIntToScalar(size.height()),
                                                 fFactory.get()));
    if (!err.isEmpty()) {
        return err;
    }
    sk_sp<SkPicture> pic(recorder.finishRecordingAsPicture());

    return draw_to_canvas(fSink.get(), bitmap, stream, log, src.size(), [&](SkCanvas* canvas) {
        const int xTiles = (size.width()  + fW - 1) / fW,
                  yTiles = (size.height() + fH - 1) / fH;
        SkMultiPictureDraw mpd(xTiles*yTiles);
        SkTArray<sk_sp<SkSurface>> surfaces;
//        surfaces.setReserve(xTiles*yTiles);

        SkImageInfo info = canvas->imageInfo().makeWH(fW, fH);
        for (int j = 0; j < yTiles; j++) {
            for (int i = 0; i < xTiles; i++) {
                // This lets our ultimate Sink determine the best kind of surface.
                // E.g., if it's a GpuSink, the surfaces and images are textures.
                auto s = canvas->makeSurface(info);
                if (!s) {
                    s = SkSurface::MakeRaster(info);  // Some canvases can't create surfaces.
                }
                surfaces.push_back(s);
                SkCanvas* c = s->getCanvas();
                c->translate(SkIntToScalar(-i * fW),
                             SkIntToScalar(-j * fH));  // Line up the canvas with this tile.
                mpd.add(c, pic.get());
            }
        }
        mpd.draw();
        for (int j = 0; j < yTiles; j++) {
            for (int i = 0; i < xTiles; i++) {
                sk_sp<SkImage> image(surfaces[i+xTiles*j]->makeImageSnapshot());
                canvas->drawImage(image, SkIntToScalar(i*fW), SkIntToScalar(j*fH));
            }
        }
        return "";
    });
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Error ViaPicture::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    auto size = src.size();
    return draw_to_canvas(fSink.get(), bitmap, stream, log, size, [&](SkCanvas* canvas) -> Error {
        SkPictureRecorder recorder;
        sk_sp<SkPicture> pic;
        Error err = src.draw(recorder.beginRecording(SkIntToScalar(size.width()),
                                                     SkIntToScalar(size.height())));
        if (!err.isEmpty()) {
            return err;
        }
        pic = recorder.finishRecordingAsPicture();
        canvas->drawPicture(pic);
        return check_against_reference(bitmap, src, fSink.get());
    });
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Error ViaPipe::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    auto size = src.size();
    return draw_to_canvas(fSink.get(), bitmap, stream, log, size, [&](SkCanvas* canvas) -> Error {
        SkDynamicMemoryWStream tmpStream;
        Error err = src.draw(SkPipeSerializer().beginWrite(SkRect::Make(size), &tmpStream));
        if (!err.isEmpty()) {
            return err;
        }
        sk_sp<SkData> data = tmpStream.detachAsData();
        SkPipeDeserializer().playback(data->data(), data->size(), canvas);
        return check_against_reference(bitmap, src, fSink.get());
    });
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef TEST_VIA_SVG
#include "SkXMLWriter.h"
#include "SkSVGCanvas.h"
#include "SkSVGDOM.h"

Error ViaSVG::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    auto size = src.size();
    return draw_to_canvas(fSink.get(), bitmap, stream, log, size, [&](SkCanvas* canvas) -> Error {
        SkDynamicMemoryWStream wstream;
        SkXMLStreamWriter writer(&wstream);
        Error err = src.draw(SkSVGCanvas::Make(SkRect::Make(size), &writer).get());
        if (!err.isEmpty()) {
            return err;
        }
        std::unique_ptr<SkStream> rstream(wstream.detachAsStream());
        auto dom = SkSVGDOM::MakeFromStream(*rstream);
        if (dom) {
            dom->setContainerSize(SkSize::Make(size));
            dom->render(canvas);
        }
        return "";
    });
}
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Error ViaLite::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    auto size = src.size();
    SkIRect bounds = {0,0, size.width(), size.height()};
    return draw_to_canvas(fSink.get(), bitmap, stream, log, size, [&](SkCanvas* canvas) -> Error {
        SkLiteDL dl;
        SkLiteRecorder rec;
        rec.reset(&dl, bounds);

        Error err = src.draw(&rec);
        if (!err.isEmpty()) {
            return err;
        }
        dl.draw(canvas);
        return check_against_reference(bitmap, src, fSink.get());
    });
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaCSXform::ViaCSXform(Sink* sink, sk_sp<SkColorSpace> cs, bool colorSpin)
    : Via(sink)
    , fCS(std::move(cs))
    , fColorSpin(colorSpin) {}

Error ViaCSXform::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    return draw_to_canvas(fSink.get(), bitmap, stream, log, src.size(),
                          [&](SkCanvas* canvas) -> Error {
        {
            SkAutoCanvasRestore acr(canvas, true);
            auto proxy = SkCreateColorSpaceXformCanvas(canvas, fCS);
            Error err = src.draw(proxy.get());
            if (!err.isEmpty()) {
                return err;
            }
        }

        // Undo the color spin, so we can look at the pixels in Gold.
        if (fColorSpin) {
            SkBitmap pixels;
            pixels.allocPixels(canvas->imageInfo());
            canvas->readPixels(pixels, 0, 0);

            SkPaint rotateColors;
            SkScalar matrix[20] = { 0, 0, 1, 0, 0,   // B -> R
                                    1, 0, 0, 0, 0,   // R -> G
                                    0, 1, 0, 0, 0,   // G -> B
                                    0, 0, 0, 1, 0 };
            rotateColors.setBlendMode(SkBlendMode::kSrc);
            rotateColors.setColorFilter(SkColorFilter::MakeMatrixFilterRowMajor255(matrix));
            canvas->drawBitmap(pixels, 0, 0, &rotateColors);
        }

        return "";
    });
}

}  // namespace DM

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "dm/DMSrcSink.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkDocument.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkMultiPictureDraw.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/docs/SkPDFDocument.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/ports/SkImageGeneratorCG.h"
#include "include/ports/SkImageGeneratorWIC.h"
#include "include/private/SkImageInfoPriv.h"
#include "include/private/SkTLogic.h"
#include "include/third_party/skcms/skcms.h"
#include "include/utils/SkNullCanvas.h"
#include "include/utils/SkRandom.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkOpts.h"
#include "src/core/SkPictureCommon.h"
#include "src/core/SkPictureData.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkRecorder.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/utils/SkMultiPictureDocumentPriv.h"
#include "src/utils/SkOSPath.h"
#include "tools/DDLPromiseImageHelper.h"
#include "tools/DDLTileHelper.h"
#include "tools/Resources.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/gpu/MemoryCache.h"
#if defined(SK_BUILD_FOR_WIN)
    #include "include/docs/SkXPSDocument.h"
    #include "src/utils/win/SkAutoCoInitialize.h"
    #include "src/utils/win/SkHRESULT.h"
    #include "src/utils/win/SkTScopedComPtr.h"
    #include <XpsObjectModel.h>
#endif

#if defined(SK_ENABLE_SKOTTIE)
    #include "modules/skottie/include/Skottie.h"
    #include "modules/skottie/utils/SkottieUtils.h"
#endif

#if defined(SK_XML)
    #include "experimental/svg/model/SkSVGDOM.h"
    #include "include/svg/SkSVGCanvas.h"
    #include "src/xml/SkXMLWriter.h"
#endif
#include "tests/TestUtils.h"

#include <cmath>
#include <functional>

static DEFINE_bool(multiPage, false,
                   "For document-type backends, render the source into multiple pages");
static DEFINE_bool(RAW_threading, true, "Allow RAW decodes to run on multiple threads?");

DECLARE_int(gpuThreads);

using sk_gpu_test::GrContextFactory;

namespace DM {

GMSrc::GMSrc(skiagm::GMFactory factory) : fFactory(factory) {}

Error GMSrc::draw(SkCanvas* canvas) const {
    std::unique_ptr<skiagm::GM> gm(fFactory());
    SkString errorMsg;
    skiagm::DrawResult drawResult = gm->draw(canvas, &errorMsg);
    if (skiagm::DrawResult::kSkip == drawResult) {
        return Error::Nonfatal(std::move(errorMsg));  // Cause this test to be skipped.
    }
    return errorMsg;
}

SkISize GMSrc::size() const {
    std::unique_ptr<skiagm::GM> gm(fFactory());
    return gm->getISize();
}

Name GMSrc::name() const {
    std::unique_ptr<skiagm::GM> gm(fFactory());
    return gm->getName();
}

void GMSrc::modifyGrContextOptions(GrContextOptions* options) const {
    std::unique_ptr<skiagm::GM> gm(fFactory());
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

    auto recommendedCT = brd->computeOutputColorType(colorType);
    if (kRGB_565_SkColorType == colorType && recommendedCT != colorType) {
        return Error::Nonfatal("Skip decoding non-opaque to 565.");
    }
    colorType = recommendedCT;

    auto colorSpace = brd->computeOutputColorSpace(colorType, nullptr);

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
                    fSampleSize, colorType, false, colorSpace)) {
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
                            colorSpace)) {
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
    swap_rb_if_necessary(bitmap, dstColorType);
    canvas->drawBitmap(bitmap, left, top);
}

// For codec srcs, we want the "draw" step to be a memcpy.  Any interesting color space or
// color format conversions should be performed by the codec.  Sometimes the output of the
// decode will be in an interesting color space.  On our srgb and f16 backends, we need to
// "pretend" that the color space is standard sRGB to avoid triggering color conversion
// at draw time.
static void set_bitmap_color_space(SkImageInfo* info) {
    *info = info->makeColorSpace(SkColorSpace::MakeSRGB());
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

    const int bpp = decodeInfo.bytesPerPixel();
    const size_t rowBytes = size.width() * bpp;
    const size_t safeSize = decodeInfo.computeByteSize(rowBytes);
    SkAutoMalloc pixels(safeSize);

    SkCodec::Options options;
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
            int cachedFrame = SkCodec::kNoFrame;
            for (int i = 0; static_cast<size_t>(i) < frameInfos.size(); i++) {
                options.fFrameIndex = i;
                // Check for a prior frame
                const int reqFrame = frameInfos[i].fRequiredFrame;
                if (reqFrame != SkCodec::kNoFrame && reqFrame == cachedFrame
                        && priorFramePixels.get()) {
                    // Copy into pixels
                    memcpy(pixels.get(), priorFramePixels.get(), safeSize);
                    options.fPriorFrame = reqFrame;
                } else {
                    options.fPriorFrame = SkCodec::kNoFrame;
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

                // We do not need to check the return value.  On an incomplete
                // image, memory will be filled with a default value.
                codec->getScanlines(dst, height, rowBytes);
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

    int bpp = decodeInfo.bytesPerPixel();
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
            gen = SkImageGeneratorWIC::MakeFromEncodedWIC(encoded);
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

    int bpp = decodeInfo.bytesPerPixel();
    size_t rowBytes = decodeInfo.width() * bpp;
    SkAutoMalloc pixels(decodeInfo.height() * rowBytes);
    if (!gen->getPixels(decodeInfo, pixels.get(), rowBytes)) {
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

ColorCodecSrc::ColorCodecSrc(Path path, bool decode_to_dst) : fPath(path)
                                                            , fDecodeToDst(decode_to_dst) {}

bool ColorCodecSrc::veto(SinkFlags flags) const {
    // Test to direct raster backends (8888 and 565).
    return flags.type != SinkFlags::kRaster || flags.approach != SinkFlags::kDirect;
}

Error ColorCodecSrc::draw(SkCanvas* canvas) const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return SkStringPrintf("Couldn't create codec for %s.", fPath.c_str());
    }

    SkImageInfo info = codec->getInfo();
    if (fDecodeToDst) {
        info = canvas->imageInfo().makeWH(info.width(),
                                          info.height());
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(info)) {
        return SkStringPrintf("Image(%s) is too large (%d x %d)",
                              fPath.c_str(), info.width(), info.height());
    }

    switch (auto r = codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes())) {
        case SkCodec::kSuccess:
        case SkCodec::kErrorInInput:
        case SkCodec::kIncompleteInput:
            canvas->drawBitmap(bitmap, 0,0);
            return "";
        case SkCodec::kInvalidConversion:
            // TODO(mtklein): why are there formats we can't decode to?
            return Error::Nonfatal("SkCodec can't decode to this format.");
        default:
            return SkStringPrintf("Couldn't getPixels %s. Error code %d", fPath.c_str(), r);
    }
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

static DEFINE_int(skpViewportSize, 1000,
                  "Width & height of the viewport used to crop skp rendering.");

SKPSrc::SKPSrc(Path path) : fPath(path) { }

Error SKPSrc::draw(SkCanvas* canvas) const {
    std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(fPath.c_str());
    if (!stream) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }
    sk_sp<SkPicture> pic(SkPicture::MakeFromStream(stream.get()));
    if (!pic) {
        return SkStringPrintf("Couldn't parse file %s.", fPath.c_str());
    }
    stream = nullptr;  // Might as well drop this when we're done with it.
    canvas->clipRect(SkRect::MakeWH(FLAGS_skpViewportSize, FLAGS_skpViewportSize));
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
    if (!viewport.intersect((SkRect::MakeWH(FLAGS_skpViewportSize, FLAGS_skpViewportSize)))) {
        return {0, 0};
    }
    return viewport.roundOut().size();
}

Name SKPSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

BisectSrc::BisectSrc(Path path, const char* trail) : INHERITED(path), fTrail(trail) {}

Error BisectSrc::draw(SkCanvas* canvas) const {
    struct FoundPath {
        SkPath fPath;
        SkPaint fPaint;
        SkMatrix fViewMatrix;
    };

    // This subclass of SkCanvas just extracts all the SkPaths (drawn via drawPath) from an SKP.
    class PathFindingCanvas : public SkCanvas {
    public:
        PathFindingCanvas(int width, int height) : SkCanvas(width, height, nullptr) {}
        const SkTArray<FoundPath>& foundPaths() const { return fFoundPaths; }

    private:
        void onDrawPath(const SkPath& path, const SkPaint& paint) override {
            fFoundPaths.push_back() = {path, paint, this->getTotalMatrix()};
        }

        SkTArray<FoundPath> fFoundPaths;
    };

    PathFindingCanvas pathFinder(canvas->getBaseLayerSize().width(),
                                 canvas->getBaseLayerSize().height());
    Error err = this->INHERITED::draw(&pathFinder);
    if (!err.isEmpty()) {
        return err;
    }

    int start = 0, end = pathFinder.foundPaths().count();
    for (const char* ch = fTrail.c_str(); *ch; ++ch) {
        int midpt = (start + end) / 2;
        if ('l' == *ch) {
            start = midpt;
        } else if ('r' == *ch) {
            end = midpt;
        }
    }

    for (int i = start; i < end; ++i) {
        const FoundPath& path = pathFinder.foundPaths()[i];
        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(path.fViewMatrix);
        canvas->drawPath(path.fPath, path.fPaint);
    }

    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#if defined(SK_ENABLE_SKOTTIE)
SkottieSrc::SkottieSrc(Path path) : fPath(std::move(path)) {}

Error SkottieSrc::draw(SkCanvas* canvas) const {
    auto animation = skottie::Animation::Builder()
        .setResourceProvider(
                skottie_utils::FileResourceProvider::Make(SkOSPath::Dirname(fPath.c_str())))
        .makeFromFile(fPath.c_str());
    if (!animation) {
        return SkStringPrintf("Unable to parse file: %s", fPath.c_str());
    }

    canvas->drawColor(SK_ColorWHITE);

    const auto t_rate = 1.0f / (kTileCount * kTileCount - 1);

    // Draw the frames in a shuffled order to exercise non-linear
    // frame progression. The film strip will still be in order left-to-right,
    // top-down, just not drawn in that order.
    static constexpr int frameOrder[] = { 4, 0, 3, 1, 2 };
    static_assert(SK_ARRAY_COUNT(frameOrder) == kTileCount, "");

    for (int i = 0; i < kTileCount; ++i) {
        const SkScalar y = frameOrder[i] * kTileSize;

        for (int j = 0; j < kTileCount; ++j) {
            const SkScalar x = frameOrder[j] * kTileSize;
            SkRect dest = SkRect::MakeXYWH(x, y, kTileSize, kTileSize);

            const auto t = t_rate * (frameOrder[i] * kTileCount + frameOrder[j]);
            {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->clipRect(dest, true);
                canvas->concat(SkMatrix::MakeRectToRect(SkRect::MakeSize(animation->size()),
                                                        dest,
                                                        SkMatrix::kCenter_ScaleToFit));
                animation->seek(t);
                animation->render(canvas);
            }
        }
    }

    return "";
}

SkISize SkottieSrc::size() const {
    return SkISize::Make(kTargetSize, kTargetSize);
}

Name SkottieSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

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

    sk_sp<SkData> data(SkData::MakeFromFileName(path.c_str()));
    if (!data) {
        return;
    }

    SkMemoryStream stream(std::move(data));
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
        if (bitmap_to_base64_data_uri(reference, &encoded)) {
            errString.append("\nExpected: ");
            errString.append(encoded);
        } else {
            errString.append("\nExpected image failed to encode: ");
            errString.append(encoded);
        }
        if (bitmap_to_base64_data_uri(bitmap, &encoded)) {
            errString.append("\nActual: ");
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

static DEFINE_bool(gpuStats, false, "Append GPU stats to the log for each GPU task?");
static DEFINE_bool(preAbandonGpuContext, false,
                   "Test abandoning the GrContext before running the test.");
static DEFINE_bool(abandonGpuContext, false,
                   "Test abandoning the GrContext after running each test.");
static DEFINE_bool(releaseAndAbandonGpuContext, false,
                   "Test releasing all gpu resources and abandoning the GrContext "
                   "after running each test");
static DEFINE_bool(drawOpClip, false, "Clip each GrDrawOp to its device bounds for testing.");
static DEFINE_bool(programBinaryCache, true, "Use in-memory program binary cache");

GPUSink::GPUSink(const SkCommandLineConfigGpu* config,
                 const GrContextOptions& grCtxOptions)
        : fContextType(config->getContextType())
        , fContextOverrides(config->getContextOverrides())
        , fSurfType(config->getSurfType())
        , fSampleCount(config->getSamples())
        , fUseDIText(config->getUseDIText())
        , fColorType(config->getColorType())
        , fAlphaType(config->getAlphaType())
        , fColorSpace(sk_ref_sp(config->getColorSpace()))
        , fBaseContextOptions(grCtxOptions) {
    if (FLAGS_programBinaryCache) {
        fBaseContextOptions.fPersistentCache = &fMemoryCache;
    }
}

Error GPUSink::draw(const Src& src, SkBitmap* dst, SkWStream* dstStream, SkString* log) const {
    return this->onDraw(src, dst, dstStream, log, fBaseContextOptions);
}

Error GPUSink::onDraw(const Src& src, SkBitmap* dst, SkWStream*, SkString* log,
                      const GrContextOptions& baseOptions,
                      std::function<void(GrContext*)> initContext) const {
    GrContextOptions grOptions = baseOptions;

    // We don't expect the src to mess with the persistent cache or the executor.
    SkDEBUGCODE(auto cache = grOptions.fPersistentCache);
    SkDEBUGCODE(auto exec = grOptions.fExecutor);
    src.modifyGrContextOptions(&grOptions);
    SkASSERT(cache == grOptions.fPersistentCache);
    SkASSERT(exec == grOptions.fExecutor);

    GrContextFactory factory(grOptions);
    const SkISize size = src.size();
    SkImageInfo info = SkImageInfo::Make(size, fColorType, fAlphaType, fColorSpace);
    sk_sp<SkSurface> surface;
    GrContext* context = factory.getContextInfo(fContextType, fContextOverrides).grContext();
    if (initContext) {
        initContext(context);
    }
    const int maxDimension = context->priv().caps()->maxTextureSize();
    if (maxDimension < SkTMax(size.width(), size.height())) {
        return Error::Nonfatal("Src too large to create a texture.\n");
    }
    uint32_t flags = fUseDIText ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag : 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    GrBackendTexture backendTexture;
    GrBackendRenderTarget backendRT;
    switch (fSurfType) {
        case SkCommandLineConfigGpu::SurfType::kDefault:
            surface = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info, fSampleCount,
                                                  &props);
            break;
        case SkCommandLineConfigGpu::SurfType::kBackendTexture:
            backendTexture = context->createBackendTexture(
                    info.width(), info.height(), info.colorType(), SkColors::kTransparent,
                    GrMipMapped::kNo, GrRenderable::kYes, GrProtected::kNo);
            surface = SkSurface::MakeFromBackendTexture(context, backendTexture,
                                                        kTopLeft_GrSurfaceOrigin, fSampleCount,
                                                        fColorType, info.refColorSpace(), &props);
            break;
        case SkCommandLineConfigGpu::SurfType::kBackendRenderTarget:
            if (1 == fSampleCount) {
                auto colorType = SkColorTypeToGrColorType(info.colorType());
                backendRT = context->priv().getGpu()->createTestingOnlyBackendRenderTarget(
                        info.width(), info.height(), colorType);
                surface = SkSurface::MakeFromBackendRenderTarget(
                        context, backendRT, kBottomLeft_GrSurfaceOrigin, info.colorType(),
                        info.refColorSpace(), &props);
            }
            break;
    }

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
    surface->flush();
    if (FLAGS_gpuStats) {
        canvas->getGrContext()->priv().dumpCacheStats(log);
        canvas->getGrContext()->priv().dumpGpuStats(log);
    }
    if (info.colorType() == kRGB_565_SkColorType || info.colorType() == kARGB_4444_SkColorType ||
        info.colorType() == kRGB_888x_SkColorType) {
        // We don't currently support readbacks into these formats on the GPU backend. Convert to
        // 32 bit.
        info = SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType, fColorSpace);
    }
    dst->allocPixels(info);
    canvas->readPixels(*dst, 0, 0);
    if (FLAGS_abandonGpuContext) {
        factory.abandonContexts();
    } else if (FLAGS_releaseAndAbandonGpuContext) {
        factory.releaseResourcesAndAbandonContexts();
    }
    if (!context->abandoned()) {
        surface.reset();
        if (backendTexture.isValid()) {
            context->deleteBackendTexture(backendTexture);
        }
        if (backendRT.isValid()) {
            context->priv().getGpu()->deleteTestingOnlyBackendRenderTarget(backendRT);
        }
    }
    if (grOptions.fPersistentCache) {
        context->storeVkPipelineCacheData();
    }
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GPUThreadTestingSink::GPUThreadTestingSink(const SkCommandLineConfigGpu* config,
                                           const GrContextOptions& grCtxOptions)
        : INHERITED(config, grCtxOptions)
        , fExecutor(SkExecutor::MakeFIFOThreadPool(FLAGS_gpuThreads)) {
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

GPUPersistentCacheTestingSink::GPUPersistentCacheTestingSink(const SkCommandLineConfigGpu* config,
                                                             const GrContextOptions& grCtxOptions)
    : INHERITED(config, grCtxOptions)
    , fCacheType(config->getTestPersistentCache()) {}

Error GPUPersistentCacheTestingSink::draw(const Src& src, SkBitmap* dst, SkWStream* wStream,
                                          SkString* log) const {
    // Draw twice, once with a cold cache, and again with a warm cache. Verify that we get the same
    // result.
    sk_gpu_test::MemoryCache memoryCache;
    GrContextOptions contextOptions = this->baseContextOptions();
    contextOptions.fPersistentCache = &memoryCache;
    if (fCacheType == 2) {
        contextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kBackendSource;
    }
    // anglebug.com/3619
    contextOptions.fGpuPathRenderers =
            contextOptions.fGpuPathRenderers & ~GpuPathRenderers::kStencilAndCover;

    Error err = this->onDraw(src, dst, wStream, log, contextOptions);
    if (!err.isEmpty() || !dst) {
        return err;
    }

    SkBitmap reference;
    SkString refLog;
    SkDynamicMemoryWStream refStream;
    memoryCache.resetNumCacheMisses();
    Error refErr = this->onDraw(src, &reference, &refStream, &refLog, contextOptions);
    if (!refErr.isEmpty()) {
        return refErr;
    }
    SkASSERT(!memoryCache.numCacheMisses());

    return compare_bitmaps(reference, *dst);
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GPUPrecompileTestingSink::GPUPrecompileTestingSink(const SkCommandLineConfigGpu* config,
                                                   const GrContextOptions& grCtxOptions)
    : INHERITED(config, grCtxOptions) {}

Error GPUPrecompileTestingSink::draw(const Src& src, SkBitmap* dst, SkWStream* wStream,
                                     SkString* log) const {
    // Three step process:
    // 1) Draw once with an SkSL cache, and store off the shader blobs.
    // 2) For the second context, pre-compile the shaders to warm the cache.
    // 3) Draw with the second context, ensuring that we get the same result, and no cache misses.
    sk_gpu_test::MemoryCache memoryCache;
    GrContextOptions contextOptions = this->baseContextOptions();
    contextOptions.fPersistentCache = &memoryCache;
    contextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;
    // anglebug.com/3619 means that we don't cache shaders when we're using NVPR. That prevents
    // the precompile from working, so we'll trigger the assert at the end of this test.
    contextOptions.fGpuPathRenderers =
            contextOptions.fGpuPathRenderers & ~GpuPathRenderers::kStencilAndCover;

    Error err = this->onDraw(src, dst, wStream, log, contextOptions);
    if (!err.isEmpty() || !dst) {
        return err;
    }

    auto precompileShaders = [&memoryCache](GrContext* context) {
        memoryCache.foreach([context](sk_sp<const SkData> key, sk_sp<SkData> data, int /*count*/) {
            SkAssertResult(context->precompileShader(*key, *data));
        });
    };

    sk_gpu_test::MemoryCache replayCache;
    GrContextOptions replayOptions = this->baseContextOptions();
    // Ensure that the runtime cache is large enough to hold all of the shaders we pre-compile
    replayOptions.fRuntimeProgramCacheSize = memoryCache.numCacheMisses();
    replayOptions.fPersistentCache = &replayCache;
    // anglebug.com/3619
    replayOptions.fGpuPathRenderers =
            replayOptions.fGpuPathRenderers & ~GpuPathRenderers::kStencilAndCover;

    SkBitmap reference;
    SkString refLog;
    SkDynamicMemoryWStream refStream;
    Error refErr = this->onDraw(src, &reference, &refStream, &refLog, replayOptions,
                                precompileShaders);
    if (!refErr.isEmpty()) {
        return refErr;
    }
    SkASSERT(!replayCache.numCacheMisses());

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
    SkPDF::Metadata metadata;
    metadata.fTitle = src.name();
    metadata.fSubject = "rendering correctness test";
    metadata.fCreator = "Skia/DM";
    metadata.fRasterDPI = fRasterDpi;
    metadata.fPDFA = fPDFA;
#if SK_PDF_TEST_EXECUTOR
    std::unique_ptr<SkExecutor> executor = SkExecutor::MakeFIFOThreadPool();
    metadata.fExecutor = executor.get();
#endif
    auto doc = SkPDF::MakeDocument(dst, metadata);
    if (!doc) {
        return "SkPDF::MakeDocument() returned nullptr";
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
    auto doc = SkXPS::MakeDocument(dst, factory.get());
    if (!doc) {
        return "SkXPS::MakeDocument() returned nullptr";
    }
    return draw_skdocument(src, doc.get(), dst);
}
#else
Error XPSSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    return "XPS not supported on this platform.";
}
#endif

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
    DebugCanvas debugCanvas(src.size().width(), src.size().height());
    Error err = src.draw(&debugCanvas);
    if (!err.isEmpty()) {
        return err;
    }
    std::unique_ptr<SkCanvas> nullCanvas = SkMakeNullCanvas();
    UrlDataManager dataManager(SkString("data"));
    SkJSONWriter writer(dst, SkJSONWriter::Mode::kPretty);
    writer.beginObject(); // root
    debugCanvas.toJSON(writer, dataManager, debugCanvas.getSize(), nullCanvas.get());
    writer.endObject(); // root
    writer.flush();
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SVGSink::SVGSink(int pageIndex) : fPageIndex(pageIndex) {}

Error SVGSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
#if defined(SK_XML)
    if (src.pageCount() > 1) {
        int pageCount = src.pageCount();
        if (fPageIndex > pageCount - 1) {
            return Error(SkStringPrintf("Page index %d too high for document with only %d pages.",
                                        fPageIndex, pageCount));
        }
    }
    return src.draw(fPageIndex,
                    SkSVGCanvas::Make(SkRect::MakeWH(SkIntToScalar(src.size().width()),
                                                     SkIntToScalar(src.size().height())),
                                      dst)
                            .get());
#else
    (void)fPageIndex;
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

    dst->allocPixelsFlags(SkImageInfo::Make(size, fColorType, alphaType, fColorSpace),
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

static DEFINE_bool(check, true, "If true, have most Via- modes fail if they affect the output.");

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

    err = draw_to_canvas(fSink.get(), bitmap, stream, log, size, [&](SkCanvas* canvas) {
        canvas->drawPicture(deserialized);
        return "";
    });
    if (!err.isEmpty()) {
        return err;
    }

    return check_against_reference(bitmap, src, fSink.get());
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

ViaDDL::ViaDDL(int numReplays, int numDivisions, Sink* sink)
        : Via(sink), fNumReplays(numReplays), fNumDivisions(numDivisions) {}

Error ViaDDL::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    auto size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(SkIntToScalar(size.width()),
                                                 SkIntToScalar(size.height())));
    if (!err.isEmpty()) {
        return err;
    }
    sk_sp<SkPicture> inputPicture(recorder.finishRecordingAsPicture());

    // this is our ultimate final drawing area/rect
    SkIRect viewport = SkIRect::MakeWH(size.fWidth, size.fHeight);

    DDLPromiseImageHelper promiseImageHelper;
    sk_sp<SkData> compressedPictureData = promiseImageHelper.deflateSKP(inputPicture.get());
    if (!compressedPictureData) {
        return SkStringPrintf("ViaDDL: Couldn't deflate SkPicture");
    }
    auto draw = [&](SkCanvas* canvas) -> Error {
        GrContext* context = canvas->getGrContext();
        if (!context || !context->priv().getGpu()) {
            return SkStringPrintf("DDLs are GPU only");
        }

        // This is here bc this is the first point where we have access to the context
        promiseImageHelper.uploadAllToGPU(context);
        // We draw N times, with a clear between.
        for (int replay = 0; replay < fNumReplays; ++replay) {
            if (replay > 0) {
                // Clear the drawing of the previous replay
                canvas->clear(SK_ColorTRANSPARENT);
            }
            // First, create all the tiles (including their individual dest surfaces)
            DDLTileHelper tiles(canvas, viewport, fNumDivisions);

            // Second, reinflate the compressed picture individually for each thread
            // This recreates the promise SkImages on each replay iteration. We are currently
            // relying on this to test using a SkPromiseImageTexture to fulfill different
            // SkImages. On each replay the promise SkImages are recreated in createSKPPerTile.
            tiles.createSKPPerTile(compressedPictureData.get(), promiseImageHelper);

            // Third, create the DDLs in parallel
            tiles.createDDLsInParallel();

            if (replay == fNumReplays - 1) {
                // This drops the promiseImageHelper's refs on all the promise images if we're in
                // the last run.
                promiseImageHelper.reset();
            }

            // Fourth, synchronously render the display lists into the dest tiles
            // TODO: it would be cool to not wait until all the tiles are drawn to begin
            // drawing to the GPU and composing to the final surface
            tiles.drawAllTilesAndFlush(context, false);

            // Finally, compose the drawn tiles into the result
            // Note: the separation between the tiles and the final composition better
            // matches Chrome but costs us a copy
            tiles.composeAllTiles(canvas);
            context->flush();
        }
        return "";
    };
    return draw_to_canvas(fSink.get(), bitmap, stream, log, size, draw);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Error ViaPicture::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    auto size = src.size();
    Error err = draw_to_canvas(fSink.get(), bitmap, stream, log, size, [&](SkCanvas* canvas) {
        SkPictureRecorder recorder;
        sk_sp<SkPicture> pic;
        Error err = src.draw(recorder.beginRecording(SkIntToScalar(size.width()),
                                                     SkIntToScalar(size.height())));
        if (!err.isEmpty()) {
            return err;
        }
        pic = recorder.finishRecordingAsPicture();
        canvas->drawPicture(pic);
        return err;
    });
    if (!err.isEmpty()) {
        return err;
    }

    return check_against_reference(bitmap, src, fSink.get());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef TEST_VIA_SVG
#include "experimental/svg/model/SkSVGDOM.h"
#include "include/svg/SkSVGCanvas.h"
#include "src/xml/SkXMLWriter.h"

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

}  // namespace DM

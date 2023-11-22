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
#include "include/core/SkDocument.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/docs/SkMultiPictureDocument.h"
#include "include/docs/SkPDFDocument.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/ports/SkImageGeneratorCG.h"
#include "include/ports/SkImageGeneratorNDK.h"
#include "include/ports/SkImageGeneratorWIC.h"
#include "include/private/base/SkTLogic.h"
#include "include/private/chromium/GrDeferredDisplayList.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "include/utils/SkNullCanvas.h"
#include "include/utils/SkPaintFilterCanvas.h"
#include "modules/skcms/skcms.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkRandom.h"
#include "src/base/SkTLazy.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkPictureData.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkRecorder.h"
#include "src/core/SkSwizzlePriv.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/image/SkImage_Base.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkMultiPictureDocumentPriv.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkTestCanvas.h"
#include "tools/DDLPromiseImageHelper.h"
#include "tools/DDLTileHelper.h"
#include "tools/EncodeUtils.h"
#include "tools/GpuToolUtils.h"
#include "tools/Resources.h"
#include "tools/RuntimeBlendUtils.h"
#include "tools/ToolUtils.h"
#include "tools/UrlDataManager.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/gpu/BackendSurfaceFactory.h"
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
    #include "modules/skresources/include/SkResources.h"
#endif

#if defined(SK_ENABLE_SVG)
    #include "include/svg/SkSVGCanvas.h"
    #include "modules/svg/include/SkSVGDOM.h"
    #include "modules/svg/include/SkSVGNode.h"
    #include "src/xml/SkXMLWriter.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/private/gpu/graphite/ContextOptionsPriv.h"
// TODO: Remove this src include once we figure out public readPixels call for Graphite.
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteTestContext.h"
#endif

#if defined(SK_ENABLE_ANDROID_UTILS)
    #include "client_utils/android/BitmapRegionDecoder.h"
#endif
#include "tests/TestUtils.h"

#include <cmath>
#include <functional>

using namespace skia_private;

static DEFINE_bool(RAW_threading, true, "Allow RAW decodes to run on multiple threads?");
static DEFINE_int(mskpFrame, 0, "Which MSKP frame to draw?");

DECLARE_int(gpuThreads);

using sk_gpu_test::GrContextFactory;
using sk_gpu_test::ContextInfo;

namespace DM {

GMSrc::GMSrc(skiagm::GMFactory factory) : fFactory(factory) {}

Result GMSrc::draw(SkCanvas* canvas) const {
    std::unique_ptr<skiagm::GM> gm(fFactory());
    if (gm->isBazelOnly()) {
        // We skip Bazel-only GMs because they might overlap with existing DM functionality. See
        // comments in the skiagm::GM::isBazelOnly function declaration for context.
        return Result(Result::Status::Skip, SkString("Bazel-only GM"));
    }
    SkString msg;

    skiagm::DrawResult gpuSetupResult = gm->gpuSetup(canvas, &msg);
    switch (gpuSetupResult) {
        case skiagm::DrawResult::kOk  : break;
        case skiagm::DrawResult::kFail: return Result(Result::Status::Fatal, msg);
        case skiagm::DrawResult::kSkip: return Result(Result::Status::Skip,  msg);
        default: SK_ABORT("");
    }

    skiagm::DrawResult drawResult = gm->draw(canvas, &msg);
    switch (drawResult) {
        case skiagm::DrawResult::kOk  : return Result(Result::Status::Ok,    msg);
        case skiagm::DrawResult::kFail: return Result(Result::Status::Fatal, msg);
        case skiagm::DrawResult::kSkip: return Result(Result::Status::Skip,  msg);
        default: SK_ABORT("");
    }

    // Note: we don't call "gpuTeardown" here because, when testing DDL recording, we want
    // the gpu-backed images to live past the lifetime of the GM.
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

#if defined(SK_GRAPHITE)
void GMSrc::modifyGraphiteContextOptions(skgpu::graphite::ContextOptions* options) const {
    std::unique_ptr<skiagm::GM> gm(fFactory());
    gm->modifyGraphiteContextOptions(options);
}
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static SkString get_scaled_name(const Path& path, float scale) {
    return SkStringPrintf("%s_%.3f", SkOSPath::Basename(path.c_str()).c_str(), scale);
}

#ifdef SK_ENABLE_ANDROID_UTILS
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

static std::unique_ptr<android::skia::BitmapRegionDecoder> create_brd(Path path) {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
    return android::skia::BitmapRegionDecoder::Make(encoded);
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

Result BRDSrc::draw(SkCanvas* canvas) const {
    SkColorType colorType = canvas->imageInfo().colorType();
    if (kRGB_565_SkColorType == colorType &&
        CodecSrc::kGetFromCanvas_DstColorType != fDstColorType)
    {
        return Result::Skip("Testing non-565 to 565 is uninteresting.");
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

    auto brd = create_brd(fPath);
    if (nullptr == brd) {
        return Result::Skip("Could not create brd for %s.", fPath.c_str());
    }

    auto recommendedCT = brd->computeOutputColorType(colorType);
    if (kRGB_565_SkColorType == colorType && recommendedCT != colorType) {
        return Result::Skip("Skip decoding non-opaque to 565.");
    }
    colorType = recommendedCT;

    auto colorSpace = brd->computeOutputColorSpace(colorType, nullptr);

    const uint32_t width = brd->width();
    const uint32_t height = brd->height();
    // Visually inspecting very small output images is not necessary.
    if ((width / fSampleSize <= 10 || height / fSampleSize <= 10) && 1 != fSampleSize) {
        return Result::Skip("Scaling very small images is uninteresting.");
    }
    switch (fMode) {
        case kFullImage_Mode: {
            SkBitmap bitmap;
            if (!brd->decodeRegion(&bitmap, nullptr, SkIRect::MakeXYWH(0, 0, width, height),
                    fSampleSize, colorType, false, colorSpace)) {
                return Result::Fatal("Cannot decode (full) region.");
            }
            alpha8_to_gray8(&bitmap);

            canvas->drawImage(bitmap.asImage(), 0, 0);
            return Result::Ok();
        }
        case kDivisor_Mode: {
            const uint32_t divisor = 2;
            if (width < divisor || height < divisor) {
                return Result::Skip("Divisor is larger than image dimension.");
            }

            // Use a border to test subsets that extend outside the image.
            // We will not allow the border to be larger than the image dimensions.  Allowing
            // these large borders causes off by one errors that indicate a problem with the
            // test suite, not a problem with the implementation.
            const uint32_t maxBorder = std::min(width, height) / (fSampleSize * divisor);
            const uint32_t scaledBorder = std::min(5u, maxBorder);
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
                        return Result::Fatal("Cannot decode region.");
                    }

                    alpha8_to_gray8(&bitmap);
                    canvas->drawImageRect(bitmap.asImage().get(),
                            SkRect::MakeXYWH((SkScalar) scaledBorder, (SkScalar) scaledBorder,
                                    (SkScalar) (subsetWidth / fSampleSize),
                                    (SkScalar) (subsetHeight / fSampleSize)),
                            SkRect::MakeXYWH((SkScalar) (left / fSampleSize),
                                    (SkScalar) (top / fSampleSize),
                                    (SkScalar) (subsetWidth / fSampleSize),
                                    (SkScalar) (subsetHeight / fSampleSize)),
                            SkSamplingOptions(), nullptr,
                            SkCanvas::kStrict_SrcRectConstraint);
                }
            }
            return Result::Ok();
        }
        default:
            SkASSERT(false);
            return Result::Fatal("Error: Should not be reached.");
    }
}

SkISize BRDSrc::size() const {
    auto brd = create_brd(fPath);
    if (brd) {
        return {std::max(1, brd->width() / (int)fSampleSize),
                std::max(1, brd->height() / (int)fSampleSize)};
    }
    return {0, 0};
}

Name BRDSrc::name() const {
    // We will replicate the names used by CodecSrc so that images can
    // be compared in Gold.
    if (1 == fSampleSize) {
        return SkOSPath::Basename(fPath.c_str());
    }
    return get_scaled_name(fPath, 1.0f / (float) fSampleSize);
}

#endif // SK_ENABLE_ANDROID_UTILS

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
    canvas->drawImage(bitmap.asImage(), left, top);
}

// For codec srcs, we want the "draw" step to be a memcpy.  Any interesting color space or
// color format conversions should be performed by the codec.  Sometimes the output of the
// decode will be in an interesting color space.  On our srgb and f16 backends, we need to
// "pretend" that the color space is standard sRGB to avoid triggering color conversion
// at draw time.
static void set_bitmap_color_space(SkImageInfo* info) {
    *info = info->makeColorSpace(SkColorSpace::MakeSRGB());
}

Result CodecSrc::draw(SkCanvas* canvas) const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return Result::Fatal("Couldn't read %s.", fPath.c_str());
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return Result::Fatal("Couldn't create codec for %s.", fPath.c_str());
    }

    SkImageInfo decodeInfo = codec->getInfo();
    if (!get_decode_info(&decodeInfo, canvas->imageInfo().colorType(), fDstColorType,
                         fDstAlphaType)) {
        return Result::Skip("Skipping uninteresting test.");
    }

    // Try to scale the image if it is desired
    SkISize size = codec->getScaledDimensions(fScale);

    std::unique_ptr<SkAndroidCodec> androidCodec;
    if (1.0f != fScale && fMode == kAnimated_Mode) {
        androidCodec = SkAndroidCodec::MakeFromData(encoded);
        size = androidCodec->getSampledDimensions(1 / fScale);
    }

    if (size == decodeInfo.dimensions() && 1.0f != fScale) {
        return Result::Skip("Test without scaling is uninteresting.");
    }

    // Visually inspecting very small output images is not necessary.  We will
    // cover these cases in unit testing.
    if ((size.width() <= 10 || size.height() <= 10) && 1.0f != fScale) {
        return Result::Skip("Scaling very small images is uninteresting.");
    }
    decodeInfo = decodeInfo.makeDimensions(size);

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
            SkAndroidCodec::AndroidOptions androidOptions;
            if (fScale != 1.0f) {
                SkASSERT(androidCodec);
                androidOptions.fSampleSize = 1 / fScale;
                auto dims = androidCodec->getSampledDimensions(androidOptions.fSampleSize);
                decodeInfo = decodeInfo.makeDimensions(dims);
            }

            std::vector<SkCodec::FrameInfo> frameInfos = androidCodec
                    ? androidCodec->codec()->getFrameInfo() : codec->getFrameInfo();
            if (frameInfos.size() <= 1) {
                return Result::Fatal("%s is not an animated image.", fPath.c_str());
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
                androidOptions.fFrameIndex = i;
                // Check for a prior frame
                const int reqFrame = frameInfos[i].fRequiredFrame;
                if (reqFrame != SkCodec::kNoFrame && reqFrame == cachedFrame
                        && priorFramePixels.get()) {
                    // Copy into pixels
                    memcpy(pixels.get(), priorFramePixels.get(), safeSize);
                    androidOptions.fPriorFrame = reqFrame;
                } else {
                    androidOptions.fPriorFrame = SkCodec::kNoFrame;
                }
                SkCodec::Result result = androidCodec
                        ? androidCodec->getAndroidPixels(decodeInfo, pixels.get(), rowBytes,
                                                         &androidOptions)
                        : codec->getPixels(decodeInfo, pixels.get(), rowBytes, &androidOptions);
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
                            return Result::Ok();
                        }
                        break;
                    }
                    case SkCodec::kInvalidConversion:
                        if (i > 0 && (decodeInfo.colorType() == kRGB_565_SkColorType)) {
                            return Result::Skip(
                                "Cannot decode frame %i to 565 (%s).", i, fPath.c_str());
                        }
                        [[fallthrough]];
                    default:
                        return Result::Fatal(
                            "Couldn't getPixels for frame %i in %s.", i, fPath.c_str());
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
                    return Result::Fatal("Couldn't getPixels %s.", fPath.c_str());
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
                        return Result::Fatal("Could not start incremental decode");
                    }
                    // Otherwise, this is an ICO. Since incremental failed, it must contain a BMP,
                    // which should work via startScanlineDecode
                    useOldScanlineMethod = true;
                }
            }

            if (useOldScanlineMethod) {
                if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo)) {
                    return Result::Fatal("Could not start scanline decoder");
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
                return Result::Fatal("Could not start scanline decoder");
            }

            // This mode was designed to test the new skip scanlines API in libjpeg-turbo.
            // Jpegs have kTopDown_SkScanlineOrder, and at this time, it is not interesting
            // to run this test for image types that do not have this scanline ordering.
            // We only run this on Jpeg, which is always kTopDown.
            SkASSERT(SkCodec::kTopDown_SkScanlineOrder == codec->getScanlineOrder());

            for (int i = 0; i < numStripes; i += 2) {
                // Skip a stripe
                const int linesToSkip = std::min(stripeHeight, height - i * stripeHeight);
                codec->skipScanlines(linesToSkip);

                // Read a stripe
                const int startY = (i + 1) * stripeHeight;
                const int linesToRead = std::min(stripeHeight, height - startY);
                if (linesToRead > 0) {
                    codec->getScanlines(SkTAddOffset<void>(dst, rowBytes * startY), linesToRead,
                                        rowBytes);
                }
            }

            // Decode even stripes
            const SkCodec::Result startResult = codec->startScanlineDecode(decodeInfo);
            if (SkCodec::kSuccess != startResult) {
                return Result::Fatal("Failed to restart scanline decoder with same parameters.");
            }
            for (int i = 0; i < numStripes; i += 2) {
                // Read a stripe
                const int startY = i * stripeHeight;
                const int linesToRead = std::min(stripeHeight, height - startY);
                codec->getScanlines(SkTAddOffset<void>(dst, rowBytes * startY), linesToRead,
                                    rowBytes);

                // Skip a stripe
                const int linesToSkip = std::min(stripeHeight, height - (i + 1) * stripeHeight);
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
                subset = SkIRect::MakeXYWH(x, 0, std::min(tileSize, width - x), height);
                options.fSubset = &subset;
                if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo, &options)) {
                    return Result::Fatal("Could not start scanline decoder.");
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
                return Result::Skip("Cannot codec subset: divisor %d is too big "
                                    "for %s with dimensions (%d x %d)", divisor,
                                    fPath.c_str(), W, H);
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
                    const int preScaleW = std::min(w, W - x);
                    const int preScaleH = std::min(h, H - y);
                    subset.setXYWH(x, y, preScaleW, preScaleH);
                    // And scale
                    // FIXME: Should we have a version of getScaledDimensions that takes a subset
                    // into account?
                    const int scaledW = std::max(1, SkScalarRoundToInt(preScaleW * fScale));
                    const int scaledH = std::max(1, SkScalarRoundToInt(preScaleH * fScale));
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
                            return Result::Fatal("subset codec failed to decode (%d, %d, %d, %d) "
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
            return Result::Ok();
        }
        default:
            SkASSERT(false);
            return Result::Fatal("Invalid fMode");
    }
    return Result::Ok();
}

SkISize CodecSrc::size() const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return {0, 0};
    }

    if (fMode != kAnimated_Mode) {
        return codec->getScaledDimensions(fScale);
    }

    // We'll draw one of each frame, so make it big enough to hold them all
    // in a grid. The grid will be roughly square, with "factor" frames per
    // row and up to "factor" rows.
    const size_t count = codec->getFrameInfo().size();
    const float root = sqrt((float) count);
    const int factor = sk_float_ceil2int(root);

    auto androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec));
    auto imageSize = androidCodec->getSampledDimensions(1 / fScale);
    imageSize.fWidth  = imageSize.fWidth  * factor;
    imageSize.fHeight = imageSize.fHeight * sk_float_ceil2int((float) count / (float) factor);
    return imageSize;
}

Name CodecSrc::name() const {
    Name name = SkOSPath::Basename(fPath.c_str());
    if (fMode == kAnimated_Mode) {
        name.append("_animated");
    }
    if (1.0f == fScale) {
        return name;
    }
    return get_scaled_name(name.c_str(), fScale);
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

Result AndroidCodecSrc::draw(SkCanvas* canvas) const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return Result::Fatal("Couldn't read %s.", fPath.c_str());
    }
    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return Result::Fatal("Couldn't create android codec for %s.", fPath.c_str());
    }

    SkImageInfo decodeInfo = codec->getInfo();
    if (!get_decode_info(&decodeInfo, canvas->imageInfo().colorType(), fDstColorType,
                         fDstAlphaType)) {
        return Result::Skip("Skipping uninteresting test.");
    }

    // Scale the image if it is desired.
    SkISize size = codec->getSampledDimensions(fSampleSize);

    // Visually inspecting very small output images is not necessary.  We will
    // cover these cases in unit testing.
    if ((size.width() <= 10 || size.height() <= 10) && 1 != fSampleSize) {
        return Result::Skip("Scaling very small images is uninteresting.");
    }
    decodeInfo = decodeInfo.makeDimensions(size);

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
            return Result::Fatal("Couldn't getPixels %s.", fPath.c_str());
    }
    draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes, fDstColorType);
    return Result::Ok();
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

Result ImageGenSrc::draw(SkCanvas* canvas) const {
    if (kRGB_565_SkColorType == canvas->imageInfo().colorType()) {
        return Result::Skip("Uninteresting to test image generator to 565.");
    }

    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return Result::Fatal("Couldn't read %s.", fPath.c_str());
    }

#if defined(SK_BUILD_FOR_WIN)
    // Initialize COM in order to test with WIC.
    SkAutoCoInitialize com;
    if (!com.succeeded()) {
        return Result::Fatal("Could not initialize COM.");
    }
#endif

    std::unique_ptr<SkImageGenerator> gen(nullptr);
    switch (fMode) {
        case kCodec_Mode:
            gen = SkCodecImageGenerator::MakeFromEncodedCodec(encoded);
            if (!gen) {
                return Result::Fatal("Could not create codec image generator.");
            }
            break;
        case kPlatform_Mode: {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
            gen = SkImageGeneratorCG::MakeFromEncodedCG(encoded);
#elif defined(SK_BUILD_FOR_WIN)
            gen = SkImageGeneratorWIC::MakeFromEncodedWIC(encoded);
#elif defined(SK_ENABLE_NDK_IMAGES)
            gen = SkImageGeneratorNDK::MakeFromEncodedNDK(encoded);
#endif
            if (!gen) {
                return Result::Fatal("Could not create platform image generator.");
            }
            break;
        }
        default:
            SkASSERT(false);
            return Result::Fatal("Invalid image generator mode");
    }

    // Test deferred decoding path on GPU
    if (fIsGpu) {
        sk_sp<SkImage> image(SkImages::DeferredFromGenerator(std::move(gen)));
        if (!image) {
            return Result::Fatal("Could not create image from codec image generator.");
        }
        canvas->drawImage(image, 0, 0);
        return Result::Ok();
    }

    // Test various color and alpha types on CPU
    SkImageInfo decodeInfo = gen->getInfo().makeAlphaType(fDstAlphaType);

    int bpp = decodeInfo.bytesPerPixel();
    size_t rowBytes = decodeInfo.width() * bpp;
    SkAutoMalloc pixels(decodeInfo.height() * rowBytes);
    if (!gen->getPixels(decodeInfo, pixels.get(), rowBytes)) {
        Result::Status status = Result::Status::Fatal;
#if defined(SK_BUILD_FOR_WIN)
        if (kPlatform_Mode == fMode) {
            // Do not issue a fatal error for WIC flakiness.
            status = Result::Status::Skip;
        }
#endif
        return Result(
                status,
                SkStringPrintf("Image generator could not getPixels() for %s\n", fPath.c_str()));
    }

    set_bitmap_color_space(&decodeInfo);
    draw_to_canvas(canvas, decodeInfo, pixels.get(), rowBytes,
                   CodecSrc::kGetFromCanvas_DstColorType);
    return Result::Ok();
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

Result ColorCodecSrc::draw(SkCanvas* canvas) const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return Result::Fatal("Couldn't read %s.", fPath.c_str());
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec) {
        return Result::Fatal("Couldn't create codec for %s.", fPath.c_str());
    }

    SkImageInfo info = codec->getInfo();
    if (fDecodeToDst) {
        SkImageInfo canvasInfo = canvas->imageInfo();
        if (!canvasInfo.colorSpace()) {
            // This will skip color conversion, and the resulting images will
            // look different from images they are compared against in Gold, but
            // that doesn't mean they are wrong. We have a test verifying that
            // passing a null SkColorSpace skips conversion, so skip this
            // misleading test.
            return Result::Skip("Skipping decoding without color transform.");
        }
        info = canvasInfo.makeDimensions(info.dimensions());
    }

    auto [image, result] = codec->getImage(info);
    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kErrorInInput:
        case SkCodec::kIncompleteInput:
            canvas->drawImage(image, 0,0);
            return Result::Ok();
        case SkCodec::kInvalidConversion:
            // TODO(mtklein): why are there formats we can't decode to?
            return Result::Skip("SkCodec can't decode to this format.");
        default:
            return Result::Fatal("Couldn't getPixels %s. Error code %d", fPath.c_str(), result);
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

Result SKPSrc::draw(SkCanvas* canvas) const {

    struct DeserializationContext {
        GrDirectContext*           fDirectContext = nullptr;
#if defined(SK_GRAPHITE)
        skgpu::graphite::Recorder* fRecorder = nullptr;
#endif
    } ctx {
        GrAsDirectContext(canvas->recordingContext()),
#if defined(SK_GRAPHITE)
        canvas->recorder()
#endif
    };

    SkDeserialProcs procs;
    procs.fImageProc = [](const void* data, size_t size, void* ctx) -> sk_sp<SkImage> {
        sk_sp<SkData> tmpData = SkData::MakeWithoutCopy(data, size);
        sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(std::move(tmpData));
        image = image->makeRasterImage(); // force decoding

        if (image) {
            DeserializationContext* context = reinterpret_cast<DeserializationContext*>(ctx);

            if (context->fDirectContext) {
                return SkImages::TextureFromImage(context->fDirectContext, image);
            }
        }
        return image;
    };
    procs.fImageCtx = &ctx;

    // SKPs may have typefaces encoded in them (e.g. with FreeType). We can try falling back
    // to the Test FontMgr (possibly a native one) if we have do not have FreeType built-in.
    procs.fTypefaceProc = [](const void* data, size_t size, void*) -> sk_sp<SkTypeface> {
        SkStream** stream = reinterpret_cast<SkStream**>(const_cast<void*>(data));
        return SkTypeface::MakeDeserialize(*stream, ToolUtils::TestFontMgr());
    };


    std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(fPath.c_str());
    if (!stream) {
        return Result::Fatal("Couldn't read %s.", fPath.c_str());
    }
    sk_sp<SkPicture> pic(SkPicture::MakeFromStream(stream.get(), &procs));
    if (!pic) {
        return Result::Fatal("Couldn't parse file %s.", fPath.c_str());
    }
    stream = nullptr;  // Might as well drop this when we're done with it.
    canvas->clipRect(SkRect::MakeWH(FLAGS_skpViewportSize, FLAGS_skpViewportSize));
    canvas->drawPicture(pic);
    return Result::Ok();
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

Result BisectSrc::draw(SkCanvas* canvas) const {
    struct FoundPath {
        SkPath fPath;
        SkPaint fPaint;
        SkMatrix fViewMatrix;
    };

    // This subclass of SkCanvas just extracts all the SkPaths (drawn via drawPath) from an SKP.
    class PathFindingCanvas : public SkCanvas {
    public:
        PathFindingCanvas(int width, int height) : SkCanvas(width, height, nullptr) {}
        const TArray<FoundPath>& foundPaths() const { return fFoundPaths; }

    private:
        void onDrawPath(const SkPath& path, const SkPaint& paint) override {
            fFoundPaths.push_back() = {path, paint, this->getTotalMatrix()};
        }

        TArray<FoundPath> fFoundPaths;
    };

    PathFindingCanvas pathFinder(canvas->getBaseLayerSize().width(),
                                 canvas->getBaseLayerSize().height());
    Result result = this->INHERITED::draw(&pathFinder);
    if (!result.isOk()) {
        return result;
    }

    int start = 0, end = pathFinder.foundPaths().size();
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

    return Result::Ok();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#if defined(SK_ENABLE_SKOTTIE)
static DEFINE_bool(useLottieGlyphPaths, false,
                   "Prioritize embedded glyph paths over native fonts.");

SkottieSrc::SkottieSrc(Path path) : fPath(std::move(path)) {}

Result SkottieSrc::draw(SkCanvas* canvas) const {
    auto predecode = skresources::ImageDecodeStrategy::kPreDecode;
    auto resource_provider = skresources::DataURIResourceProviderProxy::Make(
            skresources::FileResourceProvider::Make(SkOSPath::Dirname(fPath.c_str()), predecode),
            predecode,
            ToolUtils::TestFontMgr());

    static constexpr char kInterceptPrefix[] = "__";
    auto precomp_interceptor =
            sk_make_sp<skottie_utils::ExternalAnimationPrecompInterceptor>(resource_provider,
                                                                           kInterceptPrefix);
    uint32_t flags = 0;
    if (FLAGS_useLottieGlyphPaths) {
        flags |= skottie::Animation::Builder::kPreferEmbeddedFonts;
    }

    auto animation = skottie::Animation::Builder(flags)
        .setFontManager(ToolUtils::TestFontMgr())
        .setResourceProvider(std::move(resource_provider))
        .setPrecompInterceptor(std::move(precomp_interceptor))
        .makeFromFile(fPath.c_str());
    if (!animation) {
        return Result::Fatal("Unable to parse file: %s", fPath.c_str());
    }

    canvas->drawColor(SK_ColorWHITE);

    const auto t_rate = 1.0f / (kTileCount * kTileCount - 1);

    // Draw the frames in a shuffled order to exercise non-linear
    // frame progression. The film strip will still be in order left-to-right,
    // top-down, just not drawn in that order.
    static constexpr int frameOrder[] = { 4, 0, 3, 1, 2 };
    static_assert(std::size(frameOrder) == kTileCount, "");

    for (int i = 0; i < kTileCount; ++i) {
        const SkScalar y = frameOrder[i] * kTileSize;

        for (int j = 0; j < kTileCount; ++j) {
            const SkScalar x = frameOrder[j] * kTileSize;
            SkRect dest = SkRect::MakeXYWH(x, y, kTileSize, kTileSize);

            const auto t = t_rate * (frameOrder[i] * kTileCount + frameOrder[j]);
            {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->clipRect(dest, true);
                canvas->concat(SkMatrix::RectToRect(SkRect::MakeSize(animation->size()), dest,
                                                    SkMatrix::kCenter_ScaleToFit));
                animation->seek(t);
                animation->render(canvas);
            }
        }
    }

    return Result::Ok();
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
#if defined(SK_ENABLE_SVG)
// Used when the image doesn't have an intrinsic size.
static const SkSize kDefaultSVGSize = {1000, 1000};

// Used to force-scale tiny fixed-size images.
static const SkSize kMinimumSVGSize = {128, 128};

SVGSrc::SVGSrc(Path path)
    : fName(SkOSPath::Basename(path.c_str()))
    , fScale(1) {

    auto stream = SkStream::MakeFromFile(path.c_str());
    if (!stream) {
        return;
    }

    auto predecode = skresources::ImageDecodeStrategy::kPreDecode;
    auto rp = skresources::DataURIResourceProviderProxy::Make(
            skresources::FileResourceProvider::Make(SkOSPath::Dirname(path.c_str()), predecode),
            predecode,
            ToolUtils::TestFontMgr());

    fDom = SkSVGDOM::Builder().setResourceProvider(std::move(rp))
                              .setFontManager(ToolUtils::TestFontMgr())
                              .make(*stream);
    if (!fDom) {
        return;
    }

    const SkSize& sz = fDom->containerSize();
    if (sz.isEmpty()) {
        // no intrinsic size
        fDom->setContainerSize(kDefaultSVGSize);
    } else {
        fScale = std::max(1.f, std::max(kMinimumSVGSize.width()  / sz.width(),
                                        kMinimumSVGSize.height() / sz.height()));
    }
}

Result SVGSrc::draw(SkCanvas* canvas) const {
    if (!fDom) {
        return Result::Fatal("Unable to parse file: %s", fName.c_str());
    }

    SkAutoCanvasRestore acr(canvas, true);
    canvas->scale(fScale, fScale);
    canvas->drawColor(SK_ColorWHITE);
    fDom->render(canvas);

    return Result::Ok();
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

#endif // defined(SK_ENABLE_SVG)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

MSKPSrc::MSKPSrc(Path path) : fPath(path) {
    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(fPath.c_str());
    int count = SkMultiPictureDocument::ReadPageCount(stream.get());
    if (count > 0) {
        fPages.reset(count);
        SkASSERT_RELEASE(SkMultiPictureDocument::ReadPageSizes(stream.get(), &fPages[0],
                                                               fPages.size()));
    }
}

int MSKPSrc::pageCount() const { return fPages.size(); }

SkISize MSKPSrc::size() const { return this->size(FLAGS_mskpFrame); }
SkISize MSKPSrc::size(int i) const {
    return i >= 0 && i < fPages.size() ? fPages[i].fSize.toCeil() : SkISize{0, 0};
}

Result MSKPSrc::draw(SkCanvas* c) const {
    return this->draw(FLAGS_mskpFrame, c);
}
Result MSKPSrc::draw(int i, SkCanvas* canvas) const {
    if (this->pageCount() == 0) {
        return Result::Fatal("Unable to parse MultiPictureDocument file: %s", fPath.c_str());
    }
    if (i >= fPages.size() || i < 0) {
        return Result::Fatal("MultiPictureDocument page number out of range: %d", i);
    }
    SkPicture* page = fPages[i].fPicture.get();
    if (!page) {
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(fPath.c_str());
        if (!stream) {
            return Result::Fatal("Unable to open file: %s", fPath.c_str());
        }
        if (!SkMultiPictureDocument::Read(stream.get(), &fPages[0], fPages.size())) {
            return Result::Fatal("SkMultiPictureDocument reader failed on page %d: %s", i,
                                 fPath.c_str());
        }
        page = fPages[i].fPicture.get();
    }
    canvas->drawPicture(page);
    return Result::Ok();
}

Name MSKPSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Result NullSink::draw(const Src& src, SkBitmap*, SkWStream*, SkString*) const {
    return src.draw(SkMakeNullCanvas().get());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static Result compare_bitmaps(const SkBitmap& reference, const SkBitmap& bitmap) {
    // The dimensions are a property of the Src only, and so should be identical.
    SkASSERT(reference.computeByteSize() == bitmap.computeByteSize());
    if (reference.computeByteSize() != bitmap.computeByteSize()) {
        return Result::Fatal("Dimensions don't match reference");
    }
    // All SkBitmaps in DM are tight, so this comparison is easy.
    if (0 != memcmp(reference.getPixels(), bitmap.getPixels(), reference.computeByteSize())) {
        SkString encoded;
        SkString errString("Pixels don't match reference");
        if (ToolUtils::BitmapToBase64DataURI(reference, &encoded)) {
            errString.append("\nExpected: ");
            errString.append(encoded);
        } else {
            errString.append("\nExpected image failed to encode: ");
            errString.append(encoded);
        }
        if (ToolUtils::BitmapToBase64DataURI(bitmap, &encoded)) {
            errString.append("\nActual: ");
            errString.append(encoded);
        } else {
            errString.append("\nActual image failed to encode: ");
            errString.append(encoded);
        }
        return Result(Result::Status::Fatal, errString);
    }
    return Result::Ok();
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
        , fSurfaceFlags(config->getSurfaceFlags())
        , fColorType(config->getColorType())
        , fAlphaType(config->getAlphaType())
        , fBaseContextOptions(grCtxOptions) {
    if (FLAGS_programBinaryCache) {
        fBaseContextOptions.fPersistentCache = &fMemoryCache;
    }
}

Result GPUSink::draw(const Src& src, SkBitmap* dst, SkWStream* dstStream, SkString* log) const {
    return this->onDraw(src, dst, dstStream, log, fBaseContextOptions);
}

sk_sp<SkSurface> GPUSink::createDstSurface(GrDirectContext* context, SkISize size) const {
    sk_sp<SkSurface> surface;

    SkImageInfo info = SkImageInfo::Make(size, this->colorInfo());
    SkSurfaceProps props(fSurfaceFlags, kRGB_H_SkPixelGeometry);

    switch (fSurfType) {
        case SkCommandLineConfigGpu::SurfType::kDefault:
            surface = SkSurfaces::RenderTarget(
                    context, skgpu::Budgeted::kNo, info, fSampleCount, &props);
            break;
        case SkCommandLineConfigGpu::SurfType::kBackendTexture:
            surface = sk_gpu_test::MakeBackendTextureSurface(context,
                                                             info,
                                                             kTopLeft_GrSurfaceOrigin,
                                                             fSampleCount,
                                                             skgpu::Mipmapped::kNo,
                                                             GrProtected::kNo,
                                                             &props);
            break;
        case SkCommandLineConfigGpu::SurfType::kBackendRenderTarget:
            surface = sk_gpu_test::MakeBackendRenderTargetSurface(context,
                                                                  info,
                                                                  kBottomLeft_GrSurfaceOrigin,
                                                                  fSampleCount,
                                                                  GrProtected::kNo,
                                                                  &props);
            break;
    }

    return surface;
}

bool GPUSink::readBack(SkSurface* surface, SkBitmap* dst) const {
    SkCanvas* canvas = surface->getCanvas();
    SkISize size = surface->imageInfo().dimensions();

    SkImageInfo info = SkImageInfo::Make(size, this->colorInfo());
    dst->allocPixels(info);
    return canvas->readPixels(*dst, 0, 0);
}

Result GPUSink::onDraw(const Src& src, SkBitmap* dst, SkWStream*, SkString* log,
                       const GrContextOptions& baseOptions,
                       std::function<void(GrDirectContext*)> initContext,
                       std::function<SkCanvas*(SkCanvas*)> wrapCanvas) const {
    GrContextOptions grOptions = baseOptions;

    // We don't expect the src to mess with the persistent cache or the executor.
    SkDEBUGCODE(auto cache = grOptions.fPersistentCache);
    SkDEBUGCODE(auto exec = grOptions.fExecutor);
    src.modifyGrContextOptions(&grOptions);
    SkASSERT(cache == grOptions.fPersistentCache);
    SkASSERT(exec == grOptions.fExecutor);

    GrContextFactory factory(grOptions);
    auto direct = factory.getContextInfo(fContextType, fContextOverrides).directContext();
    if (initContext) {
        initContext(direct);
    }

    const int maxDimension = direct->priv().caps()->maxTextureSize();
    if (maxDimension < std::max(src.size().width(), src.size().height())) {
        return Result::Skip("Src too large to create a texture.\n");
    }

    sk_sp<SkSurface> surface = this->createDstSurface(direct, src.size());
    if (!surface) {
        return Result::Fatal("Could not create a surface.");
    }
    if (FLAGS_preAbandonGpuContext) {
        factory.abandonContexts();
    }

    auto canvas = surface->getCanvas();
    if (wrapCanvas != nullptr) {
        canvas = wrapCanvas(canvas);
    }

    Result result = src.draw(canvas);
    if (!result.isOk()) {
        return result;
    }
    direct->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    if (FLAGS_gpuStats) {
        direct->priv().dumpCacheStats(log);
        direct->priv().dumpGpuStats(log);
        direct->priv().dumpContextStats(log);
    }

    this->readBack(surface.get(), dst);

    if (FLAGS_abandonGpuContext) {
        factory.abandonContexts();
    } else if (FLAGS_releaseAndAbandonGpuContext) {
        factory.releaseResourcesAndAbandonContexts();
    }

    if (grOptions.fPersistentCache) {
        direct->storeVkPipelineCacheData();
    }
    return Result::Ok();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
GPUSlugSink::GPUSlugSink(const SkCommandLineConfigGpu* config, const GrContextOptions& options)
        : GPUSink(config, options) {}

Result GPUSlugSink::draw(const Src& src, SkBitmap* dst, SkWStream* write, SkString* log) const {
    GrContextOptions grOptions = this->baseContextOptions();
    // Force padded atlas entries for slug drawing.
    grOptions.fSupportBilerpFromGlyphAtlas |= true;

    SkTLazy<SkTestCanvas<SkSlugTestKey>> testCanvas;

    return onDraw(src, dst, write, log, grOptions, nullptr,
        [&](SkCanvas* canvas){
            testCanvas.init(canvas);
            return testCanvas.get();
        });
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
GPUSerializeSlugSink::GPUSerializeSlugSink(
        const SkCommandLineConfigGpu* config, const GrContextOptions& options)
    : GPUSink(config, options) {}

Result GPUSerializeSlugSink::draw(
        const Src& src, SkBitmap* dst, SkWStream* write, SkString* log) const {
    GrContextOptions grOptions = this->baseContextOptions();
    // Force padded atlas entries for slug drawing.
    grOptions.fSupportBilerpFromGlyphAtlas |= true;

    SkTLazy<SkTestCanvas<SkSerializeSlugTestKey>> testCanvas;

    return onDraw(src, dst, write, log, grOptions, nullptr,
                  [&](SkCanvas* canvas){
                      testCanvas.init(canvas);
                      return testCanvas.get();
                  });
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
GPURemoteSlugSink::GPURemoteSlugSink(
        const SkCommandLineConfigGpu* config, const GrContextOptions& options)
        : GPUSink(config, options) {}

Result GPURemoteSlugSink::draw(
        const Src& src, SkBitmap* dst, SkWStream* write, SkString* log) const {
    GrContextOptions grOptions = this->baseContextOptions();
    // Force padded atlas entries for slug drawing.
    grOptions.fSupportBilerpFromGlyphAtlas |= true;

    SkTLazy<SkTestCanvas<SkRemoteSlugTestKey>> testCanvas;

    return onDraw(src, dst, write, log, grOptions, nullptr,
                  [&](SkCanvas* canvas) {
                      testCanvas.init(canvas);
                      return testCanvas.get();
                  });
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
GPUPersistentCacheTestingSink::GPUPersistentCacheTestingSink(const SkCommandLineConfigGpu* config,
                                                             const GrContextOptions& grCtxOptions)
    : INHERITED(config, grCtxOptions)
    , fCacheType(config->getTestPersistentCache()) {}

Result GPUPersistentCacheTestingSink::draw(const Src& src, SkBitmap* dst, SkWStream* wStream,
                                           SkString* log) const {
    // Draw twice, once with a cold cache, and again with a warm cache. Verify that we get the same
    // result.
    sk_gpu_test::MemoryCache memoryCache;
    GrContextOptions contextOptions = this->baseContextOptions();
    contextOptions.fPersistentCache = &memoryCache;
    if (fCacheType == 2) {
        contextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kBackendSource;
    }

    Result result = this->onDraw(src, dst, wStream, log, contextOptions);
    if (!result.isOk() || !dst) {
        return result;
    }

    SkBitmap reference;
    SkString refLog;
    SkDynamicMemoryWStream refStream;
    memoryCache.resetCacheStats();
    Result refResult = this->onDraw(src, &reference, &refStream, &refLog, contextOptions);
    if (!refResult.isOk()) {
        return refResult;
    }
    SkASSERT(!memoryCache.numCacheMisses());
    SkASSERT(!memoryCache.numCacheStores());

    return compare_bitmaps(reference, *dst);
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

GPUPrecompileTestingSink::GPUPrecompileTestingSink(const SkCommandLineConfigGpu* config,
                                                   const GrContextOptions& grCtxOptions)
    : INHERITED(config, grCtxOptions) {}

Result GPUPrecompileTestingSink::draw(const Src& src, SkBitmap* dst, SkWStream* wStream,
                                      SkString* log) const {
    // Three step process:
    // 1) Draw once with an SkSL cache, and store off the shader blobs.
    // 2) For the second context, pre-compile the shaders to warm the cache.
    // 3) Draw with the second context, ensuring that we get the same result, and no cache misses.
    sk_gpu_test::MemoryCache memoryCache;
    GrContextOptions contextOptions = this->baseContextOptions();
    contextOptions.fPersistentCache = &memoryCache;
    contextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;

    Result result = this->onDraw(src, dst, wStream, log, contextOptions);
    if (!result.isOk() || !dst) {
        return result;
    }

    auto precompileShaders = [&memoryCache](GrDirectContext* dContext) {
        memoryCache.foreach([dContext](sk_sp<const SkData> key,
                                       sk_sp<SkData> data,
                                       const SkString& /*description*/,
                                       int /*count*/) {
            SkAssertResult(dContext->precompileShader(*key, *data));
        });
    };

    sk_gpu_test::MemoryCache replayCache;
    GrContextOptions replayOptions = this->baseContextOptions();
    // Ensure that the runtime cache is large enough to hold all of the shaders we pre-compile
    replayOptions.fRuntimeProgramCacheSize = memoryCache.numCacheMisses();
    replayOptions.fPersistentCache = &replayCache;

    SkBitmap reference;
    SkString refLog;
    SkDynamicMemoryWStream refStream;
    Result refResult = this->onDraw(src, &reference, &refStream, &refLog, replayOptions,
                                    precompileShaders);
    if (!refResult.isOk()) {
        return refResult;
    }
    SkASSERT(!replayCache.numCacheMisses());

    return compare_bitmaps(reference, *dst);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
GPUDDLSink::GPUDDLSink(const SkCommandLineConfigGpu* config, const GrContextOptions& ctxOptions)
        : INHERITED(config, ctxOptions)
        , fRecordingExecutor(SkExecutor::MakeLIFOThreadPool(1))
        , fGPUExecutor(SkExecutor::MakeFIFOThreadPool(1, false)) {
}

Result GPUDDLSink::ddlDraw(const Src& src,
                           sk_sp<SkSurface> dstSurface,
                           SkTaskGroup* recordingTaskGroup,
                           SkTaskGroup* gpuTaskGroup,
                           sk_gpu_test::TestContext* gpuTestCtx,
                           GrDirectContext* dContext) const {

    // We have to do this here bc characterization can hit the SkGpuDevice's thread guard (i.e.,
    // leaving it until the DDLTileHelper ctor will result in multiple threads trying to use the
    // same context (this thread and the gpuThread - which will be uploading textures)).
    GrSurfaceCharacterization dstCharacterization;
    SkAssertResult(dstSurface->characterize(&dstCharacterization));

    auto size = src.size();
    SkPictureRecorder recorder;
    Result result = src.draw(recorder.beginRecording(SkIntToScalar(size.width()),
                                                     SkIntToScalar(size.height())));
    if (!result.isOk()) {
        return result;
    }
    sk_sp<SkPicture> inputPicture(recorder.finishRecordingAsPicture());

    // this is our ultimate final drawing area/rect
    SkIRect viewport = SkIRect::MakeWH(size.fWidth, size.fHeight);

    auto supportedYUVADataTypes = skgpu::ganesh::SupportedTextureFormats(*dContext);
    DDLPromiseImageHelper promiseImageHelper(supportedYUVADataTypes);
    sk_sp<SkPicture> newSKP = promiseImageHelper.recreateSKP(dContext, inputPicture.get());
    if (!newSKP) {
        return Result::Fatal("GPUDDLSink: Couldn't recreate the SKP");
    }

    // 'gpuTestCtx/gpuThreadCtx' is being shifted to the gpuThread. Leave the main (this)
    // thread w/o a context.
    gpuTestCtx->makeNotCurrent();

    // Job one for the GPU thread is to make 'gpuTestCtx' current!
    gpuTaskGroup->add([gpuTestCtx] { gpuTestCtx->makeCurrent(); });

    // TODO: move the image upload to the utility thread
    promiseImageHelper.uploadAllToGPU(gpuTaskGroup, dContext);

    // Care must be taken when using 'gpuThreadCtx' bc it moves between the gpu-thread and this
    // one. About all it can be consistently used for is GrCaps access and 'defaultBackendFormat'
    // calls.
    constexpr int kNumDivisions = 3;
    DDLTileHelper tiles(dContext, dstCharacterization, viewport,
                        kNumDivisions, kNumDivisions,
                        /* addRandomPaddingToDst */ false);

    tiles.createBackendTextures(gpuTaskGroup, dContext);

    tiles.kickOffThreadedWork(recordingTaskGroup, gpuTaskGroup, dContext, newSKP.get());

    // We have to wait for the recording threads to schedule all their work on the gpu thread
    // before we can schedule the composition draw and the flush. Note that the gpu thread
    // is not blocked at this point and this thread is borrowing recording work.
    recordingTaskGroup->wait();

    // Note: at this point the recording thread(s) are stalled out w/ nothing to do.

    if (FLAGS_preAbandonGpuContext) {
        dContext->abandonContext();
    }

    // The recording threads have already scheduled the drawing of each tile's DDL on the gpu
    // thread. The composition DDL must be scheduled last bc it relies on the result of all
    // the tiles' rendering. Additionally, bc we're aliasing the tiles' backend textures,
    // there is nothing in the DAG to automatically force the required order.
    gpuTaskGroup->add([dstSurface, ddl = tiles.composeDDL()]() {
                          skgpu::ganesh::DrawDDL(dstSurface, ddl);
                      });

    // This should be the only explicit flush for the entire DDL draw.
    gpuTaskGroup->add([dContext]() {
                                           // We need to ensure all the GPU work is finished so
                                           // the following 'deleteAllFromGPU' call will work
                                           // on Vulkan.
                                           // TODO: switch over to using the promiseImage callbacks
                                           // to free the backendTextures. This is complicated a
                                           // bit by which thread possesses the direct context.
                                           dContext->flush();
                                           dContext->submit(GrSyncCpu::kYes);
                                       });

    // The backend textures are created on the gpuThread by the 'uploadAllToGPU' call.
    // It is simpler to also delete them at this point on the gpuThread.
    promiseImageHelper.deleteAllFromGPU(gpuTaskGroup, dContext);

    tiles.deleteBackendTextures(gpuTaskGroup, dContext);

    // A flush has already been scheduled on the gpu thread along with the clean up of the backend
    // textures so it is safe to schedule making 'gpuTestCtx' not current on the gpuThread.
    gpuTaskGroup->add([gpuTestCtx] { gpuTestCtx->makeNotCurrent(); });

    // All the work is scheduled on the gpu thread, we just need to wait
    gpuTaskGroup->wait();

    return Result::Ok();
}

Result GPUDDLSink::draw(const Src& src, SkBitmap* dst, SkWStream*, SkString* log) const {
    GrContextOptions contextOptions = this->baseContextOptions();
    src.modifyGrContextOptions(&contextOptions);
    contextOptions.fPersistentCache = nullptr;
    contextOptions.fExecutor = nullptr;

    GrContextFactory factory(contextOptions);

    // This captures the context destined to be the main gpu context
    ContextInfo mainCtxInfo = factory.getContextInfo(this->contextType(), this->contextOverrides());
    sk_gpu_test::TestContext* mainTestCtx = mainCtxInfo.testContext();
    auto mainCtx = mainCtxInfo.directContext();
    if (!mainCtx) {
        return Result::Fatal("Could not create context.");
    }

    SkASSERT(mainCtx->priv().getGpu());

    // TODO: make use of 'otherCtx' for uploads & compilation
#if 0
    // This captures the context destined to be the utility context. It is in a share group
    // with the main context
    ContextInfo otherCtxInfo = factory.getSharedContextInfo(mainCtx);
    sk_gpu_test::TestContext* otherTestCtx = otherCtxInfo.testContext();
    auto otherCtx = otherCtxInfo.directContext();
    if (!otherCtx) {
        return Result::Fatal("Cound not create shared context.");
    }

    SkASSERT(otherCtx->priv().getGpu());
#endif

    SkTaskGroup recordingTaskGroup(*fRecordingExecutor);
    SkTaskGroup gpuTaskGroup(*fGPUExecutor);

    // Make sure 'mainCtx' is current
    mainTestCtx->makeCurrent();

    sk_sp<SkSurface> surface = this->createDstSurface(mainCtx, src.size());
    if (!surface) {
        return Result::Fatal("Could not create a surface.");
    }

    Result result = this->ddlDraw(src, surface, &recordingTaskGroup, &gpuTaskGroup,
                                  mainTestCtx, mainCtx);
    if (!result.isOk()) {
        return result;
    }

    // 'ddlDraw' will have made 'mainCtx' not current on the gpuThread
    mainTestCtx->makeCurrent();

    if (FLAGS_gpuStats) {
        mainCtx->priv().dumpCacheStats(log);
        mainCtx->priv().dumpGpuStats(log);
        mainCtx->priv().dumpContextStats(log);

#if 0
        otherCtx->priv().dumpCacheStats(log);
        otherCtx->priv().dumpGpuStats(log);
        otherCtx->priv().dumpContextStats(log);
#endif
    }

    if (!this->readBack(surface.get(), dst)) {
        return Result::Fatal("Could not readback from surface.");
    }

    return Result::Ok();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static Result draw_skdocument(const Src& src, SkDocument* doc, SkWStream* dst) {
    if (src.size().isEmpty()) {
        return Result::Fatal("Source has empty dimensions");
    }
    SkASSERT(doc);
    int pageCount = src.pageCount();
    for (int i = 0; i < pageCount; ++i) {
        int width = src.size(i).width(), height = src.size(i).height();
        SkCanvas* canvas =
                doc->beginPage(SkIntToScalar(width), SkIntToScalar(height));
        if (!canvas) {
            return Result::Fatal("SkDocument::beginPage(w,h) returned nullptr");
        }
        Result result = src.draw(i, canvas);
        if (!result.isOk()) {
            return result;
        }
        doc->endPage();
    }
    doc->close();
    dst->flush();
    return Result::Ok();
}

Result PDFSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
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
        return Result::Fatal("SkPDF::MakeDocument() returned nullptr");
    }
    return draw_skdocument(src, doc.get(), dst);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

XPSSink::XPSSink() {}

#if defined(SK_SUPPORT_XPS)
static SkTScopedComPtr<IXpsOMObjectFactory> make_xps_factory() {
    IXpsOMObjectFactory* factory;
    HRN(CoCreateInstance(CLSID_XpsOMObjectFactory,
                         nullptr,
                         CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&factory)));
    return SkTScopedComPtr<IXpsOMObjectFactory>(factory);
}

Result XPSSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    SkAutoCoInitialize com;
    if (!com.succeeded()) {
        return Result::Fatal("Could not initialize COM.");
    }
    SkTScopedComPtr<IXpsOMObjectFactory> factory = make_xps_factory();
    if (!factory) {
        return Result::Fatal("Failed to create XPS Factory.");
    }
    auto doc = SkXPS::MakeDocument(dst, factory.get());
    if (!doc) {
        return Result::Fatal("SkXPS::MakeDocument() returned nullptr");
    }
    return draw_skdocument(src, doc.get(), dst);
}
#else
Result XPSSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    return Result::Fatal("XPS not supported on this platform.");
}
#endif

static SkSerialProcs serial_procs_using_png() {
    static SkSerialProcs procs;
    procs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
        return SkPngEncoder::Encode(as_IB(img)->directContext(), img, SkPngEncoder::Options{});
    };
    return procs;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SKPSink::SKPSink() {}

Result SKPSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    auto size = SkSize::Make(src.size());
    SkPictureRecorder recorder;
    Result result = src.draw(recorder.beginRecording(size.width(), size.height()));
    if (!result.isOk()) {
        return result;
    }
    SkSerialProcs procs = serial_procs_using_png();
    recorder.finishRecordingAsPicture()->serialize(dst, &procs);
    return Result::Ok();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Result DebugSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    DebugCanvas debugCanvas(src.size().width(), src.size().height());
    Result result = src.draw(&debugCanvas);
    if (!result.isOk()) {
        return result;
    }
    std::unique_ptr<SkCanvas> nullCanvas = SkMakeNullCanvas();
    UrlDataManager dataManager(SkString("data"));
    SkJSONWriter writer(dst, SkJSONWriter::Mode::kPretty);
    writer.beginObject(); // root
    debugCanvas.toJSON(writer, dataManager, nullCanvas.get());
    writer.endObject(); // root
    writer.flush();
    return Result::Ok();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SVGSink::SVGSink(int pageIndex) : fPageIndex(pageIndex) {}

Result SVGSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
#if defined(SK_ENABLE_SVG)
    if (src.pageCount() > 1) {
        int pageCount = src.pageCount();
        if (fPageIndex > pageCount - 1) {
            return Result::Fatal("Page index %d too high for document with only %d pages.",
                                 fPageIndex, pageCount);
        }
    }
    return src.draw(fPageIndex,
                    SkSVGCanvas::Make(SkRect::MakeWH(SkIntToScalar(src.size().width()),
                                                     SkIntToScalar(src.size().height())),
                                      dst)
                            .get());
#else
    (void)fPageIndex;
    return Result::Fatal("SVG sink is disabled.");
#endif // SK_ENABLE_SVG
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

RasterSink::RasterSink(SkColorType colorType)
    : fColorType(colorType) {}

Result RasterSink::draw(const Src& src, SkBitmap* dst, SkWStream*, SkString*) const {
    const SkISize size = src.size();

    dst->allocPixelsFlags(SkImageInfo::Make(size, this->colorInfo()),
                          SkBitmap::kZeroPixels_AllocFlag);

    SkSurfaceProps props(/*flags=*/0, kRGB_H_SkPixelGeometry);
    auto surface = SkSurfaces::WrapPixels(dst->pixmap(), &props);
    return src.draw(surface->getCanvas());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#if defined(SK_GRAPHITE)

GraphiteSink::GraphiteSink(const SkCommandLineConfigGraphite* config)
        : fContextType(config->getContextType())
        , fColorType(config->getColorType())
        , fAlphaType(config->getAlphaType()) {}

Result GraphiteSink::draw(const Src& src,
                          SkBitmap* dst,
                          SkWStream* dstStream,
                          SkString* log) const {
    skgpu::graphite::ContextOptions options = fBaseContextOptions;
    // If we've copied context options from an external source we can't trust that the
    // priv pointer is still in scope, so assume it should be NULL and set our own up.
    SkASSERT(!options.fOptionsPriv);
    skgpu::graphite::ContextOptionsPriv optionsPriv;
    options.fOptionsPriv = &optionsPriv;

    src.modifyGraphiteContextOptions(&options);

    SkImageInfo ii = SkImageInfo::Make(src.size(), this->colorInfo());

    skiatest::graphite::ContextFactory factory(options);
    skiatest::graphite::ContextInfo ctxInfo = factory.getContextInfo(fContextType);
    skgpu::graphite::Context* context = ctxInfo.fContext;
    if (!context) {
        return Result::Fatal("Could not create a context.");
    }

    std::unique_ptr<skgpu::graphite::Recorder> recorder =
                                context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
    if (!recorder) {
        return Result::Fatal("Could not create a recorder.");
    }

    dst->allocPixels(ii);

    {
        SkSurfaceProps props(0, kRGB_H_SkPixelGeometry);
        sk_sp<SkSurface> surface =
                SkSurfaces::RenderTarget(recorder.get(), ii, skgpu::Mipmapped::kNo, &props);
        if (!surface) {
            return Result::Fatal("Could not create a surface.");
        }
        Result result = src.draw(surface->getCanvas());
        if (!result.isOk()) {
            return result;
        }

        SkPixmap pm;
        if (!dst->peekPixels(&pm) ||
            !surface->readPixels(pm, 0, 0)) {
            return Result::Fatal("Could not readback from surface.");
        }
    }

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        return Result::Fatal("Could not create a recording.");
    }

    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();
    if (!context->insertRecording(info)) {
        return Result::Fatal("Context::insertRecording failed.");
    }
    context->submit(skgpu::graphite::SyncToCpu::kYes);

    return Result::Ok();
}
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Handy for front-patching a Src.  Do whatever up-front work you need, then call draw_to_canvas(),
// passing the Sink draw() arguments, a size, and a function draws into an SkCanvas.
// Several examples below.

using DrawToCanvasFn = std::function<DM::Result(SkCanvas*)>;

static Result draw_to_canvas(Sink* sink, SkBitmap* bitmap, SkWStream* stream,
                             SkString* log, SkISize size, const DrawToCanvasFn& draw) {
    class ProxySrc : public Src {
    public:
        ProxySrc(SkISize size, const DrawToCanvasFn& draw) : fSize(size), fDraw(draw) {}
        Result draw(SkCanvas* canvas) const override {
            return fDraw(canvas);
        }
        Name    name() const override { return "ProxySrc"; }
        SkISize size() const override { return fSize; }
    private:
        SkISize               fSize;
        const DrawToCanvasFn& fDraw;
    };
    return sink->draw(ProxySrc(size, draw), bitmap, stream, log);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static DEFINE_bool(check, true, "If true, have most Via- modes fail if they affect the output.");

// Is *bitmap identical to what you get drawing src into sink?
static Result check_against_reference(const SkBitmap* bitmap, const Src& src, Sink* sink) {
    // We can only check raster outputs.
    // (Non-raster outputs like .pdf, .skp, .svg may differ but still draw identically.)
    if (FLAGS_check && bitmap) {
        SkBitmap reference;
        SkString log;
        SkDynamicMemoryWStream wStream;
        Result result = sink->draw(src, &reference, &wStream, &log);
        // If we can draw into this Sink via some pipeline, we should be able to draw directly.
        SkASSERT(result.isOk());
        if (!result.isOk()) {
            return result;
        }
        return compare_bitmaps(reference, *bitmap);
    }
    return Result::Ok();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static SkISize auto_compute_translate(SkMatrix* matrix, int srcW, int srcH) {
    SkRect bounds = SkRect::MakeIWH(srcW, srcH);
    matrix->mapRect(&bounds);
    matrix->postTranslate(-bounds.x(), -bounds.y());
    return {SkScalarRoundToInt(bounds.width()), SkScalarRoundToInt(bounds.height())};
}

ViaMatrix::ViaMatrix(SkMatrix matrix, Sink* sink) : Via(sink), fMatrix(matrix) {}

Result ViaMatrix::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    SkMatrix matrix = fMatrix;
    SkISize size = auto_compute_translate(&matrix, src.size().width(), src.size().height());
    return draw_to_canvas(fSink.get(), bitmap, stream, log, size,
                          [&](SkCanvas* canvas) {
                              canvas->concat(matrix);
                              return src.draw(canvas);
                          });
}

// Undoes any flip or 90 degree rotate without changing the scale of the bitmap.
// This should be pixel-preserving.
ViaUpright::ViaUpright(SkMatrix matrix, Sink* sink) : Via(sink), fMatrix(matrix) {}

Result ViaUpright::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    Result result = fSink->draw(src, bitmap, stream, log);
    if (!result.isOk()) {
        return result;
    }

    SkMatrix inverse;
    if (!fMatrix.rectStaysRect() || !fMatrix.invert(&inverse)) {
        return Result::Fatal("Cannot upright --matrix.");
    }
    SkMatrix upright = SkMatrix::I();
    upright.setScaleX(SkScalarSignAsScalar(inverse.getScaleX()));
    upright.setScaleY(SkScalarSignAsScalar(inverse.getScaleY()));
    upright.setSkewX(SkScalarSignAsScalar(inverse.getSkewX()));
    upright.setSkewY(SkScalarSignAsScalar(inverse.getSkewY()));

    SkBitmap uprighted;
    SkISize size = auto_compute_translate(&upright, bitmap->width(), bitmap->height());
    uprighted.allocPixels(bitmap->info().makeDimensions(size));

    SkCanvas canvas(uprighted);
    canvas.concat(upright);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas.drawImage(bitmap->asImage(), 0, 0, SkSamplingOptions(), &paint);

    *bitmap = uprighted;
    return Result::Ok();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Result ViaSerialization::draw(
        const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    // Record our Src into a picture.
    auto size = src.size();
    SkPictureRecorder recorder;
    Result result = src.draw(recorder.beginRecording(SkIntToScalar(size.width()),
                                                     SkIntToScalar(size.height())));
    if (!result.isOk()) {
        return result;
    }
    sk_sp<SkPicture> pic(recorder.finishRecordingAsPicture());

    SkSerialProcs procs = serial_procs_using_png();
    // Serialize it and then deserialize it.
    sk_sp<SkPicture> deserialized = SkPicture::MakeFromData(pic->serialize(&procs).get());

    result = draw_to_canvas(fSink.get(), bitmap, stream, log, size,
                            [&](SkCanvas* canvas) {
                                canvas->drawPicture(deserialized);
                                return Result::Ok();
                            });
    if (!result.isOk()) {
        return result;
    }

    return check_against_reference(bitmap, src, fSink.get());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Result ViaPicture::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    auto size = src.size();
    Result result = draw_to_canvas(fSink.get(), bitmap, stream, log, size,
                                   [&](SkCanvas* canvas) {
        SkPictureRecorder recorder;
        sk_sp<SkPicture> pic;
        Result result = src.draw(recorder.beginRecording(SkIntToScalar(size.width()),
                                                         SkIntToScalar(size.height())));
        if (!result.isOk()) {
            return result;
        }
        pic = recorder.finishRecordingAsPicture();
        canvas->drawPicture(pic);
        return result;
    });
    if (!result.isOk()) {
        return result;
    }

    return check_against_reference(bitmap, src, fSink.get());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Result ViaRuntimeBlend::draw(const Src& src,
                             SkBitmap* bitmap,
                             SkWStream* stream,
                             SkString* log) const {
    class RuntimeBlendFilterCanvas : public SkPaintFilterCanvas {
    public:
        RuntimeBlendFilterCanvas(SkCanvas* canvas) : INHERITED(canvas) { }

    protected:
        bool onFilter(SkPaint& paint) const override {
            if (std::optional<SkBlendMode> mode = paint.asBlendMode()) {
                paint.setBlender(GetRuntimeBlendForBlendMode(*mode));
            }
            return true;
        }

    private:
        using INHERITED = SkPaintFilterCanvas;
    };

    return draw_to_canvas(fSink.get(), bitmap, stream, log, src.size(),
                          [&](SkCanvas* canvas) {
        RuntimeBlendFilterCanvas runtimeBlendCanvas{canvas};
        return src.draw(&runtimeBlendCanvas);
    });
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef TEST_VIA_SVG
#include "include/svg/SkSVGCanvas.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "src/xml/SkXMLWriter.h"

Result ViaSVG::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    auto size = src.size();
    return draw_to_canvas(fSink.get(), bitmap, stream, log, size,
                          [&](SkCanvas* canvas) -> Result {
        SkDynamicMemoryWStream wstream;
        SkXMLStreamWriter writer(&wstream);
        Result result = src.draw(SkSVGCanvas::Make(SkRect::Make(size), &writer).get());
        if (!result.isOk()) {
            return result;
        }
        std::unique_ptr<SkStream> rstream(wstream.detachAsStream());
        sk_sp<SkSVGDOM> dom =
                SkSVGDOM::Builder().setFontManager(ToolUtils::TestFontMgr()).make(*rstream);
        if (dom) {
            dom->setContainerSize(SkSize::Make(size));
            dom->render(canvas);
        }
        return Result::Ok();
    });
}
#endif

}  // namespace DM

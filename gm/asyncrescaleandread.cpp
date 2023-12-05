/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/base/SkScopeExit.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/YUVUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "tools/graphite/GraphiteTestContext.h"
#endif

#include <variant>

namespace {
/// We test reading from images and surfaces
enum class ReadSource {
    kImage,
    kSurface,
};

// We test reading to RGBA, YUV, and YUVA
enum class Type {
    kRGBA,
    kYUV,
    kYUVA
};

template <ReadSource> struct SourceS;
template <> struct SourceS<ReadSource::kImage>   { using Type = SkImage;   };
template <> struct SourceS<ReadSource::kSurface> { using Type = SkSurface; };

template <ReadSource RS> using Source = typename SourceS<RS>::Type;

// Converts a source image to either an SkImage or SkSurface, backed by GPU if canvas is. Returns
// kSkip or kFail if the image cannot be converted.
template <ReadSource RS>
std::variant<sk_sp<Source<RS>>, skiagm::DrawResult> convert_image_to_source(SkCanvas* canvas,
                                                                            sk_sp<SkImage> image,
                                                                            SkString* errorMsg);

template <>
std::variant<sk_sp<SkImage>, skiagm::DrawResult> convert_image_to_source<ReadSource::kImage>(
        SkCanvas* canvas,
        sk_sp<SkImage> image,
        SkString* errorMsg) {
#if defined(SK_GRAPHITE)
    if (auto recorder = canvas->recorder()) {
        image = SkImages::TextureFromImage(recorder, image);
        if (image) {
            return image;
        }
        *errorMsg = "Could not create Graphite image";
        return skiagm::DrawResult::kFail;
    }
#endif
    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext && canvas->recordingContext()) {
        *errorMsg = "Not supported in DDL mode";
        return skiagm::DrawResult::kSkip;
    }
    if (dContext) {
        image = SkImages::TextureFromImage(dContext, image);
        if (image) {
            return image;
        }
        // When testing abandoned GrContext we expect surface creation to fail.
        if (dContext && dContext->abandoned()) {
            return skiagm::DrawResult::kSkip;
        }
        *errorMsg = "Could not create Ganesh image";
        return skiagm::DrawResult::kFail;
    }
    return image;
}

template <>
std::variant<sk_sp<SkSurface>, skiagm::DrawResult> convert_image_to_source<ReadSource::kSurface>(
        SkCanvas* canvas,
        sk_sp<SkImage> image,
        SkString* errorMsg) {
    // Turn the image into a surface in order to call the read and rescale API
    auto surfInfo = image->imageInfo().makeDimensions(image->dimensions());
    auto surface = canvas->makeSurface(surfInfo);
    if (!surface && surfInfo.colorType() == kBGRA_8888_SkColorType) {
        surfInfo = surfInfo.makeColorType(kRGBA_8888_SkColorType);
        surface = canvas->makeSurface(surfInfo);
    }
    if (!surface) {
        *errorMsg = "Could not create surface for image.";
        // When testing abandoned GrContext we expect surface creation to fail.
        if (canvas->recordingContext() && canvas->recordingContext()->abandoned()) {
            return skiagm::DrawResult::kSkip;
        }
        return skiagm::DrawResult::kFail;
    }
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surface->getCanvas()->drawImage(image, 0, 0, SkSamplingOptions(), &paint);
    return surface;
}

class AsyncReadGMBase : public skiagm::GM {
public:
    AsyncReadGMBase(const char* name) : fName(name) {}

    SkString getName() const override { return fName; }

protected:
    // Does a rescale and read using Graphite, Ganesh, or CPU and returns the result as a pixmap
    // image.
    template <ReadSource ReadSource>
    sk_sp<SkImage> readAndScaleRGBA(Source<ReadSource>* src,
                                    SkIRect srcRect,
                                    GrDirectContext* direct,
                                    skgpu::graphite::Recorder* recorder,
                                    const SkImageInfo& ii,
                                    SkImage::RescaleGamma rescaleGamma,
                                    SkImage::RescaleMode rescaleMode) {
        auto* asyncContext = new AsyncContext();
        if (recorder) {
#if defined(SK_GRAPHITE)
            skgpu::graphite::Context* graphiteContext = recorder->priv().context();
            if (!graphiteContext) {
                return nullptr;
            }
            // We need to flush the existing drawing commands before we try to read
            std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
            if (!recording) {
                return nullptr;
            }
            skgpu::graphite::InsertRecordingInfo recordingInfo;
            recordingInfo.fRecording = recording.get();
            if (!graphiteContext->insertRecording(recordingInfo)) {
                return nullptr;
            }

            graphiteContext->asyncRescaleAndReadPixels(src,
                                                       ii,
                                                       srcRect,
                                                       rescaleGamma,
                                                       rescaleMode,
                                                       AsyncCallback,
                                                       asyncContext);
            graphiteContext->submit();
            while (!asyncContext->fCalled) {
                graphiteContext->checkAsyncWorkCompletion();
                this->graphiteTestContext()->tick();
            }
#endif
        } else {
            src->asyncRescaleAndReadPixels(ii,
                                           srcRect,
                                           rescaleGamma,
                                           rescaleMode,
                                           AsyncCallback,
                                           asyncContext);
            if (direct) {
                direct->submit();
            }
            while (!asyncContext->fCalled) {
                // Only GPU should actually be asynchronous.
                SkASSERT(direct);
                direct->checkAsyncWorkCompletion();
            }
        }
        if (!asyncContext->fResult) {
            return nullptr;
        }
        SkPixmap pixmap(ii, asyncContext->fResult->data(0), asyncContext->fResult->rowBytes(0));
        auto releasePixels = [](const void*, void* c) { delete static_cast<AsyncContext*>(c); };
        return SkImages::RasterFromPixmap(pixmap, releasePixels, asyncContext);
    }

    // Does a YUV[A] rescale and read using Graphite or Ganesh (no CPU support) and returns the
    // result as a YUVA planar texture image.
    template <ReadSource ReadSource>
    sk_sp<SkImage> readAndScaleYUVA(Source<ReadSource>* src,
                                    SkIRect srcRect,
                                    SkISize resultSize,
                                    bool readAlpha,
                                    GrDirectContext* direct,
                                    skgpu::graphite::Recorder* recorder,
                                    SkYUVColorSpace yuvCS,
                                    SkImage::RescaleGamma rescaleGamma,
                                    SkImage::RescaleMode rescaleMode,
                                    SkScopeExit* cleanup) {
        SkASSERT(!(resultSize.width() & 0b1) && !(resultSize.height() & 0b1));

        SkISize uvSize = {resultSize.width() / 2, resultSize.height() / 2};
        SkImageInfo yaII = SkImageInfo::Make(resultSize, kGray_8_SkColorType, kPremul_SkAlphaType);
        SkImageInfo uvII = SkImageInfo::Make(uvSize,     kGray_8_SkColorType, kPremul_SkAlphaType);

        AsyncContext asyncContext;
        if (recorder) {
#if defined(SK_GRAPHITE)
            skgpu::graphite::Context* graphiteContext = recorder->priv().context();
            if (!graphiteContext) {
                return nullptr;
            }
            // We need to flush the existing drawing commands before we try to read
            std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
            if (!recording) {
                return nullptr;
            }
            skgpu::graphite::InsertRecordingInfo recordingInfo;
            recordingInfo.fRecording = recording.get();
            if (!graphiteContext->insertRecording(recordingInfo)) {
                return nullptr;
            }

            if (readAlpha) {
                graphiteContext->asyncRescaleAndReadPixelsYUVA420(src,
                                                                  yuvCS,
                                                                  SkColorSpace::MakeSRGB(),
                                                                  srcRect,
                                                                  resultSize,
                                                                  rescaleGamma,
                                                                  rescaleMode,
                                                                  AsyncCallback,
                                                                  &asyncContext);
            } else {
                graphiteContext->asyncRescaleAndReadPixelsYUV420(src,
                                                                 yuvCS,
                                                                 SkColorSpace::MakeSRGB(),
                                                                 srcRect,
                                                                 resultSize,
                                                                 rescaleGamma,
                                                                 rescaleMode,
                                                                 AsyncCallback,
                                                                 &asyncContext);
            }
            graphiteContext->submit();
            while (!asyncContext.fCalled) {
                graphiteContext->checkAsyncWorkCompletion();
                graphiteTestContext()->tick();
            }
#endif
        } else {
            if (readAlpha) {
                src->asyncRescaleAndReadPixelsYUVA420(yuvCS,
                                                      SkColorSpace::MakeSRGB(),
                                                      srcRect,
                                                      resultSize,
                                                      rescaleGamma,
                                                      rescaleMode,
                                                      AsyncCallback,
                                                      &asyncContext);
            } else {
                src->asyncRescaleAndReadPixelsYUV420(yuvCS,
                                                     SkColorSpace::MakeSRGB(),
                                                     srcRect,
                                                     resultSize,
                                                     rescaleGamma,
                                                     rescaleMode,
                                                     AsyncCallback,
                                                     &asyncContext);
            }
            if (direct) {
                direct->submit();
            }
            while (!asyncContext.fCalled) {
                // Only GPU should actually be asynchronous.
                SkASSERT(direct);
                direct->checkAsyncWorkCompletion();
            }
        }
        if (!asyncContext.fResult) {
            return nullptr;
        }
        auto planeConfig = readAlpha ? SkYUVAInfo::PlaneConfig::kY_U_V_A :
                                       SkYUVAInfo::PlaneConfig::kY_U_V;
        SkYUVAInfo yuvaInfo(resultSize,
                            planeConfig,
                            SkYUVAInfo::Subsampling::k420,
                            yuvCS);
        SkPixmap yuvPMs[4] = {
                {yaII, asyncContext.fResult->data(0), asyncContext.fResult->rowBytes(0)},
                {uvII, asyncContext.fResult->data(1), asyncContext.fResult->rowBytes(1)},
                {uvII, asyncContext.fResult->data(2), asyncContext.fResult->rowBytes(2)},
                {},
        };
        if (readAlpha) {
            yuvPMs[3] = {yaII, asyncContext.fResult->data(3), asyncContext.fResult->rowBytes(3)};
        }
        auto pixmaps = SkYUVAPixmaps::FromExternalPixmaps(yuvaInfo, yuvPMs);
        SkASSERT(pixmaps.isValid());
        auto lazyYUVImage = sk_gpu_test::LazyYUVImage::Make(pixmaps);
        SkASSERT(lazyYUVImage);
#if defined(SK_GRAPHITE)
        if (recorder) {
            return lazyYUVImage->refImage(recorder, sk_gpu_test::LazyYUVImage::Type::kFromTextures);
        } else
#endif
        {
            return lazyYUVImage->refImage(direct, sk_gpu_test::LazyYUVImage::Type::kFromTextures);
        }
    }

    // Draws a 3x2 grid of rescales. The columns are none, low, and high filter quality. The rows
    // are rescale in src gamma and rescale in linear gamma.
    template <ReadSource ReadSource>
    skiagm::DrawResult drawRescaleGrid(SkCanvas* canvas,
                                       Source<ReadSource>* src,
                                       SkIRect srcRect,
                                       SkISize readSize,
                                       Type type,
                                       SkString* errorMsg,
                                       int pad = 0) {
        SkASSERT(canvas->imageInfo().colorType() != kUnknown_SkColorType);

        auto direct = GrAsDirectContext(canvas->recordingContext());
        auto recorder = canvas->recorder();
        SkASSERT(direct || !canvas->recordingContext());

        SkYUVColorSpace yuvColorSpace = kRec601_SkYUVColorSpace;
        canvas->save();
        for (auto gamma : {SkImage::RescaleGamma::kSrc, SkImage::RescaleGamma::kLinear}) {
            canvas->save();
            for (auto mode : {SkImage::RescaleMode::kNearest,
                              SkImage::RescaleMode::kRepeatedLinear,
                              SkImage::RescaleMode::kRepeatedCubic}) {
                SkScopeExit cleanup;
                sk_sp<SkImage> result;
                switch (type) {
                    case Type::kRGBA: {
                        const auto ii = canvas->imageInfo().makeDimensions(readSize);
                        result = readAndScaleRGBA<ReadSource>(src,
                                                              srcRect,
                                                              direct,
                                                              recorder,
                                                              ii,
                                                              gamma,
                                                              mode);
                        if (!result) {
                            errorMsg->printf("async read call failed.");
                            return skiagm::DrawResult::kFail;
                        }
                        break;
                    }
                    case Type::kYUV:
                    case Type::kYUVA:
                        result = readAndScaleYUVA<ReadSource>(src,
                                                              srcRect,
                                                              readSize,
                                                              /*readAlpha=*/type == Type::kYUVA,
                                                              direct,
                                                              recorder,
                                                              yuvColorSpace,
                                                              gamma,
                                                              mode,
                                                              &cleanup);
                        if (!result) {
                            errorMsg->printf("YUV[A]420 async call failed. Allowed for now.");
                            return skiagm::DrawResult::kSkip;
                        }
                        int nextCS = static_cast<int>(yuvColorSpace + 1) %
                                     (kLastEnum_SkYUVColorSpace + 1);
                        yuvColorSpace = static_cast<SkYUVColorSpace>(nextCS);
                        break;
                }
                canvas->drawImage(result, 0, 0);
                canvas->translate(readSize.width() + pad, 0);
            }
            canvas->restore();
            canvas->translate(0, readSize.height() + pad);
        }
        canvas->restore();
        return skiagm::DrawResult::kOk;
    }

private:
    struct AsyncContext {
        bool fCalled = false;
        std::unique_ptr<const SkImage::AsyncReadResult> fResult;
    };

    // Making this a lambda in the test functions caused:
    //   "error: cannot compile this forwarded non-trivially copyable parameter yet"
    // on x86/Win/Clang bot, referring to 'result'.
    static void AsyncCallback(void* c, std::unique_ptr<const SkImage::AsyncReadResult> result) {
        auto context = static_cast<AsyncContext*>(c);
        context->fResult = std::move(result);
        context->fCalled = true;
    }

    SkString fName;
};

template <ReadSource ReadSource, Type Type>
class AsyncRescaleAndReadGridGM : public AsyncReadGMBase {
public:
    AsyncRescaleAndReadGridGM(const char* name,
                              const char* imageFile,
                              SkIRect srcRect,
                              SkISize readSize)
            : AsyncReadGMBase(name)
            , fImageFile(imageFile)
            , fSrcRect(srcRect)
            , fReadSize(readSize) {}

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        ToolUtils::draw_checkerboard(canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, 25);
        auto image = ToolUtils::GetResourceAsImage(fImageFile.c_str());
        if (!image) {
            errorMsg->printf("Could not load image file %s.", fImageFile.c_str());
            return skiagm::DrawResult::kFail;
        }
        if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
            *errorMsg = "Not supported on recording/vector backends.";
            return skiagm::DrawResult::kSkip;
        }

        auto sourceOrResult = convert_image_to_source<ReadSource>(canvas, image, errorMsg);
        if (auto dr = std::get_if<skiagm::DrawResult>(&sourceOrResult)) {
            return *dr;
        }

        using Src = sk_sp<Source<ReadSource>>;
        return drawRescaleGrid<ReadSource>(canvas,
                                           std::get<Src>(sourceOrResult).get(),
                                           fSrcRect,
                                           fReadSize,
                                           Type,
                                           errorMsg);
    }

    SkISize getISize() override { return {3 * fReadSize.width(), 2 * fReadSize.height()}; }

private:
    SkString fImageFile;
    SkIRect fSrcRect;
    SkISize fReadSize;
};
}  // anonymous namespace

#define DEF_RESCALE_AND_READ_GRID_GM(IMAGE_FILE, TAG, SRC_RECT, W, H, SOURCE, TYPE) \
    DEF_GM(return new (AsyncRescaleAndReadGridGM<SOURCE, TYPE>)(                    \
                          "async_rescale_and_read_" #TAG, #IMAGE_FILE, SRC_RECT, SkISize{W, H});)

DEF_RESCALE_AND_READ_GRID_GM(images/yellow_rose.webp,
                             yuv420_rose,
                             SkIRect::MakeXYWH(50, 5, 200, 150),
                             410,
                             376,
                             ReadSource::kSurface,
                             Type::kYUVA)

DEF_RESCALE_AND_READ_GRID_GM(images/yellow_rose.webp,
                             yuv420_rose_down,
                             SkIRect::MakeXYWH(50, 5, 200, 150),
                             106,
                             60,
                             ReadSource::kImage,
                             Type::kYUV)

DEF_RESCALE_AND_READ_GRID_GM(images/yellow_rose.webp,
                             rose,
                             SkIRect::MakeXYWH(100, 20, 100, 100),
                             410,
                             410,
                             ReadSource::kSurface,
                             Type::kRGBA)

DEF_RESCALE_AND_READ_GRID_GM(images/dog.jpg,
                             dog_down,
                             SkIRect::MakeXYWH(0, 10, 180, 150),
                             45,
                             45,
                             ReadSource::kSurface,
                             Type::kRGBA)

DEF_RESCALE_AND_READ_GRID_GM(images/dog.jpg,
                             dog_up,
                             SkIRect::MakeWH(180, 180),
                             800,
                             400,
                             ReadSource::kImage,
                             Type::kRGBA)

DEF_RESCALE_AND_READ_GRID_GM(images/text.png,
                             text_down,
                             SkIRect::MakeWH(637, 105),
                             (int)(0.7 * 637),
                             (int)(0.7 * 105),
                             ReadSource::kImage,
                             Type::kRGBA)

DEF_RESCALE_AND_READ_GRID_GM(images/text.png,
                             text_up,
                             SkIRect::MakeWH(637, 105),
                             (int)(1.2 * 637),
                             (int)(1.2 * 105),
                             ReadSource::kSurface,
                             Type::kRGBA)

DEF_RESCALE_AND_READ_GRID_GM(images/text.png,
                             text_up_large,
                             SkIRect::MakeXYWH(300, 0, 300, 105),
                             (int)(2.4 * 300),
                             (int)(2.4 * 105),
                             ReadSource::kImage,
                             Type::kRGBA)

namespace {
class AyncYUVNoScaleGM : public AsyncReadGMBase {
public:
    AyncYUVNoScaleGM() : AsyncReadGMBase("async_yuv_no_scale") {}
    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        auto surface = canvas->getSurface();
        if (!surface) {
            *errorMsg = "Not supported on recording/vector backends.";
            return skiagm::DrawResult::kSkip;
        }

        auto dContext = GrAsDirectContext(surface->recordingContext());
        if (!dContext && surface->recordingContext()) {
            *errorMsg = "Not supported in DDL mode";
            return skiagm::DrawResult::kSkip;
        }

        auto image = ToolUtils::GetResourceAsImage("images/yellow_rose.webp");
        if (!image) {
            return skiagm::DrawResult::kFail;
        }
        SkPaint paint;
        canvas->drawImage(image.get(), 0, 0);

        skgpu::graphite::Recorder* recorder = canvas->recorder();
        SkScopeExit scopeExit;
        auto yuvImage = readAndScaleYUVA<ReadSource::kSurface>(surface,
                                                               SkIRect::MakeWH(400, 300),
                                                               SkISize{400, 300},
                                                               /*readAlpha=*/false,
                                                               dContext,
                                                               recorder,
                                                               kRec601_SkYUVColorSpace,
                                                               SkImage::RescaleGamma::kSrc,
                                                               SkImage::RescaleMode::kNearest,
                                                               &scopeExit);

        canvas->clear(SK_ColorWHITE);
        canvas->drawImage(yuvImage.get(), 0, 0);

        return skiagm::DrawResult::kOk;
    }
    SkISize getISize() override { return {400, 300}; }
};
}  // namespace

DEF_GM(return new AyncYUVNoScaleGM();)

namespace {
class AsyncRescaleAndReadNoBleedGM : public AsyncReadGMBase {
public:
    AsyncRescaleAndReadNoBleedGM() : AsyncReadGMBase("async_rescale_and_read_no_bleed") {}

    SkISize getISize() override { return {60, 60}; }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
            *errorMsg = "Not supported on recording/vector backends.";
            return skiagm::DrawResult::kSkip;
        }

        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext && canvas->recordingContext()) {
            *errorMsg = "Not supported in DDL mode";
            return skiagm::DrawResult::kSkip;
        }

        static constexpr int kBorder = 5;
        static constexpr int kInner = 5;
        const auto srcRect = SkIRect::MakeXYWH(kBorder, kBorder, kInner, kInner);
        auto surfaceII = SkImageInfo::Make(kInner + 2 * kBorder,
                                           kInner + 2 * kBorder,
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType,
                                           SkColorSpace::MakeSRGB());
        auto surface = canvas->makeSurface(surfaceII);
        if (!surface) {
            *errorMsg = "Could not create surface for image.";
            // When testing abandoned GrContext we expect surface creation to fail.
            if (canvas->recordingContext() && canvas->recordingContext()->abandoned()) {
                return skiagm::DrawResult::kSkip;
            }
            return skiagm::DrawResult::kFail;
        }
        surface->getCanvas()->clear(SK_ColorRED);
        surface->getCanvas()->save();
        surface->getCanvas()->clipRect(SkRect::Make(srcRect), SkClipOp::kIntersect, false);
        surface->getCanvas()->clear(SK_ColorBLUE);
        surface->getCanvas()->restore();
        static constexpr int kPad = 2;
        canvas->translate(kPad, kPad);
        skiagm::DrawResult result;
        SkISize downSize = {static_cast<int>(kInner / 2), static_cast<int>(kInner / 2)};
        result = drawRescaleGrid<ReadSource::kSurface>(canvas,
                                                       surface.get(),
                                                       srcRect,
                                                       downSize,
                                                       Type::kRGBA,
                                                       errorMsg,
                                                       kPad);
        if (result != skiagm::DrawResult::kOk) {
            return result;
        }
        canvas->translate(0, 4 * downSize.height());
        SkISize upSize = {static_cast<int>(kInner * 3.5), static_cast<int>(kInner * 4.6)};
        result = drawRescaleGrid<ReadSource::kSurface>(canvas,
                                                       surface.get(),
                                                       srcRect,
                                                       upSize,
                                                       Type::kRGBA,
                                                       errorMsg,
                                                       kPad);
        if (result != skiagm::DrawResult::kOk) {
            return result;
        }
        return skiagm::DrawResult::kOk;
    }
};
}  // namespace

DEF_GM(return new AsyncRescaleAndReadNoBleedGM();)

namespace {
class AsyncRescaleAndReadAlphaTypeGM : public AsyncReadGMBase {
public:
    AsyncRescaleAndReadAlphaTypeGM() : AsyncReadGMBase("async_rescale_and_read_alpha_type") {}

    SkISize getISize() override { return {512, 512}; }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext && canvas->recordingContext()) {
            *errorMsg = "Not supported in DDL mode";
            return skiagm::DrawResult::kSkip;
        }

        if (canvas->recorder()) {
            *errorMsg = "Reading to unpremul not supported in Graphite.";
            return skiagm::DrawResult::kSkip;
        }

        auto upmII = SkImageInfo::Make(200, 200, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);

        auto pmII = upmII.makeAlphaType(kPremul_SkAlphaType);

        auto upmSurf = SkSurfaces::Raster(upmII);
        auto pmSurf = SkSurfaces::Raster(pmII);

        SkColor4f colors[] = {
                {.3f, .3f, .3f, .3f},
                {1.f, .2f, .6f, .9f},
                {0.f, .1f, 1.f, .1f},
                {.7f, .8f, .2f, .7f},
        };
        auto shader = SkGradientShader::MakeRadial({100, 100},
                                                   230,
                                                   colors,
                                                   nullptr,
                                                   nullptr,
                                                   std::size(colors),
                                                   SkTileMode::kRepeat);
        SkPaint paint;
        paint.setShader(std::move(shader));

        upmSurf->getCanvas()->drawPaint(paint);
        pmSurf ->getCanvas()->drawPaint(paint);

        auto pmImg  =  pmSurf->makeImageSnapshot();
        auto upmImg = upmSurf->makeImageSnapshot();

        auto imageOrResult = convert_image_to_source<ReadSource::kImage>(canvas,
                                                                         std::move(pmImg),
                                                                         errorMsg);
        if (const auto* dr = std::get_if<skiagm::DrawResult>(&imageOrResult)) {
            return *dr;
        }
        pmImg = std::move(std::get<sk_sp<SkImage>>(imageOrResult));

        imageOrResult = convert_image_to_source<ReadSource::kImage>(canvas,
                                                                    std::move(upmImg),
                                                                    errorMsg);
        if (const auto* dr = std::get_if<skiagm::DrawResult>(&imageOrResult)) {
            return *dr;
        }
        upmImg = std::move(std::get<sk_sp<SkImage>>(imageOrResult));

        int size = 256;

        ToolUtils::draw_checkerboard(canvas, SK_ColorWHITE, SK_ColorBLACK, 32);

        for (const auto& img : {pmImg, upmImg}) {
            canvas->save();
            for (auto readAT : {kPremul_SkAlphaType, kUnpremul_SkAlphaType}) {
                auto readInfo = img->imageInfo().makeAlphaType(readAT).makeWH(size, size);
                auto result =
                        readAndScaleRGBA<ReadSource::kImage>(img.get(),
                                                             SkIRect::MakeSize(img->dimensions()),
                                                             dContext,
                                                             canvas->recorder(),
                                                             readInfo,
                                                             SkImage::RescaleGamma::kSrc,
                                                             SkImage::RescaleMode::kRepeatedCubic);
                if (!result) {
                    *errorMsg = "async readback failed";
                    return skiagm::DrawResult::kFail;
                }
                canvas->drawImage(result, 0, 0);
                canvas->translate(size, 0);
            }
            canvas->restore();
            canvas->translate(0, size);
        }
        return skiagm::DrawResult::kOk;
    }
};
}  // namespace

DEF_GM(return new AsyncRescaleAndReadAlphaTypeGM();)

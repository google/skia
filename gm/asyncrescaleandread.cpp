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
#endif

namespace {
struct AsyncContext {
    bool fCalled = false;
    std::unique_ptr<const SkImage::AsyncReadResult> fResult;
};
}  // anonymous namespace

// Making this a lambda in the test functions caused:
//   "error: cannot compile this forwarded non-trivially copyable parameter yet"
// on x86/Win/Clang bot, referring to 'result'.
static void async_callback(void* c, std::unique_ptr<const SkImage::AsyncReadResult> result) {
    auto context = static_cast<AsyncContext*>(c);
    context->fResult = std::move(result);
    context->fCalled = true;
}

// Draws the image to a surface, does a asyncRescaleAndReadPixels of the image, and then sticks
// the result in a raster image.
template <typename Src>
static sk_sp<SkImage> do_read_and_scale(Src* src,
                                        GrDirectContext* direct,
                                        skgpu::graphite::Recorder* recorder,
                                        const SkIRect& srcRect,
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

        graphiteContext->asyncRescaleAndReadPixels(src, ii, srcRect, rescaleGamma, rescaleMode,
                                                   async_callback, asyncContext);
        graphiteContext->submit();
        while (!asyncContext->fCalled) {
            graphiteContext->checkAsyncWorkCompletion();
        }
#endif
    } else {
        src->asyncRescaleAndReadPixels(ii, srcRect, rescaleGamma, rescaleMode, async_callback,
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

template <typename Src>
static sk_sp<SkImage> do_read_and_scale_yuv(Src* src,
                                            GrDirectContext* direct,
                                            skgpu::graphite::Recorder* recorder,
                                            SkYUVColorSpace yuvCS,
                                            bool readAlpha,
                                            const SkIRect& srcRect,
                                            SkISize size,
                                            SkImage::RescaleGamma rescaleGamma,
                                            SkImage::RescaleMode rescaleMode,
                                            SkScopeExit* cleanup) {
    SkASSERT(!(size.width() & 0b1) && !(size.height() & 0b1));

    SkISize uvSize = {size.width()/2, size.height()/2};
    SkImageInfo yaII = SkImageInfo::Make(size  , kGray_8_SkColorType, kPremul_SkAlphaType);
    SkImageInfo uvII = SkImageInfo::Make(uvSize, kGray_8_SkColorType, kPremul_SkAlphaType);

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
            graphiteContext->asyncRescaleAndReadPixelsYUVA420(src, yuvCS, SkColorSpace::MakeSRGB(),
                                                              srcRect, size, rescaleGamma,
                                                              rescaleMode, async_callback,
                                                              &asyncContext);
        } else {
            graphiteContext->asyncRescaleAndReadPixelsYUV420(src, yuvCS, SkColorSpace::MakeSRGB(),
                                                             srcRect, size, rescaleGamma,
                                                             rescaleMode, async_callback,
                                                             &asyncContext);
        }
        graphiteContext->submit();
        while (!asyncContext.fCalled) {
            graphiteContext->checkAsyncWorkCompletion();
        }
#endif
    } else {
        if (readAlpha) {
            src->asyncRescaleAndReadPixelsYUVA420(yuvCS, SkColorSpace::MakeSRGB(),
                                                  srcRect, size, rescaleGamma, rescaleMode,
                                                  async_callback, &asyncContext);
        } else {
            src->asyncRescaleAndReadPixelsYUV420(yuvCS, SkColorSpace::MakeSRGB(),
                                                 srcRect, size, rescaleGamma, rescaleMode,
                                                 async_callback, &asyncContext);
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
    auto planeConfig = readAlpha ? SkYUVAInfo::PlaneConfig::kY_U_V_A
                                 : SkYUVAInfo::PlaneConfig::kY_U_V;
    SkYUVAInfo yuvaInfo(size,
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

enum class ReadSource {
    kImage,
    kSurface,
};

enum class Type {
    kRGBA,
    kYUV,
    kYUVA
};

// Draws a grid of rescales. The columns are none, low, and high filter quality. The rows are
// rescale in src gamma and rescale in linear gamma.
template <typename Src>
static skiagm::DrawResult do_rescale_grid(SkCanvas* canvas,
                                          Src* src,
                                          GrDirectContext* direct,
                                          skgpu::graphite::Recorder* recorder,
                                          const SkIRect& srcRect,
                                          SkISize newSize,
                                          Type type,
                                          SkString* errorMsg,
                                          int pad = 0) {
    if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
        *errorMsg = "Not supported on recording/vector backends.";
        return skiagm::DrawResult::kSkip;
    }
    const auto ii = canvas->imageInfo().makeDimensions(newSize);

    SkYUVColorSpace yuvColorSpace = kRec601_SkYUVColorSpace;
    canvas->save();
    for (auto gamma : {SkImage::RescaleGamma::kSrc, SkImage::RescaleGamma::kLinear}) {
        canvas->save();
        for (auto mode : {
                SkImage::RescaleMode::kNearest,
                SkImage::RescaleMode::kRepeatedLinear,
                SkImage::RescaleMode::kRepeatedCubic}) {
            SkScopeExit cleanup;
            sk_sp<SkImage> result;
            switch (type) {
                case Type::kRGBA:
                    result = do_read_and_scale(src, direct, recorder, srcRect, ii, gamma, mode);
                    if (!result) {
                        errorMsg->printf("async read call failed.");
                        return skiagm::DrawResult::kFail;
                    }
                    break;
                case Type::kYUV:
                case Type::kYUVA:
                    result = do_read_and_scale_yuv(src, direct, recorder, yuvColorSpace,
                                                   /*readAlpha=*/type == Type::kYUVA, srcRect,
                                                   newSize, gamma, mode, &cleanup);
                    if (!result) {
                        errorMsg->printf("YUV[A]420 async call failed. Allowed for now.");
                        return skiagm::DrawResult::kSkip;
                    }
                    int nextCS =
                            static_cast<int>(yuvColorSpace + 1) % (kLastEnum_SkYUVColorSpace + 1);
                    yuvColorSpace = static_cast<SkYUVColorSpace>(nextCS);
                    break;
            }
            canvas->drawImage(result, 0, 0);
            canvas->translate(newSize.width() + pad, 0);
        }
        canvas->restore();
        canvas->translate(0, newSize.height() + pad);
    }
    canvas->restore();
    return skiagm::DrawResult::kOk;
}

static skiagm::DrawResult do_rescale_image_grid(SkCanvas* canvas,
                                                const char* imageFile,
                                                const SkIRect& srcRect,
                                                SkISize newSize,
                                                ReadSource source,
                                                Type type,
                                                SkString* errorMsg) {
    auto image = ToolUtils::GetResourceAsImage(imageFile);
    if (!image) {
        errorMsg->printf("Could not load image file %s.", imageFile);
        return skiagm::DrawResult::kFail;
    }
    if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
        *errorMsg = "Not supported on recording/vector backends.";
        return skiagm::DrawResult::kSkip;
    }

    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext && canvas->recordingContext()) {
        *errorMsg = "Not supported in DDL mode";
        return skiagm::DrawResult::kSkip;
    }
    auto recorder = canvas->recorder();

    switch (source) {
        case ReadSource::kImage:
#if defined(SK_GRAPHITE)
            if (recorder) {
                image = SkImages::TextureFromImage(recorder, image);
                if (!image) {
                    *errorMsg = "Could not create image.";
                    return skiagm::DrawResult::kFail;
                }
            } else
#endif
            if (dContext) {
                image = SkImages::TextureFromImage(dContext, image);
                if (!image) {
                    *errorMsg = "Could not create image.";
                    // When testing abandoned GrContext we expect surface creation to fail.
                    if (canvas->recordingContext() && canvas->recordingContext()->abandoned()) {
                        return skiagm::DrawResult::kSkip;
                    }
                    return skiagm::DrawResult::kFail;
                }
            }
            return do_rescale_grid(canvas, image.get(), dContext, recorder, srcRect, newSize, type,
                                   errorMsg);
        case ReadSource::kSurface:
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
            return do_rescale_grid(canvas, surface.get(), dContext, recorder, srcRect, newSize,
                                   type, errorMsg);
    }
    SkUNREACHABLE;
}

#define DEF_RESCALE_AND_READ_GM(IMAGE_FILE, TAG, SRC_RECT, W, H, SOURCE, TYPE)             \
    DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_##TAG, canvas, errorMsg, 3 * W, 2 * H) { \
        ToolUtils::draw_checkerboard(canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, 25);          \
        return do_rescale_image_grid(                                                      \
                canvas, #IMAGE_FILE, SRC_RECT, {W, H}, SOURCE, TYPE, errorMsg);            \
    }

DEF_RESCALE_AND_READ_GM(images/yellow_rose.webp,
                        yuv420_rose,
                        SkIRect::MakeXYWH(50, 5, 200, 150),
                        410,
                        376,
                        ReadSource::kSurface,
                        Type::kYUVA)

DEF_RESCALE_AND_READ_GM(images/yellow_rose.webp,
                        yuv420_rose_down,
                        SkIRect::MakeXYWH(50, 5, 200, 150),
                        106,
                        60,
                        ReadSource::kImage,
                        Type::kYUV)

DEF_RESCALE_AND_READ_GM(images/yellow_rose.webp,
                        rose,
                        SkIRect::MakeXYWH(100, 20, 100, 100),
                        410,
                        410,
                        ReadSource::kSurface,
                        Type::kRGBA)

DEF_RESCALE_AND_READ_GM(images/dog.jpg,
                        dog_down,
                        SkIRect::MakeXYWH(0, 10, 180, 150),
                        45,
                        45,
                        ReadSource::kSurface,
                        Type::kRGBA)

DEF_RESCALE_AND_READ_GM(images/dog.jpg,
                        dog_up,
                        SkIRect::MakeWH(180, 180),
                        800,
                        400,
                        ReadSource::kImage,
                        Type::kRGBA)

DEF_RESCALE_AND_READ_GM(images/text.png,
                        text_down,
                        SkIRect::MakeWH(637, 105),
                        (int)(0.7 * 637),
                        (int)(0.7 * 105),
                        ReadSource::kImage,
                        Type::kRGBA)

DEF_RESCALE_AND_READ_GM(images/text.png,
                        text_up,
                        SkIRect::MakeWH(637, 105),
                        (int)(1.2 * 637),
                        (int)(1.2 * 105),
                        ReadSource::kSurface,
                        Type::kRGBA)

DEF_RESCALE_AND_READ_GM(images/text.png,
                        text_up_large,
                        SkIRect::MakeXYWH(300, 0, 300, 105),
                        (int)(2.4 * 300),
                        (int)(2.4 * 105),
                        ReadSource::kImage,
                        Type::kRGBA)

// Exercises non-scaling YUV420. Reads from the original canvas's surface in order to
// exercise case where source surface is not a texture (in glbert config).
DEF_SIMPLE_GM_CAN_FAIL(async_yuv_no_scale, canvas, errorMsg, 400, 300) {
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
    auto yuvImage = do_read_and_scale_yuv(surface, dContext, recorder, kRec601_SkYUVColorSpace,
                                          /*readAlpha=*/false, SkIRect::MakeWH(400, 300),
                                          {400, 300}, SkImage::RescaleGamma::kSrc,
                                          SkImage::RescaleMode::kNearest, &scopeExit);

    canvas->clear(SK_ColorWHITE);
    canvas->drawImage(yuvImage.get(), 0, 0);

    return skiagm::DrawResult::kOk;
}

DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_no_bleed, canvas, errorMsg, 60, 60) {
    if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
        *errorMsg = "Not supported on recording/vector backends.";
        return skiagm::DrawResult::kSkip;
    }

    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext && canvas->recordingContext()) {
        *errorMsg = "Not supported in DDL mode";
        return skiagm::DrawResult::kSkip;
    }
    auto recorder = canvas->recorder();

    static constexpr int kBorder = 5;
    static constexpr int kInner = 5;
    const auto srcRect = SkIRect::MakeXYWH(kBorder, kBorder, kInner, kInner);
    auto surfaceII =
            SkImageInfo::Make(kInner + 2 * kBorder, kInner + 2 * kBorder, kRGBA_8888_SkColorType,
                              kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
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
    SkISize downSize = {static_cast<int>(kInner/2),  static_cast<int>(kInner / 2)};
    result = do_rescale_grid(canvas, surface.get(), dContext, recorder, srcRect, downSize,
                             Type::kRGBA, errorMsg, kPad);

    if (result != skiagm::DrawResult::kOk) {
        return result;
    }
    canvas->translate(0, 4 * downSize.height());
    SkISize upSize = {static_cast<int>(kInner * 3.5), static_cast<int>(kInner * 4.6)};
    result = do_rescale_grid(canvas, surface.get(), dContext, recorder, srcRect, upSize,
                             Type::kRGBA, errorMsg, kPad);
    if (result != skiagm::DrawResult::kOk) {
        return result;
    }
    return skiagm::DrawResult::kOk;
}

DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_alpha_type, canvas, errorMsg, 512, 512) {
    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext && canvas->recordingContext()) {
        *errorMsg = "Not supported in DDL mode";
        return skiagm::DrawResult::kSkip;
    }
    if (dContext && dContext->abandoned()) {
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

    if (dContext) {
        pmImg = SkImages::TextureFromImage(dContext, pmImg);
        upmImg = SkImages::TextureFromImage(dContext, upmImg);
        if (!pmImg || !upmImg) {
            *errorMsg = "could not make texture images";
            return skiagm::DrawResult::kFail;
        }
    }
    int size = 256;

    ToolUtils::draw_checkerboard(canvas, SK_ColorWHITE, SK_ColorBLACK, 32);

    for (const auto& img : {pmImg, upmImg}) {
        canvas->save();
        for (auto readAT : {kPremul_SkAlphaType, kUnpremul_SkAlphaType}) {
            auto readInfo = img->imageInfo().makeAlphaType(readAT).makeWH(size, size);

            auto* asyncContext = new AsyncContext();
            img->asyncRescaleAndReadPixels(readInfo,
                                           SkIRect::MakeSize(img->dimensions()),
                                           SkImage::RescaleGamma::kSrc,
                                           SkImage::RescaleMode::kRepeatedCubic,
                                           async_callback,
                                           asyncContext);
            if (dContext) {
                dContext->submit();
            }
            while (!asyncContext->fCalled) {
                // Only GPU should actually be asynchronous.
                SkASSERT(dContext);
                dContext->checkAsyncWorkCompletion();
            }
            if (asyncContext->fResult) {
                SkPixmap pixmap(readInfo,
                                asyncContext->fResult->data(0),
                                asyncContext->fResult->rowBytes(0));
                auto releasePixels = [](const void*, void* c) {
                    delete static_cast<AsyncContext*>(c);
                };
                auto result = SkImages::RasterFromPixmap(pixmap, releasePixels, asyncContext);

                canvas->drawImage(result, 0, 0);
            } else {
                delete asyncContext;
                *errorMsg = "async readback failed";
                return skiagm::DrawResult::kFail;
            }
            canvas->translate(size, 0);
        }
        canvas->restore();
        canvas->translate(0, size);
    }
    return skiagm::DrawResult::kOk;
}

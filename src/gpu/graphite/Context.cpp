/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPathTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkYUVMath.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ClientMappedBufferManager.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/UploadTask.h"

#if defined(GRAPHITE_TEST_UTILS)
#include "include/private/gpu/graphite/ContextOptionsPriv.h"
#endif

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())

Context::ContextID Context::ContextID::Next() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return ContextID(id);
}

//--------------------------------------------------------------------------------------------------
Context::Context(sk_sp<SharedContext> sharedContext,
                 std::unique_ptr<QueueManager> queueManager,
                 const ContextOptions& options)
        : fSharedContext(std::move(sharedContext))
        , fQueueManager(std::move(queueManager))
        , fContextID(ContextID::Next()) {
    // We have to create this outside the initializer list because we need to pass in the Context's
    // SingleOwner object and it is declared last
    fResourceProvider = fSharedContext->makeResourceProvider(&fSingleOwner,
                                                             SK_InvalidGenID,
                                                             options.fGpuBudgetInBytes);
    fMappedBufferManager = std::make_unique<ClientMappedBufferManager>(this->contextID());
    fPlotUploadTracker = std::make_unique<PlotUploadTracker>();
#if defined(GRAPHITE_TEST_UTILS)
    if (options.fOptionsPriv) {
        fStoreContextRefInRecorder = options.fOptionsPriv->fStoreContextRefInRecorder;
    }
#endif
}

Context::~Context() {
#if defined(GRAPHITE_TEST_UTILS)
    ASSERT_SINGLE_OWNER
    for (auto& recorder : fTrackedRecorders) {
        recorder->priv().setContext(nullptr);
    }
#endif
}

bool Context::finishInitialization() {
    SkASSERT(!fSharedContext->rendererProvider()); // Can only initialize once

    StaticBufferManager bufferManager{fResourceProvider.get(), fSharedContext->caps()};
    std::unique_ptr<RendererProvider> renderers{
            new RendererProvider(fSharedContext->caps(), &bufferManager)};

    auto result = bufferManager.finalize(this, fQueueManager.get(), fSharedContext->globalCache());
    if (result == StaticBufferManager::FinishResult::kFailure) {
        // If something went wrong filling out the static vertex buffers, any Renderer that would
        // use it will draw incorrectly, so it's better to fail the Context creation.
        return false;
    }
    if (result == StaticBufferManager::FinishResult::kSuccess &&
        !fQueueManager->submitToGpu()) {
        SKGPU_LOG_W("Failed to submit initial command buffer for Context creation.\n");
        return false;
    } // else result was kNoWork so skip submitting to the GPU
    fSharedContext->setRendererProvider(std::move(renderers));
    return true;
}

BackendApi Context::backend() const { return fSharedContext->backend(); }

std::unique_ptr<Recorder> Context::makeRecorder(const RecorderOptions& options) {
    ASSERT_SINGLE_OWNER

    auto recorder = std::unique_ptr<Recorder>(new Recorder(fSharedContext, options));
#if defined(GRAPHITE_TEST_UTILS)
    if (fStoreContextRefInRecorder) {
        recorder->priv().setContext(this);
    }
#endif
    return recorder;
}

bool Context::insertRecording(const InsertRecordingInfo& info) {
    ASSERT_SINGLE_OWNER

    return fQueueManager->addRecording(info, this);
}

bool Context::submit(SyncToCpu syncToCpu) {
    ASSERT_SINGLE_OWNER

    bool success = fQueueManager->submitToGpu();
    fQueueManager->checkForFinishedWork(syncToCpu);
    return success;
}

void Context::asyncRescaleAndReadPixels(const SkImage* image,
                                        const SkImageInfo& dstImageInfo,
                                        const SkIRect& srcRect,
                                        SkImage::RescaleGamma rescaleGamma,
                                        SkImage::RescaleMode rescaleMode,
                                        SkImage::ReadPixelsCallback callback,
                                        SkImage::ReadPixelsContext callbackContext) {
    if (!image || !as_IB(image)->isGraphiteBacked()) {
        callback(callbackContext, nullptr);
        return;
    }
    // TODO(b/238756380): YUVA read not supported right now
    if (as_IB(image)->isYUVA()) {
        callback(callbackContext, nullptr);
        return;
    }

    if (!SkIRect::MakeSize(image->imageInfo().dimensions()).contains(srcRect)) {
        callback(callbackContext, nullptr);
        return;
    }

    if (srcRect.size() == dstImageInfo.bounds().size()) {
        // No need for rescale
        auto graphiteImage = reinterpret_cast<const skgpu::graphite::Image*>(image);
        const TextureProxyView& proxyView = graphiteImage->textureProxyView();
        return this->asyncReadPixels(proxyView.proxy(),
                                     image->imageInfo(),
                                     dstImageInfo.colorInfo(),
                                     srcRect,
                                     callback,
                                     callbackContext);
    }

    // Make a recorder to record drawing commands into
    std::unique_ptr<Recorder> recorder = this->makeRecorder();

    sk_sp<SkImage> scaledImage = RescaleImage(recorder.get(),
                                              image,
                                              srcRect,
                                              dstImageInfo,
                                              rescaleGamma,
                                              rescaleMode);
    if (!scaledImage) {
        callback(callbackContext, nullptr);
        return;
    }

    // Add draw commands to queue before starting the transfer
    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        callback(callbackContext, nullptr);
        return;
    }
    InsertRecordingInfo recordingInfo;
    recordingInfo.fRecording = recording.get();
    if (!this->insertRecording(recordingInfo)) {
        callback(callbackContext, nullptr);
        return;
    }

    SkASSERT(scaledImage->imageInfo() == dstImageInfo);

    auto scaledGraphiteImage = reinterpret_cast<const skgpu::graphite::Image*>(scaledImage.get());
    const TextureProxyView& scaledProxyView = scaledGraphiteImage->textureProxyView();

    this->asyncReadPixels(scaledProxyView.proxy(),
                          dstImageInfo,
                          dstImageInfo.colorInfo(),
                          dstImageInfo.bounds(),
                          callback,
                          callbackContext);
}

void Context::asyncRescaleAndReadPixels(const SkSurface* surface,
                                        const SkImageInfo& dstImageInfo,
                                        const SkIRect& srcRect,
                                        SkImage::RescaleGamma rescaleGamma,
                                        SkImage::RescaleMode rescaleMode,
                                        SkImage::ReadPixelsCallback callback,
                                        SkImage::ReadPixelsContext callbackContext) {
    if (!static_cast<const SkSurface_Base*>(surface)->isGraphiteBacked()) {
        callback(callbackContext, nullptr);
        return;
    }

    sk_sp<SkImage> surfaceImage = SkSurfaces::AsImage(sk_ref_sp(surface));
    this->asyncRescaleAndReadPixels(surfaceImage.get(),
                                    dstImageInfo,
                                    srcRect,
                                    rescaleGamma,
                                    rescaleMode,
                                    callback,
                                    callbackContext);
}

void Context::asyncReadPixels(const TextureProxy* proxy,
                              const SkImageInfo& srcImageInfo,
                              const SkColorInfo& dstColorInfo,
                              const SkIRect& srcRect,
                              SkImage::ReadPixelsCallback callback,
                              SkImage::ReadPixelsContext callbackContext) {
    TRACE_EVENT2("skia.gpu", TRACE_FUNC, "width", srcRect.width(), "height", srcRect.height());

    if (!proxy || proxy->textureInfo().isProtected() == Protected::kYes) {
        callback(callbackContext, nullptr);
        return;
    }

    if (!SkImageInfoIsValid(srcImageInfo) || !SkColorInfoIsValid(dstColorInfo)) {
        callback(callbackContext, nullptr);
        return;
    }

    if (!SkIRect::MakeSize(srcImageInfo.dimensions()).contains(srcRect)) {
        callback(callbackContext, nullptr);
        return;
    }

    const Caps* caps = fSharedContext->caps();
    if (!caps->supportsReadPixels(proxy->textureInfo())) {
        // TODO: try to copy to a readable texture instead
        callback(callbackContext, nullptr);
        return;
    }

    PixelTransferResult transferResult = this->transferPixels(proxy, srcImageInfo,
                                                              dstColorInfo, srcRect);

    if (!transferResult.fTransferBuffer) {
        // TODO: try to do a synchronous readPixels instead
        callback(callbackContext, nullptr);
        return;
    }

    using AsyncReadResult = skgpu::TAsyncReadResult<Buffer, ContextID, PixelTransferResult>;
    struct FinishContext {
        SkImage::ReadPixelsCallback* fClientCallback;
        SkImage::ReadPixelsContext fClientContext;
        SkISize fSize;
        ClientMappedBufferManager* fMappedBufferManager;
        PixelTransferResult fTransferResult;
    };
    auto* finishContext = new FinishContext{callback,
                                            callbackContext,
                                            srcRect.size(),
                                            fMappedBufferManager.get(),
                                            std::move(transferResult)};
    GpuFinishedProc finishCallback = [](GpuFinishedContext c, CallbackResult status) {
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        if (status == CallbackResult::kSuccess) {
            ClientMappedBufferManager* manager = context->fMappedBufferManager;
            auto result = std::make_unique<AsyncReadResult>(manager->ownerID());
            if (!result->addTransferResult(context->fTransferResult, context->fSize,
                                           context->fTransferResult.fRowBytes, manager)) {
                result.reset();
            }
            (*context->fClientCallback)(context->fClientContext, std::move(result));
        } else {
            (*context->fClientCallback)(context->fClientContext, nullptr);
        }
        delete context;
    };

    InsertFinishInfo info;
    info.fFinishedContext = finishContext;
    info.fFinishedProc = finishCallback;
    // If addFinishInfo() fails, it invokes the finish callback automatically, which handles all the
    // required clean up for us, just log an error message.
    if (!fQueueManager->addFinishInfo(info, fResourceProvider.get())) {
        SKGPU_LOG_E("Failed to register finish callbacks for asyncReadPixels.");
    }
}

void Context::asyncRescaleAndReadPixelsYUV420(const SkImage* image,
                                              SkYUVColorSpace yuvColorSpace,
                                              sk_sp<SkColorSpace> dstColorSpace,
                                              const SkIRect& srcRect,
                                              const SkISize& dstSize,
                                              SkImage::RescaleGamma rescaleGamma,
                                              SkImage::RescaleMode rescaleMode,
                                              SkImage::ReadPixelsCallback callback,
                                              SkImage::ReadPixelsContext callbackContext) {
    this->asyncRescaleAndReadPixelsYUV420Impl(image,
                                              yuvColorSpace,
                                              /*readAlpha=*/false,
                                              dstColorSpace,
                                              srcRect,
                                              dstSize,
                                              rescaleGamma,
                                              rescaleMode,
                                              callback,
                                              callbackContext);
}

void Context::asyncRescaleAndReadPixelsYUV420(const SkSurface* surface,
                                              SkYUVColorSpace yuvColorSpace,
                                              sk_sp<SkColorSpace> dstColorSpace,
                                              const SkIRect& srcRect,
                                              const SkISize& dstSize,
                                              SkImage::RescaleGamma rescaleGamma,
                                              SkImage::RescaleMode rescaleMode,
                                              SkImage::ReadPixelsCallback callback,
                                              SkImage::ReadPixelsContext callbackContext) {
    if (!static_cast<const SkSurface_Base*>(surface)->isGraphiteBacked()) {
        callback(callbackContext, nullptr);
        return;
    }

    sk_sp<SkImage> surfaceImage = SkSurfaces::AsImage(sk_ref_sp(surface));
    this->asyncRescaleAndReadPixelsYUV420(surfaceImage.get(),
                                          yuvColorSpace,
                                          dstColorSpace,
                                          srcRect,
                                          dstSize,
                                          rescaleGamma,
                                          rescaleMode,
                                          callback,
                                          callbackContext);
}

void Context::asyncRescaleAndReadPixelsYUVA420(const SkImage* image,
                                               SkYUVColorSpace yuvColorSpace,
                                               sk_sp<SkColorSpace> dstColorSpace,
                                               const SkIRect& srcRect,
                                               const SkISize& dstSize,
                                               SkImage::RescaleGamma rescaleGamma,
                                               SkImage::RescaleMode rescaleMode,
                                               SkImage::ReadPixelsCallback callback,
                                               SkImage::ReadPixelsContext callbackContext) {
    this->asyncRescaleAndReadPixelsYUV420Impl(image,
                                              yuvColorSpace,
                                              /*readAlpha=*/true,
                                              dstColorSpace,
                                              srcRect,
                                              dstSize,
                                              rescaleGamma,
                                              rescaleMode,
                                              callback,
                                              callbackContext);
}

void Context::asyncRescaleAndReadPixelsYUVA420(const SkSurface* surface,
                                               SkYUVColorSpace yuvColorSpace,
                                               sk_sp<SkColorSpace> dstColorSpace,
                                               const SkIRect& srcRect,
                                               const SkISize& dstSize,
                                               SkImage::RescaleGamma rescaleGamma,
                                               SkImage::RescaleMode rescaleMode,
                                               SkImage::ReadPixelsCallback callback,
                                               SkImage::ReadPixelsContext callbackContext) {
    if (!static_cast<const SkSurface_Base*>(surface)->isGraphiteBacked()) {
        callback(callbackContext, nullptr);
        return;
    }

    sk_sp<SkImage> surfaceImage = SkSurfaces::AsImage(sk_ref_sp(surface));
    this->asyncRescaleAndReadPixelsYUVA420(surfaceImage.get(),
                                           yuvColorSpace,
                                           dstColorSpace,
                                           srcRect,
                                           dstSize,
                                           rescaleGamma,
                                           rescaleMode,
                                           callback,
                                           callbackContext);
}

void Context::asyncRescaleAndReadPixelsYUV420Impl(const SkImage* image,
                                                  SkYUVColorSpace yuvColorSpace,
                                                  bool readAlpha,
                                                  sk_sp<SkColorSpace> dstColorSpace,
                                                  const SkIRect& srcRect,
                                                  const SkISize& dstSize,
                                                  SkImage::RescaleGamma rescaleGamma,
                                                  SkImage::RescaleMode rescaleMode,
                                                  SkImage::ReadPixelsCallback callback,
                                                  SkImage::ReadPixelsContext callbackContext) {
    if (!image || !as_IB(image)->isGraphiteBacked()) {
        callback(callbackContext, nullptr);
        return;
    }

    const SkImageInfo& srcImageInfo = image->imageInfo();
    if (!SkIRect::MakeSize(srcImageInfo.dimensions()).contains(srcRect)) {
        callback(callbackContext, nullptr);
        return;
    }

    // Make a recorder to record drawing commands into
    std::unique_ptr<Recorder> recorder = this->makeRecorder();

    if (srcRect.size() == dstSize &&
        SkColorSpace::Equals(srcImageInfo.colorInfo().colorSpace(),
                             dstColorSpace.get())) {
        // No need for rescale
        return this->asyncReadPixelsYUV420(recorder.get(),
                                           image,
                                           yuvColorSpace,
                                           readAlpha,
                                           srcRect,
                                           callback,
                                           callbackContext);
    }

    SkImageInfo dstImageInfo = SkImageInfo::Make(dstSize,
                                                 kRGBA_8888_SkColorType,
                                                 srcImageInfo.colorInfo().alphaType(),
                                                 dstColorSpace);
    sk_sp<SkImage> scaledImage = RescaleImage(recorder.get(),
                                              image,
                                              srcRect,
                                              dstImageInfo,
                                              rescaleGamma,
                                              rescaleMode);
    if (!scaledImage) {
        callback(callbackContext, nullptr);
        return;
    }

    this->asyncReadPixelsYUV420(recorder.get(),
                                scaledImage.get(),
                                yuvColorSpace,
                                readAlpha,
                                SkIRect::MakeSize(dstSize),
                                callback,
                                callbackContext);
}

void Context::asyncReadPixelsYUV420(Recorder* recorder,
                                    const SkImage* srcImage,
                                    SkYUVColorSpace yuvColorSpace,
                                    bool readAlpha,
                                    const SkIRect& srcRect,
                                    SkImage::ReadPixelsCallback callback,
                                    SkImage::ReadPixelsContext callbackContext) {
    TRACE_EVENT2("skia.gpu", TRACE_FUNC, "width", srcRect.width(), "height", srcRect.height());

    // Make three or four Surfaces to draw the YUV[A] planes into
    SkImageInfo yaInfo = SkImageInfo::MakeA8(srcRect.size());
    sk_sp<SkSurface> ySurface = Surface::MakeGraphite(recorder, yaInfo, Budgeted::kNo);
    sk_sp<SkSurface> aSurface;
    if (readAlpha) {
        aSurface = Surface::MakeGraphite(recorder, yaInfo, Budgeted::kNo);
    }

    SkImageInfo uvInfo = yaInfo.makeWH(yaInfo.width()/2, yaInfo.height()/2);
    sk_sp<SkSurface> uSurface = Surface::MakeGraphite(recorder, uvInfo, Budgeted::kNo);
    sk_sp<SkSurface> vSurface = Surface::MakeGraphite(recorder, uvInfo, Budgeted::kNo);

    if (!ySurface || !uSurface || !vSurface || (readAlpha && !aSurface)) {
        callback(callbackContext, nullptr);
        return;
    }

    // Set up draws and transfers
    // TODO: Use one transfer buffer for all three planes to reduce map/unmap cost?
    auto drawPlane = [](SkSurface* dstSurface,
                        const SkImage* srcImage,
                        float rgb2yuv[20],
                        const SkMatrix& texMatrix) {
        // Render the plane defined by rgb2yuv from srcImage into dstSurface
        SkPaint paint;
        const SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNone);
        sk_sp<SkShader> imgShader = srcImage->makeShader(SkTileMode::kClamp, SkTileMode::kClamp,
                                                         sampling, texMatrix);
        paint.setShader(std::move(imgShader));

        if (rgb2yuv) {
            sk_sp<SkColorFilter> matrixFilter = SkColorFilters::Matrix(rgb2yuv);
            paint.setColorFilter(std::move(matrixFilter));
        }

        SkCanvas* canvas = dstSurface->getCanvas();
        canvas->drawPaint(paint);
    };

    auto copyPlane = [this](SkSurface* surface) {
        // Transfer result from dstSurface
        auto graphiteSurface = reinterpret_cast<const skgpu::graphite::Surface*>(surface);
        TextureProxyView proxyView = graphiteSurface->readSurfaceView();

        auto srcImageInfo = surface->imageInfo();
        auto dstColorInfo = srcImageInfo.colorInfo().makeColorType(kAlpha_8_SkColorType);
        return this->transferPixels(proxyView.proxy(),
                                    srcImageInfo,
                                    dstColorInfo,
                                    SkIRect::MakeWH(surface->width(), surface->height()));
    };

    float baseM[20];
    SkColorMatrix_RGB2YUV(yuvColorSpace, baseM);
    SkMatrix texMatrix = SkMatrix::Translate(srcRect.fLeft, srcRect.fTop);

    // This matrix generates (r,g,b,a) = (0, 0, 0, y)
    float yM[20];
    std::fill_n(yM, 15, 0.f);
    std::copy_n(baseM + 0, 5, yM + 15);
    drawPlane(ySurface.get(), srcImage, yM, texMatrix);
    if (readAlpha) {
        // No matrix, straight copy of alpha channel
        SkASSERT(baseM[15] == 0 &&
                 baseM[16] == 0 &&
                 baseM[17] == 0 &&
                 baseM[18] == 1 &&
                 baseM[19] == 0);
        drawPlane(aSurface.get(), srcImage, nullptr, texMatrix);
    }

    texMatrix.preScale(0.5f, 0.5f);
    // This matrix generates (r,g,b,a) = (0, 0, 0, u)
    float uM[20];
    std::fill_n(uM, 15, 0.f);
    std::copy_n(baseM + 5, 5, uM + 15);
    drawPlane(uSurface.get(), srcImage, uM, texMatrix);

    // This matrix generates (r,g,b,a) = (0, 0, 0, v)
    float vM[20];
    std::fill_n(vM, 15, 0.f);
    std::copy_n(baseM + 10, 5, vM + 15);
    drawPlane(vSurface.get(), srcImage, vM, texMatrix);

    // Add draw commands to queue
    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        callback(callbackContext, nullptr);
        return;
    }
    InsertRecordingInfo recordingInfo;
    recordingInfo.fRecording = recording.get();
    if (!this->insertRecording(recordingInfo)) {
        callback(callbackContext, nullptr);
        return;
    }

    // Now set up transfers
    PixelTransferResult yTransfer, uTransfer, vTransfer, aTransfer;
    yTransfer = copyPlane(ySurface.get());
    if (!yTransfer.fTransferBuffer) {
        callback(callbackContext, nullptr);
        return;
    }
    uTransfer = copyPlane(uSurface.get());
    if (!uTransfer.fTransferBuffer) {
        callback(callbackContext, nullptr);
        return;
    }
    vTransfer = copyPlane(vSurface.get());
    if (!vTransfer.fTransferBuffer) {
        callback(callbackContext, nullptr);
        return;
    }
    if (readAlpha) {
        aTransfer = copyPlane(aSurface.get());
        if (!aTransfer.fTransferBuffer) {
            callback(callbackContext, nullptr);
            return;
        }
    }

    // Set up FinishContext and add transfer commands to queue
    using AsyncReadResult = skgpu::TAsyncReadResult<Buffer, ContextID, PixelTransferResult>;
    struct FinishContext {
        SkImage::ReadPixelsCallback* fClientCallback;
        SkImage::ReadPixelsContext fClientContext;
        SkISize fSize;
        ClientMappedBufferManager* fMappedBufferManager;
        PixelTransferResult fYTransfer;
        PixelTransferResult fUTransfer;
        PixelTransferResult fVTransfer;
        PixelTransferResult fATransfer;
    };
    auto* finishContext = new FinishContext{callback,
                                            callbackContext,
                                            srcRect.size(),
                                            fMappedBufferManager.get(),
                                            std::move(yTransfer),
                                            std::move(uTransfer),
                                            std::move(vTransfer),
                                            std::move(aTransfer)};
    GpuFinishedProc finishCallback = [](GpuFinishedContext c, CallbackResult status) {
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        if (status == CallbackResult::kSuccess) {
            auto manager = context->fMappedBufferManager;
            auto result = std::make_unique<AsyncReadResult>(manager->ownerID());
            if (!result->addTransferResult(context->fYTransfer, context->fSize,
                                           context->fYTransfer.fRowBytes, manager)) {
                result.reset();
            }
            SkISize uvSize = {context->fSize.width() / 2, context->fSize.height() / 2};
            if (result && !result->addTransferResult(context->fUTransfer, uvSize,
                                                     context->fUTransfer.fRowBytes, manager)) {
                result.reset();
            }
            if (result && !result->addTransferResult(context->fVTransfer, uvSize,
                                                     context->fVTransfer.fRowBytes, manager)) {
                result.reset();
            }
            if (result && context->fATransfer.fTransferBuffer &&
                !result->addTransferResult(context->fATransfer, context->fSize,
                                           context->fATransfer.fRowBytes, manager)) {
                result.reset();
            }
            (*context->fClientCallback)(context->fClientContext, std::move(result));
        } else {
            (*context->fClientCallback)(context->fClientContext, nullptr);
        }
        delete context;
    };

    InsertFinishInfo finishInfo;
    finishInfo.fFinishedContext = finishContext;
    finishInfo.fFinishedProc = finishCallback;
    // If addFinishInfo() fails, it invokes the finish callback automatically, which handles all the
    // required clean up for us, just log an error message.
    if (!fQueueManager->addFinishInfo(finishInfo, fResourceProvider.get())) {
        SKGPU_LOG_E("Failed to register finish callbacks for asyncReadPixels.");
    }
}

Context::PixelTransferResult Context::transferPixels(const TextureProxy* proxy,
                                                     const SkImageInfo& srcImageInfo,
                                                     const SkColorInfo& dstColorInfo,
                                                     const SkIRect& srcRect) {
    SkASSERT(srcImageInfo.bounds().contains(srcRect));

    const Caps* caps = fSharedContext->caps();
    SkColorType supportedColorType =
            caps->supportedReadPixelsColorType(srcImageInfo.colorType(),
                                               proxy->textureInfo(),
                                               dstColorInfo.colorType());
    if (supportedColorType == kUnknown_SkColorType) {
        return {};
    }

    // Fail if read color type does not have all of dstCT's color channels and those missing color
    // channels are in the src.
    uint32_t dstChannels = SkColorTypeChannelFlags(dstColorInfo.colorType());
    uint32_t legalReadChannels = SkColorTypeChannelFlags(supportedColorType);
    uint32_t srcChannels = SkColorTypeChannelFlags(srcImageInfo.colorType());
    if ((~legalReadChannels & dstChannels) & srcChannels) {
        return {};
    }

    size_t rowBytes = caps->getAlignedTextureDataRowBytes(
                              SkColorTypeBytesPerPixel(supportedColorType) * srcRect.width());
    size_t size = SkAlignTo(rowBytes * srcRect.height(), caps->requiredTransferBufferAlignment());
    sk_sp<Buffer> buffer = fResourceProvider->findOrCreateBuffer(
            size, BufferType::kXferGpuToCpu, AccessPattern::kHostVisible);
    if (!buffer) {
        return {};
    }

    // Set up copy task. Since we always use a new buffer the offset can be 0 and we don't need to
    // worry about aligning it to the required transfer buffer alignment.
    sk_sp<CopyTextureToBufferTask> copyTask = CopyTextureToBufferTask::Make(sk_ref_sp(proxy),
                                                                            srcRect,
                                                                            buffer,
                                                                            /*bufferOffset=*/0,
                                                                            rowBytes);
    if (!copyTask || !fQueueManager->addTask(copyTask.get(), this)) {
        return {};
    }
    sk_sp<SynchronizeToCpuTask> syncTask = SynchronizeToCpuTask::Make(buffer);
    if (!syncTask || !fQueueManager->addTask(syncTask.get(), this)) {
        return {};
    }

    PixelTransferResult result;
    result.fTransferBuffer = std::move(buffer);
    if (srcImageInfo.colorInfo() != dstColorInfo) {
        SkISize dims = srcRect.size();
        SkImageInfo srcInfo = SkImageInfo::Make(dims, srcImageInfo.colorInfo());
        SkImageInfo dstInfo = SkImageInfo::Make(dims, dstColorInfo);
        result.fRowBytes = dstInfo.minRowBytes();
        result.fPixelConverter = [dstInfo, srcInfo, rowBytes](
                void* dst, const void* src) {
            SkAssertResult(SkConvertPixels(dstInfo, dst, dstInfo.minRowBytes(),
                                           srcInfo, src, rowBytes));
        };
    } else {
        result.fRowBytes = rowBytes;
    }

    return result;
}


void Context::checkAsyncWorkCompletion() {
    ASSERT_SINGLE_OWNER

    fQueueManager->checkForFinishedWork(SyncToCpu::kNo);
}

void Context::deleteBackendTexture(const BackendTexture& texture) {
    ASSERT_SINGLE_OWNER

    if (!texture.isValid() || texture.backend() != this->backend()) {
        return;
    }
    fResourceProvider->deleteBackendTexture(texture);
}

void Context::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    this->checkAsyncWorkCompletion();

    fResourceProvider->freeGpuResources();
}

void Context::performDeferredCleanup(std::chrono::milliseconds msNotUsed) {
    ASSERT_SINGLE_OWNER

    this->checkAsyncWorkCompletion();

    auto purgeTime = skgpu::StdSteadyClock::now() - msNotUsed;
    fResourceProvider->purgeResourcesNotUsedSince(purgeTime);
}

size_t Context::currentBudgetedBytes() const {
    ASSERT_SINGLE_OWNER
    return fResourceProvider->getResourceCacheCurrentBudgetedBytes();
}

///////////////////////////////////////////////////////////////////////////////////

#if defined(GRAPHITE_TEST_UTILS)
bool ContextPriv::readPixels(const SkPixmap& pm,
                             const TextureProxy* textureProxy,
                             const SkImageInfo& srcImageInfo,
                             int srcX, int srcY) {
    auto rect = SkIRect::MakeXYWH(srcX, srcY, pm.width(), pm.height());
    struct AsyncContext {
        bool fCalled = false;
        std::unique_ptr<const SkImage::AsyncReadResult> fResult;
    } asyncContext;
    fContext->asyncReadPixels(textureProxy, srcImageInfo, pm.info().colorInfo(), rect,
                              [](void* c, std::unique_ptr<const SkImage::AsyncReadResult> result) {
                                  auto context = static_cast<AsyncContext*>(c);
                                  context->fResult = std::move(result);
                                  context->fCalled = true;
                              },
                              &asyncContext);

    if (!asyncContext.fCalled) {
        fContext->submit(SyncToCpu::kYes);
    }
    SkASSERT(asyncContext.fCalled);
    if (!asyncContext.fResult) {
        return false;
    }
    SkRectMemcpy(pm.writable_addr(), pm.rowBytes(), asyncContext.fResult->data(0),
                 asyncContext.fResult->rowBytes(0), pm.info().minRowBytes(),
                 pm.height());
    return true;
}

void ContextPriv::deregisterRecorder(const Recorder* recorder) {
    SKGPU_ASSERT_SINGLE_OWNER(fContext->singleOwner())
    for (auto it = fContext->fTrackedRecorders.begin();
         it != fContext->fTrackedRecorders.end();
         it++) {
        if (*it == recorder) {
            fContext->fTrackedRecorders.erase(it);
            return;
        }
    }
}

bool ContextPriv::supportsPathRendererStrategy(PathRendererStrategy strategy) {
    AtlasProvider::PathAtlasFlagsBitMask pathAtlasFlags =
            AtlasProvider::QueryPathAtlasSupport(this->caps());
    switch (strategy) {
        case PathRendererStrategy::kDefault:
            return true;
        case PathRendererStrategy::kComputeAnalyticAA:
            return SkToBool(pathAtlasFlags & AtlasProvider::PathAtlasFlags::kCompute);
        case PathRendererStrategy::kRasterAA:
            return SkToBool(pathAtlasFlags & AtlasProvider::PathAtlasFlags::kRaster);
        case PathRendererStrategy::kTessellation:
            return true;
    }

    return false;
}

#endif

///////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Context> ContextCtorAccessor::MakeContext(
        sk_sp<SharedContext> sharedContext,
        std::unique_ptr<QueueManager> queueManager,
        const ContextOptions& options) {
    auto context = std::unique_ptr<Context>(new Context(std::move(sharedContext),
                                                        std::move(queueManager),
                                                        options));
    if (context && context->finishInitialization()) {
        return context;
    } else {
        return nullptr;
    }
}

} // namespace skgpu::graphite

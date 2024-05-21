/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkColorFilterPriv.h"
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
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Image_Base_Graphite.h"
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
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/task/UploadTask.h"

#include "src/image/SkSurface_Base.h"

#if defined(GRAPHITE_TEST_UTILS)
#include "include/private/gpu/graphite/ContextOptionsPriv.h"
#if defined(SK_DAWN)
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE
#endif
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

    // This is a client-owned Recorder so pass a null context so it creates its own ResourceProvider
    auto recorder = std::unique_ptr<Recorder>(new Recorder(fSharedContext, options, nullptr));
#if defined(GRAPHITE_TEST_UTILS)
    if (fStoreContextRefInRecorder) {
        recorder->priv().setContext(this);
    }
#endif
    return recorder;
}

std::unique_ptr<Recorder> Context::makeInternalRecorder() const {
    ASSERT_SINGLE_OWNER

    // Unlike makeRecorder(), this Recorder is meant to be short-lived and go
    // away before a Context public API function returns to the caller. As such
    // it shares the Context's resource provider (no separate budget) and does
    // not get tracked. The internal drawing performed with an internal recorder
    // should not require a client image provider.
    return std::unique_ptr<Recorder>(new Recorder(fSharedContext, {}, this));
}

bool Context::insertRecording(const InsertRecordingInfo& info) {
    ASSERT_SINGLE_OWNER

    return fQueueManager->addRecording(info, this);
}

bool Context::submit(SyncToCpu syncToCpu) {
    ASSERT_SINGLE_OWNER

    if (syncToCpu == SyncToCpu::kYes && !fSharedContext->caps()->allowCpuSync()) {
        SKGPU_LOG_E("SyncToCpu::kYes not supported with ContextOptions::fNeverYieldToWebGPU. "
                    "The parameter is ignored and no synchronization will occur.");
        syncToCpu = SyncToCpu::kNo;
    }
    bool success = fQueueManager->submitToGpu();
    this->checkForFinishedWork(syncToCpu);
    return success;
}

bool Context::hasUnfinishedGpuWork() const { return fQueueManager->hasUnfinishedGpuWork(); }

template <typename SrcPixels>
struct Context::AsyncParams {
    const SrcPixels* fSrcImage;
    SkIRect          fSrcRect;
    SkImageInfo      fDstImageInfo;

    SkImage::ReadPixelsCallback* fCallback;
    SkImage::ReadPixelsContext   fCallbackContext;

    template <typename S>
    AsyncParams<S> withNewSource(const S* newPixels, const SkIRect& newSrcRect) const {
        return AsyncParams<S>{newPixels, newSrcRect,
                                fDstImageInfo, fCallback, fCallbackContext};
    }

    void fail() const {
        (*fCallback)(fCallbackContext, nullptr);
    }

    bool validate() const {
        if (!fSrcImage) {
            return false;
        }
        if (fSrcImage->isProtected()) {
            return false;
        }
        if (!SkIRect::MakeSize(fSrcImage->dimensions()).contains(fSrcRect)) {
            return false;
        }
        if (!SkImageInfoIsValid(fDstImageInfo)) {
            return false;
        }
        return true;
    }
};

template <typename ReadFn, typename... ExtraArgs>
void Context::asyncRescaleAndReadImpl(ReadFn Context::* asyncRead,
                                      SkImage::RescaleGamma rescaleGamma,
                                      SkImage::RescaleMode rescaleMode,
                                      const AsyncParams<SkImage>& params,
                                      ExtraArgs... extraParams) {
    if (!params.validate()) {
        return params.fail();
    }

    if (params.fSrcRect.size() == params.fDstImageInfo.dimensions()) {
        // No need to rescale so do a direct readback
        return (this->*asyncRead)(/*recorder=*/nullptr, params, extraParams...);
    }

    // Make a recorder to collect the rescale drawing commands and the copy commands
    std::unique_ptr<Recorder> recorder = this->makeInternalRecorder();
    sk_sp<SkImage> scaledImage = RescaleImage(recorder.get(),
                                              params.fSrcImage,
                                              params.fSrcRect,
                                              params.fDstImageInfo,
                                              rescaleGamma,
                                              rescaleMode);
    if (!scaledImage) {
        SKGPU_LOG_W("AsyncRead failed because rescaling failed");
        return params.fail();
    }
    (this->*asyncRead)(std::move(recorder),
                       params.withNewSource(scaledImage.get(), params.fDstImageInfo.bounds()),
                       extraParams...);
}

void Context::asyncRescaleAndReadPixels(const SkImage* image,
                                        const SkImageInfo& dstImageInfo,
                                        const SkIRect& srcRect,
                                        SkImage::RescaleGamma rescaleGamma,
                                        SkImage::RescaleMode rescaleMode,
                                        SkImage::ReadPixelsCallback callback,
                                        SkImage::ReadPixelsContext callbackContext) {
    this->asyncRescaleAndReadImpl(&Context::asyncReadPixels,
                                  rescaleGamma, rescaleMode,
                                  {image, srcRect, dstImageInfo, callback, callbackContext});
}

void Context::asyncRescaleAndReadPixels(const SkSurface* surface,
                                        const SkImageInfo& dstImageInfo,
                                        const SkIRect& srcRect,
                                        SkImage::RescaleGamma rescaleGamma,
                                        SkImage::RescaleMode rescaleMode,
                                        SkImage::ReadPixelsCallback callback,
                                        SkImage::ReadPixelsContext callbackContext) {
    sk_sp<SkImage> surfaceImage = SkSurfaces::AsImage(sk_ref_sp(surface));
    if (!surfaceImage) {
        // The surface is not texturable, so the only supported readback is if there's no rescaling
        if (surface && asConstSB(surface)->isGraphiteBacked() &&
            srcRect.size() == dstImageInfo.dimensions()) {
            TextureProxy* proxy = static_cast<const Surface*>(surface)->backingTextureProxy();
            return this->asyncReadTexture(/*recorder=*/nullptr,
                                          {proxy, srcRect, dstImageInfo, callback, callbackContext},
                                          surface->imageInfo().colorInfo());
        }
        // else fall through and let asyncRescaleAndReadPixels() invoke the callback when it detects
        // the null image.
    }
    this->asyncRescaleAndReadPixels(surfaceImage.get(),
                                    dstImageInfo,
                                    srcRect,
                                    rescaleGamma,
                                    rescaleMode,
                                    callback,
                                    callbackContext);
}

void Context::asyncReadPixels(std::unique_ptr<Recorder> recorder,
                              const AsyncParams<SkImage>& params) {
    TRACE_EVENT2("skia.gpu", TRACE_FUNC,
                 "width", params.fSrcRect.width(),
                 "height", params.fSrcRect.height());
    SkASSERT(params.validate());
    SkASSERT(params.fSrcRect.size() == params.fDstImageInfo.dimensions());

    const Caps* caps = fSharedContext->caps();
    TextureProxyView view = AsView(params.fSrcImage);
    if (!view || !caps->supportsReadPixels(view.proxy()->textureInfo())) {
        // This is either a YUVA image (null view) or the texture can't be read directly, so
        // perform a draw into a compatible texture format and/or flatten any YUVA planes to RGBA.
        if (!recorder) {
            recorder = this->makeInternalRecorder();
        }
        sk_sp<SkImage> flattened = CopyAsDraw(recorder.get(),
                                            params.fSrcImage,
                                            params.fSrcRect,
                                            params.fDstImageInfo.colorInfo(),
                                            Budgeted::kYes,
                                            Mipmapped::kNo,
                                            SkBackingFit::kApprox,
                                            "AsyncReadPixelsFallbackTexture");
        if (!flattened) {
            SKGPU_LOG_W("AsyncRead failed because copy-as-drawing into a readable format failed");
            return params.fail();
        }
        // Use the original fSrcRect and not flattened's size since it's approx-fit.
        return this->asyncReadPixels(std::move(recorder),
                                     params.withNewSource(flattened.get(),
                                     SkIRect::MakeSize(params.fSrcRect.size())));
    }

    // Can copy directly from the image's texture
    this->asyncReadTexture(std::move(recorder), params.withNewSource(view.proxy(), params.fSrcRect),
                           params.fSrcImage->imageInfo().colorInfo());
}

void Context::asyncReadTexture(std::unique_ptr<Recorder> recorder,
                               const AsyncParams<TextureProxy>& params,
                               const SkColorInfo& srcColorInfo) {
    SkASSERT(params.fSrcRect.size() == params.fDstImageInfo.dimensions());

    // We can get here directly from surface or testing-only read pixels, so re-validate
    if (!params.validate()) {
        return params.fail();
    }
    PixelTransferResult transferResult = this->transferPixels(recorder.get(),
                                                              params.fSrcImage,
                                                              srcColorInfo,
                                                              params.fDstImageInfo.colorInfo(),
                                                              params.fSrcRect);

    if (!transferResult.fTransferBuffer) {
        // TODO: try to do a synchronous readPixels instead
        return params.fail();
    }

    this->finalizeAsyncReadPixels(std::move(recorder),
                                  {&transferResult, 1},
                                  params.fCallback,
                                  params.fCallbackContext);
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
    // Use kOpaque alpha type to signal that we don't read back the alpha channel
    SkImageInfo dstImageInfo = SkImageInfo::Make(dstSize,
                                                 kRGBA_8888_SkColorType,
                                                 kOpaque_SkAlphaType,
                                                 std::move(dstColorSpace));
    this->asyncRescaleAndReadImpl(&Context::asyncReadPixelsYUV420,
                                  rescaleGamma, rescaleMode,
                                  {image, srcRect, dstImageInfo, callback, callbackContext},
                                  yuvColorSpace);
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
    // YUV[A] readback requires the surface to be texturable since the plane conversion is performed
    // by draws. If AsImage() returns null, the image version of asyncRescaleAndReadback will
    // automatically fail.
    // TODO: Is it worth performing an extra copy from 'surface' into a texture in order to succeed?
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
    SkImageInfo dstImageInfo = SkImageInfo::Make(dstSize,
                                                 kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType,
                                                 std::move(dstColorSpace));
    this->asyncRescaleAndReadImpl(&Context::asyncReadPixelsYUV420,
                                  rescaleGamma, rescaleMode,
                                  {image, srcRect, dstImageInfo, callback, callbackContext},
                                  yuvColorSpace);
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

void Context::asyncReadPixelsYUV420(std::unique_ptr<Recorder> recorder,
                                    const AsyncParams<SkImage>& params,
                                    SkYUVColorSpace yuvColorSpace) {
    TRACE_EVENT2("skia.gpu", TRACE_FUNC,
                 "width", params.fSrcRect.width(),
                 "height", params.fSrcRect.height());
    SkASSERT(params.fSrcRect.size() == params.fDstImageInfo.dimensions());

    // The planes are always extracted via drawing, so create the Recorder if there isn't one yet.
    if (!recorder) {
        recorder = this->makeInternalRecorder();
    }

    // copyPlane renders the source image into an A8 image and sets up a transfer stored in 'result'
    auto copyPlane = [&](SkImageInfo planeInfo,
                         std::string_view label,
                         float rgb2yuv[20],
                         const SkMatrix& texMatrix,
                         PixelTransferResult* result) {
        sk_sp<Surface> dstSurface = Surface::MakeScratch(recorder.get(),
                                                         planeInfo,
                                                         std::move(label),
                                                         Budgeted::kYes,
                                                         Mipmapped::kNo,
                                                         SkBackingFit::kApprox);
        if (!dstSurface) {
            return false;
        }

        // Render the plane defined by rgb2yuv from srcImage into dstSurface
        SkPaint paint;
        const SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNone);
        sk_sp<SkShader> imgShader = params.fSrcImage->makeShader(
                SkTileMode::kClamp, SkTileMode::kClamp, sampling, texMatrix);
        paint.setShader(std::move(imgShader));
        paint.setBlendMode(SkBlendMode::kSrc);

        if (rgb2yuv) {
            // NOTE: The dstSurface's color space is set to the requested RGB dstColorSpace, so
            // the rendered image is automatically converted to that RGB color space before the
            // RGB->YUV color filter is evaluated, putting the plane data into the alpha channel.
            paint.setColorFilter(SkColorFilters::Matrix(rgb2yuv));
        }

        SkCanvas* canvas = dstSurface->getCanvas();
        canvas->drawPaint(paint);

        // Manually flush the surface before transferPixels() is called to ensure the rendering
        // operations run before the CopyTextureToBuffer task.
        Flush(dstSurface);
        // Must use planeInfo.bounds() for srcRect since dstSurface is kApprox-fit.
        *result = this->transferPixels(recorder.get(),
                                       dstSurface->backingTextureProxy(),
                                       dstSurface->imageInfo().colorInfo(),
                                       planeInfo.colorInfo(),
                                       planeInfo.bounds());
        return SkToBool(result->fTransferBuffer);
    };

    // Set up draws and transfers. This interleaves the drawing to a plane and the copy to the
    // transfer buffer, which will allow the scratch A8 surface to be reused for each plane.
    // TODO: Use one transfer buffer for all three planes to reduce map/unmap cost?
    const bool readAlpha = params.fDstImageInfo.colorInfo().alphaType() != kOpaque_SkAlphaType;
    SkImageInfo yaInfo = params.fDstImageInfo.makeColorType(kAlpha_8_SkColorType)
                                             .makeAlphaType(kPremul_SkAlphaType);
    SkImageInfo uvInfo = yaInfo.makeWH(yaInfo.width()/2, yaInfo.height()/2);
    PixelTransferResult transfers[4];

    float baseM[20];
    SkColorMatrix_RGB2YUV(yuvColorSpace, baseM);
    SkMatrix texMatrix = SkMatrix::Translate(params.fSrcRect.fLeft, params.fSrcRect.fTop);

    // This matrix generates (r,g,b,a) = (0, 0, 0, y)
    float yM[20];
    std::fill_n(yM, 15, 0.f);
    std::copy_n(baseM + 0, 5, yM + 15);
    if (!copyPlane(yaInfo, "AsyncReadPixelsYPlane", yM, texMatrix, &transfers[0])) {
        return params.fail();
    }

    // No matrix, straight copy of alpha channel
    SkASSERT(baseM[15] == 0 &&
             baseM[16] == 0 &&
             baseM[17] == 0 &&
             baseM[18] == 1 &&
             baseM[19] == 0);
    if (readAlpha &&
        !copyPlane(yaInfo, "AsyncReadPixelsAPlane", nullptr, texMatrix, &transfers[3])) {
        return params.fail();
    }

    // The UV planes are at half resolution compared to Y and A in 4:2:0
    texMatrix.preScale(0.5f, 0.5f);

    // This matrix generates (r,g,b,a) = (0, 0, 0, u)
    float uM[20];
    std::fill_n(uM, 15, 0.f);
    std::copy_n(baseM + 5, 5, uM + 15);
    if (!copyPlane(uvInfo, "AsyncReadPixelsUPlane", uM, texMatrix, &transfers[1])) {
        return params.fail();
    }

    // This matrix generates (r,g,b,a) = (0, 0, 0, v)
    float vM[20];
    std::fill_n(vM, 15, 0.f);
    std::copy_n(baseM + 10, 5, vM + 15);
    if (!copyPlane(uvInfo, "AsyncReadPixelsVPlane", vM, texMatrix, &transfers[2])) {
        return params.fail();
    }

    this->finalizeAsyncReadPixels(std::move(recorder),
                                  {transfers, readAlpha ? 4 : 3},
                                  params.fCallback,
                                  params.fCallbackContext);
}

void Context::finalizeAsyncReadPixels(std::unique_ptr<Recorder> recorder,
                                      SkSpan<PixelTransferResult> transferResults,
                                      SkImage::ReadPixelsCallback callback,
                                      SkImage::ReadPixelsContext callbackContext) {
    // If the async readback work required a Recorder, insert the recording with all of the
    // accumulated work (which includes any copies). Otherwise, for pure copy readbacks,
    // transferPixels() already added the tasks directly to the QueueManager.
    if (recorder) {
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
    }

    // Set up FinishContext and add transfer commands to queue
    struct AsyncReadFinishContext {
        SkImage::ReadPixelsCallback* fClientCallback;
        SkImage::ReadPixelsContext fClientContext;
        ClientMappedBufferManager* fMappedBufferManager;
        std::array<PixelTransferResult, 4> fTransferResults;
    };

    auto finishContext = std::make_unique<AsyncReadFinishContext>();
    finishContext->fClientCallback      = callback;
    finishContext->fClientContext       = callbackContext;
    finishContext->fMappedBufferManager = fMappedBufferManager.get();

    SkASSERT(transferResults.size() <= std::size(finishContext->fTransferResults));
    skia_private::STArray<4, sk_sp<Buffer>> buffersToAsyncMap;
    for (size_t i = 0; i < transferResults.size(); ++i) {
        finishContext->fTransferResults[i] = std::move(transferResults[i]);
        if (fSharedContext->caps()->bufferMapsAreAsync()) {
            buffersToAsyncMap.push_back(finishContext->fTransferResults[i].fTransferBuffer);
        }
    }

    InsertFinishInfo info;
    info.fFinishedContext = finishContext.release();
    info.fFinishedProc = [](GpuFinishedContext c, CallbackResult status) {
        std::unique_ptr<const AsyncReadFinishContext> context(
                reinterpret_cast<const AsyncReadFinishContext*>(c));
        using AsyncReadResult = skgpu::TAsyncReadResult<Buffer, ContextID, PixelTransferResult>;

        ClientMappedBufferManager* manager = context->fMappedBufferManager;
        std::unique_ptr<AsyncReadResult> result;
        if (status == CallbackResult::kSuccess) {
            result = std::make_unique<AsyncReadResult>(manager->ownerID());
        }
        for (const auto& r : context->fTransferResults) {
            if (!r.fTransferBuffer) {
                break;
            }
            if (result && !result->addTransferResult(r, r.fSize, r.fRowBytes, manager)) {
                result.reset();
            }
            // If we didn't get this buffer into the mapped buffer manager then make sure it gets
            // unmapped if it has a pending or completed async map.
            if (!result && r.fTransferBuffer->isUnmappable()) {
                r.fTransferBuffer->unmap();
            }
        }
        (*context->fClientCallback)(context->fClientContext, std::move(result));
    };

    // If addFinishInfo() fails, it invokes the finish callback automatically, which handles all the
    // required clean up for us, just log an error message. The buffers will never be mapped and
    // thus don't need an unmap.
    if (!fQueueManager->addFinishInfo(info, fResourceProvider.get(), buffersToAsyncMap)) {
        SKGPU_LOG_E("Failed to register finish callbacks for asyncReadPixels.");
        return;
    }
}

Context::PixelTransferResult Context::transferPixels(Recorder* recorder,
                                                     const TextureProxy* srcProxy,
                                                     const SkColorInfo& srcColorInfo,
                                                     const SkColorInfo& dstColorInfo,
                                                     const SkIRect& srcRect) {
    SkASSERT(SkIRect::MakeSize(srcProxy->dimensions()).contains(srcRect));
    SkASSERT(SkColorInfoIsValid(dstColorInfo));

    const Caps* caps = fSharedContext->caps();
    if (!srcProxy || !caps->supportsReadPixels(srcProxy->textureInfo())) {
        return {};
    }

    const SkColorType srcColorType = srcColorInfo.colorType();
    SkColorType supportedColorType;
    bool isRGB888Format;
    std::tie(supportedColorType, isRGB888Format) =
            caps->supportedReadPixelsColorType(srcColorType,
                                               srcProxy->textureInfo(),
                                               dstColorInfo.colorType());
    if (supportedColorType == kUnknown_SkColorType) {
        return {};
    }

    // Fail if read color type does not have all of dstCT's color channels and those missing color
    // channels are in the src.
    uint32_t dstChannels = SkColorTypeChannelFlags(dstColorInfo.colorType());
    uint32_t legalReadChannels = SkColorTypeChannelFlags(supportedColorType);
    uint32_t srcChannels = SkColorTypeChannelFlags(srcColorType);
    if ((~legalReadChannels & dstChannels) & srcChannels) {
        return {};
    }

    int bpp = isRGB888Format ? 3 : SkColorTypeBytesPerPixel(supportedColorType);
    size_t rowBytes = caps->getAlignedTextureDataRowBytes(bpp * srcRect.width());
    size_t size = SkAlignTo(rowBytes * srcRect.height(), caps->requiredTransferBufferAlignment());
    sk_sp<Buffer> buffer = fResourceProvider->findOrCreateBuffer(
            size, BufferType::kXferGpuToCpu, AccessPattern::kHostVisible, "TransferToCpu");
    if (!buffer) {
        return {};
    }

    // Set up copy task. Since we always use a new buffer the offset can be 0 and we don't need to
    // worry about aligning it to the required transfer buffer alignment.
    sk_sp<CopyTextureToBufferTask> copyTask = CopyTextureToBufferTask::Make(sk_ref_sp(srcProxy),
                                                                            srcRect,
                                                                            buffer,
                                                                            /*bufferOffset=*/0,
                                                                            rowBytes);
    const bool addTasksDirectly = !SkToBool(recorder);

    if (!copyTask || (addTasksDirectly && !fQueueManager->addTask(copyTask.get(), this))) {
        return {};
    } else if (!addTasksDirectly) {
        // Add the task to the Recorder instead of the QueueManager if that's been required for
        // collecting tasks to prepare the copied textures.
        recorder->priv().add(std::move(copyTask));
    }
    sk_sp<SynchronizeToCpuTask> syncTask = SynchronizeToCpuTask::Make(buffer);
    if (!syncTask || (addTasksDirectly && !fQueueManager->addTask(syncTask.get(), this))) {
        return {};
    } else if (!addTasksDirectly) {
        recorder->priv().add(std::move(syncTask));
    }

    PixelTransferResult result;
    result.fTransferBuffer = std::move(buffer);
    result.fSize = srcRect.size();
    // srcColorInfo describes the texture; readColorInfo describes the result of the copy-to-buffer,
    // which may be different; dstColorInfo is what we have to transform it into when invoking the
    // async callbacks.
    SkColorInfo readColorInfo = srcColorInfo.makeColorType(supportedColorType);
    if (readColorInfo != dstColorInfo || isRGB888Format) {
        SkISize dims = srcRect.size();
        SkImageInfo srcInfo = SkImageInfo::Make(dims, readColorInfo);
        SkImageInfo dstInfo = SkImageInfo::Make(dims, dstColorInfo);
        result.fRowBytes = dstInfo.minRowBytes();
        result.fPixelConverter = [dstInfo, srcInfo, rowBytes, isRGB888Format](
                void* dst, const void* src) {
            SkAutoPixmapStorage temp;
            size_t srcRowBytes = rowBytes;
            if (isRGB888Format) {
                temp.alloc(srcInfo);
                size_t tRowBytes = temp.rowBytes();
                auto* sRow = reinterpret_cast<const char*>(src);
                auto* tRow = reinterpret_cast<char*>(temp.writable_addr());
                for (int y = 0; y < srcInfo.height(); ++y, sRow += srcRowBytes, tRow += tRowBytes) {
                    for (int x = 0; x < srcInfo.width(); ++x) {
                        auto s = sRow + x*3;
                        auto t = tRow + x*sizeof(uint32_t);
                        memcpy(t, s, 3);
                        t[3] = static_cast<char>(0xFF);
                    }
                }
                src = temp.addr();
                srcRowBytes = tRowBytes;
            }
            SkAssertResult(SkConvertPixels(dstInfo, dst, dstInfo.minRowBytes(),
                                           srcInfo, src, srcRowBytes));
        };
    } else {
        result.fRowBytes = rowBytes;
    }

    return result;
}

void Context::checkForFinishedWork(SyncToCpu syncToCpu) {
    ASSERT_SINGLE_OWNER

    fQueueManager->checkForFinishedWork(syncToCpu);
    fMappedBufferManager->process();
}

void Context::checkAsyncWorkCompletion() {
    this->checkForFinishedWork(SyncToCpu::kNo);
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

size_t Context::maxBudgetedBytes() const {
    ASSERT_SINGLE_OWNER
    return fResourceProvider->getResourceCacheLimit();
}

void Context::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    ASSERT_SINGLE_OWNER
    fResourceProvider->dumpMemoryStatistics(traceMemoryDump);
    // TODO: What is the graphite equivalent for the text blob cache and how do we print out its
    // used bytes here (see Ganesh implementation).
}

bool Context::isDeviceLost() const {
    return fSharedContext->isDeviceLost();
}

int Context::maxTextureSize() const {
    return fSharedContext->caps()->maxTextureSize();
}

bool Context::supportsProtectedContent() const {
    return fSharedContext->isProtected() == Protected::kYes;
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

    auto asyncCallback = [](void* c, std::unique_ptr<const SkImage::AsyncReadResult> out) {
        auto context = static_cast<AsyncContext*>(c);
        context->fResult = std::move(out);
        context->fCalled = true;
    };

    const SkColorInfo& srcColorInfo = srcImageInfo.colorInfo();

    // This is roughly equivalent to the logic taken in asyncRescaleAndRead(SkSurface) to either
    // try the image-based readback (with copy-as-draw fallbacks) or read the texture directly
    // if it supports reading.
    if (!fContext->fSharedContext->caps()->supportsReadPixels(textureProxy->textureInfo())) {
        // Since this is a synchronous testing-only API, callers should have flushed any pending
        // work that modifies this texture proxy already. This means we don't have to worry about
        // re-wrapping the proxy in a new Image (that wouldn't tbe connected to any Device, etc.).
        sk_sp<SkImage> image{new Image(TextureProxyView(sk_ref_sp(textureProxy)), srcColorInfo)};
        fContext->asyncReadPixels(/*recorder=*/nullptr,
                                  {image.get(), rect, pm.info(), asyncCallback, &asyncContext});
    } else {
        fContext->asyncReadTexture(/*recorder=*/nullptr,
                                   {textureProxy, rect, pm.info(), asyncCallback, &asyncContext},
                                   srcImageInfo.colorInfo());
    }

    if (fContext->fSharedContext->caps()->allowCpuSync()) {
        fContext->submit(SyncToCpu::kYes);
    } else {
        fContext->submit(SyncToCpu::kNo);
        if (fContext->fSharedContext->backend() == BackendApi::kDawn) {
            while (!asyncContext.fCalled) {
#if defined(SK_DAWN)
                auto dawnContext = static_cast<DawnSharedContext*>(fContext->fSharedContext.get());
                dawnContext->device().Tick();
                fContext->checkAsyncWorkCompletion();
#endif
            }
        } else {
            SK_ABORT("Only Dawn supports non-synching contexts.");
        }
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
        case PathRendererStrategy::kComputeMSAA16:
        case PathRendererStrategy::kComputeMSAA8:
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

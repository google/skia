/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"

#include "include/core/SkPathTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/CombinationBuilder.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ClientMappedBufferManager.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/UploadTask.h"

#ifdef SK_DAWN
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/dawn/DawnQueueManager.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#endif

#ifdef SK_METAL
#include "src/gpu/graphite/mtl/MtlTrampoline.h"
#endif

#ifdef SK_VULKAN
#include "include/gpu/vk/VulkanBackendContext.h"
#include "src/gpu/graphite/vk/VulkanQueueManager.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#endif

#ifdef SK_ENABLE_PRECOMPILE
#include "src/gpu/graphite/PaintOptionsPriv.h"
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
#if GRAPHITE_TEST_UTILS
        , fStoreContextRefInRecorder(options.fStoreContextRefInRecorder)
#endif
        , fContextID(ContextID::Next()) {
    // We have to create this outside the initializer list because we need to pass in the Context's
    // SingleOwner object and it is declared last
    fResourceProvider = fSharedContext->makeResourceProvider(&fSingleOwner);
    fMappedBufferManager = std::make_unique<ClientMappedBufferManager>(this->contextID());
}

Context::~Context() {
#if GRAPHITE_TEST_UTILS
    ASSERT_SINGLE_OWNER
    for (auto& recorder : fTrackedRecorders) {
        recorder->priv().setContext(nullptr);
    }
#endif
}

BackendApi Context::backend() const { return fSharedContext->backend(); }

#ifdef SK_DAWN
std::unique_ptr<Context> Context::MakeDawn(const DawnBackendContext& backendContext,
                                           const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = DawnSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    auto queueManager =
            std::make_unique<DawnQueueManager>(backendContext.fQueue, sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    auto context = std::unique_ptr<Context>(new Context(std::move(sharedContext),
                                                        std::move(queueManager),
                                                        options));
    SkASSERT(context);
    return context;
}
#endif

#ifdef SK_METAL
std::unique_ptr<Context> Context::MakeMetal(const MtlBackendContext& backendContext,
                                            const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = MtlTrampoline::MakeSharedContext(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    auto queueManager = MtlTrampoline::MakeQueueManager(backendContext, sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    auto context = std::unique_ptr<Context>(new Context(std::move(sharedContext),
                                                        std::move(queueManager),
                                                        options));
    SkASSERT(context);
    return context;
}
#endif

#ifdef SK_VULKAN
std::unique_ptr<Context> Context::MakeVulkan(const VulkanBackendContext& backendContext,
                                             const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = VulkanSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    std::unique_ptr<QueueManager> queueManager(new VulkanQueueManager(backendContext.fQueue,
                                                                      sharedContext.get()));
    if (!queueManager) {
        return nullptr;
    }

    auto context = std::unique_ptr<Context>(new Context(std::move(sharedContext),
                                                        std::move(queueManager),
                                                        options));
    SkASSERT(context);
    return context;
}
#endif

std::unique_ptr<Recorder> Context::makeRecorder(const RecorderOptions& options) {
    ASSERT_SINGLE_OWNER

    auto recorder = std::unique_ptr<Recorder>(new Recorder(fSharedContext, options));
#if GRAPHITE_TEST_UTILS
    if (fStoreContextRefInRecorder) {
        recorder->priv().setContext(this);
    }
#endif
    return recorder;
}

bool Context::insertRecording(const InsertRecordingInfo& info) {
    ASSERT_SINGLE_OWNER

    return fQueueManager->addRecording(info, fResourceProvider.get());
}

bool Context::submit(SyncToCpu syncToCpu) {
    ASSERT_SINGLE_OWNER

    bool success = fQueueManager->submitToGpu();
    fQueueManager->checkForFinishedWork(syncToCpu);
    return success;
}

void Context::asyncReadPixels(const SkImage* image,
                              const SkColorInfo& dstColorInfo,
                              const SkIRect& srcRect,
                              SkImage::ReadPixelsCallback callback,
                              SkImage::ReadPixelsContext callbackContext) {
    if (!as_IB(image)->isGraphiteBacked()) {
        callback(callbackContext, nullptr);
        return;
    }
    auto graphiteImage = reinterpret_cast<const skgpu::graphite::Image*>(image);
    TextureProxyView proxyView = graphiteImage->textureProxyView();

    this->asyncReadPixels(proxyView.proxy(),
                          image->imageInfo(),
                          dstColorInfo,
                          srcRect,
                          callback,
                          callbackContext);
}

void Context::asyncReadPixels(const SkSurface* surface,
                              const SkColorInfo& dstColorInfo,
                              const SkIRect& srcRect,
                              SkImage::ReadPixelsCallback callback,
                              SkImage::ReadPixelsContext callbackContext) {
    if (!static_cast<const SkSurface_Base*>(surface)->isGraphiteBacked()) {
        callback(callbackContext, nullptr);
        return;
    }
    auto graphiteSurface = reinterpret_cast<const skgpu::graphite::Surface*>(surface);
    TextureProxyView proxyView = graphiteSurface->readSurfaceView();

    this->asyncReadPixels(proxyView.proxy(),
                          surface->imageInfo(),
                          dstColorInfo,
                          srcRect,
                          callback,
                          callbackContext);
}

void Context::asyncReadPixels(const TextureProxy* proxy,
                              const SkImageInfo& srcImageInfo,
                              const SkColorInfo& dstColorInfo,
                              const SkIRect& srcRect,
                              SkImage::ReadPixelsCallback callback,
                              SkImage::ReadPixelsContext callbackContext) {
    if (!proxy) {
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
        size_t fRowBytes;
        ClientMappedBufferManager* fMappedBufferManager;
        PixelTransferResult fTransferResult;
    };
    size_t rowBytes = fSharedContext->caps()->getAlignedTextureDataRowBytes(
            srcRect.width() * SkColorTypeBytesPerPixel(dstColorInfo.colorType()));
    auto* finishContext = new FinishContext{callback,
                                            callbackContext,
                                            srcRect.size(),
                                            rowBytes,
                                            fMappedBufferManager.get(),
                                            std::move(transferResult)};
    GpuFinishedProc finishCallback = [](GpuFinishedContext c, CallbackResult status) {
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        if (status == CallbackResult::kSuccess) {
            ClientMappedBufferManager* manager = context->fMappedBufferManager;
            auto result = std::make_unique<AsyncReadResult>(manager->ownerID());
            if (!result->addTransferResult(context->fTransferResult, context->fSize,
                                        context->fRowBytes, manager)) {
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
    size_t size = rowBytes * srcRect.height();
    sk_sp<Buffer> buffer = fResourceProvider->findOrCreateBuffer(
            size,
            BufferType::kXferCpuToGpu,
            PrioritizeGpuReads::kNo);
    if (!buffer) {
        return {};
    }

    // Set up copy task
    sk_sp<CopyTextureToBufferTask> copyTask = CopyTextureToBufferTask::Make(sk_ref_sp(proxy),
                                                                            srcRect,
                                                                            buffer,
                                                                            /*bufferOffset=*/0,
                                                                            rowBytes);
    if (!copyTask || !fQueueManager->addTask(copyTask.get(), fResourceProvider.get())) {
        return {};
    }
    sk_sp<SynchronizeToCpuTask> syncTask = SynchronizeToCpuTask::Make(buffer);
    if (!syncTask || !fQueueManager->addTask(syncTask.get(), fResourceProvider.get())) {
        return {};
    }

    PixelTransferResult result;
    result.fTransferBuffer = std::move(buffer);
    if (srcImageInfo.colorInfo() != dstColorInfo) {
        result.fPixelConverter = [dims = srcRect.size(), dstColorInfo, srcImageInfo](
                void* dst, const void* src) {
            SkImageInfo srcInfo = SkImageInfo::Make(dims, srcImageInfo.colorInfo());
            SkImageInfo dstInfo = SkImageInfo::Make(dims, dstColorInfo);
            SkAssertResult(SkConvertPixels(dstInfo, dst, dstInfo.minRowBytes(),
                                           srcInfo, src, srcInfo.minRowBytes()));
        };
    }

    return result;
}


void Context::checkAsyncWorkCompletion() {
    ASSERT_SINGLE_OWNER

    fQueueManager->checkForFinishedWork(SyncToCpu::kNo);
}

#ifdef SK_ENABLE_PRECOMPILE

void Context::precompile(const PaintOptions& options) {
    ASSERT_SINGLE_OWNER

    auto rtEffectDict = std::make_unique<RuntimeEffectDictionary>();

    KeyContext keyContext(fSharedContext->shaderCodeDictionary(), rtEffectDict.get());

    options.priv().buildCombinations(
        keyContext,
        [&](UniquePaintParamsID uniqueID) {
            for (const Renderer* r : fSharedContext->rendererProvider()->renderers()) {
                for (auto&& s : r->steps()) {
                    if (s->performsShading()) {
                        GraphicsPipelineDesc pipelineDesc(s, uniqueID);
                        (void) pipelineDesc;

                        // TODO: Combine the desc with the renderpass description set to generate a
                        // full GraphicsPipeline and MSL program. Cache that compiled pipeline on
                        // the resource provider in a map from desc -> pipeline so that any
                        // later desc created from equivalent RenderStep + Combination maps to it.
                    }
                }
            }
        });
}

BlenderID Context::addUserDefinedBlender(sk_sp<SkRuntimeEffect> effect) {
    return fSharedContext->shaderCodeDictionary()->addUserDefinedBlender(std::move(effect));
}

void Context::precompile(CombinationBuilder* combinationBuilder) {
    ASSERT_SINGLE_OWNER

    combinationBuilder->buildCombinations(
            fSharedContext->shaderCodeDictionary(),
            [&](UniquePaintParamsID uniqueID) {
                for (const Renderer* r : fSharedContext->rendererProvider()->renderers()) {
                    for (auto&& s : r->steps()) {
                        if (s->performsShading()) {
                            GraphicsPipelineDesc desc(s, uniqueID);
                            (void) desc;
                            // TODO: Combine with renderpass description set to generate full
                            // GraphicsPipeline and MSL program. Cache that compiled pipeline on
                            // the resource provider in a map from desc -> pipeline so that any
                            // later desc created from equivalent RenderStep + Combination get it.
                        }
                    }
                }
            });

    // TODO: Iterate over the renderers and make descriptions for the steps that don't perform
    // shading, and just use ShaderType::kNone.
}

#endif // SK_ENABLE_PRECOMPILE

void Context::deleteBackendTexture(BackendTexture& texture) {
    ASSERT_SINGLE_OWNER

    if (!texture.isValid() || texture.backend() != this->backend()) {
        return;
    }
    fResourceProvider->deleteBackendTexture(texture);
}

///////////////////////////////////////////////////////////////////////////////////

#if GRAPHITE_TEST_UTILS
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

#endif

} // namespace skgpu::graphite

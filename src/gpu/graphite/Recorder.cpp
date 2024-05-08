/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recorder.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recording.h"

#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/DataUtils.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RasterPathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ScratchResourceManager.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/TaskList.h"
#include "src/gpu/graphite/task/UploadTask.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"
#include "src/image/SkImage_Base.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())
#define ASSERT_SINGLE_OWNER_PRIV SKGPU_ASSERT_SINGLE_OWNER(fRecorder->singleOwner())

/*
 * The default image provider doesn't perform any conversion so, by default, Graphite won't
 * draw any non-Graphite-backed images.
 */
class DefaultImageProvider final : public ImageProvider {
public:
    static sk_sp<DefaultImageProvider> Make() {
        return sk_ref_sp(new DefaultImageProvider);
    }

    sk_sp<SkImage> findOrCreate(Recorder* recorder,
                                const SkImage* image,
                                SkImage::RequiredProperties) override {
        SkASSERT(!as_IB(image)->isGraphiteBacked());

        return nullptr;
    }

private:
    DefaultImageProvider() {}
};

/**************************************************************************************************/
RecorderOptions::RecorderOptions() = default;
RecorderOptions::RecorderOptions(const RecorderOptions&) = default;
RecorderOptions::~RecorderOptions() = default;

/**************************************************************************************************/
static uint32_t next_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}

Recorder::Recorder(sk_sp<SharedContext> sharedContext, const RecorderOptions& options)
        : fSharedContext(std::move(sharedContext))
        , fRuntimeEffectDict(std::make_unique<RuntimeEffectDictionary>())
        , fRootTaskList(new TaskList)
        , fUniformDataCache(new UniformDataCache)
        , fTextureDataCache(new TextureDataCache)
        , fProxyReadCounts(new ProxyReadCountMap)
        , fUniqueID(next_id())
        , fAtlasProvider(std::make_unique<AtlasProvider>(this))
        , fTokenTracker(std::make_unique<TokenTracker>())
        , fStrikeCache(std::make_unique<sktext::gpu::StrikeCache>())
        , fTextBlobCache(std::make_unique<sktext::gpu::TextBlobRedrawCoordinator>(fUniqueID)) {
    fClientImageProvider = options.fImageProvider;
    if (!fClientImageProvider) {
        fClientImageProvider = DefaultImageProvider::Make();
    }

    fResourceProvider = fSharedContext->makeResourceProvider(this->singleOwner(),
                                                             fUniqueID,
                                                             options.fGpuBudgetInBytes);
    fUploadBufferManager = std::make_unique<UploadBufferManager>(fResourceProvider.get(),
                                                                 fSharedContext->caps());
    fDrawBufferManager = std::make_unique<DrawBufferManager>(fResourceProvider.get(),
                                                             fSharedContext->caps(),
                                                             fUploadBufferManager.get());

    SkASSERT(fResourceProvider);
}

Recorder::~Recorder() {
    ASSERT_SINGLE_OWNER
    // Any finished procs that haven't been passed to a Recording fail
    for (int i = 0; i < fFinishedProcs.size(); ++i) {
        fFinishedProcs[i]->setFailureResult();
    }

    for (auto& device : fTrackedDevices) {
        // deregisterDevice() may have left an entry as null previously.
        if (device) {
            device->abandonRecorder();
        }
    }
#if defined(GRAPHITE_TEST_UTILS)
    if (fContext) {
        fContext->priv().deregisterRecorder(this);
    }
#endif

    // TODO: needed?
    fStrikeCache->freeAll();
}

BackendApi Recorder::backend() const { return fSharedContext->backend(); }

std::unique_ptr<Recording> Recorder::snap() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    ASSERT_SINGLE_OWNER
    this->priv().flushTrackedDevices();

    std::unordered_set<sk_sp<TextureProxy>, Recording::ProxyHash> nonVolatileLazyProxies;
    std::unordered_set<sk_sp<TextureProxy>, Recording::ProxyHash> volatileLazyProxies;
    fTextureDataCache->foreach([&](const TextureDataBlock* block) {
        for (int j = 0; j < block->numTextures(); ++j) {
            const TextureDataBlock::SampledTexture& tex = block->texture(j);

            if (tex.first->isLazy()) {
                if (tex.first->isVolatile()) {
                    volatileLazyProxies.insert(tex.first);
                } else {
                    nonVolatileLazyProxies.insert(tex.first);
                }
            }
        }
    });

    std::unique_ptr<Recording::LazyProxyData> targetProxyData;
    if (fTargetProxyData) {
        targetProxyData = std::move(fTargetProxyData);
        fTargetProxyDevice.reset();
        fTargetProxyCanvas.reset();
    }

    // The scratch resources only need to be tracked until prepareResources() is finished, so
    // Recorder doesn't hold a persistent manager and it can be deleted when snap() returns.
    ScratchResourceManager scratchManager{fResourceProvider.get(), std::move(fProxyReadCounts)};
    fProxyReadCounts = std::make_unique<ProxyReadCountMap>();

    // In both the "task failed" case and the "everything is discarded" case, there's no work that
    // needs to be done in insertRecording(). However, we use nullptr as a failure signal, so
    // kDiscard will return a non-null Recording that has no tasks in it.
    if (fDrawBufferManager->hasMappingFailed() ||
        fRootTaskList->prepareResources(fResourceProvider.get(),
                                        &scratchManager,
                                        fRuntimeEffectDict.get()) == Task::Status::kFail) {
        // Leaving 'fTrackedDevices' alone since they were flushed earlier and could still be
        // attached to extant SkSurfaces.
        fDrawBufferManager = std::make_unique<DrawBufferManager>(fResourceProvider.get(),
                                                                 fSharedContext->caps(),
                                                                 fUploadBufferManager.get());
        fTextureDataCache = std::make_unique<TextureDataCache>();
        fUniformDataCache = std::make_unique<UniformDataCache>();
        fRootTaskList->reset();
        fRuntimeEffectDict->reset();
        return nullptr;
    }

    std::unique_ptr<Recording> recording(new Recording(fNextRecordingID++,
                                                       fUniqueID,
                                                       std::move(nonVolatileLazyProxies),
                                                       std::move(volatileLazyProxies),
                                                       std::move(targetProxyData),
                                                       std::move(fFinishedProcs)));

    // Allow the buffer managers to add any collected tasks for data transfer or initialization
    // before moving the root task list to the Recording.
    fDrawBufferManager->transferToRecording(recording.get());
    fUploadBufferManager->transferToRecording(recording.get());
    recording->priv().addTasks(std::move(*fRootTaskList));

    SkASSERT(!fRootTaskList->hasTasks());
    fRuntimeEffectDict->reset();
    fTextureDataCache = std::make_unique<TextureDataCache>();
    fUniformDataCache = std::make_unique<UniformDataCache>();
    if (!this->priv().caps()->requireOrderedRecordings()) {
        fAtlasProvider->textAtlasManager()->evictAtlases();
    }

    return recording;
}

SkCanvas* Recorder::makeDeferredCanvas(const SkImageInfo& imageInfo,
                                       const TextureInfo& textureInfo) {
    // Mipmaps can't reasonably be kept valid on a deferred surface with no actual texture.
    if (textureInfo.mipmapped() == Mipmapped::kYes) {
        SKGPU_LOG_W("Requested a deferred canvas with mipmapping; this is not supported");
        return nullptr;
    }

    if (fTargetProxyCanvas) {
        // Require snapping before requesting another canvas.
        SKGPU_LOG_W("Requested a new deferred canvas before snapping the previous one");
        return nullptr;
    }

    fTargetProxyData = std::make_unique<Recording::LazyProxyData>(textureInfo);
    // Use kLoad for the initial load op since the purpose of a deferred canvas is to draw on top
    // of an existing, late-bound texture.
    fTargetProxyDevice = Device::Make(this,
                                      fTargetProxyData->refLazyProxy(),
                                      imageInfo.dimensions(),
                                      imageInfo.colorInfo(),
                                      {},
                                      LoadOp::kLoad);
    fTargetProxyCanvas = std::make_unique<SkCanvas>(fTargetProxyDevice);
    return fTargetProxyCanvas.get();
}

void Recorder::registerDevice(sk_sp<Device> device) {
    ASSERT_SINGLE_OWNER

    SkASSERT(device);
#if defined(SK_DEBUG)
    // TODO(b/333073673): Confirm the device isn't already in the tracked list
    for (const sk_sp<Device>& tracked : fTrackedDevices) {
        SkASSERT(tracked.get() != device.get());
    }
#endif

    // By taking a ref on tracked devices, the Recorder prevents the Device from being deleted on
    // another thread unless the Recorder has been destroyed or the device has abandoned its
    // recorder (e.g. was marked immutable).
    fTrackedDevices.emplace_back(std::move(device));
}

void Recorder::deregisterDevice(const Device* device) {
    ASSERT_SINGLE_OWNER
    for (int i = 0; i < fTrackedDevices.size(); ++i) {
        if (fTrackedDevices[i].get() == device) {
            // Don't modify the list structure of fTrackedDevices within this loop
            fTrackedDevices[i] = nullptr;
            break;
        }
    }

#if defined(SK_DEBUG)
    // TODO(b/333073673): Confirm that the device is not in the tracked list anymore.
    for (const sk_sp<Device>& tracked : fTrackedDevices) {
        SkASSERT(tracked.get() != device);
    }
#endif
}

BackendTexture Recorder::createBackendTexture(SkISize dimensions, const TextureInfo& info) {
    ASSERT_SINGLE_OWNER

    if (!info.isValid() || info.backend() != this->backend()) {
        return {};
    }
    return fResourceProvider->createBackendTexture(dimensions, info);
}

#ifdef SK_BUILD_FOR_ANDROID

BackendTexture Recorder::createBackendTexture(AHardwareBuffer* hardwareBuffer,
                                              bool isRenderable,
                                              bool isProtectedContent,
                                              SkISize dimensions,
                                              bool fromAndroidWindow) const {
    if (fSharedContext->backend() != BackendApi::kVulkan) {
        SKGPU_LOG_W("Creating an AHardwareBuffer-backed BackendTexture is only supported with the"
                    "Vulkan backend.");
        return {};
    }
    return fResourceProvider->createBackendTexture(hardwareBuffer,
                                                   isRenderable,
                                                   isProtectedContent,
                                                   dimensions,
                                                   fromAndroidWindow);
}

#endif // SK_BUILD_FOR_ANDROID

bool Recorder::updateBackendTexture(const BackendTexture& backendTex,
                                    const SkPixmap srcData[],
                                    int numLevels) {
    ASSERT_SINGLE_OWNER

    if (!backendTex.isValid() || backendTex.backend() != this->backend()) {
        return false;
    }

    if (!srcData || numLevels <= 0) {
        return false;
    }

    // If the texture has MIP levels then we require that the full set is overwritten.
    int numExpectedLevels = 1;
    if (backendTex.info().mipmapped() == Mipmapped::kYes) {
        numExpectedLevels = SkMipmap::ComputeLevelCount(backendTex.dimensions().width(),
                                                        backendTex.dimensions().height()) + 1;
    }
    if (numLevels != numExpectedLevels) {
        return false;
    }

    SkColorType ct = srcData[0].colorType();

    if (!this->priv().caps()->areColorTypeAndTextureInfoCompatible(ct, backendTex.info())) {
        return false;
    }

    sk_sp<Texture> texture = this->priv().resourceProvider()->createWrappedTexture(backendTex);
    if (!texture) {
        return false;
    }

    sk_sp<TextureProxy> proxy = TextureProxy::Wrap(std::move(texture));

    std::vector<MipLevel> mipLevels;
    mipLevels.resize(numLevels);

    for (int i = 0; i < numLevels; ++i) {
        SkASSERT(srcData[i].addr());
        SkASSERT(srcData[i].info().colorInfo() == srcData[0].info().colorInfo());

        mipLevels[i].fPixels = srcData[i].addr();
        mipLevels[i].fRowBytes = srcData[i].rowBytes();
    }

    // Src and dst colorInfo are the same
    const SkColorInfo& colorInfo = srcData[0].info().colorInfo();
    // Add UploadTask to Recorder
    UploadInstance upload = UploadInstance::Make(this,
                                                 std::move(proxy),
                                                 colorInfo, colorInfo,
                                                 mipLevels,
                                                 SkIRect::MakeSize(backendTex.dimensions()),
                                                 std::make_unique<ImageUploadContext>());
    if (!upload.isValid()) {
        SKGPU_LOG_E("Recorder::updateBackendTexture: Could not create UploadInstance");
        return false;
    }
    sk_sp<Task> uploadTask = UploadTask::Make(std::move(upload));

    // Need to flush any pending work in case it depends on this texture
    this->priv().flushTrackedDevices();

    this->priv().add(std::move(uploadTask));

    return true;
}

bool Recorder::updateCompressedBackendTexture(const BackendTexture& backendTex,
                                              const void* data,
                                              size_t dataSize) {
    ASSERT_SINGLE_OWNER

    if (!backendTex.isValid() || backendTex.backend() != this->backend()) {
        return false;
    }

    if (!data) {
        return false;
    }

    sk_sp<Texture> texture = this->priv().resourceProvider()->createWrappedTexture(backendTex);
    if (!texture) {
        return false;
    }

    sk_sp<TextureProxy> proxy = TextureProxy::Wrap(std::move(texture));

    // Add UploadTask to Recorder
    UploadInstance upload = UploadInstance::MakeCompressed(this,
                                                           std::move(proxy),
                                                           data,
                                                           dataSize);
    if (!upload.isValid()) {
        SKGPU_LOG_E("Recorder::updateBackendTexture: Could not create UploadInstance");
        return false;
    }
    sk_sp<Task> uploadTask = UploadTask::Make(std::move(upload));

    // Need to flush any pending work in case it depends on this texture
    this->priv().flushTrackedDevices();

    this->priv().add(std::move(uploadTask));

    return true;
}

void Recorder::deleteBackendTexture(const BackendTexture& texture) {
    ASSERT_SINGLE_OWNER

    if (!texture.isValid() || texture.backend() != this->backend()) {
        return;
    }
    fResourceProvider->deleteBackendTexture(texture);
}

void Recorder::addFinishInfo(const InsertFinishInfo& info) {
    if (info.fFinishedProc) {
        sk_sp<RefCntedCallback> callback =
                RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
        fFinishedProcs.push_back(std::move(callback));
    }
}

void Recorder::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    // We don't want to free the Uniform/TextureDataCaches or the Draw/UploadBufferManagers since
    // all their resources need to be held on to until a Recording is snapped. And once snapped, all
    // their held resources are released. The StrikeCache and TextBlobCache don't hold onto any Gpu
    // resources.

    // The AtlasProvider gives out refs to TextureProxies so it should be safe to clear its pool
    // in the middle of Recording since those using the previous TextureProxies will have refs on
    // them.
    fAtlasProvider->clearTexturePool();

    fResourceProvider->freeGpuResources();
}

void Recorder::performDeferredCleanup(std::chrono::milliseconds msNotUsed) {
    ASSERT_SINGLE_OWNER

    auto purgeTime = skgpu::StdSteadyClock::now() - msNotUsed;
    fResourceProvider->purgeResourcesNotUsedSince(purgeTime);
}

size_t Recorder::currentBudgetedBytes() const {
    ASSERT_SINGLE_OWNER
    return fResourceProvider->getResourceCacheCurrentBudgetedBytes();
}

size_t Recorder::maxBudgetedBytes() const {
    ASSERT_SINGLE_OWNER
    return fResourceProvider->getResourceCacheLimit();
}

void Recorder::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    ASSERT_SINGLE_OWNER
    fResourceProvider->dumpMemoryStatistics(traceMemoryDump);
    // TODO: What is the graphite equivalent for the text blob cache and how do we print out its
    // used bytes here (see Ganesh implementation).
}

void RecorderPriv::addPendingRead(const TextureProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    fRecorder->fProxyReadCounts->increment(proxy);
}

void RecorderPriv::add(sk_sp<Task> task) {
    ASSERT_SINGLE_OWNER_PRIV
    fRecorder->fRootTaskList->add(std::move(task));
}

void RecorderPriv::flushTrackedDevices() {
    ASSERT_SINGLE_OWNER_PRIV

    // If this is the initial flushTrackedDevices() call, fFlushingTrackedDevicesIndex will be -1
    // so we start iterating at 0. We remember the starting device index to perform clean up only
    // when it was 0 to prevent modifying the underlying data structure while iterating over it.
    // However, when flushing one device it may register new devices as well as recursively call
    // flushTrackedDevices(). In that case, it picks up the next device after the current one that
    // triggered the recursive flush since all prior devices have been flushed are in progress
    // (and they should not be flushed while in an unfinished flush). When the control flow returns
    // to the outer flushTrackedDevices(), it will pick up with wherever the inner flush had ended.
    // TODO(b/330864257): Once paint data is extracted at draw time (so picture shaders are rendered
    // to images before a flush instead of inside a flush), we can simplify this and assert that
    // flushTrackedDevices() is not recursively called and that devices are not added or removed
    // while flushing.
    const int startingIndex = fRecorder->fFlushingDevicesIndex;
    while (fRecorder->fFlushingDevicesIndex < fRecorder->fTrackedDevices.size() - 1) {
        // Advance before calling flushPendingWorkToRecorder() so that any re-entrant clal to
        // flushTrackedDevices() will skip the current device.
        fRecorder->fFlushingDevicesIndex++;
        // Entries may be set to null from a call to deregisterDevice(), which will be cleaned up
        // along with any immutable or uniquely held Devices once everything is flushed.
        Device* device = fRecorder->fTrackedDevices[fRecorder->fFlushingDevicesIndex].get();
        if (device) {
            device->flushPendingWorkToRecorder(fRecorder);
        }
    }

    // Issue next upload flush token. This is only used by the atlasing code which
    // always uses this method. Calling in Device::flushPendingWorkToRecorder may
    // miss parent device flushes, increment too often, and lead to atlas corruption.
    this->tokenTracker()->issueFlushToken();

    if (startingIndex < 0) {
        // Initial call to flushTrackedDevices() so cleanup null/immutable devices and reset the
        // loop index.
        int i = 0;
        while (i < fRecorder->fTrackedDevices.size()) {
            Device* device = fRecorder->fTrackedDevices[i].get();
            if (!device || !device->recorder() || device->unique()) {
                if (device) {
                    device->abandonRecorder(); // Keep ~Device() happy
                }
                fRecorder->fTrackedDevices.removeShuffle(i);
                // Keep i as-is to process what was just shuffled to the ith index.
            } else {
                i++;
            }
        }

        fRecorder->fFlushingDevicesIndex = -1;
    }
}

sk_sp<TextureProxy> RecorderPriv::CreateCachedProxy(Recorder* recorder,
                                                    const SkBitmap& bitmap,
                                                    std::string_view label,
                                                    Mipmapped mipmapped) {
    SkASSERT(!bitmap.isNull());

    if (!recorder) {
        return nullptr;
    }
    return recorder->priv().proxyCache()->findOrCreateCachedProxy(recorder, bitmap, mipmapped,
                                                                  std::move(label));
}

size_t RecorderPriv::getResourceCacheLimit() const {
    return fRecorder->fResourceProvider->getResourceCacheLimit();
}

#if defined(GRAPHITE_TEST_UTILS)
bool RecorderPriv::deviceIsRegistered(Device* device) const {
    ASSERT_SINGLE_OWNER_PRIV
    for (const sk_sp<Device>& currentDevice : fRecorder->fTrackedDevices) {
        if (device == currentDevice.get()) {
            return true;
        }
    }
    return false;
}

// used by the Context that created this Recorder to set a back pointer
void RecorderPriv::setContext(Context* context) {
    fRecorder->fContext = context;
}
#endif


} // namespace skgpu::graphite

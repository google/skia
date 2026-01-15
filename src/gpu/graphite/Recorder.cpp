/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/graphite/Recorder.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCPURecorder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/Token.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ScratchResourceManager.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/TaskList.h"
#include "src/gpu/graphite/task/UploadTask.h"
#include "src/image/SkImage_Base.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

#include <algorithm>
#include <atomic>
#include <functional>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#if defined(GPU_TEST_UTILS)
#include "src/gpu/graphite/RecorderOptionsPriv.h"
#endif

enum SkColorType : int;

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())
#define ASSERT_SINGLE_OWNER_PRIV SKGPU_ASSERT_SINGLE_OWNER(fRecorder->singleOwner())

/*
 * The default image provider doesn't perform any conversion so, by default, Graphite won't
 * draw any non-Graphite-backed images.
 */
class DefaultImageProvider final : public ImageProvider {
public:
    static sk_sp<DefaultImageProvider> Make() { return sk_sp(new DefaultImageProvider); }

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

namespace {

uint32_t next_id() {
    static std::atomic<uint32_t> nextID{SK_InvalidGenID + 1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}

} // anonymous namespace

Recorder::Recorder(sk_sp<SharedContext> sharedContext,
                   const RecorderOptions& options,
                   const Context* context)
        : fSharedContext(std::move(sharedContext))
        , fRuntimeEffectDict(sk_make_sp<RuntimeEffectDictionary>())
        , fRootTaskList(new TaskList)
        , fRootUploads(new UploadList)
        , fFloatStorageManager(sk_make_sp<FloatStorageManager>())
        , fProxyReadCounts(new ProxyReadCountMap)
        , fUniqueID(next_id())
        , fRequireOrderedRecordings(options.fRequireOrderedRecordings.has_value()
                                            ? *options.fRequireOrderedRecordings
                                            : fSharedContext->caps()->requireOrderedRecordings())
        , fAtlasProvider(std::make_unique<AtlasProvider>(this))
        , fTokenTracker(std::make_unique<TokenTracker>())
        , fStrikeCache(std::make_unique<sktext::gpu::StrikeCache>())
        , fTextBlobCache(std::make_unique<sktext::gpu::TextBlobRedrawCoordinator>(fUniqueID)) {
    fClientImageProvider = options.fImageProvider;
    if (!fClientImageProvider) {
        fClientImageProvider = DefaultImageProvider::Make();
    }

    if (context) {
        fOwnedResourceProvider = nullptr;
        fResourceProvider = context->priv().resourceProvider();
    } else {
        fOwnedResourceProvider = fSharedContext->makeResourceProvider(
                this->singleOwner(),
                fUniqueID,
                options.fGpuBudgetInBytes);
        fResourceProvider = fOwnedResourceProvider.get();
    }
    fUploadBufferManager = std::make_unique<UploadBufferManager>(fResourceProvider,
                                                                 fSharedContext->caps());

    DrawBufferManager::Options dbmOpts = {};
#if defined(GPU_TEST_UTILS)
    if (options.fRecorderOptionsPriv && options.fRecorderOptionsPriv->fDbmOptions.has_value()) {
        dbmOpts = *options.fRecorderOptionsPriv->fDbmOptions;
    }
#endif

    fDrawBufferManager = std::make_unique<DrawBufferManager>(fResourceProvider,
                                                             fSharedContext->caps(),
                                                             fUploadBufferManager.get(),
                                                             dbmOpts);

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

#if defined(GPU_TEST_UTILS)
    if (fContext) {
        fContext->priv().deregisterRecorder(this);
    }
#endif
}

BackendApi Recorder::backend() const { return fSharedContext->backend(); }

skcpu::Recorder* Recorder::cpuRecorder() {
    return skcpu::Recorder::TODO();
}

std::unique_ptr<Recording> Recorder::snap() {
    TRACE_EVENT0_ALWAYS("skia.gpu", TRACE_FUNC);
    ASSERT_SINGLE_OWNER

    if (fTargetProxyData) {
        // Normally devices are marked immutable when their owning Surface goes away, but the
        // deferred canvas+device do not have a surface so mimic that operation. Do this before
        // flushing all other tracked devices to avoid a redundant flush.
        fTargetProxyDevice->setImmutable();
        fTargetProxyDevice.reset();
        fTargetProxyCanvas.reset();
    }
    // Collect all pending tasks on the deferred recording canvas and any other tracked device.
    this->priv().flushTrackedDevices(SK_DUMP_TASKS_CODE("Recorder::Snap"));

    // The scratch resources only need to be tracked until prepareResources() is finished, so
    // Recorder doesn't hold a persistent manager and it can be deleted when snap() returns.
    ScratchResourceManager scratchManager{fResourceProvider, std::move(fProxyReadCounts)};
    std::unique_ptr<Recording> recording(new Recording(fNextRecordingID++,
                                                       fRequireOrderedRecordings ? fUniqueID
                                                                                 : SK_InvalidGenID,
                                                       std::move(fTargetProxyData),
                                                       std::move(fFinishedProcs)));
    // Allow the buffer managers to add any collected tasks for data transfer or initialization
    // before moving the root task list to the Recording.
    bool valid = fFloatStorageManager->finalize(fDrawBufferManager.get());
    valid &= fDrawBufferManager->transferToRecording(recording.get());

    // We create the Recording's full task list even if the DrawBufferManager failed because it is
    // a convenient way to ensure everything else is unmapped and reset for the next Recording.
    fUploadBufferManager->transferToRecording(recording.get());
    // Add one task for all root uploads before the rest of the rendering tasks might depend on them
    if (fRootUploads->size() > 0) {
        sk_sp<Task> uploadTask = UploadTask::Make(fRootUploads.get());

        // If we are dumping tasks, we want to be able to associate each task with the current flush
        // count, so each task gets a flushToken---just an int---to track this.
        SK_DUMP_TASKS_CODE(uploadTask->fFlushToken =
                this->priv().tokenTracker()->currentFlushToken();)

        recording->priv().taskList()->add(std::move(uploadTask));
        SkASSERT(fRootUploads->size() == 0); // Drained by the newly added task
    }
    recording->priv().taskList()->add(std::move(*fRootTaskList));
    SkASSERT(!fRootTaskList->hasTasks());

    SK_DUMP_TASKS_CODE(this->dumpTasks(recording->priv().taskList()));

    // In both the "task failed" case and the "everything is discarded" case, there's no work that
    // needs to be done in insertRecording(). However, we use nullptr as a failure signal, so
    // kDiscard will return a non-null Recording that has no tasks in it.
    valid &= recording->priv().prepareResources(fResourceProvider,
                                                &scratchManager,
                                                fRuntimeEffectDict);
    if (!valid) {
        recording = nullptr;
        fAtlasProvider->invalidateAtlases();
    }

    // Process the return queue at least once to keep it from growing too large, as otherwise
    // it's only processed during an explicit cleanup or a cache miss.
    fResourceProvider->forceProcessReturnedResources();

    // Remaining cleanup that must always happen regardless of success or failure
    fRuntimeEffectDict = sk_make_sp<RuntimeEffectDictionary>();
    fProxyReadCounts = std::make_unique<ProxyReadCountMap>();
    fFloatStorageManager = sk_make_sp<FloatStorageManager>();
    if (!fRequireOrderedRecordings) {
        fAtlasProvider->invalidateAtlases();
    }

    // For each KeyAndDataBuilder owned by the Recorder, check if the high watermark of data usage
    // over the lifetime snap is less than half of allocated capacity. If so, shrink the capacity.
    for (const std::unique_ptr<KeyAndDataBuilder>& keyDB : fKeyAndDataBuilders) {
        SkASSERT(keyDB);
        keyDB->first.tryShrinkCapacity();
        keyDB->second.tryShrinkCapacity();
    }
    return recording;
}

SkCanvas* Recorder::makeDeferredCanvas(const SkImageInfo& imageInfo,
                                       const TextureInfo& textureInfo) {
    if (fTargetProxyCanvas) {
        // Require snapping before requesting another canvas.
        SKGPU_LOG_W("Requested a new deferred canvas before snapping the previous one");
        return nullptr;
    }

    fTargetProxyData = std::make_unique<Recording::LazyProxyData>(
            this->priv().caps(), imageInfo.dimensions(), textureInfo);
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

SkCanvas* Recorder::makeCaptureCanvas(SkCanvas* canvas) {
    if (fSharedContext->captureManager()) {
        return fSharedContext->captureManager()->makeCaptureCanvas(canvas);
    }
    return nullptr;
}

void Recorder::createCaptureBreakpoint(SkSurface* surface) {
   if (fSharedContext->captureManager()) {
        fSharedContext->captureManager()->snapPicture(surface);
    }
}

void Recorder::registerDevice(sk_sp<Device> device) {
    ASSERT_SINGLE_OWNER

    SkASSERT(device);

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
}

int Recorder::maxTextureSize() const {
    return this->priv().caps()->maxTextureSize();
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
                                    int numLevels,
                                    GpuFinishedProc finishedProc,
                                    GpuFinishedContext finishedContext) {
    ASSERT_SINGLE_OWNER

    auto releaseHelper = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    if (!backendTex.isValid() || backendTex.backend() != this->backend()) {
        return false;
    }

    if (!srcData || numLevels <= 0) {
        return false;
    }

    // If the texture has MIP levels then we require that the full set is overwritten.
    int numExpectedLevels = 1;
    if (backendTex.info().mipmapped() == Mipmapped::kYes) {
        numExpectedLevels = SkMipmap::ComputeLevelCount(backendTex.dimensions()) + 1;
    }
    if (numLevels != numExpectedLevels) {
        return false;
    }

    SkColorType ct = srcData[0].colorType();

    if (!this->priv().caps()->areColorTypeAndTextureInfoCompatible(ct, backendTex.info())) {
        return false;
    }

    sk_sp<Texture> texture = this->priv().resourceProvider()->createWrappedTexture(backendTex, "");
    if (!texture) {
        return false;
    }
    texture->setReleaseCallback(std::move(releaseHelper));

    std::vector<MipLevel> mipLevels;
    mipLevels.resize(numLevels);

    for (int i = 0; i < numLevels; ++i) {
        SkASSERT(srcData[i].addr());
        SkASSERT(srcData[i].info().colorInfo() == srcData[0].info().colorInfo());

        mipLevels[i].fPixels = srcData[i].addr();
        mipLevels[i].fRowBytes = srcData[i].rowBytes();
    }

    sk_sp<TextureProxy> proxy = TextureProxy::Wrap(std::move(texture));

    // Src and dst colorInfo are the same
    const SkColorInfo& colorInfo = srcData[0].info().colorInfo();

    const SkIRect dimensions = SkIRect::MakeSize(backendTex.dimensions());
    UploadSource uploadSource = UploadSource::Make(
            this->priv().caps(), *proxy, colorInfo, colorInfo, mipLevels, dimensions);
    if (!uploadSource.isValid()) {
        SKGPU_LOG_E("Recorder::updateBackendTexture: Could not create UploadSource");
        return false;
    }

    // Attempt to update the texture directly on the host if possible.
    if (uploadSource.canUploadOnHost()) {
        return proxy->texture()->uploadDataOnHost(uploadSource, dimensions);
    }

    // Add UploadTask to Recorder
    UploadInstance upload = UploadInstance::Make(this,
                                                 std::move(proxy),
                                                 colorInfo,
                                                 colorInfo,
                                                 uploadSource,
                                                 dimensions,
                                                 std::make_unique<ImageUploadContext>());
    if (!upload.isValid()) {
        SKGPU_LOG_E("Recorder::updateBackendTexture: Could not create UploadInstance");
        return false;
    }
    sk_sp<Task> uploadTask = UploadTask::Make(std::move(upload));

    // Need to flush any pending work in case it depends on this texture
    this->priv().flushTrackedDevices(
        SK_DUMP_TASKS_CODE("Recorder::updateBackendTexture: Update Backend Texture"));

    this->priv().add(std::move(uploadTask));

    return true;
}

bool Recorder::updateCompressedBackendTexture(const BackendTexture& backendTex,
                                              const void* data,
                                              size_t dataSize,
                                              GpuFinishedProc finishedProc,
                                              GpuFinishedContext finishedContext) {
    ASSERT_SINGLE_OWNER

    auto releaseHelper = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    if (!backendTex.isValid() || backendTex.backend() != this->backend()) {
        return false;
    }

    if (!data) {
        return false;
    }

    sk_sp<Texture> texture = this->priv().resourceProvider()->createWrappedTexture(backendTex, "");
    if (!texture) {
        return false;
    }
    texture->setReleaseCallback(std::move(releaseHelper));

    sk_sp<TextureProxy> proxy = TextureProxy::Wrap(std::move(texture));

    UploadSource uploadSource =
            UploadSource::MakeCompressed(this->priv().caps(), *proxy, data, dataSize);
    if (!uploadSource.isValid()) {
        SKGPU_LOG_E("Recorder::updateBackendTexture: Could not create compressed UploadSource");
        return false;
    }

    // Attempt to update the texture directly on the host if possible.
    if (uploadSource.canUploadOnHost()) {
        return proxy->texture()->uploadDataOnHost(uploadSource,
                                                  SkIRect::MakeSize(proxy->dimensions()));
    }

    // Add UploadTask to Recorder
    UploadInstance upload = UploadInstance::MakeCompressed(this, std::move(proxy), uploadSource);
    if (!upload.isValid()) {
        SKGPU_LOG_E("Recorder::updateBackendTexture: Could not create compressed UploadInstance");
        return false;
    }
    sk_sp<Task> uploadTask = UploadTask::Make(std::move(upload));

    // Need to flush any pending work in case it depends on this texture
    this->priv().flushTrackedDevices(SK_DUMP_TASKS_CODE(
            "Recorder::updateCompressedBackendTexture Update Compressed Backend Texture"));

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

    // We don't want to free the Uniform or the Draw/UploadBufferManagers sinceall their resources
    // need to be held on to until a Recording is snapped. And once snapped, all their held
    // resources are released. The StrikeCache and TextBlobCache don't hold onto any Gpu resources.

    // Notify the atlas and resource provider to free any resources it can (does not include
    // resources that are locked due to pending work).
    fAtlasProvider->freeGpuResources();

    fResourceProvider->freeGpuResources();

    // This is technically not GPU memory, but there's no other place for the client to tell us to
    // clean this up, and without any cleanup it can grow unbounded.
    fStrikeCache->freeAll();
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

size_t Recorder::currentPurgeableBytes() const {
    ASSERT_SINGLE_OWNER
    return fResourceProvider->getResourceCacheCurrentPurgeableBytes();
}

size_t Recorder::maxBudgetedBytes() const {
    ASSERT_SINGLE_OWNER
    return fResourceProvider->getResourceCacheLimit();
}

void Recorder::setMaxBudgetedBytes(size_t bytes) {
    ASSERT_SINGLE_OWNER
    return fResourceProvider->setResourceCacheLimit(bytes);
}

void Recorder::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    ASSERT_SINGLE_OWNER
    fResourceProvider->dumpMemoryStatistics(traceMemoryDump);
    // TODO: What is the graphite equivalent for the text blob cache and how do we print out its
    // used bytes here (see Ganesh implementation).
}

#if defined(SK_DUMP_TASKS)
void Recorder::dumpTasks(TaskList* taskList) const {
    ASSERT_SINGLE_OWNER
    SkDebugf("\n=========== RECORDING %u ===========\n", fUniqueID);
    int taskIndex = 0;
    uint64_t lastToken = skgpu::Token::InvalidToken().value();
    taskList->visit([&](const Task* task, bool isLast) {
        if (!task) {
            return;
        }
        uint64_t currToken = task->fFlushToken.value();
        if (currToken != lastToken) {
            SkDebugf("**** FLUSH TOKEN %llu %s ****\n", currToken, fFlushSources[currToken - 1]);
            lastToken = currToken;
        }
        task->dump(taskIndex++, "");
    });
    SkDebugf("--------------- END ---------------\n");
}
#endif

sk_sp<RuntimeEffectDictionary> RecorderPriv::runtimeEffectDictionary() {
    return fRecorder->fRuntimeEffectDict;
}

void RecorderPriv::addPendingRead(const TextureProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    fRecorder->fProxyReadCounts->increment(proxy);
}

void RecorderPriv::add(sk_sp<Task> task) {
    ASSERT_SINGLE_OWNER_PRIV
    // Associate each task with current flush count.
    SK_DUMP_TASKS_CODE(task->fFlushToken = fRecorder->fTokenTracker->nextFlushToken();)
    fRecorder->fRootTaskList->add(std::move(task));
}

void RecorderPriv::flushTrackedDevices(const TextureProxy* dependency) {
    // This version of flushTrackedDevices() must be re-entrant because it is entirely possible for
    // client-owned surfaces to read and write to each other, where this will be called with
    // different textures for `dependency`. The recursion stops once the encountered surfaces have
    // snapped remaining pending work from their DrawContext. But because we might recurse, we do
    // not perform any cleanup of the fTrackedDevices list. That is deferred until snap() time.

    for (int i = 0; i < fRecorder->fTrackedDevices.size(); ++i) {
        // Entries may be set to null from a call to deregisterDevice(), which will be cleaned up
        // along with any immutable or uniquely held Devices once everything is snapped.
        Device* device = fRecorder->fTrackedDevices[i].get();
        if (device && device->hasPendingReads(dependency)) {
            device->flushPendingWork(/*drawContext=*/nullptr);
        }
    }

    // TODO(michaelludwig): These flushes are currently only triggered for client-owned surfaces
    // drawn into other surfaces. This function could be used to flush a more targeted set of
    // devices when an atlas fills up; in that case we could increment the flush token as part of
    // that work. As-is, we don't increment the flush token because there could be a tracked atlas
    // that depended on the atlas's texture state that did *not* depend on `dependency` so it still
    // requires the atlas to be using the old flush token. The surfaces that were flushed here could
    // advance to a new token but the token tracking isn't that precise. This all may be moot
    // anyways if we can successfully switch to a rolling atlas page system.
}

void RecorderPriv::flushTrackedDevices(SK_DUMP_TASKS_CODE(const char* flushSource)) {
    ASSERT_SINGLE_OWNER_PRIV
    SkASSERT(!fRecorder->fIsFlushingTrackedDevices);
    SkDEBUGCODE(fRecorder->fIsFlushingTrackedDevices = true);

    for (int i = 0; i < fRecorder->fTrackedDevices.size(); ++i) {
        // Entries may be set to null from a call to deregisterDevice(), which will be cleaned up
        // along with any immutable or uniquely held Devices once everything is flushed.
        Device* device = fRecorder->fTrackedDevices[i].get();
        if (device) {
            device->flushPendingWork(/*drawContext=*/nullptr);
        }
    }

    // Issue next upload flush token. This is only used by the atlasing code which
    // always uses this method. Calling in Device::flushPendingWorkToRecorder may
    // miss parent device flushes, increment too often, and lead to atlas corruption.
    this->tokenTracker()->issueFlushToken();
#if defined(SK_DUMP_TASKS)
    fRecorder->fFlushSources.push_back(flushSource);
    SkASSERT(this->tokenTracker()->currentFlushToken().value() ==
             static_cast<uint64_t>(fRecorder->fFlushSources.size()));
#endif

    // This version of flushTrackedDevices() is not re-entrant, so it *does* perform the final
    // cleanup on the fTrackedDevices.
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

    SkDEBUGCODE(fRecorder->fIsFlushingTrackedDevices = false);
}

std::unique_ptr<KeyAndDataBuilder> RecorderPriv::popOrCreateKeyAndDataBuilder() {
    if (!fRecorder->fKeyAndDataBuilders.empty()) {
        std::unique_ptr<KeyAndDataBuilder> keyDB = std::move(fRecorder->fKeyAndDataBuilders.back());
        fRecorder->fKeyAndDataBuilders.pop_back();
        return keyDB;
    }

    const bool useStorageBuffers = this->caps()->storageBufferSupport();
    const auto& bindingReq = this->caps()->resourceBindingRequirements();
    auto gathererLayout = useStorageBuffers ? bindingReq.fStorageBufferLayout
                                            : bindingReq.fUniformBufferLayout;

    return std::make_unique<KeyAndDataBuilder>(
        PipelineDataGatherer(gathererLayout),
        PaintParamsKeyBuilder(this->shaderCodeDictionary()));
    }

void RecorderPriv::pushKeyAndDataBuilder(std::unique_ptr<KeyAndDataBuilder> keyDB) {
    SkASSERT(keyDB);

    if (fRecorder->fKeyAndDataBuilders.size() < Recorder::kMaxKeyAndDataBuilders) {
        fRecorder->fKeyAndDataBuilders.push_back(std::move(keyDB));
        return;
    }
    // If no empty slot was found, the "keyDB" unique_ptr goes out of scope here.
}

sk_sp<TextureProxy> RecorderPriv::CreateCachedProxy(Recorder* recorder,
                                                    const SkBitmap& bitmap,
                                                    std::string_view label) {
    SkASSERT(!bitmap.isNull());

    if (!recorder) {
        return nullptr;
    }
    return recorder->priv().proxyCache()->findOrCreateCachedProxy(recorder, bitmap, label);
}

size_t RecorderPriv::getResourceCacheLimit() const {
    return fRecorder->fResourceProvider->getResourceCacheLimit();
}

#if defined(GPU_TEST_UTILS)
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

void RecorderPriv::issueFlushToken() {
    fRecorder->fTokenTracker->issueFlushToken();
}
#endif

} // namespace skgpu::graphite

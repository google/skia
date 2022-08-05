/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recorder.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recording.h"
#include "src/core/SkPipelineData.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawBufferManager.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/TaskGraph.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/text/AtlasManager.h"
#include "src/image/SkImage_Base.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())

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
                                Mipmapped mipmapped) override {
        SkASSERT(!as_IB(image)->isGraphiteBacked());

        return nullptr;
    }

private:
    DefaultImageProvider() {}
};

/**************************************************************************************************/
RecorderOptions::~RecorderOptions() = default;

/**************************************************************************************************/
static int32_t next_id() {
    static std::atomic<int32_t> nextID{1};
    int32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}

Recorder::Recorder(sk_sp<Gpu> gpu,
                   sk_sp<GlobalCache> globalCache,
                   const RecorderOptions& options)
        : fGpu(std::move(gpu))
        , fGraph(new TaskGraph)
        , fUniformDataCache(new UniformDataCache)
        , fTextureDataCache(new TextureDataCache)
        , fRecorderID(next_id())
        , fAtlasManager(std::make_unique<AtlasManager>(this))
        , fTokenTracker(std::make_unique<TokenTracker>())
        , fStrikeCache(std::make_unique<sktext::gpu::StrikeCache>())
        , fTextBlobCache(std::make_unique<sktext::gpu::TextBlobRedrawCoordinator>(fRecorderID)) {

    fClientImageProvider = options.fImageProvider;
    if (!fClientImageProvider) {
        fClientImageProvider = DefaultImageProvider::Make();
    }

    fResourceProvider = fGpu->makeResourceProvider(std::move(globalCache), this->singleOwner());
    fDrawBufferManager.reset(new DrawBufferManager(fResourceProvider.get(),
                                                   fGpu->caps()->requiredUniformBufferAlignment()));
    fUploadBufferManager.reset(new UploadBufferManager(fResourceProvider.get()));
    SkASSERT(fResourceProvider);
}

Recorder::~Recorder() {
    ASSERT_SINGLE_OWNER
    for (auto& device : fTrackedDevices) {
        device->abandonRecorder();
    }

    // TODO: needed?
    fStrikeCache->freeAll();
}

std::unique_ptr<Recording> Recorder::snap() {
    ASSERT_SINGLE_OWNER
    for (auto& device : fTrackedDevices) {
        device->flushPendingWorkToRecorder();
    }

    // TODO: fulfill all promise images in the TextureDataCache here
    // TODO: create all the samplers needed in the TextureDataCache here

    if (!fGraph->prepareResources(fResourceProvider.get())) {
        // Leaving 'fTrackedDevices' alone since they were flushed earlier and could still be
        // attached to extant SkSurfaces.
        size_t requiredAlignment = fGpu->caps()->requiredUniformBufferAlignment();
        fDrawBufferManager.reset(new DrawBufferManager(fResourceProvider.get(), requiredAlignment));
        fTextureDataCache = std::make_unique<TextureDataCache>();
        // We leave the UniformDataCache alone
        fGraph->reset();
        fResourceProvider->resetAfterSnap();
        return nullptr;
    }

    std::unique_ptr<Recording> recording(new Recording(std::move(fGraph)));
    fDrawBufferManager->transferToRecording(recording.get());
    fUploadBufferManager->transferToRecording(recording.get());

    fGraph = std::make_unique<TaskGraph>();
    fResourceProvider->resetAfterSnap();
    fTextureDataCache = std::make_unique<TextureDataCache>();
    return recording;
}

void Recorder::registerDevice(Device* device) {
    ASSERT_SINGLE_OWNER
    fTrackedDevices.push_back(device);
}

void Recorder::deregisterDevice(const Device* device) {
    ASSERT_SINGLE_OWNER
    for (auto it = fTrackedDevices.begin(); it != fTrackedDevices.end(); it++) {
        if (*it == device) {
            fTrackedDevices.erase(it);
            return;
        }
    }
}

#if GR_TEST_UTILS
bool Recorder::deviceIsRegistered(Device* device) {
    ASSERT_SINGLE_OWNER
    for (auto& currentDevice : fTrackedDevices) {
        if (device == currentDevice) {
            return true;
        }
    }
    return false;
}
#endif

} // namespace skgpu::graphite

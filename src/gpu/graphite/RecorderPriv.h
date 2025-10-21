/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecorderPriv_DEFINED
#define skgpu_graphite_RecorderPriv_DEFINED

#include "include/core/SkRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/graphite/DebugUtils.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

class SkBitmap;

namespace skgpu {
class TokenTracker;
enum class Protected : bool;
}

namespace sktext::gpu {
class StrikeCache;
class TextBlobRedrawCoordinator;
}

namespace skgpu::graphite {

class AtlasProvider;
class Caps;
class Context;
class Device;
class DrawBufferManager;
class ProxyCache;
class RendererProvider;
class ResourceCache;
class RuntimeEffectDictionary;
class ShaderCodeDictionary;
class Task;
class TextureProxy;
class UploadBufferManager;
class UploadList;

class RecorderPriv {
public:
    void add(sk_sp<Task>);
    // Flush *all* tracked devices created by this Recorder
    void flushTrackedDevices(SK_DUMP_TASKS_CODE(const char* flushSource));
    // Flush tracked devices that have pending reads from `dependency`.
    void flushTrackedDevices(const TextureProxy* dependency);

    std::unique_ptr<KeyAndDataBuilder> popOrCreateKeyAndDataBuilder();
    void pushKeyAndDataBuilder(std::unique_ptr<KeyAndDataBuilder> keyDB);

    const Caps* caps() const { return fRecorder->fSharedContext->caps(); }

    ResourceProvider* resourceProvider() { return fRecorder->fResourceProvider; }

    sk_sp<RuntimeEffectDictionary> runtimeEffectDictionary();

    const ShaderCodeDictionary* shaderCodeDictionary() const {
        return fRecorder->fSharedContext->shaderCodeDictionary();
    }
    ShaderCodeDictionary* shaderCodeDictionary() {
        return fRecorder->fSharedContext->shaderCodeDictionary();
    }

    const RendererProvider* rendererProvider() const {
        return fRecorder->fSharedContext->rendererProvider();
    }

    Protected isProtected() const {
        return fRecorder->fSharedContext->isProtected();
    }

    UploadList* rootUploadList() { return fRecorder->fRootUploads.get(); }
    DrawBufferManager* drawBufferManager() { return fRecorder->fDrawBufferManager.get(); }
    UploadBufferManager* uploadBufferManager() { return fRecorder->fUploadBufferManager.get(); }
    FloatStorageManager* floatStorageManager() { return fRecorder->fFloatStorageManager.get(); }
    sk_sp<FloatStorageManager> refFloatStorageManager() { return fRecorder->fFloatStorageManager; }

    AtlasProvider* atlasProvider() { return fRecorder->fAtlasProvider.get(); }
    TokenTracker* tokenTracker() { return fRecorder->fTokenTracker.get(); }
    sktext::gpu::StrikeCache* strikeCache() { return fRecorder->fStrikeCache.get(); }
    sktext::gpu::TextBlobRedrawCoordinator* textBlobCache() {
        return fRecorder->fTextBlobCache.get();
    }
    ProxyCache* proxyCache() { return this->resourceProvider()->proxyCache(); }

    // NOTE: Temporary access for DrawTask to manipulate pending read counts.
    void addPendingRead(const TextureProxy*);

    static sk_sp<TextureProxy> CreateCachedProxy(Recorder*,
                                                 const SkBitmap&,
                                                 std::string_view label);

    uint32_t uniqueID() const { return fRecorder->fUniqueID; }

#if defined(SK_DEBUG)
    uint32_t nextRecordingID() const { return fRecorder->fNextRecordingID; }
#endif

    size_t getResourceCacheLimit() const;

#if defined(GPU_TEST_UTILS)
    bool deviceIsRegistered(Device*) const;
    ResourceCache* resourceCache() { return fRecorder->fResourceProvider->resourceCache(); }
    SharedContext* sharedContext() { return fRecorder->fSharedContext.get(); }
    // used by the Context that created this Recorder to set a back pointer
    void setContext(Context*);
    Context* context() { return fRecorder->fContext; }
    void issueFlushToken();
#endif

private:
    explicit RecorderPriv(Recorder* recorder) : fRecorder(recorder) {}
    RecorderPriv& operator=(const RecorderPriv&) = delete;

    // No taking addresses of this type.
    const RecorderPriv* operator&() const = delete;
    RecorderPriv* operator&() = delete;

    Recorder* fRecorder;

    friend class Recorder;  // to construct/copy this type.
};

inline RecorderPriv Recorder::priv() {
    return RecorderPriv(this);
}

inline const RecorderPriv Recorder::priv() const {  // NOLINT(readability-const-return-type)
    return RecorderPriv(const_cast<Recorder*>(this));
}

inline Recorder* AsGraphiteRecorder(SkRecorder* recorder) {
    if (!recorder) {
        return nullptr;
    }
    if (recorder->type() != SkRecorder::Type::kGraphite) {
        return nullptr;
    }
    return static_cast<Recorder*>(recorder);
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_RecorderPriv_DEFINED

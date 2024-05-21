/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecorderPriv_DEFINED
#define skgpu_graphite_RecorderPriv_DEFINED

#include <functional>

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

class SkBitmap;
class SkImage;

namespace skgpu::graphite {

class ShaderCodeDictionary;
class TextureProxy;

class RecorderPriv {
public:
    void add(sk_sp<Task>);
    void flushTrackedDevices();

    const Caps* caps() const { return fRecorder->fSharedContext->caps(); }

    ResourceProvider* resourceProvider() { return fRecorder->fResourceProvider; }

    const RuntimeEffectDictionary* runtimeEffectDictionary() const {
        return fRecorder->fRuntimeEffectDict.get();
    }
    RuntimeEffectDictionary* runtimeEffectDictionary() {
        return fRecorder->fRuntimeEffectDict.get();
    }
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

    UniformDataCache* uniformDataCache() { return fRecorder->fUniformDataCache.get(); }
    TextureDataCache* textureDataCache() { return fRecorder->fTextureDataCache.get(); }
    DrawBufferManager* drawBufferManager() { return fRecorder->fDrawBufferManager.get(); }
    UploadBufferManager* uploadBufferManager() { return fRecorder->fUploadBufferManager.get(); }

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
                                                 std::string_view label,
                                                 Mipmapped = skgpu::Mipmapped::kNo);

    uint32_t uniqueID() const { return fRecorder->fUniqueID; }

#if defined(SK_DEBUG)
    uint32_t nextRecordingID() const { return fRecorder->fNextRecordingID; }
#endif

    size_t getResourceCacheLimit() const;

#if defined(GRAPHITE_TEST_UTILS)
    bool deviceIsRegistered(Device*) const;
    ResourceCache* resourceCache() { return fRecorder->fResourceProvider->resourceCache(); }
    SharedContext* sharedContext() { return fRecorder->fSharedContext.get(); }
    // used by the Context that created this Recorder to set a back pointer
    void setContext(Context*);
    Context* context() { return fRecorder->fContext; }
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

} // namespace skgpu::graphite

#endif // skgpu_graphite_RecorderPriv_DEFINED

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
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

class ShaderCodeDictionary;
class TextureProxy;

class RecorderPriv {
public:
    void add(sk_sp<Task>);
    void flushTrackedDevices();

    const Caps* caps() const { return fRecorder->fSharedContext->caps(); }

    ResourceProvider* resourceProvider() { return fRecorder->fResourceProvider.get(); }

    const SkRuntimeEffectDictionary* runtimeEffectDictionary() const {
        return fRecorder->fRuntimeEffectDict.get();
    }
    SkRuntimeEffectDictionary* runtimeEffectDictionary() {
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

    UniformDataCache* uniformDataCache() { return fRecorder->fUniformDataCache.get(); }
    TextureDataCache* textureDataCache() { return fRecorder->fTextureDataCache.get(); }
    DrawBufferManager* drawBufferManager() { return fRecorder->fDrawBufferManager.get(); }
    UploadBufferManager* uploadBufferManager() { return fRecorder->fUploadBufferManager.get(); }

    AtlasManager* atlasManager() { return fRecorder->fAtlasManager.get(); }
    TokenTracker* tokenTracker() { return fRecorder->fTokenTracker.get(); }
    sktext::gpu::StrikeCache* strikeCache() { return fRecorder->fStrikeCache.get(); }
    sktext::gpu::TextBlobRedrawCoordinator* textBlobCache() {
        return fRecorder->fTextBlobCache.get();
    }

    // Inserts a texture to buffer transfer task, used by asyncReadPixels methods in Context
    struct PixelTransferResult {
        using ConversionFn = void(void* dst, const void* mappedBuffer);
        // If null then the transfer could not be performed. Otherwise this buffer will contain
        // the pixel data when the transfer is complete.
        sk_sp<Buffer> fTransferBuffer;
        // If this is null then the transfer buffer will contain the data in the requested
        // color type. Otherwise, when the transfer is done this must be called to convert
        // from the transfer buffer's color type to the requested color type.
        std::function<ConversionFn> fPixelConverter;
    };
    PixelTransferResult transferPixels(const TextureProxy*,
                                       const SkImageInfo& srcImageInfo,
                                       const SkColorInfo& dstColorInfo,
                                       const SkIRect& srcRect);

#if GRAPHITE_TEST_UTILS
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

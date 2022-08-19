/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecorderPriv_DEFINED
#define skgpu_graphite_RecorderPriv_DEFINED

#include "include/gpu/graphite/Recorder.h"
#include "src/text/gpu/SDFTControl.h"

namespace skgpu {
enum class MaskFormat : int;
}

namespace skgpu::graphite {

class TextureProxy;

class RecorderPriv {
public:
    void add(sk_sp<Task>);

    ResourceProvider* resourceProvider() const;
    SkRuntimeEffectDictionary* runtimeEffectDictionary() const;
    UniformDataCache* uniformDataCache() const;
    TextureDataCache* textureDataCache() const;
    DrawBufferManager* drawBufferManager() const;
    UploadBufferManager* uploadBufferManager() const;
    AtlasManager* atlasManager();
    TokenTracker* tokenTracker();
    sktext::gpu::StrikeCache* strikeCache();
    sktext::gpu::TextBlobRedrawCoordinator* textBlobCache();
    sktext::gpu::SDFTControl getSDFTControl(bool useSDFTForSmallText) const;
    const Caps* caps() const;
    sk_sp<const Caps> refCaps() const;

    const sk_sp<skgpu::graphite::TextureProxy>* getAtlasProxies(skgpu::MaskFormat maskformat,
                                                                unsigned int* numActiveProxies);

    void flushTrackedDevices();

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

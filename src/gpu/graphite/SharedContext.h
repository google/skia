/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SharedContext_DEFINED
#define skgpu_graphite_SharedContext_DEFINED

#include <memory>
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"

#include "include/gpu/graphite/GraphiteTypes.h"
#include "src/capture/SkCaptureManager.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/PipelineManager.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

class SkCaptureManager;
class SkExecutor;

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {

class BackendTexture;
class Caps;
class CommandBuffer;
class Context;
class RendererProvider;
class ResourceProvider;
class TextureInfo;

class SharedContext : public SkRefCnt {
public:
    ~SharedContext() override;

    /**
     * Gets the capabilities of the draw target.
     */
    const Caps* caps() const { return fCaps.get(); }

    BackendApi backend() const { return fBackend; }
    Protected isProtected() const;

    GlobalCache* globalCache() { return &fGlobalCache; }
    const GlobalCache* globalCache() const { return &fGlobalCache; }

    PipelineManager* pipelineManager() { return &fPipelineManager; }

    const RendererProvider* rendererProvider() const { return fRendererProvider.get(); }

    ShaderCodeDictionary* shaderCodeDictionary() { return &fShaderDictionary; }
    const ShaderCodeDictionary* shaderCodeDictionary() const { return &fShaderDictionary; }

    virtual std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                                   uint32_t recorderID,
                                                                   size_t resourceBudget) = 0;

    // Called by Context::isContextLost(). Returns true if the backend-specific SharedContext has
    // gotten into an unrecoverable, lost state.
    virtual bool isDeviceLost() const { return false; }

    virtual void deviceTick(Context*) {}

    SkCaptureManager* captureManager() { return fCaptureManager.get(); }

protected:
    SharedContext(std::unique_ptr<const Caps>,
                  BackendApi,
                  SkExecutor* executor,
                  SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects);

private:
    friend class Context; // for setRendererProvider() and setCaptureManager()

    // Must be created out-of-band to allow RenderSteps to use a QueueManager.
    void setRendererProvider(std::unique_ptr<RendererProvider> rendererProvider);

    void setCaptureManager(sk_sp<SkCaptureManager> captureManager);

    std::unique_ptr<const Caps> fCaps; // Provided by backend subclass

    BackendApi fBackend;
    GlobalCache fGlobalCache;
    PipelineManager fPipelineManager;
    std::unique_ptr<RendererProvider> fRendererProvider;
    ShaderCodeDictionary fShaderDictionary;
    sk_sp<SkCaptureManager> fCaptureManager;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_SharedContext_DEFINED

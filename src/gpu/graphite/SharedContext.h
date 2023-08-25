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
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {

class BackendTexture;
class Caps;
class CommandBuffer;
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
    Protected isProtected() const { return fProtected; }

    GlobalCache* globalCache() { return &fGlobalCache; }
    const GlobalCache* globalCache() const { return &fGlobalCache; }

    const RendererProvider* rendererProvider() const { return fRendererProvider.get(); }

    ShaderCodeDictionary* shaderCodeDictionary() { return &fShaderDictionary; }
    const ShaderCodeDictionary* shaderCodeDictionary() const { return &fShaderDictionary; }

    virtual std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                                   uint32_t recorderID,
                                                                   size_t resourceBudget) = 0;

protected:
    SharedContext(std::unique_ptr<const Caps>, BackendApi);

private:
    friend class Context; // for setRendererProvider()

    // Must be created out-of-band to allow RenderSteps to use a QueueManager.
    void setRendererProvider(std::unique_ptr<RendererProvider> rendererProvider);

    std::unique_ptr<const Caps> fCaps; // Provided by backend subclass

    BackendApi fBackend;
    Protected fProtected;
    GlobalCache fGlobalCache;
    std::unique_ptr<RendererProvider> fRendererProvider;
    ShaderCodeDictionary fShaderDictionary;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_SharedContext_DEFINED

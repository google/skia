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
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/RendererProvider.h"

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {

class BackendTexture;
class Caps;
class CommandBuffer;
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

    GlobalCache* globalCache() { return &fGlobalCache; }
    const GlobalCache* globalCache() const { return &fGlobalCache; }

    const RendererProvider* rendererProvider() const { return &fRendererProvider; }

    SkShaderCodeDictionary* shaderCodeDictionary() { return &fShaderDictionary; }
    const SkShaderCodeDictionary* shaderCodeDictionary() const { return &fShaderDictionary; }

    virtual std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*) = 0;

protected:
    SharedContext(std::unique_ptr<const Caps>, BackendApi);

private:
    std::unique_ptr<const Caps> fCaps;
    BackendApi fBackend;
    GlobalCache fGlobalCache;
    RendererProvider fRendererProvider;
    SkShaderCodeDictionary fShaderDictionary;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_SharedContext_DEFINED

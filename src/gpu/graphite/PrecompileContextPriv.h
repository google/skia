/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileContextPriv_DEFINED
#define skgpu_graphite_PrecompileContextPriv_DEFINED

#include "include/gpu/graphite/PrecompileContext.h"
#include "src/gpu/graphite/SharedContext.h"

class Caps;

namespace skgpu::graphite {

/**
 * Class that adds methods to PrecompileContext that are only intended for use internal to Skia.
 * This class is purely a privileged window into PrecompileContext. It should never have additional
 * data members or virtual methods.
 */
class PrecompileContextPriv {
public:
    const Caps* caps() const { return fPrecompileContext->fSharedContext->caps(); }
    const ShaderCodeDictionary* shaderCodeDictionary() const {
        return fPrecompileContext->fSharedContext->shaderCodeDictionary();
    }
    ShaderCodeDictionary* shaderCodeDictionary() {
        return fPrecompileContext->fSharedContext->shaderCodeDictionary();
    }
    const RendererProvider* rendererProvider() const {
        return fPrecompileContext->fSharedContext->rendererProvider();
    }
    SharedContext* sharedContext() {
        return fPrecompileContext->fSharedContext.get();
    }
    ResourceProvider* resourceProvider() {
        return fPrecompileContext->fResourceProvider.get();
    }
#if defined(GPU_TEST_UTILS)
    GlobalCache* globalCache() {
        return fPrecompileContext->fSharedContext->globalCache();
    }
#endif

private:
    friend class PrecompileContext; // to construct/copy this type.

    explicit PrecompileContextPriv(PrecompileContext* precompileContext)
            : fPrecompileContext(precompileContext) {}

    PrecompileContextPriv& operator=(const PrecompileContextPriv&) = delete;

    // No taking addresses of this type.
    const PrecompileContextPriv* operator&() const;
    PrecompileContextPriv *operator&();

    PrecompileContext* fPrecompileContext;
};

inline PrecompileContextPriv PrecompileContext::priv() { return PrecompileContextPriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const PrecompileContextPriv PrecompileContext::priv() const {
    return PrecompileContextPriv(const_cast<PrecompileContext *>(this));
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileContextPriv_DEFINED

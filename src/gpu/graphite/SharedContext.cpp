/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SharedContext.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

SharedContext::SharedContext(std::unique_ptr<const Caps> caps, BackendApi backend)
    : fCaps(std::move(caps))
    , fBackend(backend)
    , fGlobalCache()
    , fShaderDictionary() {}

SharedContext::~SharedContext() {
    // TODO: add disconnect?

    // TODO: destroyResources instead?
}

Protected SharedContext::isProtected() const { return Protected(fCaps->protectedSupport()); }

void SharedContext::setRendererProvider(std::unique_ptr<RendererProvider> rendererProvider) {
    // Should only be called once and be non-null
    SkASSERT(rendererProvider && !fRendererProvider);
    fRendererProvider = std::move(rendererProvider);
}

} // namespace skgpu::graphite

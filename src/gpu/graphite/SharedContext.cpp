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
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

SharedContext::SharedContext(sk_sp<const Caps> caps, BackendApi backend)
    : fCaps(std::move(caps))
    , fBackend(backend)
    , fGlobalCache()
    , fShaderDictionary() {}

SharedContext::~SharedContext() {
    // TODO: add disconnect?

    // TODO: destroyResources instead?
}

sk_sp<const Caps> SharedContext::refCaps() const {
    return fCaps;
}

} // namespace skgpu::graphite

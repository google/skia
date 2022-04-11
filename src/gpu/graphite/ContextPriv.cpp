/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ContextPriv.h"

#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Gpu.h"

namespace skgpu::graphite {

Gpu* ContextPriv::gpu() {
    return fContext->fGpu.get();
}

const Gpu* ContextPriv::gpu() const {
    return fContext->fGpu.get();
}

SkShaderCodeDictionary* ContextPriv::shaderCodeDictionary() {
    return fContext->fGlobalCache->shaderCodeDictionary();
}

} // namespace skgpu::graphite

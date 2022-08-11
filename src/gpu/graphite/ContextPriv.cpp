/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ContextPriv.h"

#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

#if GRAPHITE_TEST_UTILS
const Caps* ContextPriv::caps() const {
    return fContext->fSharedContext->caps();
}
#endif

SkShaderCodeDictionary* ContextPriv::shaderCodeDictionary() {
    return fContext->fGlobalCache->shaderCodeDictionary();
}

} // namespace skgpu::graphite

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
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/sksl/SkSLCompiler.h"

namespace skgpu::graphite {

SharedContext::SharedContext(sk_sp<const Caps> caps)
    : fCaps(std::move(caps)) {
}

SharedContext::~SharedContext() {
    // TODO: add disconnect?

    // TODO: destroyResources instead?
}

void SharedContext::initCompiler() {
    fCompiler = std::make_unique<SkSL::Compiler>(fCaps->shaderCaps());
}

sk_sp<const Caps> SharedContext::refCaps() const {
    return fCaps;
}

BackendTexture SharedContext::createBackendTexture(SkISize dimensions, const TextureInfo& info) {
    if (dimensions.isEmpty() || dimensions.width()  > this->caps()->maxTextureSize() ||
                                dimensions.height() > this->caps()->maxTextureSize()) {
        return {};
    }

    return this->onCreateBackendTexture(dimensions, info);
}

void SharedContext::deleteBackendTexture(BackendTexture& texture) {
    this->onDeleteBackendTexture(texture);
    // Invalidate the texture;
    texture = BackendTexture();
}


} // namespace skgpu::graphite

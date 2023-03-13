/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Texture.h"

#include "src/gpu/MutableTextureStateRef.h"
#include "src/gpu/RefCntedCallback.h"

namespace skgpu::graphite {

Texture::Texture(const SharedContext* sharedContext,
                 SkISize dimensions,
                 const TextureInfo& info,
                 sk_sp<MutableTextureStateRef> mutableState,
                 Ownership ownership,
                 skgpu::Budgeted budgeted)
        : Resource(sharedContext, ownership, budgeted)
        , fDimensions(dimensions)
        , fInfo(info)
        , fMutableState(std::move(mutableState)) {}

Texture::~Texture() {}

void Texture::setReleaseCallback(sk_sp<RefCntedCallback> releaseCallback) {
    fReleaseCallback = std::move(releaseCallback);
}

MutableTextureStateRef* Texture::mutableState() const { return fMutableState.get(); }

} // namespace skgpu::graphite

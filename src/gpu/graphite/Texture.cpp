/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Texture.h"

#include "src/gpu/MutableTextureStateRef.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

// TODO: Make this computed size more generic to handle compressed textures
size_t compute_size(const SharedContext* sharedContext,
                    SkISize dimensions,
                    const TextureInfo& info) {
    // TODO: Should we make sure the backends return zero here if the TextureInfo is for a
    // memoryless texture?
    size_t bytesPerPixel = sharedContext->caps()->bytesPerPixel(info);

    size_t colorSize = (size_t)dimensions.width() * dimensions.height() * bytesPerPixel;

    size_t finalSize = colorSize * info.numSamples();

    if (info.mipmapped() == Mipmapped::kYes) {
        finalSize += colorSize/3;
    }
    return finalSize;
}

Texture::Texture(const SharedContext* sharedContext,
                 SkISize dimensions,
                 const TextureInfo& info,
                 sk_sp<MutableTextureStateRef> mutableState,
                 Ownership ownership,
                 skgpu::Budgeted budgeted)
        : Resource(sharedContext, ownership, budgeted, compute_size(sharedContext,
                                                                    dimensions,
                                                                    info))
        , fDimensions(dimensions)
        , fInfo(info)
        , fMutableState(std::move(mutableState)) {}

Texture::~Texture() {}

void Texture::setReleaseCallback(sk_sp<RefCntedCallback> releaseCallback) {
    fReleaseCallback = std::move(releaseCallback);
}

void Texture::invokeReleaseProc() {
    if (fReleaseCallback) {
        // Depending on the ref count of fReleaseCallback this may or may not actually trigger
        // the ReleaseProc to be called.
        fReleaseCallback.reset();
    }
}

MutableTextureStateRef* Texture::mutableState() const { return fMutableState.get(); }

} // namespace skgpu::graphite

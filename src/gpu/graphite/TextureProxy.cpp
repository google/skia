/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureProxy.h"

#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"

namespace skgpu::graphite {

TextureProxy::TextureProxy(SkISize dimensions, const TextureInfo& info, SkBudgeted budgeted)
        : fDimensions(dimensions), fInfo(info), fBudgeted(budgeted) {}

TextureProxy::TextureProxy(sk_sp<Texture> texture)
        : fDimensions(texture->dimensions())
        , fInfo(texture->textureInfo())
        , fBudgeted(texture->budgeted())
        , fTexture(std::move(texture)) {}

TextureProxy::~TextureProxy() {}

bool TextureProxy::instantiate(ResourceProvider* resourceProvider) {
    if (fTexture) {
        return true;
    }
    fTexture = resourceProvider->findOrCreateScratchTexture(fDimensions, fInfo, fBudgeted);
    if (!fTexture) {
        return false;
    }
    SkDEBUGCODE(this->validateTexture(fTexture.get()));
    return true;
}

sk_sp<Texture> TextureProxy::refTexture() const {
    return fTexture;
}

const Texture* TextureProxy::texture() const {
    return fTexture.get();
}

#ifdef SK_DEBUG
void TextureProxy::validateTexture(const Texture* texture) {
    SkASSERT(fDimensions == texture->dimensions());
    SkASSERT(fInfo == texture->textureInfo());
}
#endif

} // namespace skgpu::graphite

/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureProxy.h"

#include "src/core/SkMipmap.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"

namespace skgpu::graphite {

TextureProxy::TextureProxy(SkISize dimensions, const TextureInfo& info, SkBudgeted budgeted)
        : fDimensions(dimensions), fInfo(info), fBudgeted(budgeted) {
    // TODO: Enable this assert once we are correctly handling the creation of all graphite
    // SkImages. Right now things like makeImageSnapshot create an invalid proxy with an invalid
    // TextureInfo.
    // SkASSERT(fInfo.isValid());
}

TextureProxy::TextureProxy(sk_sp<Texture> texture)
        : fDimensions(texture->dimensions())
        , fInfo(texture->textureInfo())
        , fBudgeted(texture->budgeted())
        , fTexture(std::move(texture)) {
    SkASSERT(fInfo.isValid());
}

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

sk_sp<TextureProxy> TextureProxy::Make(const Caps* caps,
                                       SkISize dimensions,
                                       SkColorType colorType,
                                       Mipmapped mipmapped,
                                       Protected isProtected,
                                       Renderable renderable,
                                       SkBudgeted budgeted) {
    if (dimensions.width() < 1 || dimensions.height() < 1) {
        return nullptr;
    }

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(colorType,
                                                                 mipmapped,
                                                                 isProtected,
                                                                 renderable);
    if (!textureInfo.isValid()) {
        return nullptr;
    }

    return sk_make_sp<TextureProxy>(dimensions, textureInfo, budgeted);
}

#ifdef SK_DEBUG
void TextureProxy::validateTexture(const Texture* texture) {
    SkASSERT(fDimensions == texture->dimensions());
    SkASSERT(fInfo == texture->textureInfo());
}
#endif

} // namespace skgpu::graphite

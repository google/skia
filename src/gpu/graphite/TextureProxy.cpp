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
        : fDimensions(dimensions)
        , fInfo(info)
        , fBudgeted(budgeted)
        , fVolatile(Volatile::kNo) {
    SkASSERT(fInfo.isValid());
}

TextureProxy::TextureProxy(sk_sp<Texture> texture)
        : fDimensions(texture->dimensions())
        , fInfo(texture->textureInfo())
        , fBudgeted(texture->budgeted())
        , fVolatile(Volatile::kNo)
        , fTexture(std::move(texture)) {
    SkASSERT(fInfo.isValid());
}

TextureProxy::TextureProxy(SkISize dimensions,
                           const TextureInfo& textureInfo,
                           SkBudgeted budgeted,
                           Volatile isVolatile,
                           LazyInstantiateCallback&& callback)
        : fDimensions(dimensions)
        , fInfo(textureInfo)
        , fBudgeted(budgeted)
        , fVolatile(isVolatile)
        , fLazyInstantiateCallback(std::move(callback)) {
    SkASSERT(fInfo.isValid());
    SkASSERT(fLazyInstantiateCallback);
}

TextureProxy::~TextureProxy() {}

bool TextureProxy::isLazy() const {
    return SkToBool(fLazyInstantiateCallback);
}

bool TextureProxy::isVolatile() const {
    SkASSERT(fVolatile == Volatile::kNo || SkToBool(fLazyInstantiateCallback));

    return fVolatile == Volatile::kYes;
}


bool TextureProxy::instantiate(ResourceProvider* resourceProvider) {
    if (fTexture) {
        return true;
    }
    if (this->isLazy()) {
        SkASSERT(!this->isVolatile());  // this must be instantiated via volatileInstantiate
        fTexture = fLazyInstantiateCallback(resourceProvider, fVolatile);
    } else {
        fTexture = resourceProvider->findOrCreateScratchTexture(fDimensions, fInfo, fBudgeted);
    }
    if (!fTexture) {
        return false;
    }
    SkDEBUGCODE(this->validateTexture(fTexture.get()));
    return true;
}

bool TextureProxy::volatileInstantiate(ResourceProvider* resourceProvider) {
    SkASSERT(this->isLazy() && this->isVolatile());
    SkASSERT(!fTexture);

    fTexture = fLazyInstantiateCallback(resourceProvider, fVolatile);
    if (!fTexture) {
        return false;
    }
    SkDEBUGCODE(this->validateTexture(fTexture.get()));
    return true;
}

bool TextureProxy::InstantiateIfNonVolatile(ResourceProvider* resourceProvider,
                                            TextureProxy* textureProxy) {
    if (textureProxy->isVolatile()) {
        return true;
    }

    return textureProxy->instantiate(resourceProvider);
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

sk_sp<TextureProxy> TextureProxy::MakeLazy(SkISize dimensions,
                                           const TextureInfo& textureInfo,
                                           SkBudgeted budgeted,
                                           Volatile isVolatile,
                                           LazyInstantiateCallback&& callback) {
    SkASSERT(textureInfo.isValid());

    return sk_sp<TextureProxy>(new TextureProxy(dimensions, textureInfo, budgeted,
                                                isVolatile, std::move(callback)));
}

#ifdef SK_DEBUG
void TextureProxy::validateTexture(const Texture* texture) {
    SkASSERT(fDimensions == texture->dimensions());
    SkASSERT(fInfo == texture->textureInfo());
}
#endif

} // namespace skgpu::graphite

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
#include "src/gpu/graphite/TextureUtils.h"

namespace skgpu::graphite {

TextureProxy::TextureProxy(SkISize dimensions, const TextureInfo& info, skgpu::Budgeted budgeted)
        : fDimensions(dimensions), fInfo(info), fBudgeted(budgeted), fVolatile(Volatile::kNo) {
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
                           skgpu::Budgeted budgeted,
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

SkISize TextureProxy::dimensions() const {
    SkASSERT(!this->isFullyLazy() || this->isInstantiated());
    return this->isInstantiated() ? fTexture->dimensions() : fDimensions;
}

bool TextureProxy::isLazy() const {
    return SkToBool(fLazyInstantiateCallback);
}

bool TextureProxy::isFullyLazy() const {
    bool result = fDimensions.width() < 0;
    SkASSERT(result == (fDimensions.height() < 0));
    SkASSERT(!result || this->isLazy());
    return result;
}

bool TextureProxy::isVolatile() const {
    SkASSERT(fVolatile == Volatile::kNo || SkToBool(fLazyInstantiateCallback));

    return fVolatile == Volatile::kYes;
}

size_t TextureProxy::uninstantiatedGpuMemorySize() const {
    return ComputeSize(fDimensions, fInfo);
}

bool TextureProxy::instantiate(ResourceProvider* resourceProvider) {
    SkASSERT(!this->isLazy());

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

bool TextureProxy::lazyInstantiate(ResourceProvider* resourceProvider) {
    SkASSERT(this->isLazy());

    if (fTexture) {
        return true;
    }

    fTexture = fLazyInstantiateCallback(resourceProvider);
    if (!fTexture) {
        return false;
    }
    SkDEBUGCODE(this->validateTexture(fTexture.get()));
    return true;
}

bool TextureProxy::InstantiateIfNotLazy(ResourceProvider* resourceProvider,
                                            TextureProxy* textureProxy) {
    if (textureProxy->isLazy()) {
        return true;
    }

    return textureProxy->instantiate(resourceProvider);
}

void TextureProxy::deinstantiate() {
    SkASSERT(fVolatile == Volatile::kYes && SkToBool(fLazyInstantiateCallback));

    fTexture.reset();
}

sk_sp<Texture> TextureProxy::refTexture() const {
    return fTexture;
}

const Texture* TextureProxy::texture() const {
    return fTexture.get();
}

sk_sp<TextureProxy> TextureProxy::Make(const Caps* caps,
                                       SkISize dimensions,
                                       const TextureInfo& textureInfo,
                                       skgpu::Budgeted budgeted) {
    if (dimensions.width() < 1 || dimensions.height() < 1 ||
        dimensions.width() > caps->maxTextureSize() ||
        dimensions.height() > caps->maxTextureSize() ||
        !textureInfo.isValid()) {
        return nullptr;
    }

    return sk_sp<TextureProxy>(new TextureProxy(dimensions, textureInfo, budgeted));
}

sk_sp<TextureProxy> TextureProxy::Make(const Caps* caps,
                                       SkISize dimensions,
                                       SkColorType colorType,
                                       Mipmapped mipmapped,
                                       Protected isProtected,
                                       Renderable renderable,
                                       skgpu::Budgeted budgeted) {
    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(colorType,
                                                                 mipmapped,
                                                                 isProtected,
                                                                 renderable);

    return Make(caps, dimensions, textureInfo, budgeted);
}

sk_sp<TextureProxy> TextureProxy::MakeLazy(const Caps* caps,
                                           SkISize dimensions,
                                           const TextureInfo& textureInfo,
                                           skgpu::Budgeted budgeted,
                                           Volatile isVolatile,
                                           LazyInstantiateCallback&& callback) {
    SkASSERT(textureInfo.isValid());
    if (dimensions.width() < 1 || dimensions.height() < 1 ||
        dimensions.width() > caps->maxTextureSize() ||
        dimensions.height() > caps->maxTextureSize()) {
        return nullptr;
    }

    return sk_sp<TextureProxy>(new TextureProxy(dimensions, textureInfo, budgeted,
                                                isVolatile, std::move(callback)));
}

sk_sp<TextureProxy> TextureProxy::MakeFullyLazy(const TextureInfo& textureInfo,
                                                skgpu::Budgeted budgeted,
                                                Volatile isVolatile,
                                                LazyInstantiateCallback&& callback) {
    SkASSERT(textureInfo.isValid());

    return sk_sp<TextureProxy>(new TextureProxy(
            SkISize::Make(-1, -1), textureInfo, budgeted, isVolatile, std::move(callback)));
}

sk_sp<TextureProxy> TextureProxy::MakeStorage(const Caps* caps,
                                              SkISize dimensions,
                                              SkColorType colorType,
                                              skgpu::Budgeted budgeted) {
    TextureInfo textureInfo = caps->getDefaultStorageTextureInfo(colorType);

    return Make(caps, dimensions, textureInfo, budgeted);
}

sk_sp<TextureProxy> TextureProxy::Wrap(sk_sp<Texture> texture) {
    return sk_sp<TextureProxy>(new TextureProxy(std::move(texture)));
}

#ifdef SK_DEBUG
void TextureProxy::validateTexture(const Texture* texture) {
    SkASSERT(this->isFullyLazy() || fDimensions == texture->dimensions());
    SkASSERTF(fInfo.isCompatible(texture->textureInfo()),
              "proxy->fInfo[%s] incompatible with texture->fInfo[%s]",
              fInfo.toString().c_str(),
              texture->textureInfo().toString().c_str());
}
#endif

} // namespace skgpu::graphite

/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureProxy_DEFINED
#define skgpu_graphite_TextureProxy_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkTo.h"

#include <functional>

enum SkColorType : int;

namespace skgpu::graphite {

class Caps;
class ResourceProvider;
class Texture;

class TextureProxy : public SkRefCnt {
public:
    TextureProxy(SkISize dimensions, const TextureInfo& info, skgpu::Budgeted budgeted);
    TextureProxy(sk_sp<Texture>);

    TextureProxy() = delete;

    ~TextureProxy() override;

    int numSamples() const { return fInfo.numSamples(); }
    Mipmapped mipmapped() const { return fInfo.mipmapped(); }

    SkISize dimensions() const;
    const TextureInfo& textureInfo() const { return fInfo; }

    bool isLazy() const;
    bool isFullyLazy() const;
    bool isVolatile() const;

    bool instantiate(ResourceProvider*);
    /*
     * We currently only instantiate lazy proxies at insertion-time. Snap-time 'instantiate'
     * calls should be wrapped in 'InstantiateIfNotLazy'.
     *
     * Unlike Ganesh, in Graphite we do not update the proxy's dimensions with the instantiating
     * texture's dimensions. This means that when a fully-lazy proxy is instantiated and
     * deinstantiated, it goes back to being fully-lazy and without dimensions, and can be
     * re-instantiated with a new texture with different dimensions than the first.
     */
    bool lazyInstantiate(ResourceProvider*);
    /*
     * For Lazy proxies this will return true. Otherwise, it will return the result of
     * calling instantiate on the texture proxy.
     */
    static bool InstantiateIfNotLazy(ResourceProvider*, TextureProxy*);
    bool isInstantiated() const { return SkToBool(fTexture); }
    void deinstantiate();
    sk_sp<Texture> refTexture() const;
    const Texture* texture() const;

    static sk_sp<TextureProxy> Make(const Caps*,
                                    SkISize dimensions,
                                    SkColorType,
                                    Mipmapped,
                                    Protected,
                                    Renderable,
                                    skgpu::Budgeted);

    using LazyInstantiateCallback = std::function<sk_sp<Texture> (ResourceProvider*)>;

    static sk_sp<TextureProxy> MakeLazy(SkISize dimensions,
                                        const TextureInfo&,
                                        skgpu::Budgeted,
                                        Volatile,
                                        LazyInstantiateCallback&&);
    static sk_sp<TextureProxy> MakeFullyLazy(const TextureInfo&,
                                             skgpu::Budgeted,
                                             Volatile,
                                             LazyInstantiateCallback&&);

private:
    TextureProxy(SkISize dimensions,
                 const TextureInfo&,
                 skgpu::Budgeted,
                 Volatile,
                 LazyInstantiateCallback&&);

#ifdef SK_DEBUG
    void validateTexture(const Texture*);
#endif

    // In the following, 'fVolatile' and 'fLazyInstantiateCallback' can be accessed from
    // multiple threads so need to remain immutable.
    SkISize fDimensions;
    const TextureInfo fInfo;

    skgpu::Budgeted fBudgeted;
    const Volatile fVolatile;

    sk_sp<Texture> fTexture;

    const LazyInstantiateCallback fLazyInstantiateCallback;
};

// Volatile texture proxy that deinstantiates itself on destruction.
class AutoDeinstantiateTextureProxy {
public:
    AutoDeinstantiateTextureProxy(TextureProxy* textureProxy) : fTextureProxy(textureProxy) {}

    ~AutoDeinstantiateTextureProxy() {
        if (fTextureProxy) {
            fTextureProxy->deinstantiate();
        }
    }

private:
    TextureProxy* const fTextureProxy;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_TextureProxy_DEFINED

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

enum SkColorType : int;

namespace skgpu::graphite {

class Caps;
enum class Renderable : bool;
class ResourceProvider;
class Texture;

class TextureProxy : public SkRefCnt {
public:
    TextureProxy(SkISize dimensions, const TextureInfo& info, SkBudgeted budgeted);
    TextureProxy(sk_sp<Texture>);

    ~TextureProxy() override;

    int numSamples() const { return fInfo.numSamples(); }
    Mipmapped mipmapped() const { return fInfo.mipmapped(); }

    SkISize dimensions() const { return fDimensions; }
    const TextureInfo& textureInfo() const { return fInfo; }

    bool isLazy() const;
    bool isVolatile() const;

    bool instantiate(ResourceProvider*);
    /*
     * We currently only instantiate lazy proxies at insertion-time. Snap-time 'instantiate'
     * calls should be wrapped in 'InstantiateIfNotLazy'
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
                                    SkBudgeted);

    using LazyInstantiateCallback = std::function<sk_sp<Texture> (ResourceProvider*)>;

    static sk_sp<TextureProxy> MakeLazy(SkISize dimensions,
                                        const TextureInfo&,
                                        SkBudgeted,
                                        Volatile,
                                        LazyInstantiateCallback&&);

private:
    TextureProxy(SkISize dimensions,
                 const TextureInfo&,
                 SkBudgeted,
                 Volatile,
                 LazyInstantiateCallback&&);

#ifdef SK_DEBUG
    void validateTexture(const Texture*);
#endif

    // In the following, 'fVolatile' and 'fLazyInstantiateCallback' can be accessed from
    // multiple threads so need to remain immutable.
    SkISize fDimensions;
    const TextureInfo fInfo;

    SkBudgeted fBudgeted;
    const Volatile fVolatile;

    sk_sp<Texture> fTexture;

    const LazyInstantiateCallback fLazyInstantiateCallback;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_TextureProxy_DEFINED

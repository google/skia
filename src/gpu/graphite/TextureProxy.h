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
     * Volatile instantiation is kept separate from normal instantiation due to their different
     * calling patterns. We don't ever want to instantiate a volatile proxy at snap time (with
     * the normal instantiate call) but also only ever want to instantiate volatile proxies
     * at insertion time. The two entry points isolate those two cases. Snap-time 'instantiate'
     * calls should be wrapped in 'InstantiateIfNonVolatile'
     */
    bool volatileInstantiate(ResourceProvider*);
    /*
     * For Volatile proxies this will return true. Otherwise, it will return the result of
     * calling instantiate on the texture proxy.
     */
    static bool InstantiateIfNonVolatile(ResourceProvider*, TextureProxy*);
    bool isInstantiated() const { return SkToBool(fTexture); }
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

    SkISize fDimensions;
    TextureInfo fInfo;

    SkBudgeted fBudgeted;
    Volatile fVolatile;

    sk_sp<Texture> fTexture;

    LazyInstantiateCallback fLazyInstantiateCallback;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_TextureProxy_DEFINED

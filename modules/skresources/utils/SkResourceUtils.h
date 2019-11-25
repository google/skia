/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkResourceUtils_DEFINED
#define SkResourceUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTHash.h"
#include "modules/skresources/include/SkResources.h"

#include <memory>

class SkAnimCodecPlayer;
class SkData;
class SkImage;

namespace skresource_utils {

class MultiFrameImageAsset final : public skresources::ImageAsset {
public:
    /**
    * By default, images are decoded on-the-fly, at rasterization time.
    * Large images may cause jank as decoding is expensive (and can thrash internal caches).
    *
    * Pass |predecode| true to force-decode all images upfront, at the cost of potentially more RAM
    * and slower animation build times.
    */
    static sk_sp<MultiFrameImageAsset> Make(sk_sp<SkData>, bool predecode = false);

    bool isMultiFrame() override;

    sk_sp<SkImage> getFrame(float t) override;

private:
    explicit MultiFrameImageAsset(std::unique_ptr<SkAnimCodecPlayer>, bool predecode);

    std::unique_ptr<SkAnimCodecPlayer> fPlayer;
    bool                               fPreDecode;

    using INHERITED = skresources::ImageAsset;
};

class FileResourceProvider final : public skresources::ResourceProvider {
public:
    static sk_sp<FileResourceProvider> Make(SkString base_dir, bool predecode = false);

    sk_sp<SkData> load(const char resource_path[], const char resource_name[]) const override;

    sk_sp<skresources::ImageAsset> loadImageAsset(const char[], const char[],
                                                  const char[]) const override;

private:
    FileResourceProvider(SkString, bool);

    const SkString fDir;
    const bool     fPredecode;

    using INHERITED = skresources::ResourceProvider;
};

class ResourceProviderProxyBase : public skresources::ResourceProvider {
protected:
    explicit ResourceProviderProxyBase(sk_sp<skresources::ResourceProvider>);

    sk_sp<SkData> load(const char[], const char[]) const override;
    sk_sp<skresources::ImageAsset> loadImageAsset(const char[], const char[],
                                                  const char[]) const override;
    sk_sp<SkData> loadFont(const char[], const char[]) const override;

private:
    const sk_sp<skresources::ResourceProvider> fProxy;
};

class CachingResourceProvider final : public ResourceProviderProxyBase {
public:
    static sk_sp<CachingResourceProvider> Make(sk_sp<skresources::ResourceProvider> rp) {
        return rp ? sk_sp<CachingResourceProvider>(new CachingResourceProvider(std::move(rp)))
                  : nullptr;
    }

private:
    explicit CachingResourceProvider(sk_sp<skresources::ResourceProvider>);

    sk_sp<skresources::ImageAsset> loadImageAsset(const char[], const char[],
                                                  const char[]) const override;

    mutable SkMutex                                 fMutex;
    mutable SkTHashMap<SkString, sk_sp<skresources::ImageAsset>> fImageCache;

    using INHERITED = ResourceProviderProxyBase;
};

class DataURIResourceProviderProxy final : public ResourceProviderProxyBase {
public:
    static sk_sp<DataURIResourceProviderProxy> Make(sk_sp<skresources::ResourceProvider> rp,
                                                    bool predecode = false);

private:
    DataURIResourceProviderProxy(sk_sp<skresources::ResourceProvider>, bool);

    sk_sp<skresources::ImageAsset> loadImageAsset(const char[], const char[],
                                                  const char[]) const override;

    const bool fPredecode;

    using INHERITED = ResourceProviderProxyBase;
};

} // namespace skresources

#endif // SkResources_DEFINED

/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_YUVATextureProxies_DEFINED
#define skgpu_graphite_YUVATextureProxies_DEFINED

#include "include/core/SkYUVAInfo.h"
#include "src/core/SkYUVAInfoLocation.h"
#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu::graphite {

class YUVATextureProxies {
public:
    YUVATextureProxies() = default;

    /**
     * When uploading pixmaps to textures it is important that we account for how the original
     * pixmaps' channels are swizzled into the texture during upload. This will compute a swizzle
     * for each texture based on the original color types and the views' swizzles.
     */
    YUVATextureProxies(const SkYUVAInfo&,
                       TextureProxyView[SkYUVAInfo::kMaxPlanes],
                       SkColorType[SkYUVAInfo::kMaxPlanes]);

    YUVATextureProxies(const YUVATextureProxies&) = default;
    YUVATextureProxies(YUVATextureProxies&&) = default;

    YUVATextureProxies& operator=(const YUVATextureProxies&) = default;
    YUVATextureProxies& operator=(YUVATextureProxies&&) = default;

    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }

    int numPlanes() const { return fYUVAInfo.numPlanes(); }

    // Overall set of YUVA proxies is mip mapped if each plane is mip mapped.
    Mipmapped mipmapped() const { return fMipmapped; }

    TextureProxy* proxy(int i) const { return fProxies[i].get(); }

    const std::array<sk_sp<TextureProxy>, SkYUVAInfo::kMaxPlanes>& proxies() const {
        return fProxies;
    }

    sk_sp<TextureProxy> refProxy(int i) const { return fProxies[i]; }

    TextureProxyView makeView(int i) const {
        return {fProxies[i], skgpu::Swizzle::RGBA()};
    }

    bool isValid() const { return fYUVAInfo.isValid(); }

    const SkYUVAInfo::YUVALocations& yuvaLocations() const { return fYUVALocations; }

private:
    std::array<sk_sp<TextureProxy>, SkYUVAInfo::kMaxPlanes> fProxies;
    SkYUVAInfo fYUVAInfo;
    Mipmapped fMipmapped = Mipmapped::kNo;
    SkYUVAInfo::YUVALocations fYUVALocations = {};
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_Image_YUVA_DEFINED

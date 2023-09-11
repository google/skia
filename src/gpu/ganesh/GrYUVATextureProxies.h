/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVATextureProxies_DEFINED
#define GrYUVATextureProxies_DEFINED

#include "include/core/SkYUVAInfo.h"
#include "src/core/SkYUVAInfoLocation.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

class GrSurfaceProxyView;

class GrYUVATextureProxies {
public:
    GrYUVATextureProxies() = default;

    /** Assumes all planes are sampled with a default "rgba" swizzle. */
    GrYUVATextureProxies(const SkYUVAInfo&,
                         sk_sp<GrSurfaceProxy>[SkYUVAInfo::kMaxPlanes],
                         GrSurfaceOrigin textureOrigin);

    /**
     * When uploading pixmaps to textures it is important that we account for how the original
     * pixmaps' channels are swizzled into the texture during upload. This will compute a swizzle
     * for each texture based on the original color types and the views' swizzles. The views must
     * all have the same origin or the result will be an invalid GrYUVATextureProxies.
     */
    GrYUVATextureProxies(const SkYUVAInfo&,
                         GrSurfaceProxyView[SkYUVAInfo::kMaxPlanes],
                         GrColorType[SkYUVAInfo::kMaxPlanes]);

    GrYUVATextureProxies(const GrYUVATextureProxies&) = default;
    GrYUVATextureProxies(GrYUVATextureProxies&&) = default;

    GrYUVATextureProxies& operator=(const GrYUVATextureProxies&) = default;
    GrYUVATextureProxies& operator=(GrYUVATextureProxies&&) = default;

    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }

    int numPlanes() const { return fYUVAInfo.numPlanes(); }

    GrSurfaceOrigin textureOrigin() const { return fTextureOrigin; }

    // Overall set of YUVA proxies is mip mapped if each plane is mip mapped.
    skgpu::Mipmapped mipmapped() const { return fMipmapped; }

    GrSurfaceProxy* proxy(int i) const { return fProxies[i].get(); }

    const std::array<sk_sp<GrSurfaceProxy>, SkYUVAInfo::kMaxPlanes>& proxies() const {
        return fProxies;
    }

    sk_sp<GrSurfaceProxy> refProxy(int i) const { return fProxies[i]; }

    GrSurfaceProxyView makeView(int i) const {
        return {fProxies[i], fTextureOrigin, skgpu::Swizzle::RGBA()};
    }

    bool isValid() const { return fYUVAInfo.isValid(); }

    const SkYUVAInfo::YUVALocations& yuvaLocations() const { return fYUVALocations; }

private:
    std::array<sk_sp<GrSurfaceProxy>, SkYUVAInfo::kMaxPlanes> fProxies;
    SkYUVAInfo fYUVAInfo;
    GrSurfaceOrigin fTextureOrigin = kTopLeft_GrSurfaceOrigin;
    skgpu::Mipmapped fMipmapped = skgpu::Mipmapped::kNo;
    SkYUVAInfo::YUVALocations fYUVALocations = {};
};

#endif

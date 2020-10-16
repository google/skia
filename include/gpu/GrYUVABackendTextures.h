/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVABackendTextures_DEFINED
#define GrYUVABackendTextures_DEFINED

#include "include/core/SkYUVAInfo.h"
#include "include/gpu/GrBackendSurface.h"

#include <tuple>

struct SkYUVASizeInfo;
struct SkYUVAIndex;

/**
 * A set of GrBackendTextures that hold the planar data for a SkYUVAInfo.
 */
class SK_API GrYUVABackendTextures {
public:
    GrYUVABackendTextures() = default;
    GrYUVABackendTextures(const GrYUVABackendTextures&) = delete;
    GrYUVABackendTextures(GrYUVABackendTextures&&) = default;

    GrYUVABackendTextures& operator=(const GrYUVABackendTextures&) = delete;
    GrYUVABackendTextures& operator=(GrYUVABackendTextures&&) = default;

    GrYUVABackendTextures(const SkYUVAInfo&,
                          const GrBackendTexture[SkYUVAInfo::kMaxPlanes],
                          GrSurfaceOrigin textureOrigin);

    const std::array<GrBackendTexture, SkYUVAInfo::kMaxPlanes>& textures() const {
        return fTextures;
    }

    GrBackendTexture texture(int i) const {
        SkASSERT(i >= 0 && i < SkYUVAInfo::kMaxPlanes);
        return fTextures[static_cast<size_t>(i)];
    }

    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }

    int numPlanes() const { return fYUVAInfo.numPlanes(); }

    GrSurfaceOrigin textureOrigin() const { return fTextureOrigin; }

    bool isValid() const { return fYUVAInfo.isValid(); }

    bool toYUVAIndices(SkYUVAIndex[SkYUVAIndex::kIndexCount]) const;

private:
    SkYUVAInfo fYUVAInfo;
    std::array<GrBackendTexture, SkYUVAInfo::kMaxPlanes> fTextures;
    GrSurfaceOrigin fTextureOrigin = kTopLeft_GrSurfaceOrigin;
};

#endif

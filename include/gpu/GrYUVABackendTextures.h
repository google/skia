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

class SK_API GrYUVABackendTextures {
public:
    GrYUVABackendTextures() = default;
    GrYUVABackendTextures(const GrYUVABackendTextures&) = delete;
    GrYUVABackendTextures(GrYUVABackendTextures&&) = default;

    GrYUVABackendTextures& operator=(const GrYUVABackendTextures&) = delete;
    GrYUVABackendTextures& operator=(GrYUVABackendTextures&&) = default;

    GrYUVABackendTextures(const SkYUVAInfo&,
                          const GrBackendTexture[SkYUVAInfo::kMaxPlanes]);

    /**
     * Returns the texture index and channel (as 'r', 'g', 'b', or 'a') that holds the passed
     * Y, U, V, or A value. If this is invalid or if the passed channel is kA and there is
     * no alpha channel then {-1, '\0'} is returned.
     */
    std::tuple<int, char> channelLocation(SkYUVAInfo::YUVAChannel) const;

    bool isValid() const { return fYUVAInfo.isValid(); }

private:
    SkYUVAInfo fYUVAInfo;
    GrBackendTexture fTextures[SkYUVAInfo::kMaxPlanes];
};

#endif
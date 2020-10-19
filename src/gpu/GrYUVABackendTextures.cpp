/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrYUVABackendTextures.h"

static int num_channels(const GrBackendFormat& format) {
    switch (format.channelMask()) {
        case kRed_SkColorChannelFlag  : return 1;
        case kAlpha_SkColorChannelFlag: return 1;
        case kGray_SkColorChannelFlag : return 1;
        case kRG_SkColorChannelFlags  : return 2;
        case kRGB_SkColorChannelFlags : return 3;
        case kRGBA_SkColorChannelFlags: return 4;
        default                       : return 0;
    }
}

GrYUVABackendTextures::GrYUVABackendTextures(
        const SkYUVAInfo& yuvaInfo,
        const GrBackendTexture textures[SkYUVAInfo::kMaxPlanes],
        GrSurfaceOrigin textureOrigin)
        : fYUVAInfo(yuvaInfo), fTextureOrigin(textureOrigin) {
    if (!fYUVAInfo.isValid()) {
        return;
    }
    SkISize planeDimensions[SkYUVAInfo::kMaxPlanes];
    int numPlanes = yuvaInfo.planeDimensions(planeDimensions);
    for (int i = 0; i < numPlanes; ++i) {
        int numRequiredChannels = fYUVAInfo.numChannelsInPlane(i);
        if (!textures[i].isValid() ||
            textures[i].dimensions() != planeDimensions[i] ||
            textures[i].backend() != textures[0].backend() ||
            num_channels(textures[i].getBackendFormat()) < numRequiredChannels) {
            *this = {};
            return;
        }
        fTextures[i] = textures[i];
    }
}

bool GrYUVABackendTextures::toYUVAIndices(SkYUVAIndex indices[SkYUVAIndex::kIndexCount]) const {
    SkASSERT(indices);
    uint32_t channelFlags[] = {fTextures[0].getBackendFormat().channelMask(),
                               fTextures[1].getBackendFormat().channelMask(),
                               fTextures[2].getBackendFormat().channelMask(),
                               fTextures[3].getBackendFormat().channelMask()};
    return fYUVAInfo.toYUVAIndices(channelFlags, indices);
}

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
GrYUVABackendTextures::GrYUVABackendTextures(const SkYUVAInfo& yuvaInfo,
                                             const GrBackendTexture textures[SkYUVAInfo::kMaxPlanes]) : fYUVAInfo(yuvaInfo) {
    if (!fYUVAInfo.isValid()) {
        return;
    }
    SkISize planeDimensions[SkYUVAInfo::kMaxPlanes];
    int numPlanes = yuvaInfo.planeDimensions(planeDimensions);
    for (int i = 0; i < numPlanes; ++i) {
        if (textures[i].dimensions() != planeDimensions[i]) {
            *this = {};
            return;
        }
        int numRequiredChannels = fYUVAInfo.numChannelsInPlane(i);
        if (!textures[i].isValid() ||
            textures[i].backend() != textures[0].backend() ||
            num_channels(textures[i].getBackendFormat()) < numRequiredChannels) {
            *this = {};
            return;
        }
        fTextures[i] = textures[i];
    }
}

std::tuple<int, char> GrYUVABackendTextures::channelLocation(
        SkYUVAInfo::YUVAChannel channel) const {
    auto [plane, chanIdx] = fYUVAInfo.channelLocation(channel);
    if (plane == -1) {
        SkASSERT(channel == SkYUVAInfo::YUVAChannel::kA);
        return {-1, '\0'};
    }
    SkASSERT(plane >= 0 && static_cast<size_t>(plane) <= SK_ARRAY_COUNT(fTextures));
    SkASSERT(fTextures[plane].isValid());
    if (fTextures[plane].getBackendFormat().channelMask() == kAlpha_SkColorChannelFlag) {
        SkASSERT(chanIdx == 0);
        return {plane, 'a'};
    }
    return {plane, "rgba"[chanIdx]};
}

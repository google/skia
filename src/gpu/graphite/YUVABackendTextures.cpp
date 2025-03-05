/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/YUVABackendTextures.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkYUVAInfoLocation.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

namespace skgpu::graphite {

namespace {
int num_channels(uint32_t ChannelMasks) {
    switch (ChannelMasks) {
        case kRed_SkColorChannelFlag        : return 1;
        case kAlpha_SkColorChannelFlag      : return 1;
        case kGray_SkColorChannelFlag       : return 1;
        case kGrayAlpha_SkColorChannelFlags : return 2;
        case kRG_SkColorChannelFlags        : return 2;
        case kRGB_SkColorChannelFlags       : return 3;
        case kRGBA_SkColorChannelFlags      : return 4;
        default                             : return 0;
    }
    SkUNREACHABLE;
}
}

YUVABackendTextureInfo::YUVABackendTextureInfo(const SkYUVAInfo& yuvaInfo,
                                               SkSpan<const TextureInfo> textureInfo,
                                               Mipmapped mipmapped)
        : fYUVAInfo(yuvaInfo)
        , fMipmapped(mipmapped) {
    int numPlanes = yuvaInfo.numPlanes();
    if (!yuvaInfo.isValid() ||
        numPlanes == 0 ||
        (size_t)numPlanes > textureInfo.size()) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }
    for (int i = 0; i < numPlanes; ++i) {
        int numRequiredChannels = yuvaInfo.numChannelsInPlane(i);
        SkASSERT(numRequiredChannels > 0);
        fPlaneChannelMasks[i] = TextureInfoPriv::ChannelMask(textureInfo[i]);
        if (!textureInfo[i].isValid() ||
            textureInfo[i].backend() != textureInfo[0].backend() ||
            num_channels(fPlaneChannelMasks[i]) < numRequiredChannels) {
            *this = {};
            SkASSERT(!this->isValid());
            return;
        }
        fPlaneTextureInfos[i] = textureInfo[i];
    }
    SkASSERT(this->isValid());
}

bool YUVABackendTextureInfo::operator==(const YUVABackendTextureInfo& that) const {
    if (fYUVAInfo != that.fYUVAInfo || fMipmapped != that.fMipmapped) {
        return false;
    }
    return fPlaneTextureInfos == that.fPlaneTextureInfos;
}

SkYUVAInfo::YUVALocations YUVABackendTextureInfo::toYUVALocations() const {
    auto result = fYUVAInfo.toYUVALocations(fPlaneChannelMasks.data());
    SkDEBUGCODE(int numPlanes;)
    SkASSERT(SkYUVAInfo::YUVALocation::AreValidLocations(result, &numPlanes));
    SkASSERT(numPlanes == this->numPlanes());
    return result;
}

//////////////////////////////////////////////////////////////////////////////

YUVABackendTextures::YUVABackendTextures(const SkYUVAInfo& yuvaInfo,
                                         SkSpan<const BackendTexture> textures)
        : fYUVAInfo(yuvaInfo) {
    if (!yuvaInfo.isValid()) {
        SkASSERT(!this->isValid());
        return;
    }
    SkISize planeDimensions[kMaxPlanes];
    int numPlanes = yuvaInfo.planeDimensions(planeDimensions);
    if (numPlanes == 0 || (size_t)numPlanes > textures.size()) {
        fYUVAInfo = {};
        SkASSERT(!this->isValid());
        return;
    }
    for (int i = 0; i < numPlanes; ++i) {
        int numRequiredChannels = yuvaInfo.numChannelsInPlane(i);
        SkASSERT(numRequiredChannels > 0);
        fPlaneChannelMasks[i] = TextureInfoPriv::ChannelMask(textures[i].info());
        if (!textures[i].isValid() ||
            textures[i].dimensions() != planeDimensions[i] ||
            textures[i].backend() != textures[0].backend() ||
            num_channels(fPlaneChannelMasks[i]) < numRequiredChannels) {
            SkASSERT(!this->isValid());
            return;
        }
        fPlaneTextures[i] = textures[i];
    }
    SkASSERT(this->isValid());
}

SkYUVAInfo::YUVALocations YUVABackendTextures::toYUVALocations() const {
    auto result = fYUVAInfo.toYUVALocations(fPlaneChannelMasks.data());
    SkDEBUGCODE(int numPlanes;)
    SkASSERT(SkYUVAInfo::YUVALocation::AreValidLocations(result, &numPlanes));
    SkASSERT(numPlanes == this->numPlanes());
    return result;
}

}  // End of namespace skgpu::graphite

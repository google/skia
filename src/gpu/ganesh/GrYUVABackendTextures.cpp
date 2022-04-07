/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrYUVABackendTextures.h"

#include "src/core/SkYUVAInfoLocation.h"

static int num_channels(const GrBackendFormat& format) {
    switch (format.channelMask()) {
        case kRed_SkColorChannelFlag        : return 1;
        case kAlpha_SkColorChannelFlag      : return 1;
        case kGray_SkColorChannelFlag       : return 1;
        case kGrayAlpha_SkColorChannelFlags : return 2;
        case kRG_SkColorChannelFlags        : return 2;
        case kRGB_SkColorChannelFlags       : return 3;
        case kRGBA_SkColorChannelFlags      : return 4;
        default                             : return 0;
    }
}

GrYUVABackendTextureInfo::GrYUVABackendTextureInfo(const SkYUVAInfo& yuvaInfo,
                                                   const GrBackendFormat formats[kMaxPlanes],
                                                   GrMipmapped mipmapped,
                                                   GrSurfaceOrigin origin)
        : fYUVAInfo(yuvaInfo), fMipmapped(mipmapped), fTextureOrigin(origin) {
    if (!yuvaInfo.isValid()) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }
    int n = yuvaInfo.numPlanes();
    for (size_t i = 0; i < static_cast<size_t>(n); ++i) {
        if (!formats[i].isValid() || formats[i].backend() != formats[0].backend()) {
            *this = {};
            SkASSERT(!this->isValid());
            return;
        }
        int numRequiredChannels = yuvaInfo.numChannelsInPlane(i);
        SkASSERT(numRequiredChannels > 0);
        int numActualChannels = num_channels(formats[i]);
        if (numActualChannels < numRequiredChannels) {
            *this = {};
            SkASSERT(!this->isValid());
        }
        fPlaneFormats[i] = formats[i];
    }
    SkASSERT(this->isValid());
}

bool GrYUVABackendTextureInfo::operator==(const GrYUVABackendTextureInfo& that) const {
    if (fYUVAInfo != that.fYUVAInfo ||
        fMipmapped != that.fMipmapped ||
        fTextureOrigin != that.fTextureOrigin) {
        return false;
    }
    int n = fYUVAInfo.numPlanes();
    return std::equal(fPlaneFormats, fPlaneFormats + n, that.fPlaneFormats);
}

SkYUVAInfo::YUVALocations GrYUVABackendTextureInfo::toYUVALocations() const {
    uint32_t channelFlags[] = {fPlaneFormats[0].channelMask(),
                               fPlaneFormats[1].channelMask(),
                               fPlaneFormats[2].channelMask(),
                               fPlaneFormats[3].channelMask()};
    auto result = fYUVAInfo.toYUVALocations(channelFlags);
    SkDEBUGCODE(int numPlanes;)
    SkASSERT(SkYUVAInfo::YUVALocation::AreValidLocations(result, &numPlanes));
    SkASSERT(numPlanes == this->numPlanes());
    return result;
}

//////////////////////////////////////////////////////////////////////////////

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

SkYUVAInfo::YUVALocations GrYUVABackendTextures::toYUVALocations() const {
    uint32_t channelFlags[] = {fTextures[0].getBackendFormat().channelMask(),
                               fTextures[1].getBackendFormat().channelMask(),
                               fTextures[2].getBackendFormat().channelMask(),
                               fTextures[3].getBackendFormat().channelMask()};
    auto result = fYUVAInfo.toYUVALocations(channelFlags);
    SkDEBUGCODE(int numPlanes;)
    SkASSERT(SkYUVAInfo::YUVALocation::AreValidLocations(result, &numPlanes));
    SkASSERT(numPlanes == this->numPlanes());
    return result;
}

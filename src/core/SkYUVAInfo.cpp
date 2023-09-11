/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVAInfo.h"

#include "include/core/SkColor.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkYUVAInfoLocation.h"

#include <algorithm>

static bool is_plane_config_compatible_with_subsampling(SkYUVAInfo::PlaneConfig config,
                                                        SkYUVAInfo::Subsampling subsampling) {
    if (config      == SkYUVAInfo::PlaneConfig::kUnknown ||
        subsampling == SkYUVAInfo::Subsampling::kUnknown) {
        return false;
    }
    return subsampling == SkYUVAInfo::Subsampling::k444 ||
           (config != SkYUVAInfo::PlaneConfig::kYUV  &&
            config != SkYUVAInfo::PlaneConfig::kYUVA &&
            config != SkYUVAInfo::PlaneConfig::kUYV  &&
            config != SkYUVAInfo::PlaneConfig::kUYVA);
}

std::tuple<int, int> SkYUVAInfo::SubsamplingFactors(Subsampling subsampling) {
    switch (subsampling) {
        case Subsampling::kUnknown: return {0, 0};
        case Subsampling::k444:     return {1, 1};
        case Subsampling::k422:     return {2, 1};
        case Subsampling::k420:     return {2, 2};
        case Subsampling::k440:     return {1, 2};
        case Subsampling::k411:     return {4, 1};
        case Subsampling::k410:     return {4, 2};
    }
    SkUNREACHABLE;
}

std::tuple<int, int> SkYUVAInfo::PlaneSubsamplingFactors(PlaneConfig planeConfig,
                                                         Subsampling subsampling,
                                                         int planeIdx) {
    if (!is_plane_config_compatible_with_subsampling(planeConfig, subsampling) ||
        planeIdx < 0                                                           ||
        planeIdx > NumPlanes(planeConfig)) {
        return {0, 0};
    }
    bool isSubsampledPlane = false;
    switch (planeConfig) {
        case PlaneConfig::kUnknown:     SkUNREACHABLE;

        case PlaneConfig::kY_U_V:
        case PlaneConfig::kY_V_U:
        case PlaneConfig::kY_U_V_A:
        case PlaneConfig::kY_V_U_A:
            isSubsampledPlane = planeIdx == 1 || planeIdx == 2;
            break;

        case PlaneConfig::kY_UV:
        case PlaneConfig::kY_VU:
        case PlaneConfig::kY_UV_A:
        case PlaneConfig::kY_VU_A:
            isSubsampledPlane = planeIdx == 1;
            break;

        case PlaneConfig::kYUV:
        case PlaneConfig::kUYV:
        case PlaneConfig::kYUVA:
        case PlaneConfig::kUYVA:
            break;
    }
    return isSubsampledPlane ? SubsamplingFactors(subsampling) : std::make_tuple(1, 1);
}

int SkYUVAInfo::PlaneDimensions(SkISize imageDimensions,
                                PlaneConfig planeConfig,
                                Subsampling subsampling,
                                SkEncodedOrigin origin,
                                SkISize planeDimensions[SkYUVAInfo::kMaxPlanes]) {
    std::fill_n(planeDimensions, SkYUVAInfo::kMaxPlanes, SkISize{0, 0});
    if (!is_plane_config_compatible_with_subsampling(planeConfig, subsampling)) {
        return 0;
    }

    int w = imageDimensions.width();
    int h = imageDimensions.height();
    if (origin >= kLeftTop_SkEncodedOrigin) {
        using std::swap;
        swap(w, h);
    }
    auto down2 = [](int x) { return (x + 1)/2; };
    auto down4 = [](int x) { return (x + 3)/4; };
    SkISize uvSize;
    switch (subsampling) {
        case Subsampling::kUnknown: SkUNREACHABLE;

        case Subsampling::k444: uvSize = {      w ,       h }; break;
        case Subsampling::k422: uvSize = {down2(w),       h }; break;
        case Subsampling::k420: uvSize = {down2(w), down2(h)}; break;
        case Subsampling::k440: uvSize = {      w , down2(h)}; break;
        case Subsampling::k411: uvSize = {down4(w),       h }; break;
        case Subsampling::k410: uvSize = {down4(w), down2(h)}; break;
    }
    switch (planeConfig) {
        case PlaneConfig::kUnknown: SkUNREACHABLE;

        case PlaneConfig::kY_U_V:
        case PlaneConfig::kY_V_U:
            planeDimensions[0] = {w, h};
            planeDimensions[1] = planeDimensions[2] = uvSize;
            return 3;

        case PlaneConfig::kY_UV:
        case PlaneConfig::kY_VU:
            planeDimensions[0] = {w, h};
            planeDimensions[1] = uvSize;
            return 2;

        case PlaneConfig::kY_U_V_A:
        case PlaneConfig::kY_V_U_A:
            planeDimensions[0] = planeDimensions[3] = {w, h};
            planeDimensions[1] = planeDimensions[2] = uvSize;
            return 4;

        case PlaneConfig::kY_UV_A:
        case PlaneConfig::kY_VU_A:
            planeDimensions[0] = planeDimensions[2] = {w, h};
            planeDimensions[1] = uvSize;
            return 3;

        case PlaneConfig::kYUV:
        case PlaneConfig::kUYV:
        case PlaneConfig::kYUVA:
        case PlaneConfig::kUYVA:
            planeDimensions[0] = {w, h};
            SkASSERT(planeDimensions[0] == uvSize);
            return 1;
    }
    SkUNREACHABLE;
}

static bool channel_index_to_channel(uint32_t channelFlags,
                                     int channelIdx,
                                     SkColorChannel* channel) {
    switch (channelFlags) {
        case kGray_SkColorChannelFlag:  // For gray returning any of R, G, or B for index 0 is ok.
        case kRed_SkColorChannelFlag:
            if (channelIdx == 0) {
                *channel = SkColorChannel::kR;
                return true;
            }
            return false;
        case kGrayAlpha_SkColorChannelFlags:
            switch (channelIdx) {
                case 0: *channel = SkColorChannel::kR; return true;
                case 1: *channel = SkColorChannel::kA; return true;

                default: return false;
            }
        case kAlpha_SkColorChannelFlag:
            if (channelIdx == 0) {
                *channel = SkColorChannel::kA;
                return true;
            }
            return false;
        case kRG_SkColorChannelFlags:
            if (channelIdx == 0 || channelIdx == 1) {
                *channel = static_cast<SkColorChannel>(channelIdx);
                return true;
            }
            return false;
        case kRGB_SkColorChannelFlags:
            if (channelIdx >= 0 && channelIdx <= 2) {
                *channel = static_cast<SkColorChannel>(channelIdx);
                return true;
            }
            return false;
        case kRGBA_SkColorChannelFlags:
            if (channelIdx >= 0 && channelIdx <= 3) {
                *channel = static_cast<SkColorChannel>(channelIdx);
                return true;
            }
            return false;
        default:
            return false;
    }
}

SkYUVAInfo::YUVALocations SkYUVAInfo::GetYUVALocations(PlaneConfig config,
                                                       const uint32_t* planeChannelFlags) {
    // Like YUVALocation but chanIdx refers to channels by index rather than absolute channel, e.g.
    // A is the 0th channel of an alpha-only texture. We'll use this plus planeChannelFlags to get
    // the actual channel.
    struct PlaneAndIndex {int plane, chanIdx;};
    const PlaneAndIndex* planesAndIndices = nullptr;
    switch (config) {
        case PlaneConfig::kUnknown:
            return {};

        case PlaneConfig::kY_U_V: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {1, 0}, {2, 0}, {-1, -1}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kY_V_U: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {2, 0}, {1, 0}, {-1, -1}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kY_UV: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {1, 0}, {1, 1}, {-1, -1}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kY_VU: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {1, 1}, {1, 0}, {-1, -1}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kYUV: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {0, 1}, {0, 2}, {-1, -1}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kUYV: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 1}, {0, 0}, {0, 2}, {-1, -1}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kY_U_V_A: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kY_V_U_A: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {2, 0}, {1, 0}, {3, 0}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kY_UV_A: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {1, 0}, {1, 1}, {2, 0}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kY_VU_A: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {1, 1}, {1, 0}, {2, 0}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kYUVA: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
        case PlaneConfig::kUYVA: {
            static constexpr PlaneAndIndex kPlanesAndIndices[] = {{0, 1}, {0, 0}, {0, 2}, {0, 3}};
            planesAndIndices = kPlanesAndIndices;
            break;
        }
    }
    SkASSERT(planesAndIndices);
    YUVALocations yuvaLocations;
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        auto [plane, chanIdx] = planesAndIndices[i];
        SkColorChannel channel;
        if (plane >= 0) {
            if (!channel_index_to_channel(planeChannelFlags[plane], chanIdx, &channel)) {
                return {};
            }
            yuvaLocations[i] = {plane, channel};
        } else {
            SkASSERT(i == 3);
            yuvaLocations[i] = {-1, SkColorChannel::kR};
        }
    }
    return yuvaLocations;
}

bool SkYUVAInfo::HasAlpha(PlaneConfig planeConfig) {
    switch (planeConfig) {
        case PlaneConfig::kUnknown: return false;

        case PlaneConfig::kY_U_V:   return false;
        case PlaneConfig::kY_V_U:   return false;
        case PlaneConfig::kY_UV:    return false;
        case PlaneConfig::kY_VU:    return false;
        case PlaneConfig::kYUV:     return false;
        case PlaneConfig::kUYV:     return false;

        case PlaneConfig::kY_U_V_A: return true;
        case PlaneConfig::kY_V_U_A: return true;
        case PlaneConfig::kY_UV_A:  return true;
        case PlaneConfig::kY_VU_A:  return true;
        case PlaneConfig::kYUVA:    return true;
        case PlaneConfig::kUYVA:    return true;
    }
    SkUNREACHABLE;
}

SkYUVAInfo::SkYUVAInfo(SkISize dimensions,
                       PlaneConfig planeConfig,
                       Subsampling subsampling,
                       SkYUVColorSpace yuvColorSpace,
                       SkEncodedOrigin origin,
                       Siting sitingX,
                       Siting sitingY)
        : fDimensions(dimensions)
        , fPlaneConfig(planeConfig)
        , fSubsampling(subsampling)
        , fYUVColorSpace(yuvColorSpace)
        , fOrigin(origin)
        , fSitingX(sitingX)
        , fSitingY(sitingY) {
    if (fDimensions.isEmpty() ||
        !is_plane_config_compatible_with_subsampling(planeConfig, subsampling)) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }
    SkASSERT(this->isValid());
}

size_t SkYUVAInfo::computeTotalBytes(const size_t rowBytes[kMaxPlanes],
                                     size_t planeSizes[kMaxPlanes]) const {
    if (!this->isValid()) {
        return 0;
    }
    SkSafeMath safe;
    size_t totalBytes = 0;
    SkISize planeDimensions[kMaxPlanes];
    int n = this->planeDimensions(planeDimensions);
    for (int i = 0; i < n; ++i) {
        SkASSERT(!planeDimensions[i].isEmpty());
        SkASSERT(rowBytes[i]);
        size_t size = safe.mul(rowBytes[i], planeDimensions[i].height());
        if (planeSizes) {
            planeSizes[i] = size;
        }
        totalBytes = safe.add(totalBytes, size);
    }
    if (planeSizes) {
        if (safe.ok()) {
            for (int i = n; i < kMaxPlanes; ++i) {
                planeSizes[i] = 0;
            }
        } else {
            for (int i = 0; n < kMaxPlanes; ++i) {
                planeSizes[i] = SIZE_MAX;
            }
        }
    }

    return safe.ok() ? totalBytes : SIZE_MAX;
}

SkYUVAInfo::YUVALocations SkYUVAInfo::toYUVALocations(const uint32_t* channelFlags) const {
    return GetYUVALocations(fPlaneConfig, channelFlags);
}

SkYUVAInfo SkYUVAInfo::makeSubsampling(SkYUVAInfo::Subsampling subsampling) const {
    return {fDimensions, fPlaneConfig, subsampling, fYUVColorSpace, fOrigin, fSitingX, fSitingY};
}

SkYUVAInfo SkYUVAInfo::makeDimensions(SkISize dimensions) const {
    return {dimensions, fPlaneConfig, fSubsampling, fYUVColorSpace, fOrigin, fSitingX, fSitingY};
}

bool SkYUVAInfo::operator==(const SkYUVAInfo& that) const {
    return fPlaneConfig   == that.fPlaneConfig   &&
           fSubsampling   == that.fSubsampling  &&
           fYUVColorSpace == that.fYUVColorSpace &&
           fDimensions    == that.fDimensions    &&
           fSitingX       == that.fSitingX       &&
           fSitingY       == that.fSitingY       &&
           fOrigin        == that.fOrigin;
}

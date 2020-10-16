/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVAInfo.h"
#include "src/core/SkSafeMath.h"

int SkYUVAInfo::PlaneDimensions(SkISize imageDimensions,
                                PlanarConfig planarConfig,
                                SkEncodedOrigin origin,
                                SkISize planeDimensions[SkYUVAInfo::kMaxPlanes]) {
    int w = imageDimensions.width();
    int h = imageDimensions.height();
    if (origin >= kLeftTop_SkEncodedOrigin) {
        using std::swap;
        swap(w, h);
    }
    auto down2 = [](int x) { return (x + 1)/2; };
    auto down4 = [](int x) { return (x + 3)/4; };
    switch (planarConfig) {
        case PlanarConfig::kUnknown:
            planeDimensions[0] =
            planeDimensions[1] =
            planeDimensions[2] =
            planeDimensions[3] = {0, 0};
            return 0;
        case PlanarConfig::kY_U_V_444:
            planeDimensions[0] = planeDimensions[1] = planeDimensions[2] = {w, h};
            planeDimensions[3] = {0, 0};
            return 3;
        case PlanarConfig::kY_U_V_422:
            planeDimensions[0] = {w, h};
            planeDimensions[1] = planeDimensions[2] = {down2(w), h};
            planeDimensions[3] = {0, 0};
            return 3;
        case PlanarConfig::kY_U_V_420:
        case PlanarConfig::kY_V_U_420:
            planeDimensions[0] = {w, h};
            planeDimensions[1] = planeDimensions[2] = {down2(w), down2(h)};
            planeDimensions[3] = {0, 0};
            return 3;
        case PlanarConfig::kY_UV_A_4204:
        case PlanarConfig::kY_VU_A_4204:
            planeDimensions[0] = planeDimensions[2] = {w, h};
            planeDimensions[1] = {down2(w), down2(h)};
            planeDimensions[3] = {0, 0};
            return 3;
        case PlanarConfig::kY_U_V_440:
            planeDimensions[0] = {w, h};
            planeDimensions[1] = planeDimensions[2] = {w, down2(h)};
            planeDimensions[3] = {0, 0};
            return 3;
        case PlanarConfig::kY_U_V_411:
            planeDimensions[0] = {w, h};
            planeDimensions[1] = planeDimensions[2] = {down4(w), h};
            planeDimensions[3] = {0, 0};
            return 3;
        case PlanarConfig::kY_U_V_410:
            planeDimensions[0] = {w, h};
            planeDimensions[1] = planeDimensions[2] = {down4(w), down2(h)};
            planeDimensions[3] = {0, 0};
            return 3;
        case PlanarConfig::kY_U_V_A_4204:
        case PlanarConfig::kY_V_U_A_4204:
            planeDimensions[0] = planeDimensions[3] = {w, h};
            planeDimensions[1] = planeDimensions[2] = {down2(w), down2(h)};
            return 4;
        case PlanarConfig::kY_UV_420:
        case PlanarConfig::kY_VU_420:
            planeDimensions[0] = {w, h};
            planeDimensions[1] = {down2(w), down2(h)};
            planeDimensions[2] = planeDimensions[3] = {0, 0};
            return 2;
        case PlanarConfig::kYUV_444:
        case PlanarConfig::kUYV_444:
        case PlanarConfig::kYUVA_4444:
        case PlanarConfig::kUYVA_4444:
            planeDimensions[0] = {w, h};
            planeDimensions[1] = planeDimensions[2] = planeDimensions[3] = {0, 0};
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

bool SkYUVAInfo::GetYUVAIndices(PlanarConfig config,
                                const uint32_t planeChannelFlags[kMaxPlanes],
                                SkYUVAIndex indices[SkYUVAIndex::kIndexCount]) {
    struct Location {int plane, chanIdx;};
    const Location* locations = nullptr;
    switch (config) {
        case PlanarConfig::kUnknown:
            return false;

        case PlanarConfig::kY_U_V_444:
        case PlanarConfig::kY_U_V_422:
        case PlanarConfig::kY_U_V_420:
        case PlanarConfig::kY_U_V_440:
        case PlanarConfig::kY_U_V_411:
        case PlanarConfig::kY_U_V_410: {
            static constexpr Location kLocations[] = {{0, 0}, {1, 0}, {2, 0}, {-1, -1}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kY_V_U_420: {
            static constexpr Location kLocations[] = {{0, 0}, {2, 0}, {1, 0}, {-1, -1}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kY_U_V_A_4204: {
            static constexpr Location kLocations[] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kY_V_U_A_4204: {
            static constexpr Location kLocations[] = {{0, 0}, {2, 0}, {1, 0}, {3, 0}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kY_UV_420: {
            static constexpr Location kLocations[] = {{0, 0}, {1, 0}, {1, 1}, {-1, -1}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kY_VU_420: {
            static constexpr Location kLocations[] = {{0, 0}, {1, 1}, {1, 0}, {-1, -1}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kY_UV_A_4204: {
            static constexpr Location kLocations[] = {{0, 0}, {1, 0}, {1, 1}, {2, 0}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kY_VU_A_4204: {
            static constexpr Location kLocations[] = {{0, 0}, {1, 1}, {1, 0}, {2, 0}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kYUV_444: {
            static constexpr Location kLocations[] = {{0, 0}, {0, 1}, {0, 2}, {-1, -1}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kUYV_444: {
            static constexpr Location kLocations[] = {{0, 1}, {0, 0}, {0, 2}, {-1, -1}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kYUVA_4444: {
            static constexpr Location kLocations[] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
            locations = kLocations;
            break;
        }
        case PlanarConfig::kUYVA_4444: {
            static constexpr Location kLocations[] = {{0, 1}, {0, 0}, {0, 2}, {0, 3}};
            locations = kLocations;
            break;
        }
    }
    SkASSERT(locations);
    for (int i = 0; i < SkYUVAIndex::kIndexCount; ++i) {
        auto [plane, chanIdx] = locations[i];
        SkColorChannel channel;
        if (plane >= 0) {
            if (!channel_index_to_channel(planeChannelFlags[plane], chanIdx, &channel)) {
                return false;
            }
            indices[i] = {plane, channel};
        } else {
            SkASSERT(i == 3);
            indices[i] = {-1, SkColorChannel::kR};
        }
    }
    return true;
}

bool SkYUVAInfo::HasAlpha(PlanarConfig planarConfig) {
    switch (planarConfig) {
        case PlanarConfig::kUnknown:      return false;

        case PlanarConfig::kY_U_V_444:    return false;
        case PlanarConfig::kY_U_V_422:    return false;
        case PlanarConfig::kY_U_V_420:    return false;
        case PlanarConfig::kY_V_U_420:    return false;
        case PlanarConfig::kY_U_V_440:    return false;
        case PlanarConfig::kY_U_V_411:    return false;
        case PlanarConfig::kY_U_V_410:    return false;

        case PlanarConfig::kY_U_V_A_4204: return true;
        case PlanarConfig::kY_V_U_A_4204: return true;

        case PlanarConfig::kY_UV_420:     return false;
        case PlanarConfig::kY_VU_420:     return false;

        case PlanarConfig::kY_UV_A_4204:  return true;
        case PlanarConfig::kY_VU_A_4204:  return true;

        case PlanarConfig::kYUV_444:      return false;
        case PlanarConfig::kUYV_444:      return false;

        case PlanarConfig::kYUVA_4444:    return true;
        case PlanarConfig::kUYVA_4444:    return true;
    }
    SkUNREACHABLE;
}

SkYUVAInfo::SkYUVAInfo(SkISize dimensions,
                       PlanarConfig planarConfig,
                       SkYUVColorSpace yuvColorSpace,
                       SkEncodedOrigin origin,
                       Siting sitingX,
                       Siting sitingY)
    : fDimensions(dimensions)
    , fPlanarConfig(planarConfig)
    , fYUVColorSpace(yuvColorSpace)
    , fOrigin(origin)
    , fSitingX(sitingX)
    , fSitingY(sitingY) {
    if (fDimensions.width() <= 0 ||
        fDimensions.height() <= 0 ||
        planarConfig == PlanarConfig::kUnknown) {
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

bool SkYUVAInfo::operator==(const SkYUVAInfo& that) const {
    return fPlanarConfig  == that.fPlanarConfig  &&
           fYUVColorSpace == that.fYUVColorSpace &&
           fDimensions    == that.fDimensions    &&
           fSitingX       == that.fSitingX       &&
           fSitingY       == that.fSitingY       &&
           fOrigin        == that.fOrigin;
}

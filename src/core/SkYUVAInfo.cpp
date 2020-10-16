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
                                SkISize* planeDimensions) {
    int w = imageDimensions.width();
    int h = imageDimensions.height();
    if (origin >= kLeftTop_SkEncodedOrigin) {
        using std::swap;
        swap(w, h);
    }
    auto down2 = [](int x) { return (x + 1)/2; };
    auto down4 = [](int x) { return (x + 3)/4; };
    switch (planarConfig) {
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

bool SkYUVAInfo::HasAlpha(PlanarConfig planarConfig) {
    switch (planarConfig) {
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
    , fSitingY(sitingY) {}

size_t SkYUVAInfo::computeTotalBytes(const size_t rowBytes[kMaxPlanes],
                                     size_t planeSizes[kMaxPlanes]) const {
    if (fDimensions.isEmpty()) {
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

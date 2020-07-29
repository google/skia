/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVASizeInfo.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkSafeMath.h"

int SkYUVASpec::ExpectedPlaneDims(Planes planes,
                                  SkEncodedOrigin origin,
                                  SkISize imageDims,
                                  SkISize planeDims[4]) {
    int w = imageDims.width();
    int h = imageDims.height();
    if (origin >= kLeftTop_SkEncodedOrigin) {
        using std::swap;
        swap(w, h);
    }
    auto down2 = [](int x) { return (x + 1)/2; };
    auto down4 = [](int x) { return (x + 3)/4; };
    planeDims[0] = {w, h};
    switch (planes) {
        case SkYUVASpec::Planes::kY_U_V_444:
            planeDims[0] = planeDims[1] = planeDims[2] = {w, h};
            return 3;
        case SkYUVASpec::Planes::kY_U_V_422:
            planeDims[0] = {w, h};
            planeDims[1] = planeDims[2] = {down2(w), h};
            return 3;
        case SkYUVASpec::Planes::kY_U_V_420:
            planeDims[0] = {w, h};
            planeDims[1] = planeDims[2] = {down2(w), down2(h)};
            return 3;
        case SkYUVASpec::Planes::kY_U_V_440:
            planeDims[0] = {w, h};
            planeDims[1] = planeDims[2] = {w, down2(h)};
            return 3;
        case SkYUVASpec::Planes::kY_U_V_411:
            planeDims[1] = {w, h};
            planeDims[1] = planeDims[2] = {down4(w), h};
            return 3;
        case SkYUVASpec::Planes::kY_U_V_410:
            planeDims[0] = {w, h};
            planeDims[1] = planeDims[2] = {down4(w), down2(h)};
            return 3;
    }
    SkUNREACHABLE;
}

int SkYUVASpec::NumPlanes(Planes planes) {
    switch (planes) {
        case SkYUVASpec::Planes::kY_U_V_444: return 3;
        case SkYUVASpec::Planes::kY_U_V_422: return 3;
        case SkYUVASpec::Planes::kY_U_V_420: return 3;
        case SkYUVASpec::Planes::kY_U_V_440: return 3;
        case SkYUVASpec::Planes::kY_U_V_411: return 3;
        case SkYUVASpec::Planes::kY_U_V_410: return 3;
    }
    SkUNREACHABLE;
}

size_t SkYUVASizeInfo::computeTotalBytes() const {
    SkSafeMath safe;
    size_t totalBytes = 0;

    for (int i = 0; i < kMaxCount; ++i) {
        SkASSERT((!fSizes[i].isEmpty() && fWidthBytes[i]) ||
                 (fSizes[i].isEmpty() && !fWidthBytes[i]));
        totalBytes = safe.add(totalBytes, safe.mul(fWidthBytes[i], fSizes[i].height()));
    }

    return safe.ok() ? totalBytes : SIZE_MAX;
}

void SkYUVASizeInfo::computePlanes(void* base, void* planes[SkYUVASizeInfo::kMaxCount]) const {
    planes[0] = base;
    int i = 1;
    for (; i < SkYUVASizeInfo::kMaxCount; ++i) {
        if (fSizes[i].isEmpty()) {
            break;
        }
        planes[i] = SkTAddOffset<void>(planes[i - 1], fWidthBytes[i - 1] * fSizes[i - 1].height());
    }
    for (; i < SkYUVASizeInfo::kMaxCount; ++i) {
        planes[i] = nullptr;
    }
}

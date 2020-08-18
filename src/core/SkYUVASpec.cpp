/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVASpec.h"

int SkYUVASpec::ExpectedPlaneDims(Planes planes,
                                  SkEncodedOrigin origin,
                                  SkISize imageDims,
                                  SkISize planeDims[kMaxPlanes]) {
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

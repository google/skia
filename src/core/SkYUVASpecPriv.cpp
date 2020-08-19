/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkYUVASpecPriv.h"

#include "include/core/SkPixmap.h"
#include "include/core/SkYUVASpec.h"
#include "include/private/SkImageInfoPriv.h"

namespace SkYUVASpecPriv {

bool InitLegacyInfo(SkYUVASpec spec,
                    const SkPixmap planes[SkYUVASpec::kMaxPlanes],
                    SkYUVASizeInfo* yuvaSizeInfo,
                    SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount]) {
    bool ok = true;
    auto getIthChannel = [&ok](SkColorType ct, int idx) -> SkColorChannel {
        switch (SkColorTypeChannelFlags(ct)) {
            case kAlpha_SkColorChannelFlag:
                ok |= idx == 0;
                return SkColorChannel::kA;
            case kGray_SkColorChannelFlag:
            case kRed_SkColorChannelFlag:
                ok |= idx == 0;
                return SkColorChannel::kR;
            case kRG_SkColorChannelFlags:
                ok |= idx < 2;
                return static_cast<SkColorChannel>(idx);
            case kRGB_SkColorChannelFlags:
                ok |= idx < 3;
                return static_cast<SkColorChannel>(idx);
            case kRGBA_SkColorChannelFlags:
                ok |= idx < 4;
                return static_cast<SkColorChannel>(idx);
            default:
                ok = false;
                return SkColorChannel::kR;
        }
    };
    SkColorType cts[] = {planes[0].colorType(), planes[1].colorType(), planes[2].colorType(),
                         planes[3].colorType()};
    switch (spec.fPlanes) {
        case SkYUVASpec::Planes::kY_U_V_444:
        case SkYUVASpec::Planes::kY_U_V_422:
        case SkYUVASpec::Planes::kY_U_V_420:
        case SkYUVASpec::Planes::kY_U_V_440:
        case SkYUVASpec::Planes::kY_U_V_411:
        case SkYUVASpec::Planes::kY_U_V_410:
            yuvaIndices[SkYUVAIndex::kY_Index].fIndex =  0;
            yuvaIndices[SkYUVAIndex::kU_Index].fIndex =  1;
            yuvaIndices[SkYUVAIndex::kV_Index].fIndex =  2;
            yuvaIndices[SkYUVAIndex::kA_Index].fIndex = -1;
            yuvaIndices[SkYUVAIndex::kY_Index].fChannel = getIthChannel(cts[0], 0);
            yuvaIndices[SkYUVAIndex::kU_Index].fChannel = getIthChannel(cts[1], 0);
            yuvaIndices[SkYUVAIndex::kV_Index].fChannel = getIthChannel(cts[2], 0);
            break;
    }
    if (!ok) {
        return false;
    }
    yuvaSizeInfo->fOrigin = spec.fOrigin;
    SkISize expectedDims[SkYUVASpec::kMaxPlanes];
    int n = spec.expectedPlaneDims(expectedDims);
    for (int i = 0; i < n; ++i) {
        yuvaSizeInfo->fSizes[i] = planes[i].dimensions();
        yuvaSizeInfo->fWidthBytes[i] = planes[i].rowBytes();
        if (planes[i].dimensions() != expectedDims[i]) {
            return false;
        }
    }
    return true;
}

}  // namespace SkYUVASpecPriv

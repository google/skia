/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVAPixmaps.h"

#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "include/private/SkImageInfoPriv.h"

static int num_channels_in_plane(SkYUVAInfo::PlanarConfig config, int planeIdx) {
  switch (config) {
      case SkYUVAInfo::PlanarConfig::kY_U_V_444:
          SkASSERT(planeIdx >= 0 && planeIdx < 3);
          return 1;
      case SkYUVAInfo::PlanarConfig::kY_U_V_422:
          SkASSERT(planeIdx >= 0 && planeIdx < 3);
          return 1;
      case SkYUVAInfo::PlanarConfig::kY_U_V_420:
          SkASSERT(planeIdx >= 0 && planeIdx < 3);
          return 1;
      case SkYUVAInfo::PlanarConfig::kY_U_V_440:
          SkASSERT(planeIdx >= 0 && planeIdx < 3);
          return 1;
      case SkYUVAInfo::PlanarConfig::kY_U_V_411:
          SkASSERT(planeIdx >= 0 && planeIdx < 3);
          return 1;
      case SkYUVAInfo::PlanarConfig::kY_U_V_410:
          SkASSERT(planeIdx >= 0 && planeIdx < 3);
          return 1;
  }
  return SK_MaxS32;
};

SkYUVAPixmapInfo::SkYUVAPixmapInfo(const SkYUVAInfo& yuvaInfo,
                                   const SkColorType colorTypes[kMaxPlanes],
                                   const size_t rowBytes[kMaxPlanes])
        : fYUVAInfo(yuvaInfo) {
    if (yuvaInfo.dimensions().isEmpty()) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }
    SkISize planeDims[4];
    int n = yuvaInfo.expectedPlaneDims(planeDims);
    for (int i = 0; i < n; ++i) {
        fRowBytes[i] = rowBytes[i];
        fPlaneInfos[i] = SkImageInfo::Make(planeDims[i], colorTypes[i], kPremul_SkAlphaType);
        int numRequiredChannels = num_channels_in_plane(yuvaInfo.planarConfig(), i);
        bool ok = false;
        switch (SkColorTypeChannelFlags(colorTypes[i])) {
            case kGray_SkColorChannelFlag:
            case kRed_SkColorChannelFlag:
            case kAlpha_SkColorChannelFlag:
                ok = numRequiredChannels == 1;
                break;
            case kRG_SkColorChannelFlags:
                ok = numRequiredChannels <= 2;
                break;
            case kRGB_SkColorChannelFlags:
                ok = numRequiredChannels <= 3;
                break;
            case kRGBA_SkColorChannelFlags:
                ok = numRequiredChannels <= 4;
                break;
        }
        ok |= fPlaneInfos[i].validRowBytes(fRowBytes[i]);
        if (!ok) {
            *this = {};
            SkASSERT(!this->isValid());
            return;
        }
    }
    SkASSERT(this->isValid());
}

size_t SkYUVAPixmapInfo::computeTotalBytes(size_t planeSizes[kMaxPlanes]) const {
    if (!this->isValid()) {
        return 0;
    }
    return fYUVAInfo.computeTotalBytes(fRowBytes, planeSizes);
}

bool SkYUVAPixmapInfo::initPixmapsFromSingleAllocation(void* memory,
                                                       SkPixmap pixmaps[kMaxPlanes]) const {
    if (!this->isValid()) {
        return false;
    }
    SkASSERT(pixmaps);
    char* addr = static_cast<char*>(memory);
    for (int i = 0; i < kMaxPlanes; ++i) {
        pixmaps[i].reset(fPlaneInfos[i], addr, fRowBytes[i]);
        size_t planeSize = pixmaps[i].rowBytes()*pixmaps[i].height();
        SkASSERT(planeSize);
        addr += planeSize;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////

SkYUVAPixmaps SkYUVAPixmaps::Allocate(const SkYUVAPixmapInfo& yuvaPixmapInfo) {
    if (!yuvaPixmapInfo.isValid()) {
        return {};
    }
    if (yuvaPixmapInfo.yuvaInfo().dimensions().isEmpty()) {
        return {};
    }
    return SkYUVAPixmaps(yuvaPixmapInfo,
                         SkData::MakeUninitialized(yuvaPixmapInfo.computeTotalBytes()));
}

SkYUVAPixmaps SkYUVAPixmaps::FromData(const SkYUVAPixmapInfo& yuvaPixmapInfo, sk_sp<SkData> data) {
    if (!yuvaPixmapInfo.isValid()) {
        return {};
    }
    if (yuvaPixmapInfo.computeTotalBytes() > data->size()) {
        return {};
    }
    return SkYUVAPixmaps(yuvaPixmapInfo, std::move(data));
}

SkYUVAPixmaps SkYUVAPixmaps::FromExternalMemory(const SkYUVAPixmapInfo& yuvaPixmapInfo,
                                                void* memory) {
    if (!yuvaPixmapInfo.isValid()) {
        return {};
    }
    SkPixmap pixmaps[kMaxPlanes];
    yuvaPixmapInfo.initPixmapsFromSingleAllocation(memory, pixmaps);
    return SkYUVAPixmaps(yuvaPixmapInfo.yuvaInfo(), pixmaps);
}

SkYUVAPixmaps SkYUVAPixmaps::FromExternalPixmaps(const SkYUVAInfo& yuvaInfo,
                                                 const SkPixmap pixmaps[kMaxPlanes]) {
    SkColorType colorTypes[kMaxPlanes];
    size_t rowBytes[kMaxPlanes];
    for (int i = 0; i < kMaxPlanes; ++i) {
        colorTypes[i] = pixmaps[i].colorType();
        rowBytes[i] = pixmaps[i].rowBytes();
    }
    if (!SkYUVAPixmapInfo(yuvaInfo, colorTypes, rowBytes).isValid()) {
        return {};
    }
    return SkYUVAPixmaps(yuvaInfo, pixmaps);
}

SkYUVAPixmaps::SkYUVAPixmaps(const SkYUVAPixmapInfo& yuvaPixmapInfo, sk_sp<SkData> data)
        : fYUVAInfo(yuvaPixmapInfo.yuvaInfo())
        , fData(std::move(data)) {
    SkASSERT(yuvaPixmapInfo.computeTotalBytes() <= data->size());
    SkASSERT(!yuvaPixmapInfo.yuvaInfo().dimensions().isEmpty());
    SkAssertResult(yuvaPixmapInfo.initPixmapsFromSingleAllocation(fData->writable_data(),
                                                                  fPlanes.data()));
}

SkYUVAPixmaps::SkYUVAPixmaps(const SkYUVAInfo& yuvaInfo, const SkPixmap pixmaps[kMaxPlanes])
        : fYUVAInfo(yuvaInfo) {
    std::copy_n(pixmaps, kMaxPlanes, fPlanes.data());
}

bool SkYUVAPixmaps::toLegacy(SkYUVASizeInfo* yuvaSizeInfo, SkYUVAIndex yuvaIndices[4]) {
    if (!this->isValid()) {
        return false;
    }
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
    SkColorType cts[] = {fPlanes[0].colorType(),
                         fPlanes[1].colorType(),
                         fPlanes[2].colorType(),
                         fPlanes[3].colorType()};
    switch (fYUVAInfo.planarConfig()) {
        case SkYUVAInfo::PlanarConfig::kY_U_V_444:
        case SkYUVAInfo::PlanarConfig::kY_U_V_422:
        case SkYUVAInfo::PlanarConfig::kY_U_V_420:
        case SkYUVAInfo::PlanarConfig::kY_U_V_440:
        case SkYUVAInfo::PlanarConfig::kY_U_V_411:
        case SkYUVAInfo::PlanarConfig::kY_U_V_410:
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
    yuvaSizeInfo->fOrigin = fYUVAInfo.origin();
    SkISize expectedDims[SkYUVAInfo::kMaxPlanes];
    int n = fYUVAInfo.expectedPlaneDims(expectedDims);
    for (int i = 0; i < n; ++i) {
        yuvaSizeInfo->fSizes[i] = fPlanes[i].dimensions();
        yuvaSizeInfo->fWidthBytes[i] = fPlanes[i].rowBytes();
        if (fPlanes[i].dimensions() != expectedDims[i]) {
            return false;
        }
    }
    return true;
}

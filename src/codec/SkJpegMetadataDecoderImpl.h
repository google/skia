/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegMetadataDecoderImpl_DEFINED
#define SkJpegMetadataDecoderImpl_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkJpegMetadataDecoder.h"

#include <vector>

class SkData;
class SkJpegSourceMgr;
struct SkGainmapInfo;

using SkJpegMarker = SkJpegMetadataDecoder::Segment;
using SkJpegMarkerList = std::vector<SkJpegMarker>;

class SkJpegMetadataDecoderImpl : public SkJpegMetadataDecoder {
public:
    SkJpegMetadataDecoderImpl(SkJpegMarkerList markerList);
    bool findGainmapImage(SkJpegSourceMgr* sourceMgr,
                          sk_sp<SkData>& outGainmapImageData,
                          SkGainmapInfo& outGainmapInfo) const;

    // SkJpegMetadataDecoder implementation:
    sk_sp<SkData> getExifMetadata(bool copyData) const override;
    sk_sp<SkData> getICCProfileData(bool copyData) const override;
    bool mightHaveGainmapImage() const override;
    bool findGainmapImage(sk_sp<SkData> baseImageData,
                          sk_sp<SkData>& outGainmapImageData,
                          SkGainmapInfo& outGainmapInfo) override;

private:
    SkJpegMarkerList fMarkerList;
};

#endif  // SkJpegMetadataDecoderImpl_DEFINED

/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegMetadataDecoderImpl_DEFINED
#define SkJpegMetadataDecoderImpl_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkJpegMetadataDecoder.h"

#include <vector>

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
#include "include/private/SkXmp.h"
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

class SkData;
class SkJpegSourceMgr;
struct SkGainmapInfo;

using SkJpegMarker = SkJpegMetadataDecoder::Segment;
using SkJpegMarkerList = std::vector<SkJpegMarker>;

class SkJpegMetadataDecoderImpl : public SkJpegMetadataDecoder {
public:
    SkJpegMetadataDecoderImpl(SkJpegMarkerList markerList);
    SkJpegMetadataDecoderImpl(sk_sp<SkData> data);

    bool findGainmapImage(SkJpegSourceMgr* sourceMgr,
                          sk_sp<SkData>& outGainmapImageData,
                          SkGainmapInfo& outGainmapInfo) const;

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    std::unique_ptr<SkXmp> getXmpMetadata() const;
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

    // SkJpegMetadataDecoder implementation:
    sk_sp<SkData> getExifMetadata(bool copyData) const override;
    sk_sp<SkData> getICCProfileData(bool copyData) const override;
    sk_sp<SkData> getISOGainmapMetadata(bool copyData) const override;
    bool mightHaveGainmapImage() const override;
    bool findGainmapImage(sk_sp<SkData> baseImageData,
                          sk_sp<SkData>& outGainmapImageData,
                          SkGainmapInfo& outGainmapInfo) override;
    sk_sp<SkData> getJUMBFMetadata(bool copyData) const override;

private:
    SkJpegMarkerList fMarkerList;
};

#endif  // SkJpegMetadataDecoderImpl_DEFINED

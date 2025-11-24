/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegMetadataDecoderImpl_DEFINED
#define SkJpegMetadataDecoderImpl_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkGainmapInfo.h"
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
    explicit SkJpegMetadataDecoderImpl(SkJpegMarkerList markerList);
    explicit SkJpegMetadataDecoderImpl(sk_sp<const SkData> data);

    std::pair<sk_sp<const SkData>, SkGainmapInfo> findGainmapImage(
            SkJpegSourceMgr* sourceMgr) const;

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    std::unique_ptr<SkXmp> getXmpMetadata() const;
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

    // SkJpegMetadataDecoder implementation:
    sk_sp<const SkData> getExifMetadata(bool copyData) const override;
    sk_sp<const SkData> getICCProfileData(bool copyData) const override;
    sk_sp<const SkData> getISOGainmapMetadata(bool copyData) const override;
    bool mightHaveGainmapImage() const override;
    std::pair<sk_sp<const SkData>, SkGainmapInfo> findGainmapImage(
            sk_sp<const SkData>) const override;
    sk_sp<const SkData> getJUMBFMetadata(bool copyData) const override;

    // TODO(kjlubick): delete after updating Chromium
    bool findGainmapImage(sk_sp<const SkData> baseImageData,
                          sk_sp<SkData>& outGainmapImagedata,
                          SkGainmapInfo& outGainmapInfo) override {
        auto [data, info] = this->findGainmapImage(baseImageData);
        if (data) {
            outGainmapImagedata = sk_sp<SkData>(const_cast<SkData*>(data.release()));
            outGainmapInfo = info;
            return true;
        }
        return false;
    }

private:
    SkJpegMarkerList fMarkerList;
};

#endif  // SkJpegMetadataDecoderImpl_DEFINED

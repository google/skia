/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegEncoderImpl_DEFINED
#define SkJpegEncoderImpl_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/encode/SkEncoder.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

class SkColorSpace;
class SkJpegEncoderMgr;
class SkPixmap;
class SkWStream;

namespace SkJpegEncoder {
struct Options;
}  // namespace SkJpegEncoder

// JPEG metadata is included in marker-based segments in the header of the image (the part before
// the first StartOfScan marker). These functions append these parameters to an SkJpegMarkerList.
namespace SkJpegMetadataEncoder {

// Metadata segments that will be added to the encoded file using
struct Segment {
    Segment(uint8_t marker, sk_sp<SkData> parameters)
            : fMarker(marker), fParameters(std::move(parameters)) {}
    uint8_t fMarker = 0;
    sk_sp<SkData> fParameters;
};

using SegmentList = std::vector<Segment>;

// Include an ICC profile in the image. If |colorSpace| is nullptr, then include no profile. If
// |options| specifies ICC profile data, then use that data, otherwise, generate a profile for
// |colorSpace|.
void AppendICC(SegmentList& segmentList,
               const SkJpegEncoder::Options& options,
               const SkColorSpace* colorSpace);

// Include a standard (as opposed to extended) XMP metadata segment.
void AppendXMPStandard(SegmentList& segmentList, const SkData* xmpMetadata);

}  // namespace SkJpegMetadataEncoder

class SkJpegEncoderImpl : public SkEncoder {
public:
    // Make an encoder from RGB or YUV data. Encoding options are specified in |options|. Metadata
    // markers are listed in |metadata|. The ICC profile and XMP metadata are read from |metadata|
    // and not from |options|.
    static std::unique_ptr<SkEncoder> MakeRGB(SkWStream* dst,
                                              const SkPixmap& src,
                                              const SkJpegEncoder::Options& options,
                                              const SkJpegMetadataEncoder::SegmentList& metadata);
    static std::unique_ptr<SkEncoder> MakeYUV(SkWStream* dst,
                                              const SkYUVAPixmaps& srcYUVA,
                                              const SkColorSpace* srcYUVAColorSpace,
                                              const SkJpegEncoder::Options& options,
                                              const SkJpegMetadataEncoder::SegmentList& metadata);

    ~SkJpegEncoderImpl() override;

protected:
    bool onEncodeRows(int numRows) override;

private:
    SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr>, const SkPixmap& src);
    SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr>, const SkYUVAPixmaps& srcYUVA);

    std::unique_ptr<SkJpegEncoderMgr> fEncoderMgr;
    std::optional<SkYUVAPixmaps> fSrcYUVA;
};

#endif

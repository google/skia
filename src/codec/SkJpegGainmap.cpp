/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegGainmap.h"

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkJpegSourceMgr.h"
#include "src/codec/SkJpegXmp.h"

#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

bool SkJpegGetMultiPictureGainmap(const SkJpegMultiPictureParameters* mpParams,
                                  const SkJpegSegment& mpParamsSegment,
                                  SkJpegSourceMgr* decoderSource,
                                  SkGainmapInfo* outInfo,
                                  std::unique_ptr<SkStream>* outGainmapImageStream) {
    // Extract the Multi-Picture image streams in the original decoder stream (we needed the scan to
    // find the offsets of the MP images within the original decoder stream).
    auto mpStreams = SkJpegExtractMultiPictureStreams(mpParams, mpParamsSegment, decoderSource);
    if (!mpStreams) {
        SkCodecPrintf("Failed to extract MP image streams.\n");
        return false;
    }

    // Iterate over the MP image streams.
    for (auto& mpImage : mpStreams->images) {
        if (!mpImage.stream) {
            continue;
        }

        // Create a temporary source manager for this MP image.
        auto mpImageSource = SkJpegSourceMgr::Make(mpImage.stream.get());

        // Collect the potential XMP segments.
        std::vector<sk_sp<SkData>> app1Params;
        for (const auto& segment : mpImageSource->getAllSegments()) {
            if (segment.marker != kXMPMarker) {
                continue;
            }
            auto parameters = mpImageSource->getSegmentParameters(segment);
            if (!parameters) {
                continue;
            }
            app1Params.push_back(std::move(parameters));
        }

        // Build XMP if possible.
        auto xmp = SkJpegXmp::Make(app1Params);
        if (!xmp) {
            continue;
        }

        // Check if this is an MPF gainmap.
        SkGainmapInfo info;
        if (!xmp->getGainmapInfoHDRGM(&info) && !xmp->getGainmapInfoHDRGainMap(&info)) {
            continue;
        }

        // This MP image is the gainmap image. Populate its stream and the rendering parameters
        // for its format.
        if (outGainmapImageStream) {
            if (!mpImage.stream->rewind()) {
                SkCodecPrintf("Failed to rewind gainmap image stream.\n");
                return false;
            }
            *outGainmapImageStream = std::move(mpImage.stream);
        }
        *outInfo = info;
        return true;
    }
    return false;
}

bool SkJpegGetJpegRGainmap(const SkJpegXmp* xmp,
                           SkJpegSourceMgr* decoderSource,
                           SkGainmapInfo* outInfo,
                           std::unique_ptr<SkStream>* outGainmapImageStream) {
    // Parse the XMP metadata of the original image, to see if it specifies a RecoveryMap.
    SkGainmapInfo info;
    size_t itemOffsetFromEndOfImage = 0;
    size_t itemSize = 0;
    if (!xmp->getGainmapInfoJpegR(&info, &itemOffsetFromEndOfImage, &itemSize)) {
        return false;
    }

    // The offset read from the XMP metadata is relative to the end of the EndOfImage marker in the
    // original decoder stream. Create a full scan of the original decoder stream, so we can find
    // that EndOfImage marker's offset in the decoder stream.
    const std::vector<SkJpegSegment>& segments = decoderSource->getAllSegments();
    if (segments.empty() || segments.back().marker != SkJpegSegmentScanner::kMarkerEndOfImage) {
        SkCodecPrintf("Failed to construct segments through EndOfImage.\n");
        return false;
    }
    const auto& lastSegment = segments.back();
    const size_t endOfImageOffset = lastSegment.offset + SkJpegSegmentScanner::kMarkerCodeSize;
    const size_t itemOffsetFromStartOfImage = endOfImageOffset + itemOffsetFromEndOfImage;

    // Extract the gainmap image's stream.
    auto gainmapImageStream = decoderSource->getSubsetStream(itemOffsetFromStartOfImage, itemSize);
    if (!gainmapImageStream) {
        SkCodecPrintf("Failed to extract gainmap stream.");
        return false;
    }

    // Populate the output parameters for this format.
    if (outGainmapImageStream) {
        if (!gainmapImageStream->rewind()) {
            SkCodecPrintf("Failed to rewind gainmap image stream.");
            return false;
        }
        *outGainmapImageStream = std::move(gainmapImageStream);
    }

    *outInfo = info;
    return true;
}

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
    // Iterate over the MP images
    for (size_t mpImageIndex = 1; mpImageIndex < mpParams->images.size(); ++mpImageIndex) {
        // Extract the SkData for this image.
        sk_sp<SkData> mpImage;
        bool mpImageWasCopied = false;
        {
            size_t mpImageOffset = SkJpegMultiPictureParameters::GetAbsoluteOffset(
                    mpParams->images[mpImageIndex].dataOffset, mpParamsSegment.offset);
            size_t mpImageSize = mpParams->images[mpImageIndex].size;
            mpImage = decoderSource->getSubsetData(mpImageOffset, mpImageSize, &mpImageWasCopied);
        }
        if (!mpImage) {
            SkCodecPrintf("Failed to extract MP image.\n");
            continue;
        }

        // Scan through the image up to the StartOfScan. We'll be searching for the XMP metadata.
        SkJpegSegmentScanner scan(SkJpegSegmentScanner::kMarkerStartOfScan);
        scan.onBytes(mpImage->data(), mpImage->size());
        if (scan.hadError() || !scan.isDone()) {
            SkCodecPrintf("Failed to scan header of MP image.\n");
            continue;
        }

        // Collect the potential XMP segments.
        std::vector<sk_sp<SkData>> app1Params;
        for (const auto& segment : scan.getSegments()) {
            if (segment.marker != kXMPMarker) {
                continue;
            }
            auto parameters = SkData::MakeSubset(
                    mpImage.get(),
                    segment.offset + SkJpegSegmentScanner::kMarkerCodeSize +
                            SkJpegSegmentScanner::kParameterLengthSize,
                    segment.parameterLength - SkJpegSegmentScanner::kParameterLengthSize);
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

        // Check if this image identifies itself as a gainmap.
        SkGainmapInfo info;
        if (!xmp->getGainmapInfoHDRGM(&info) && !xmp->getGainmapInfoHDRGainMap(&info)) {
            continue;
        }

        // This MP image is the gainmap image. Populate its stream and the rendering parameters
        // for its format.
        if (outGainmapImageStream) {
            if (mpImageWasCopied) {
                *outGainmapImageStream = SkMemoryStream::Make(mpImage);
            } else {
                *outGainmapImageStream = SkMemoryStream::MakeCopy(mpImage->data(), mpImage->size());
            }
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
    auto gainmapImageData = decoderSource->getSubsetData(itemOffsetFromStartOfImage, itemSize);
    if (!gainmapImageData) {
        SkCodecPrintf("Failed to extract gainmap data.");
        return false;
    }

    // Populate the output parameters for this format.
    if (outGainmapImageStream) {
        *outGainmapImageStream =
                SkMemoryStream::MakeCopy(gainmapImageData->data(), gainmapImageData->size());
    }

    *outInfo = info;
    return true;
}

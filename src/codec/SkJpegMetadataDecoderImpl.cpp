/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegMetadataDecoderImpl.h"

#include "include/core/SkData.h"
#include "include/private/base/SkTemplates.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegConstants.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
#include "include/core/SkStream.h"
#include "include/private/SkExif.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkXmp.h"
#include "src/base/SkEndian.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkJpegSourceMgr.h"
#include "src/codec/SkJpegXmp.h"
#else
struct SkGainmapInfo;
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
std::unique_ptr<SkXmp> SkJpegMetadataDecoderImpl::getXmpMetadata() const {
    std::vector<sk_sp<SkData>> decoderApp1Params;
    for (const auto& marker : fMarkerList) {
        if (marker.fMarker == kXMPMarker) {
            decoderApp1Params.push_back(marker.fData);
        }
    }
    return SkJpegMakeXmp(decoderApp1Params);
}

// Extract the SkJpegMultiPictureParameters from this image (if they exist). If |sourceMgr| and
// |outMpParamsSegment| are non-nullptr, then also return the SkJpegSegment that the parameters came
// from (and return nullptr if one cannot be found).
static std::unique_ptr<SkJpegMultiPictureParameters> find_mp_params(
        const SkJpegMarkerList& markerList,
        SkJpegSourceMgr* sourceMgr,
        SkJpegSegment* outMpParamsSegment) {
    std::unique_ptr<SkJpegMultiPictureParameters> mpParams;
    size_t skippedSegmentCount = 0;

    // Search though the libjpeg segments until we find a segment that parses as MP parameters. Keep
    // track of how many segments with the MPF marker we skipped over to get there.
    for (const auto& marker : markerList) {
        if (marker.fMarker != kMpfMarker) {
            continue;
        }
        mpParams = SkJpegMultiPictureParameters::Make(marker.fData);
        if (mpParams) {
            break;
        }
        ++skippedSegmentCount;
    }
    if (!mpParams) {
        return nullptr;
    }

    // If |sourceMgr| is not specified, then do not try to find the SkJpegSegment.
    if (!sourceMgr) {
        SkASSERT(!outMpParamsSegment);
        return mpParams;
    }

    // Now, find the SkJpegSegmentScanner segment that corresponds to the libjpeg marker.
    // TODO(ccameron): It may be preferable to make SkJpegSourceMgr save segments with certain
    // markers to avoid this strangeness.
    for (const auto& segment : sourceMgr->getAllSegments()) {
        if (segment.marker != kMpfMarker) {
            continue;
        }
        if (skippedSegmentCount == 0) {
            *outMpParamsSegment = segment;
            return mpParams;
        }
        skippedSegmentCount--;
    }
    return nullptr;
}

// Attempt to extract a gainmap image from a specified offset and size within the decoder's stream.
// Returns true only if the extracted gainmap image includes XMP metadata that specifies HDR gainmap
// rendering parameters.
static bool extract_gainmap(SkJpegSourceMgr* decoderSource,
                            size_t offset,
                            size_t size,
                            bool baseImageHasIsoVersion,
                            bool baseImageHasAdobeXmp,
                            bool baseImageHasAppleExif,
                            float baseImageAppleHdrHeadroom,
                            SkGainmapInfo& outInfo,
                            sk_sp<SkData>& outData) {
    // Extract the SkData for this image.
    bool imageDataWasCopied = false;
    auto imageData = decoderSource->getSubsetData(offset, size, &imageDataWasCopied);
    if (!imageData) {
        SkCodecPrintf("Failed to extract MP image.\n");
        return false;
    }

    // Parse the potential gainmap image's metadata.
    SkJpegMetadataDecoderImpl metadataDecoder(imageData);

    // If this image identifies itself as a gainmap, then populate |info|.
    bool didPopulateInfo = false;
    SkGainmapInfo info;

    // Check for ISO 21496-1 gain map metadata.
    if (baseImageHasIsoVersion) {
        didPopulateInfo = SkGainmapInfo::Parse(
                metadataDecoder.getISOGainmapMetadata(/*copyData=*/false).get(), info);
        if (didPopulateInfo && info.fGainmapMathColorSpace) {
            auto iccData = metadataDecoder.getICCProfileData(/*copyData=*/false);
            skcms_ICCProfile iccProfile;
            if (iccData && skcms_Parse(iccData->data(), iccData->size(), &iccProfile)) {
                auto iccProfileSpace = SkColorSpace::Make(iccProfile);
                if (iccProfileSpace) {
                    info.fGainmapMathColorSpace = std::move(iccProfileSpace);
                }
            }
        }
    }

    if (!didPopulateInfo) {
        // The Adobe and Apple gain map metadata require XMP. Parse it now.
        auto xmp = metadataDecoder.getXmpMetadata();
        if (!xmp) {
            return false;
        }

        // Check for Adobe gain map metadata only if the base image specified hdrgm:Version="1.0".
        if (!didPopulateInfo && baseImageHasAdobeXmp) {
            didPopulateInfo = xmp->getGainmapInfoAdobe(&info);
        }

        // Next try for Apple gain map metadata. This does not require anything specific from the
        // base image.
        if (!didPopulateInfo && baseImageHasAppleExif) {
            didPopulateInfo = xmp->getGainmapInfoApple(baseImageAppleHdrHeadroom, &info);
        }
    }

    // If none of the formats identified itself as a gainmap and populated |info| then fail.
    if (!didPopulateInfo) {
        return false;
    }

    // This image is a gainmap.
    outInfo = info;
    if (imageDataWasCopied) {
        outData = imageData;
    } else {
        outData = SkData::MakeWithCopy(imageData->data(), imageData->size());
    }
    return true;
}
#endif

bool SkJpegMetadataDecoderImpl::findGainmapImage(SkJpegSourceMgr* sourceMgr,
                                                 sk_sp<SkData>& outData,
                                                 SkGainmapInfo& outInfo) const {
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    SkExifMetadata baseExif(getExifMetadata(/*copyData=*/false));
    auto xmp = getXmpMetadata();

    // Determine if a support ISO 21496-1 gain map version is present in the base image.
    bool isoGainmapPresent =
            SkGainmapInfo::ParseVersion(getISOGainmapMetadata(/*copyData=*/false).get());

    // Determine if Apple HDR headroom is indicated in the base image.
    float appleHdrHeadroom = 1.f;
    bool appleHdrHeadroomPresent = baseExif.getHdrHeadroom(&appleHdrHeadroom);

    // Determine if Adobe HDR gain map is indicated in the base image.
    bool adobeGainmapPresent = xmp && xmp->getGainmapInfoAdobe(nullptr);

    // Attempt to locate the gainmap from the container XMP.
    size_t containerGainmapOffset = 0;
    size_t containerGainmapSize = 0;
    if (xmp && xmp->getContainerGainmapLocation(&containerGainmapOffset, &containerGainmapSize)) {
        const auto& segments = sourceMgr->getAllSegments();
        if (!segments.empty()) {
            const auto& lastSegment = segments.back();
            if (lastSegment.marker == kJpegMarkerEndOfImage) {
                containerGainmapOffset += lastSegment.offset + kJpegMarkerCodeSize;
            }
        }
    }

    // Attempt to find MultiPicture parameters.
    SkJpegSegment mpParamsSegment;
    auto mpParams = find_mp_params(fMarkerList, sourceMgr, &mpParamsSegment);

    // First, search through the Multi-Picture images.
    if (mpParams) {
        for (size_t mpImageIndex = 1; mpImageIndex < mpParams->images.size(); ++mpImageIndex) {
            size_t mpImageOffset = SkJpegMultiPictureParameters::GetAbsoluteOffset(
                    mpParams->images[mpImageIndex].dataOffset, mpParamsSegment.offset);
            size_t mpImageSize = mpParams->images[mpImageIndex].size;

            if (extract_gainmap(sourceMgr,
                                mpImageOffset,
                                mpImageSize,
                                isoGainmapPresent,
                                adobeGainmapPresent,
                                appleHdrHeadroomPresent,
                                appleHdrHeadroom,
                                outInfo,
                                outData)) {
                // If the GContainer also suggested an offset and size, assert that we found the
                // image that the GContainer suggested.
                if (containerGainmapOffset) {
                    SkASSERT(containerGainmapOffset == mpImageOffset);
                    SkASSERT(containerGainmapSize == mpImageSize);
                }
                return true;
            }
        }
    }

    // Next, try the location suggested by the container XMP.
    if (containerGainmapOffset) {
        if (extract_gainmap(sourceMgr,
                            containerGainmapOffset,
                            containerGainmapSize,
                            /*baseImageHasIsoVersion=*/false,
                            adobeGainmapPresent,
                            /*baseImageHasAppleExif=*/false,
                            /*baseImageAppleHdrHeadroom=*/1.0,
                            outInfo,
                            outData)) {
            return true;
        }
        SkCodecPrintf("Failed to extract container-specified gainmap.\n");
    }
#endif
    return false;
}

/**
 * Return true if the specified SkJpegMarker has marker |targetMarker| and begins with the specified
 * signature.
 */
static bool marker_has_signature(const SkJpegMarker& marker,
                                 const uint32_t targetMarker,
                                 const uint8_t* signature,
                                 size_t signatureSize) {
    if (targetMarker != marker.fMarker) {
        return false;
    }
    if (marker.fData->size() <= signatureSize) {
        return false;
    }
    if (memcmp(marker.fData->bytes(), signature, signatureSize) != 0) {
        return false;
    }
    return true;
}

/*
 * Return metadata with a specific marker and signature.
 *
 * Search for segments that start with the specified targetMarker, followed by the specified
 * signature, followed by (optional) padding.
 *
 * Some types of metadata (e.g, ICC profiles) are too big to fit into a single segment's data (which
 * is limited to 64k), and come in multiple parts. For this type of data, bytesInIndex is >0. After
 * the signature comes bytesInIndex bytes (big endian) for the index of the segment's part, followed
 * by bytesInIndex bytes (big endian) for the total number of parts. If all parts are present,
 * stitch them together and return the combined result. Return failure if parts are absent, there
 * are duplicate parts, or parts disagree on the total number of parts.
 *
 * Visually, each segment is:
 * [|signatureSize| bytes containing |signature|]
 * [|signaturePadding| bytes that are unexamined]
 * [|bytesInIndex] bytes listing the segment index for multi-segment metadata]
 * [|bytesInIndex] bytes listing the segment count for multi-segment metadata]
 * [the returned data]
 *
 * If alwaysCopyData is true, then return a copy of the data. If alwaysCopyData is false, then
 * return a direct reference to the data pointed to by dinfo, if possible.
 */
static sk_sp<SkData> read_metadata(const SkJpegMarkerList& markerList,
                                   const uint32_t targetMarker,
                                   const uint8_t* signature,
                                   size_t signatureSize,
                                   size_t signaturePadding,
                                   size_t bytesInIndex,
                                   bool alwaysCopyData) {
    // Compute the total size of the entire header (signature plus padding plus index plus count),
    // since we'll use it often.
    const size_t headerSize = signatureSize + signaturePadding + 2 * bytesInIndex;

    // A map from part index to the data in each part.
    std::vector<sk_sp<SkData>> parts;

    // Running total of number of data in all parts.
    size_t partsTotalSize = 0;

    // Running total number of parts found.
    uint32_t foundPartCount = 0;

    // The expected number of parts (initialized at the first part we encounter).
    uint32_t expectedPartCount = 0;

    // Iterate through the image's segments.
    for (const auto& marker : markerList) {
        // Skip segments that don't have the right marker or signature.
        if (!marker_has_signature(marker, targetMarker, signature, signatureSize)) {
            continue;
        }

        // Skip segments that are too small to include the index and count.
        const size_t dataLength = marker.fData->size();
        if (dataLength <= headerSize) {
            continue;
        }

        // Read this part's index and count as big-endian (if they are present, otherwise hard-code
        // them to 1).
        const uint8_t* data = marker.fData->bytes();
        uint32_t partIndex = 0;
        uint32_t partCount = 0;
        if (bytesInIndex == 0) {
            partIndex = 1;
            partCount = 1;
        } else {
            for (size_t i = 0; i < bytesInIndex; ++i) {
                const size_t offset = signatureSize + signaturePadding;
                partIndex = (partIndex << 8) + data[offset + i];
                partCount = (partCount << 8) + data[offset + bytesInIndex + i];
            }
        }

        // A part count of 0 is invalid.
        if (!partCount) {
            SkCodecPrintf("Invalid marker part count zero\n");
            return nullptr;
        }

        // The indices must in the range 1, ..., count.
        if (partIndex <= 0 || partIndex > partCount) {
            SkCodecPrintf("Invalid marker index %u for count %u\n", partIndex, partCount);
            return nullptr;
        }

        // If this is the first marker we've encountered set the expected part count to its count.
        if (expectedPartCount == 0) {
            expectedPartCount = partCount;
            parts.resize(expectedPartCount);
        }

        // If this does not match the expected part count, then fail.
        if (partCount != expectedPartCount) {
            SkCodecPrintf("Conflicting marker counts %u vs %u\n", partCount, expectedPartCount);
            return nullptr;
        }

        // Make an SkData directly referencing the decoder's data for this part.
        auto partData = SkData::MakeWithoutCopy(data + headerSize, dataLength - headerSize);

        // Fail if duplicates are found.
        if (parts[partIndex - 1]) {
            SkCodecPrintf("Duplicate parts for index %u of %u\n", partIndex, expectedPartCount);
            return nullptr;
        }

        // Save part in the map.
        partsTotalSize += partData->size();
        parts[partIndex - 1] = std::move(partData);
        foundPartCount += 1;

        // Stop as soon as we find all of the parts.
        if (foundPartCount == expectedPartCount) {
            break;
        }
    }

    // Return nullptr if we don't find the data (this is not an error).
    if (expectedPartCount == 0) {
        return nullptr;
    }

    // Fail if we don't have all of the parts.
    if (foundPartCount != expectedPartCount) {
        SkCodecPrintf("Incomplete set of markers (expected %u got %u)\n",
                      expectedPartCount,
                      foundPartCount);
        return nullptr;
    }

    // Return a direct reference to the data if there is only one part and we're allowed to.
    if (!alwaysCopyData && expectedPartCount == 1) {
        return std::move(parts[0]);
    }

    // Copy all of the markers and stitch them together.
    auto result = SkData::MakeUninitialized(partsTotalSize);
    void* copyDest = result->writable_data();
    for (const auto& part : parts) {
        memcpy(copyDest, part->data(), part->size());
        copyDest = SkTAddOffset<void>(copyDest, part->size());
    }
    return result;
}

SkJpegMetadataDecoderImpl::SkJpegMetadataDecoderImpl(SkJpegMarkerList markerList)
        : fMarkerList(std::move(markerList)) {}

SkJpegMetadataDecoderImpl::SkJpegMetadataDecoderImpl(sk_sp<SkData> data) {
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    SkJpegSegmentScanner scan(kJpegMarkerStartOfScan);
    scan.onBytes(data->data(), data->size());
    if (scan.hadError() || !scan.isDone()) {
        SkCodecPrintf("Failed to scan header of MP image.\n");
        return;
    }
    for (const auto& segment : scan.getSegments()) {
        // Save the APP1 and APP2 parameters (which includes Exif, XMP, ICC, and MPF).
        if (segment.marker != kJpegMarkerAPP0 + 1 && segment.marker != kJpegMarkerAPP0 + 2) {
            continue;
        }
        auto parameters = SkJpegSegmentScanner::GetParameters(data.get(), segment);
        if (!parameters) {
            continue;
        }
        fMarkerList.emplace_back(segment.marker, std::move(parameters));
    }
#endif
}

sk_sp<SkData> SkJpegMetadataDecoderImpl::getExifMetadata(bool copyData) const {
    return read_metadata(fMarkerList,
                         kExifMarker,
                         kExifSig,
                         sizeof(kExifSig),
                         /*signaturePadding=*/1,
                         /*bytesInIndex=*/0,
                         copyData);
}

sk_sp<SkData> SkJpegMetadataDecoderImpl::getICCProfileData(bool copyData) const {
    return read_metadata(fMarkerList,
                         kICCMarker,
                         kICCSig,
                         sizeof(kICCSig),
                         /*signaturePadding=*/0,
                         kICCMarkerIndexSize,
                         copyData);
}

sk_sp<SkData> SkJpegMetadataDecoderImpl::getISOGainmapMetadata(bool copyData) const {
    return read_metadata(fMarkerList,
                         kISOGainmapMarker,
                         kISOGainmapSig,
                         sizeof(kISOGainmapSig),
                         /*signaturePadding=*/0,
                         /*bytesInIndex=*/0,
                         copyData);
}

bool SkJpegMetadataDecoderImpl::mightHaveGainmapImage() const {
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    // All supported gainmap formats require MPF. Reject images that do not have MPF.
    return find_mp_params(fMarkerList, nullptr, nullptr) != nullptr;
#else
    return false;
#endif
}

bool SkJpegMetadataDecoderImpl::findGainmapImage(sk_sp<SkData> baseImageData,
                                                 sk_sp<SkData>& outGainmapImageData,
                                                 SkGainmapInfo& outGainmapInfo) {
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    auto baseImageStream = SkMemoryStream::Make(baseImageData);
    auto sourceMgr = SkJpegSourceMgr::Make(baseImageStream.get());
    return findGainmapImage(sourceMgr.get(), outGainmapImageData, outGainmapInfo);
#else
    return false;
#endif
}

std::unique_ptr<SkJpegMetadataDecoder> SkJpegMetadataDecoder::Make(std::vector<Segment> segments) {
    return std::make_unique<SkJpegMetadataDecoderImpl>(std::move(segments));
}

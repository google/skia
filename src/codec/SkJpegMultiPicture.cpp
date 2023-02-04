/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegMultiPicture.h"

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkJpegSourceMgr.h"

#include <cstring>

// Helper macro for SkJpegParseMultiPicture. Define the indicated variable VAR of type TYPE, and
// read it from the stream, performing any endian-ness conversions as needed. If any errors are
// encountered, then return nullptr. The last void line is present to suppress unused variable
// warnings for parameters that we don't use.
#define DEFINE_AND_READ_UINT(TYPE, VAR)                                                 \
    TYPE VAR = 0;                                                                       \
    {                                                                                   \
        uint8_t VAR##Data[sizeof(TYPE)] = {0};                                          \
        if (!stream->read(VAR##Data, sizeof(TYPE))) {                                   \
            return nullptr;                                                             \
        }                                                                               \
        for (size_t VAR##i = 0; VAR##i < sizeof(TYPE); ++VAR##i) {                      \
            VAR *= 256;                                                                 \
            VAR += VAR##Data[streamIsBigEndian ? VAR##i : (sizeof(TYPE) - VAR##i - 1)]; \
        }                                                                               \
    }                                                                                   \
    (void)VAR

std::unique_ptr<SkJpegMultiPictureParameters> SkJpegParseMultiPicture(
        const sk_sp<const SkData>& segmentParameters) {
    // Read the MP Format identifier starting after the APP2 Field Length. See Figure 4 of CIPA
    // DC-x007-2009.
    if (segmentParameters->size() < sizeof(kMpfSig)) {
        return nullptr;
    }
    if (memcmp(segmentParameters->data(), kMpfSig, sizeof(kMpfSig)) != 0) {
        return nullptr;
    }
    std::unique_ptr<SkMemoryStream> stream =
            SkMemoryStream::MakeDirect(segmentParameters->bytes() + sizeof(kMpfSig),
                                       segmentParameters->size() - sizeof(kMpfSig));

    // The rest of this function reads the structure described in Figure 6 of CIPA DC-x007-2009.
    // Determine the endianness of the values in the structure. See Figure 5 (MP endian tag
    // structure).
    bool streamIsBigEndian = false;
    {
        constexpr uint8_t kMpLittleEndian[] = {0x49, 0x49, 0x2A, 0x00};
        constexpr uint8_t kMpBigEndian[] = {0x4D, 0x4D, 0x00, 0x2A};
        uint8_t endianTag[4] = {0};
        if (!stream->read(endianTag, sizeof(endianTag))) {
            SkCodecPrintf("Failed to read MP endian tag.\n");
            return nullptr;
        }
        if (!memcmp(endianTag, kMpBigEndian, 4)) {
            streamIsBigEndian = true;
        } else if (!memcmp(endianTag, kMpLittleEndian, 4)) {
            streamIsBigEndian = false;
        } else {
            SkCodecPrintf("MP endian tag was invalid.\n");
            return nullptr;
        }
    }

    // Seek to the Index Image File Directory (Index IFD).
    DEFINE_AND_READ_UINT(uint32_t, indexIfdOffset);
    if (stream->getPosition() < indexIfdOffset) {
        SkCodecPrintf("MP Index IFD offset moves backwards.\n");
        return nullptr;
    }
    if (!stream->seek(indexIfdOffset)) {
        SkCodecPrintf("Failed to seek to MPF IFD.\n");
        return nullptr;
    }

    // Read the number of tags in the Index IFD. See Table 3 (MP Index IFD Tags) for a description
    // of all possible tags.
    DEFINE_AND_READ_UINT(uint16_t, tagCount);

    // We will extract the number of images from the tags.
    uint32_t numberOfImages = 0;

    // We will need to MP Entries in order to determine the image offsets.
    bool hasMpEntryTag = false;

    // The MP Index IFD tags shall be specified in the order of their tag IDs (text from
    // section 5.2.3), so keep track of the previous tag id read.
    uint16_t previousTagId = 0;
    for (uint16_t tagIndex = 0; tagIndex < tagCount; ++tagIndex) {
        DEFINE_AND_READ_UINT(uint16_t, tagId);
        DEFINE_AND_READ_UINT(uint16_t, type);
        DEFINE_AND_READ_UINT(uint32_t, count);
        DEFINE_AND_READ_UINT(uint32_t, value);

        if (previousTagId >= tagId) {
            SkCodecPrintf("MPF tags not in order.\n");
            return nullptr;
        }
        previousTagId = tagId;

        switch (tagId) {
            case 0xB000:
                // Version. We ignore this.
                break;
            case 0xB001:
                // Number of images.
                numberOfImages = value;
                if (numberOfImages < 1) {
                    SkCodecPrintf("Invalid number of images.\n");
                    return nullptr;
                }
                break;
            case 0xB002:
                // MP Entry.
                hasMpEntryTag = true;
                if (count != 16 * numberOfImages) {
                    SkCodecPrintf("Invalid MPEntry count.\n");
                    return nullptr;
                }
                break;
            case 0xB003:
                // Individual Image Unique ID list. Validate it, but otherwise ignore it.
                if (count != 33 * numberOfImages) {
                    SkCodecPrintf("Invalid Image Unique ID count.\n");
                    return nullptr;
                }
                break;
            case 0xB004:
                // Total number of captured frames. We ignore this.
                break;
            default:
                return nullptr;
        }
    }
    if (!numberOfImages) {
        SkCodecPrintf("Number of images must be greater than zero.\n");
        return nullptr;
    }
    if (!hasMpEntryTag) {
        SkCodecPrintf("MP Entry tag was not present.\n");
        return nullptr;
    }

    // Start to prepare the result that we will return.
    auto result = std::make_unique<SkJpegMultiPictureParameters>();
    result->images.resize(numberOfImages);

    // Read the Attribute IFD offset, and verify that it is zero (absent) or greater than our
    // current offset. We will not read or validate the Attribute IFD.
    DEFINE_AND_READ_UINT(uint32_t, attributeIfdOffset);
    if (attributeIfdOffset > 0) {
        if (stream->getPosition() < attributeIfdOffset) {
            SkCodecPrintf("MP Attribute IFD offset moves backwards.\n");
            return nullptr;
        }
    }

    // Read the MP Entries (which we verified to be present with hasMpEntryTag).
    for (uint32_t i = 0; i < numberOfImages; ++i) {
        DEFINE_AND_READ_UINT(uint32_t, attribute);
        constexpr uint32_t kAttributeTypeMask = 0x7000000;
        if ((attribute & kAttributeTypeMask) != 0) {
            SkCodecPrintf("Image type must be 0 (JPEG).\n");
        }

        DEFINE_AND_READ_UINT(uint32_t, size);
        DEFINE_AND_READ_UINT(uint32_t, dataOffset);
        if (i == 0 && dataOffset != 0) {
            SkCodecPrintf("First individual Image offset must be NULL.\n");
        }

        DEFINE_AND_READ_UINT(uint16_t, dependentImage1EntryNumber);
        DEFINE_AND_READ_UINT(uint16_t, dependentImage2EntryNumber);
        result->images[i].dataOffset = dataOffset;
        result->images[i].size = size;
    }

    return result;
}

std::unique_ptr<SkJpegMultiPictureStreams> SkJpegExtractMultiPictureStreams(
        const SkJpegMultiPictureParameters* mpParams, SkJpegSourceMgr* decoderSource) {
    // Look through the scanned segments until we arrive at the MultiPicture segment.
    size_t mpSegmentOffset = 0;
    for (const auto& segment : decoderSource->getAllSegments()) {
        if (segment.marker == kMpfMarker) {
            // TODO(ccameron): It is not guaranteed that this segment is the one that produced
            // |mpParams|. Plumb through a parameter to fix this.
            mpSegmentOffset = segment.offset;
            break;
        }
    }
    // It is impossible for the MP segment to be 0 and be correct, so use 0 to mean failure.
    if (mpSegmentOffset == 0) {
        return nullptr;
    }

    // Create streams for each of the specified segments.
    auto result = std::make_unique<SkJpegMultiPictureStreams>();
    size_t numberOfImages = mpParams->images.size();
    result->images.resize(numberOfImages);
    for (size_t i = 0; i < numberOfImages; ++i) {
        const auto& imageParams = mpParams->images[i];
        if (imageParams.dataOffset == 0) {
            continue;
        }
        size_t imageStreamOffset = mpSegmentOffset + SkJpegSegmentScanner::kMarkerCodeSize +
                                   SkJpegSegmentScanner::kParameterLengthSize + sizeof(kMpfSig) +
                                   imageParams.dataOffset;
        size_t imageStreamSize = imageParams.size;
        result->images[i].stream =
                decoderSource->getSubsetStream(imageStreamOffset, imageStreamSize);
    }
    return result;
}

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
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/core/SkEndian.h"

#include <cstring>

constexpr size_t kMpEndianSize = 4;
constexpr uint8_t kMpLittleEndian[kMpEndianSize] = {0x49, 0x49, 0x2A, 0x00};
constexpr uint8_t kMpBigEndian[kMpEndianSize] = {0x4D, 0x4D, 0x00, 0x2A};

constexpr uint16_t kTypeLong = 0x4;
constexpr uint16_t kTypeUndefined = 0x7;

constexpr uint32_t kTagSize = 12;
constexpr uint32_t kTagSerializedCount = 3;

constexpr uint16_t kVersionTag = 0xB000;
constexpr uint16_t kVersionType = kTypeUndefined;
constexpr uint32_t kVersionCount = 4;
constexpr size_t kVersionSize = 4;
constexpr uint8_t kVersionExpected[kVersionSize] = {'0', '1', '0', '0'};

constexpr uint16_t kNumberOfImagesTag = 0xB001;
constexpr uint16_t kNumberOfImagesType = kTypeLong;
constexpr uint32_t kNumberOfImagesCount = 1;

constexpr uint16_t kMPEntryTag = 0xB002;
constexpr uint16_t kMPEntryType = kTypeUndefined;
constexpr uint32_t kMPEntrySize = 16;

constexpr uint32_t kMPEntryAttributeFormatMask = 0x7000000;
constexpr uint32_t kMPEntryAttributeFormatJpeg = 0x0000000;

constexpr uint32_t kMPEntryAttributeTypeMask = 0xFFFFFF;
constexpr uint32_t kMPEntryAttributeTypePrimary = 0x030000;

constexpr uint16_t kIndividualImageUniqueIDTag = 0xB003;
constexpr uint32_t kIndividualImageUniqueIDSize = 33;

constexpr uint16_t kTotalNumberCapturedFramesTag = 0xB004;
constexpr uint32_t kTotalNumberCaptureFramesCount = 1;

// Helper macro for SkJpegMultiPictureParameters::Make. Define the indicated variable VAR of type
// TYPE, and read it from the stream, performing any endian-ness conversions as needed. Also define
// the variable VAR##Bytes with the raw bytes (with no endian-ness conversion applied). If any
// errors are encountered, then return nullptr. The last void line is present to suppress unused
// variable warnings for parameters that we don't use.
#define DEFINE_AND_READ_UINT(TYPE, VAR)                                                  \
    TYPE VAR = 0;                                                                        \
    uint8_t VAR##Bytes[sizeof(TYPE)] = {0};                                              \
    {                                                                                    \
        if (!stream->read(VAR##Bytes, sizeof(TYPE))) {                                   \
            return nullptr;                                                              \
        }                                                                                \
        for (size_t VAR##i = 0; VAR##i < sizeof(TYPE); ++VAR##i) {                       \
            VAR *= 256;                                                                  \
            VAR += VAR##Bytes[streamIsBigEndian ? VAR##i : (sizeof(TYPE) - VAR##i - 1)]; \
        }                                                                                \
    }                                                                                    \
    (void)VAR

std::unique_ptr<SkJpegMultiPictureParameters> SkJpegMultiPictureParameters::Make(
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
        uint8_t endianTag[kMpEndianSize] = {0};
        if (!stream->read(endianTag, kMpEndianSize)) {
            SkCodecPrintf("Failed to read MP endian tag.\n");
            return nullptr;
        }
        if (!memcmp(endianTag, kMpBigEndian, kMpEndianSize)) {
            streamIsBigEndian = true;
        } else if (!memcmp(endianTag, kMpLittleEndian, kMpEndianSize)) {
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

    // The offset to the MP entries. Zero is an invalid value.
    uint32_t mpEntryOffset = 0;

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
            case kVersionTag:
                // See 5.2.3.1: MP Format Version.
                if (memcmp(valueBytes, kVersionExpected, kVersionSize) != 0) {
                    SkCodecPrintf("Version value is not 0100.\n");
                    return nullptr;
                }
                if (count != kVersionCount) {
                    SkCodecPrintf("Version count not 4.\n");
                    return nullptr;
                }
                break;
            case kNumberOfImagesTag:
                // See 5.2.3.2: Number of Images.
                numberOfImages = value;
                if (type != kTypeLong) {
                    SkCodecPrintf("Invalid Total Number of Captured Frames type.\n");
                    return nullptr;
                }
                if (numberOfImages < 1) {
                    SkCodecPrintf("Invalid number of images.\n");
                    return nullptr;
                }
                break;
            case kMPEntryTag: {
                // See 5.2.3.3: MP Entry.
                if (count != kMPEntrySize * numberOfImages) {
                    SkCodecPrintf("Invalid MPEntry count.\n");
                    return nullptr;
                }
                mpEntryOffset = value;
                break;
            }
            case kIndividualImageUniqueIDTag:
                // See 5.2.3.4: Individual Image Unique ID List.
                // Validate that the count parameter is correct, but do not extract any other
                // information.
                if (count != kIndividualImageUniqueIDSize * numberOfImages) {
                    SkCodecPrintf("Invalid Image Unique ID count.\n");
                    return nullptr;
                }
                break;
            case kTotalNumberCapturedFramesTag:
                // See 5.2.3.5: Total Number of Captured Frames.
                if (type != kTypeLong) {
                    SkCodecPrintf("Invalid Total Number of Captured Frames type.\n");
                    return nullptr;
                }
                if (count != kTotalNumberCaptureFramesCount) {
                    SkCodecPrintf("Invalid Total Number of Captured Frames count.\n");
                    return nullptr;
                }
                break;
            default:
                return nullptr;
        }
    }
    if (!numberOfImages) {
        SkCodecPrintf("Number of images must be greater than zero.\n");
        return nullptr;
    }
    if (!mpEntryOffset) {
        SkCodecPrintf("MP Entry tag was not present or had invalid offset.\n");
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

    // Read the MP Entries starting at the offset that we read earlier.
    if (!stream->seek(mpEntryOffset)) {
        SkCodecPrintf("Failed to seek to MP entries' offset.\n");
        return nullptr;
    }
    for (uint32_t i = 0; i < numberOfImages; ++i) {
        DEFINE_AND_READ_UINT(uint32_t, attribute);
        const bool isPrimary =
                (attribute & kMPEntryAttributeTypeMask) == kMPEntryAttributeTypePrimary;
        const bool isJpeg =
                (attribute & kMPEntryAttributeFormatMask) == kMPEntryAttributeFormatJpeg;

        if (isPrimary != (i == 0)) {
            SkCodecPrintf("Image must be primary iff it is the first image..\n");
            return nullptr;
        }
        if (!isJpeg) {
            SkCodecPrintf("Image format must be 0 (JPEG).\n");
            return nullptr;
        }

        DEFINE_AND_READ_UINT(uint32_t, size);
        DEFINE_AND_READ_UINT(uint32_t, dataOffset);
        if (i == 0 && dataOffset != 0) {
            SkCodecPrintf("First individual Image offset must be NULL.\n");
            return nullptr;
        }

        DEFINE_AND_READ_UINT(uint16_t, dependentImage1EntryNumber);
        DEFINE_AND_READ_UINT(uint16_t, dependentImage2EntryNumber);
        result->images[i].dataOffset = dataOffset;
        result->images[i].size = size;
    }

    return result;
}

#undef DEFINE_AND_READ_UINT

// Return the number of bytes that will be written by SkJpegMultiPictureParametersSerialize, for a
// given number of images.
size_t multi_picture_params_serialized_size(size_t numberOfImages) {
    return sizeof(kMpfSig) +                 // Signature
           kMpEndianSize +                   // Endianness
           sizeof(uint32_t) +                // Index IFD Offset
           sizeof(uint16_t) +                // Tag count
           kTagSerializedCount * kTagSize +  // 3 tags at 12 bytes each
           sizeof(uint32_t) +                // Attribute IFD offset
           numberOfImages * kMPEntrySize;    // MP Entries for each image
}

// Helper macros for SkJpegMultiPictureParameters::serialize. Byte-swap and write the specified
// value, and return nullptr on failure.
#define WRITE_UINT16(value)                         \
    do {                                            \
        if (!s.write16(SkEndian_SwapBE16(value))) { \
            return nullptr;                         \
        }                                           \
    } while (0)

#define WRITE_UINT32(value)                         \
    do {                                            \
        if (!s.write32(SkEndian_SwapBE32(value))) { \
            return nullptr;                         \
        }                                           \
    } while (0)

sk_sp<SkData> SkJpegMultiPictureParameters::serialize() const {
    // Write the MPF signature.
    SkDynamicMemoryWStream s;
    if (!s.write(kMpfSig, sizeof(kMpfSig))) {
        SkCodecPrintf("Failed to write signature.\n");
        return nullptr;
    }

    // We will always write as big-endian.
    if (!s.write(kMpBigEndian, kMpEndianSize)) {
        SkCodecPrintf("Failed to write endianness.\n");
        return nullptr;
    }
    // Compute the number of images.
    uint32_t numberOfImages = static_cast<uint32_t>(images.size());

    // Set the Index IFD offset be the position after the endianness value and this offset.
    constexpr uint32_t indexIfdOffset =
            static_cast<uint16_t>(sizeof(kMpBigEndian) + sizeof(uint32_t));
    WRITE_UINT32(indexIfdOffset);

    // We will write 3 tags (version, number of images, MP entries).
    constexpr uint32_t numberOfTags = 3;
    WRITE_UINT16(numberOfTags);

    // Write the version tag.
    WRITE_UINT16(kVersionTag);
    WRITE_UINT16(kVersionType);
    WRITE_UINT32(kVersionCount);
    if (!s.write(kVersionExpected, kVersionSize)) {
        SkCodecPrintf("Failed to write version.\n");
        return nullptr;
    }

    // Write the number of images.
    WRITE_UINT16(kNumberOfImagesTag);
    WRITE_UINT16(kNumberOfImagesType);
    WRITE_UINT32(kNumberOfImagesCount);
    WRITE_UINT32(numberOfImages);

    // Write the MP entries.
    WRITE_UINT16(kMPEntryTag);
    WRITE_UINT16(kMPEntryType);
    WRITE_UINT32(kMPEntrySize * numberOfImages);
    const uint32_t mpEntryOffset =
            static_cast<uint32_t>(s.bytesWritten() -  // The bytes written so far
                                  sizeof(kMpfSig) +   // Excluding the MPF signature
                                  sizeof(uint32_t) +  // The 4 bytes for this offset
                                  sizeof(uint32_t));  // The 4 bytes for the attribute IFD offset.
    WRITE_UINT32(mpEntryOffset);

    // Write the attribute IFD offset (zero because we don't write it).
    WRITE_UINT32(0);

    // Write the MP entries.
    for (size_t i = 0; i < images.size(); ++i) {
        const auto& image = images[i];

        uint32_t attribute = kMPEntryAttributeFormatJpeg;
        if (i == 0) {
            attribute |= kMPEntryAttributeTypePrimary;
        }

        WRITE_UINT32(attribute);
        WRITE_UINT32(image.size);
        WRITE_UINT32(image.dataOffset);
        // Dependent image 1 and 2 entries are zero.
        WRITE_UINT16(0);
        WRITE_UINT16(0);
    }

    SkASSERT(s.bytesWritten() == multi_picture_params_serialized_size(images.size()));
    return s.detachAsData();
}

#undef WRITE_UINT16
#undef WRITE_UINT32

size_t SkJpegMultiPictureParameters::GetAbsoluteOffset(uint32_t dataOffset,
                                                       size_t mpSegmentOffset) {
    // The value of zero is used by the primary image.
    if (dataOffset == 0) {
        return 0;
    }
    return mpSegmentOffset +                  // The offset to the marker
           kJpegMarkerCodeSize +              // The marker itself
           kJpegSegmentParameterLengthSize +  // The parameter length
           sizeof(kMpfSig) +                  // The signature
           dataOffset;
}

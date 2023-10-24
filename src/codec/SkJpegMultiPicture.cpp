/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegMultiPicture.h"

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/base/SkEndian.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkTiffUtility.h"

#include <cstring>

constexpr size_t kMpEndianSize = 4;
constexpr uint8_t kMpBigEndian[kMpEndianSize] = {0x4D, 0x4D, 0x00, 0x2A};

constexpr uint16_t kTypeUnsignedLong = 0x4;
constexpr uint16_t kTypeUndefined = 0x7;

constexpr uint32_t kIfdEntrySize = 12;
constexpr uint32_t kIfdSerializedEntryCount = 3;

constexpr uint16_t kVersionTag = 0xB000;
constexpr uint32_t kVersionCount = 4;
constexpr size_t kVersionSize = 4;
constexpr uint8_t kVersionExpected[kVersionSize] = {'0', '1', '0', '0'};

constexpr uint16_t kNumberOfImagesTag = 0xB001;
constexpr uint32_t kNumberOfImagesCount = 1;

constexpr uint16_t kMPEntryTag = 0xB002;
constexpr uint32_t kMPEntrySize = 16;

constexpr uint32_t kMPEntryAttributeFormatMask = 0x7000000;
constexpr uint32_t kMPEntryAttributeFormatJpeg = 0x0000000;

constexpr uint32_t kMPEntryAttributeTypeMask = 0xFFFFFF;
constexpr uint32_t kMPEntryAttributeTypePrimary = 0x030000;

constexpr uint16_t kIndividualImageUniqueIDTag = 0xB003;
constexpr uint32_t kIndividualImageUniqueIDSize = 33;

constexpr uint16_t kTotalNumberCapturedFramesTag = 0xB004;

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
    auto ifdData = SkData::MakeSubset(
            segmentParameters.get(), sizeof(kMpfSig), segmentParameters->size() - sizeof(kMpfSig));
    SkASSERT(ifdData);

    // The rest of this function reads the structure described in Figure 6 of CIPA DC-x007-2009.
    // Determine the endianness of the values in the structure. See Figure 5 (MP endian tag
    // structure), and read the Index IFD offset.
    bool littleEndian = false;
    uint32_t ifdOffset = 0;
    if (!SkTiffImageFileDirectory::ParseHeader(ifdData.get(), &littleEndian, &ifdOffset)) {
        SkCodecPrintf("Failed to parse endian-ness and index IFD offset.\n");
        return nullptr;
    }

    // Create the Index Image File Directory (Index IFD).
    auto ifd = SkTiffImageFileDirectory::MakeFromOffset(ifdData, littleEndian, ifdOffset);
    if (!ifd) {
        SkCodecPrintf("Failed to create MP Index IFD offset.\n");
        return nullptr;
    }

    // Read the number of tags in the Index IFD. See Table 3 (MP Index IFD Tags) for a description
    // of all possible tags.
    uint16_t tagCount = ifd->getNumEntries();

    // We will extract the number of images from the tags.
    uint32_t numberOfImages = 0;

    // The data for the MP entries.
    sk_sp<SkData> mpEntriesData;

    // The MP Index IFD tags shall be specified in the order of their tag IDs (text from
    // section 5.2.3), so keep track of the previous tag id read.
    uint16_t previousTag = 0;
    for (uint16_t idfEntryIndex = 0; idfEntryIndex < tagCount; ++idfEntryIndex) {
        uint16_t tag = ifd->getEntryTag(idfEntryIndex);
        if (previousTag >= tag) {
            SkCodecPrintf("MPF tags not in order.\n");
            return nullptr;
        }
        previousTag = tag;

        switch (tag) {
            case kVersionTag: {
                // See 5.2.3.1: MP Format Version.
                sk_sp<SkData> data = ifd->getEntryUndefinedData(idfEntryIndex);
                if (!data) {
                    SkCodecPrintf("Version must be undefined type.\n");
                    return nullptr;
                }
                if (data->size() != kVersionSize) {
                    SkCodecPrintf("Version must be 4 bytes.\n");
                    return nullptr;
                }
                if (memcmp(data->data(), kVersionExpected, kVersionSize) != 0) {
                    SkCodecPrintf("Version value is not 0100.\n");
                    return nullptr;
                }
                break;
            }
            case kNumberOfImagesTag:
                // See 5.2.3.2: Number of Images.
                if (!ifd->getEntryUnsignedLong(idfEntryIndex, 1, &numberOfImages)) {
                    SkCodecPrintf("Number of Images was not 1 unsigned long.\n");
                }
                if (numberOfImages < 1) {
                    SkCodecPrintf("Invalid number of images.\n");
                    return nullptr;
                }
                break;
            case kMPEntryTag: {
                // See 5.2.3.3: MP Entry.
                mpEntriesData = ifd->getEntryUndefinedData(idfEntryIndex);
                if (!mpEntriesData) {
                    SkCodecPrintf("MP entries data could not be extracted.\n");
                    return nullptr;
                }
                if (mpEntriesData->size() != kMPEntrySize * numberOfImages) {
                    SkCodecPrintf("MP entries data should be %ux%u bytes, was %u.\n",
                                  kMPEntrySize,
                                  numberOfImages,
                                  static_cast<uint32_t>(mpEntriesData->size()));
                    return nullptr;
                }
                break;
            }
            case kIndividualImageUniqueIDTag: {
                // See 5.2.3.4: Individual Image Unique ID List.
                // Validate that the count parameter is correct, but do not extract any other
                // information.
                sk_sp<SkData> data = ifd->getEntryUndefinedData(idfEntryIndex);
                if (!data) {
                    SkCodecPrintf("Image Unique ID must be undefined type.\n");
                    return nullptr;
                }
                if (data->size() != kIndividualImageUniqueIDSize * numberOfImages) {
                    SkCodecPrintf("Invalid Image Unique ID count.\n");
                    return nullptr;
                }
                break;
            }
            case kTotalNumberCapturedFramesTag: {
                // See 5.2.3.5: Total Number of Captured Frames.
                uint32_t totalNumCapturedFrames = 0;
                if (!ifd->getEntryUnsignedLong(idfEntryIndex, 1, &totalNumCapturedFrames)) {
                    SkCodecPrintf("Total Number of Captures Frames was not 1 unsigned long.\n");
                }
                break;
            }
            default:
                return nullptr;
        }
    }
    if (!numberOfImages) {
        SkCodecPrintf("Number of images must be greater than zero.\n");
        return nullptr;
    }
    if (!mpEntriesData) {
        SkCodecPrintf("MP Entry data was not present.\n");
        return nullptr;
    }

    // Start to prepare the result that we will return.
    auto result = std::make_unique<SkJpegMultiPictureParameters>();
    result->images.resize(numberOfImages);

    // The next IFD is the Attribute IFD offset. We will not read or validate the Attribute IFD.

    // Parse the MP Entries data.
    for (uint32_t i = 0; i < numberOfImages; ++i) {
        const uint8_t* mpEntryData = mpEntriesData->bytes() + kMPEntrySize * i;
        const uint32_t attribute = get_endian_int(mpEntryData + 0, littleEndian);
        const uint32_t size = get_endian_int(mpEntryData + 4, littleEndian);
        const uint32_t dataOffset = get_endian_int(mpEntryData + 8, littleEndian);

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

        if (i == 0 && dataOffset != 0) {
            SkCodecPrintf("First individual Image offset must be NULL.\n");
            return nullptr;
        }

        result->images[i].dataOffset = dataOffset;
        result->images[i].size = size;
    }

    return result;
}

// Return the number of bytes that will be written by SkJpegMultiPictureParametersSerialize, for a
// given number of images.
size_t multi_picture_params_serialized_size(size_t numberOfImages) {
    return sizeof(kMpfSig) +                           // Signature
           kMpEndianSize +                             // Endianness
           sizeof(uint32_t) +                          // Index IFD Offset
           sizeof(uint16_t) +                          // IFD entry count
           kIfdSerializedEntryCount * kIfdEntrySize +  // 3 IFD entries at 12 bytes each
           sizeof(uint32_t) +                          // Attribute IFD offset
           numberOfImages * kMPEntrySize;              // MP Entries for each image
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
    WRITE_UINT16(kTypeUndefined);
    WRITE_UINT32(kVersionCount);
    if (!s.write(kVersionExpected, kVersionSize)) {
        SkCodecPrintf("Failed to write version.\n");
        return nullptr;
    }

    // Write the number of images.
    WRITE_UINT16(kNumberOfImagesTag);
    WRITE_UINT16(kTypeUnsignedLong);
    WRITE_UINT32(kNumberOfImagesCount);
    WRITE_UINT32(numberOfImages);

    // Write the MP entries.
    WRITE_UINT16(kMPEntryTag);
    WRITE_UINT16(kTypeUndefined);
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

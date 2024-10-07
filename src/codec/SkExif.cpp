/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkExif.h"

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkTiffUtility.h"
#include "src/core/SkStreamPriv.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <utility>

namespace SkExif {

constexpr uint16_t kSubIFDOffsetTag = 0x8769;
constexpr uint16_t kMarkerNoteTag = 0x927c;

static std::optional<float> get_maker_note_hdr_headroom(sk_sp<SkData> data) {
    // No little endian images that specify this data have been observed. Do not add speculative
    // support.
    const bool kLittleEndian = false;
    const uint8_t kSig[] = {
            'A', 'p', 'p', 'l', 'e', ' ', 'i', 'O', 'S', 0, 0, 1, 'M', 'M',  //
    };
    if (!data || data->size() < sizeof(kSig)) {
        return std::nullopt;
    }
    if (memcmp(data->data(), kSig, sizeof(kSig)) != 0) {
        return std::nullopt;
    }
    auto ifd = SkTiff::ImageFileDirectory::MakeFromOffset(
            std::move(data), kLittleEndian, sizeof(kSig));
    if (!ifd) {
        return std::nullopt;
    }

    // See documentation at:
    // https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos
    bool hasMaker33 = false;
    bool hasMaker48 = false;
    float maker33 = 0.f;
    float maker48 = 0.f;
    for (uint32_t i = 0; i < ifd->getNumEntries(); ++i) {
        switch (ifd->getEntryTag(i)) {
            case 33:
                if (!hasMaker33) {
                    hasMaker33 = ifd->getEntrySignedRational(i, 1, &maker33);
                }
                break;
            case 48:
                if (!hasMaker48) {
                    hasMaker48 = ifd->getEntrySignedRational(i, 1, &maker48);
                }
                break;
            default:
                break;
        }
    }
    // Many images have a maker33 but not a maker48. Treat them as having maker48 of 0.
    if (!hasMaker33) {
        return std::nullopt;
    }
    float stops = 0.f;
    if (maker33 < 1.0f) {
        if (maker48 <= 0.01f) {
            stops = -20.0f * maker48 + 1.8f;
        } else {
            stops = -0.101f * maker48 + 1.601f;
        }
    } else {
        if (maker48 <= 0.01f) {
            stops = -70.0f * maker48 + 3.0f;
        } else {
            stops = -0.303f * maker48 + 2.303f;
        }
    }
    return std::pow(2.f, std::max(stops, 0.f));
}

static void parse_ifd(Metadata& exif,
                      sk_sp<SkData> data,
                      std::unique_ptr<SkTiff::ImageFileDirectory> ifd,
                      bool littleEndian,
                      bool isRoot) {
    if (!ifd) {
        return;
    }
    for (uint32_t i = 0; i < ifd->getNumEntries(); ++i) {
        switch (ifd->getEntryTag(i)) {
            case kOriginTag: {
                uint16_t value = 0;
                if (!exif.fOrigin.has_value() && ifd->getEntryUnsignedShort(i, 1, &value)) {
                    if (0 < value && value <= kLast_SkEncodedOrigin) {
                        exif.fOrigin = static_cast<SkEncodedOrigin>(value);
                    }
                }
                break;
            }
            case kMarkerNoteTag:
                if (!exif.fHdrHeadroom.has_value()) {
                    if (auto makerNoteData = ifd->getEntryUndefinedData(i)) {
                        exif.fHdrHeadroom = get_maker_note_hdr_headroom(std::move(makerNoteData));
                    }
                }
                break;
            case kSubIFDOffsetTag: {
                uint32_t subIfdOffset = 0;
                if (isRoot && ifd->getEntryUnsignedLong(i, 1, &subIfdOffset)) {
                    auto subIfd = SkTiff::ImageFileDirectory::MakeFromOffset(
                            data, littleEndian, subIfdOffset, /*allowTruncated=*/true);
                    parse_ifd(exif,
                              data,
                              std::move(subIfd),
                              littleEndian,
                              /*isRoot=*/false);
                }
                break;
            }
            case kXResolutionTag: {
                float value = 0.f;
                if (!exif.fXResolution.has_value() && ifd->getEntryUnsignedRational(i, 1, &value)) {
                    exif.fXResolution = value;
                }
                break;
            }
            case kYResolutionTag: {
                float value = 0.f;
                if (!exif.fYResolution.has_value() && ifd->getEntryUnsignedRational(i, 1, &value)) {
                    exif.fYResolution = value;
                }
                break;
            }
            case kResolutionUnitTag: {
                uint16_t value = 0;
                if (!exif.fResolutionUnit.has_value() && ifd->getEntryUnsignedShort(i, 1, &value)) {
                    exif.fResolutionUnit = value;
                }
                break;
            }
            case kPixelXDimensionTag: {
                // The type for this tag can be unsigned short or unsigned long (as per the Exif 2.3
                // spec, aka CIPA DC-008-2012). Support for unsigned long was added in
                // https://crrev.com/817600.
                uint16_t value16 = 0;
                if (!exif.fPixelXDimension.has_value() &&
                    ifd->getEntryUnsignedShort(i, 1, &value16)) {
                    exif.fPixelXDimension = value16;
                }
                uint32_t value32 = 0;
                if (!exif.fPixelXDimension.has_value() &&
                    ifd->getEntryUnsignedLong(i, 1, &value32)) {
                    exif.fPixelXDimension = value32;
                }
                break;
            }
            case kPixelYDimensionTag: {
                uint16_t value16 = 0;
                if (!exif.fPixelYDimension.has_value() &&
                    ifd->getEntryUnsignedShort(i, 1, &value16)) {
                    exif.fPixelYDimension = value16;
                }
                uint32_t value32 = 0;
                if (!exif.fPixelYDimension.has_value() &&
                    ifd->getEntryUnsignedLong(i, 1, &value32)) {
                    exif.fPixelYDimension = value32;
                }
                break;
            }
            default:
                break;
        }
    }
}

void Parse(Metadata& metadata, const SkData* data) {
    bool littleEndian = false;
    uint32_t ifdOffset = 0;
    if (data && SkTiff::ImageFileDirectory::ParseHeader(data, &littleEndian, &ifdOffset)) {
        auto dataRef = SkData::MakeWithoutCopy(data->data(), data->size());
        auto ifd = SkTiff::ImageFileDirectory::MakeFromOffset(
                dataRef, littleEndian, ifdOffset, /*allowTruncated=*/true);
        parse_ifd(metadata, std::move(dataRef), std::move(ifd), littleEndian, /*isRoot=*/true);
    }
}

// Helper function to write a single IFD entry.
bool write_entry(uint16_t tag, uint16_t type, uint32_t count, uint32_t value,
                uint32_t* endOfData, SkWStream* stream, SkWStream* buffer) {
    bool success = true;
    success &= SkWStreamWriteU16BE(stream, tag);
    success &= SkWStreamWriteU16BE(stream, type);
    success &= SkWStreamWriteU32BE(stream, count);
    switch (tag) {
      case kOriginTag:
      case kResolutionUnitTag:
        success &= SkWStreamWriteU16BE(stream, value);
        success &= SkWStreamWriteU16BE(stream, 0); // Complete the IFD entry.
        return success;
      case kPixelXDimensionTag:
      case kPixelYDimensionTag:
        success &= SkWStreamWriteU32BE(stream, value);
        return success;
      case kXResolutionTag:
      case kYResolutionTag:
        // If the number of bytes for the type of the entry is greater than 4, we have
        // to append the value to the end and replace the value section with an offset
        // to where the data can be found.
        success &= SkWStreamWriteU32BE(stream, *endOfData);
        *endOfData += 8;
        success &= SkWStreamWriteU32BE(buffer, value); // Numerator
        success &= SkWStreamWriteU32BE(buffer, 1); // Denominator
        return success;
      case kSubIFDOffsetTag:
        // This does not write the subIFD itself, just the IFD0 entry that points
        // to where it is located.
        success &= SkWStreamWriteU32BE(stream, value);
        return success;
      default:
        return false;
    }
}

sk_sp<SkData> WriteExif(Metadata& metadata) {
    // Cannot write an IFD entry for MakerNote from the HDR Headroom. Information
    // about maker48 and maker33 is lost in encode.
    // See documentation at:
    // https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos
    if (metadata.fHdrHeadroom.has_value()) {
        SkCodecPrintf("Cannot encode maker noter from the headroom value.\n");
        return nullptr;
    }

    SkDynamicMemoryWStream stream;
    // If there exists metadata that belongs in a subIFD, we will write that to a
    // separate stream and append it to the end of the data, before |bufferForLargerValues|.
    bool subIFDExists = false;
    // This buffer will hold the values that are more than 4 bytes and will be
    // appended to the end of the data after going through all available fields.
    SkDynamicMemoryWStream bufferForLargerValues;
    constexpr uint32_t kOffset = 8;

    // Write the IFD header.
    if (!stream.write(SkTiff::kEndianBig, sizeof(SkTiff::kEndianBig))) {
      return nullptr;
    }
    // Offset of index IFD.
    if (!SkWStreamWriteU32BE(&stream, kOffset)) {
        return nullptr;
    }
    // Count the number of valid metadata entries.
    uint16_t numTags = 0;
    uint16_t numSubIFDTags = 0;
    if (metadata.fOrigin.has_value()) numTags++;
    if (metadata.fResolutionUnit.has_value()) numTags++;
    if (metadata.fXResolution.has_value()) numTags++;
    if (metadata.fYResolution.has_value()) numTags++;
    if (metadata.fPixelXDimension.has_value()) numSubIFDTags++;
    if (metadata.fPixelYDimension.has_value()) numSubIFDTags++;
    if (numSubIFDTags > 0) {
      subIFDExists = true;
      numTags++;
    }

    // Offset that represents where data will be appended.
    uint32_t endOfData = kOffset
                        + SkTiff::kSizeShort  // Number of tags
                        + (SkTiff::kSizeEntry * numTags) // Entries
                        + SkTiff::kSizeLong; // Next IFD offset
    // Offset that represents where the subIFD will start if it exists.
    const uint32_t kSubIfdOffset = endOfData;
    if (subIFDExists) {
      endOfData += SkTiff::kSizeShort  // Number of subIFD tags
                  + (SkTiff::kSizeEntry * numSubIFDTags) // SubIFD entries
                  + SkTiff::kSizeLong; // SubIFD next offset;
    }

    // Write the number of tags in the IFD.
    SkWStreamWriteU16BE(&stream, numTags);

    // Write the IFD entries.
    if (metadata.fOrigin.has_value()
        && !write_entry(kOriginTag, SkTiff::kTypeUnsignedShort, 1,
                        metadata.fOrigin.value(), &endOfData, &stream,
                        &bufferForLargerValues)) {
          return nullptr;
        }

    if (metadata.fResolutionUnit.has_value()
        && !write_entry(kResolutionUnitTag, SkTiff::kTypeUnsignedShort, 1,
                        metadata.fResolutionUnit.value(), &endOfData, &stream,
                        &bufferForLargerValues)) {
          return nullptr;
        }

    if (metadata.fXResolution.has_value()
        && !write_entry(kXResolutionTag, SkTiff::kTypeUnsignedRational, 1,
                        metadata.fXResolution.value(), &endOfData, &stream,
                        &bufferForLargerValues)) {
          return nullptr;
        }

    if (metadata.fYResolution.has_value()
        && !write_entry(kYResolutionTag, SkTiff::kTypeUnsignedRational, 1,
                        metadata.fYResolution.value(), &endOfData, &stream,
                        &bufferForLargerValues)) {
          return nullptr;
        }

    if (subIFDExists && !write_entry(kSubIFDOffsetTag, SkTiff::kTypeUnsignedLong, 1,
                      kSubIfdOffset, &endOfData, &stream, &bufferForLargerValues)) {
          return nullptr;
        }

    // Next IFD offset (0 for no next IFD).
    if (!SkWStreamWriteU32BE(&stream, 0)) {
        return nullptr;
    }

    // After all IFD0 data has been written, then write the SubIFD (ExifIFD).
    if (subIFDExists) {
      // Write the number of tags in the subIFD.
      if (!SkWStreamWriteU16BE(&stream, numSubIFDTags)) {
        return nullptr;
      }

      if (metadata.fPixelXDimension.has_value()
          && !write_entry(kPixelXDimensionTag, SkTiff::kTypeUnsignedLong, 1,
                          metadata.fPixelXDimension.value(), &endOfData, &stream,
                          &bufferForLargerValues)) {
            return nullptr;
          }

      if (metadata.fPixelYDimension.has_value()
          && !write_entry(kPixelYDimensionTag, SkTiff::kTypeUnsignedLong, 1,
                          metadata.fPixelYDimension.value(), &endOfData, &stream,
                          &bufferForLargerValues)) {
            return nullptr;
          }

      // Write the SubIFD next offset (0).
      if (!SkWStreamWriteU32BE(&stream, 0)) {
        return nullptr;
      }
    }

    // Append the data buffer to the end of the stream.
    if (!bufferForLargerValues.writeToStream(&stream)) {
        return nullptr;
    }

    return stream.detachAsData();
}

}  // namespace SkExif

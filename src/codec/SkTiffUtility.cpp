/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkTiffUtility.h"

#include "include/core/SkData.h"
#include "src/codec/SkCodecPriv.h"

#include <cstddef>
#include <utility>

namespace SkTiff {

bool ImageFileDirectory::IsValidType(uint16_t type) { return type >= 1 && type <= 12; }

size_t ImageFileDirectory::BytesForType(uint16_t type) {
    switch (type) {
        case kTypeUnsignedByte:
            return 1;
        case kTypeAsciiString:
            return 1;
        case kTypeUnsignedShort:
            return kSizeShort;
        case kTypeUnsignedLong:
            return kSizeLong;
        case kTypeUnsignedRational:
            return 8;
        case kTypeSignedByte:
            return 1;
        case kTypeUndefined:
            return 1;
        case kTypeSignedShort:
            return kSizeShort;
        case kTypeSignedLong:
            return kSizeLong;
        case kTypeSignedRational:
            return 8;
        case kTypeSingleFloat:
            return 4;
        case kTypeDoubleFloat:
            return 8;
    }
    return 0;
}

// Helper function for computing the address of an entry.
static const uint8_t* get_entry_address(const SkData* data,
                                        uint32_t ifdOffset,
                                        uint16_t entryIndex) {
    return data->bytes() +           // Base address
           ifdOffset +               // IFD offset
           kSizeShort +              // IFD number of entries
           kSizeEntry * entryIndex;  // Entries
}

// Return true if the IFD starting at |ifdOffset| contains valid number of entries (that doesn't
// overrun |data|).
static bool validate_ifd(const SkData* data,
                         bool littleEndian,
                         uint32_t ifdOffset,
                         bool allowTruncated,
                         uint16_t* outNumEntries,
                         uint32_t* outNextIfdOffset) {
    const uint8_t* dataCurrent = data->bytes();
    size_t dataSize = data->size();

    // Seek to the IFD offset.
    if (dataSize < ifdOffset) {
        SkCodecPrintf("IFD offset is too large.\n");
        return false;
    }
    dataCurrent += ifdOffset;
    dataSize -= ifdOffset;

    // Read the number of entries.
    if (dataSize < kSizeShort) {
        SkCodecPrintf("Insufficient space to store number of entries.\n");
        return false;
    }
    uint16_t numEntries = SkCodecPriv::GetEndianShort(dataCurrent, littleEndian);
    dataCurrent += kSizeShort;
    dataSize -= kSizeShort;

    // Check that there is enough space for all entries.
    if (dataSize < kSizeEntry * numEntries) {
        SkCodecPrintf("Insufficient space (%u) to store all %u entries.\n",
                      static_cast<uint32_t>(data->size()),
                      numEntries);
        if (allowTruncated) {
            // Set the number of entries to the number of entries that can be fully read, and set
            // the next IFD offset to 0 (indicating that there is no next IFD).
            *outNumEntries = dataSize / kSizeEntry;
            *outNextIfdOffset = 0;
            return true;
        }
        return false;
    }

    // Save the number of entries.
    *outNumEntries = numEntries;

    // Seek past the entries.
    dataCurrent += kSizeEntry * numEntries;
    dataSize -= kSizeEntry * numEntries;

    // Read the next IFD offset.
    if (dataSize < kSizeLong) {
        SkCodecPrintf("Insufficient space to store next IFD offset.\n");
        if (allowTruncated) {
            // Set the next IFD offset to 0 (indicating that there is no next IFD).
            *outNextIfdOffset = 0;
            return true;
        }
        return false;
    }

    // Save the next IFD offset.
    *outNextIfdOffset = SkCodecPriv::GetEndianInt(dataCurrent, littleEndian);
    return true;
}

bool ImageFileDirectory::ParseHeader(const SkData* data,
                                     bool* outLittleEndian,
                                     uint32_t* outIfdOffset) {
    // Read the endianness (4 bytes) and IFD offset (4 bytes).
    if (data->size() < 8) {
        SkCodecPrintf("Tiff header must be at least 8 bytes.\n");
        return false;
    }
    if (!SkCodecPriv::IsValidEndianMarker(data->bytes(), outLittleEndian)) {
        SkCodecPrintf("Tiff header had invalid endian marker 0x%x,0x%x,0x%x,0x%x.\n",
                      data->bytes()[0],
                      data->bytes()[1],
                      data->bytes()[2],
                      data->bytes()[3]);
        return false;
    }
    *outIfdOffset = SkCodecPriv::GetEndianInt(data->bytes() + 4, *outLittleEndian);
    return true;
}

std::unique_ptr<ImageFileDirectory> ImageFileDirectory::MakeFromOffset(sk_sp<SkData> data,
                                                                       bool littleEndian,
                                                                       uint32_t ifdOffset,
                                                                       bool allowTruncated) {
    uint16_t numEntries = 0;
    uint32_t nextOffset = 0;
    if (!validate_ifd(
                data.get(), littleEndian, ifdOffset, allowTruncated, &numEntries, &nextOffset)) {
        SkCodecPrintf("Failed to validate IFD.\n");
        return nullptr;
    }
    return std::unique_ptr<ImageFileDirectory>(new ImageFileDirectory(
            std::move(data), littleEndian, ifdOffset, numEntries, nextOffset));
}

ImageFileDirectory::ImageFileDirectory(sk_sp<SkData> data,
                                       bool littleEndian,
                                       uint32_t offset,
                                       uint16_t numEntries,
                                       uint32_t nextIfdOffset)
        : fData(std::move(data))
        , fLittleEndian(littleEndian)
        , fOffset(offset)
        , fNumEntries(numEntries)
        , fNextIfdOffset(nextIfdOffset) {}

uint16_t ImageFileDirectory::getEntryTag(uint16_t entryIndex) const {
    const uint8_t* entry = get_entry_address(fData.get(), fOffset, entryIndex);
    return SkCodecPriv::GetEndianShort(entry, fLittleEndian);
}

bool ImageFileDirectory::getEntryRawData(uint16_t entryIndex,
                                         uint16_t* outTag,
                                         uint16_t* outType,
                                         uint32_t* outCount,
                                         const uint8_t** outData,
                                         size_t* outDataSize) const {
    const uint8_t* entry = get_entry_address(fData.get(), fOffset, entryIndex);

    // Read the tag
    const uint16_t tag = SkCodecPriv::GetEndianShort(entry, fLittleEndian);
    entry += kSizeShort;

    // Read the type.
    const uint16_t type = SkCodecPriv::GetEndianShort(entry, fLittleEndian);
    entry += kSizeShort;
    if (!IsValidType(type)) {
        return false;
    }

    // Read the count.
    const uint32_t count = SkCodecPriv::GetEndianInt(entry, fLittleEndian);
    entry += kSizeLong;

    // If the entry fits in the remaining 4 bytes, use that.
    const size_t entryDataBytes = BytesForType(type) * count;
    const uint8_t* entryData = nullptr;
    if (entryDataBytes <= kSizeLong) {
        entryData = entry;
    } else {
        // Otherwise, the next 4 bytes specify an offset where the data can be found.
        const uint32_t entryDataOffset = SkCodecPriv::GetEndianInt(entry, fLittleEndian);
        if (fData->size() < entryDataOffset || fData->size() - entryDataOffset < entryDataBytes) {
            return false;
        }
        entryData = fData->bytes() + entryDataOffset;
    }

    if (outTag) *outTag = tag;
    if (outType) *outType = type;
    if (outCount) *outCount = count;
    if (outData) *outData = entryData;
    if (outDataSize) *outDataSize = entryDataBytes;
    return true;
}

sk_sp<SkData> ImageFileDirectory::getEntryUndefinedData(uint16_t entryIndex) const {
    uint16_t type = 0;
    uint32_t count = 0;
    const uint8_t* data = nullptr;
    size_t size = 0;
    if (!getEntryRawData(entryIndex, nullptr, &type, &count, &data, &size)) {
        return nullptr;
    }
    if (type != kTypeUndefined) {
        return nullptr;
    }
    return SkData::MakeSubset(fData.get(), data - fData->bytes(), size);
}

bool ImageFileDirectory::getEntryValuesGeneric(uint16_t entryIndex,
                                               uint16_t type,
                                               uint32_t count,
                                               void* values) const {
    uint16_t entryType = 0;
    uint32_t entryCount = 0;
    const uint8_t* entryData = nullptr;
    if (!getEntryRawData(entryIndex, nullptr, &entryType, &entryCount, &entryData, nullptr)) {
        return false;
    }
    if (type != entryType) {
        return false;
    }
    if (count != entryCount) {
        return false;
    }
    for (uint32_t i = 0; i < count; ++i) {
        const uint8_t* data = entryData + i * BytesForType(kTypeUnsignedLong);
        switch (type) {
            case kTypeUnsignedShort:
                reinterpret_cast<uint16_t*>(values)[i] =
                        SkCodecPriv::GetEndianShort(data, fLittleEndian);
                break;
            case kTypeUnsignedLong:
                reinterpret_cast<uint32_t*>(values)[i] =
                        SkCodecPriv::GetEndianInt(data, fLittleEndian);
                break;
            case kTypeSignedRational: {
                uint32_t numerator = SkCodecPriv::GetEndianInt(data, fLittleEndian);
                uint32_t denominator = SkCodecPriv::GetEndianInt(data + kSizeLong, fLittleEndian);
                if (denominator == 0) {
                    // The TIFF specification does not indicate a behavior when the denominator is
                    // zero.  The behavior of returning zero for a denominator of zero is a
                    // preservation of the behavior introduced in https://crrev.com/767874.
                    reinterpret_cast<float*>(values)[i] = 0;
                } else {
                    reinterpret_cast<float*>(values)[i] =
                            numerator / static_cast<float>(denominator);
                }
                break;
            }
            case kTypeUnsignedRational: {
                uint32_t numerator = SkCodecPriv::GetEndianInt(data, fLittleEndian);
                uint32_t denominator = SkCodecPriv::GetEndianInt(data + kSizeLong, fLittleEndian);
                if (denominator == 0) {
                    // See comments in kTypeSignedRational.
                    reinterpret_cast<float*>(values)[i] = 0.f;
                } else {
                    reinterpret_cast<float*>(values)[i] =
                            numerator / static_cast<float>(denominator);
                }
                break;
            }
            default:
                SkCodecPrintf("Unsupported type %u\n", type);
                return false;
                break;
        }
    }
    return true;
}

}  // namespace SkTiff

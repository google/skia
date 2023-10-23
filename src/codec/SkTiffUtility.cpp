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

constexpr size_t kSizeEntry = 12;
constexpr size_t kSizeShort = 2;
constexpr size_t kSizeLong = 4;

bool SkTiffImageFileDirectory::IsValidType(uint16_t type) { return type >= 1 && type <= 12; }

size_t SkTiffImageFileDirectory::BytesForType(uint16_t type) {
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
    uint16_t numEntries = get_endian_short(dataCurrent, littleEndian);
    dataCurrent += kSizeShort;
    dataSize -= kSizeShort;

    // Check that there is enough space for all entries.
    if (dataSize < kSizeEntry * numEntries) {
        SkCodecPrintf("Insufficient space (%u) to store all %u entries.\n",
                      static_cast<uint32_t>(data->size()),
                      numEntries);
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
        return false;
    }

    // Save the next IFD offset.
    *outNextIfdOffset = get_endian_int(dataCurrent, littleEndian);
    return true;
}

bool SkTiffImageFileDirectory::ParseHeader(const SkData* data,
                                           bool* outLittleEndian,
                                           uint32_t* outIfdOffset) {
    // Read the endianness (4 bytes) and IFD offset (4 bytes).
    if (data->size() < 8) {
        SkCodecPrintf("Tiff header must be at least 8 bytes.\n");
        return false;
    }
    if (!is_valid_endian_marker(data->bytes(), outLittleEndian)) {
        SkCodecPrintf("Tiff header had invalid endian marker 0x%x,0x%x,0x%x,0x%x.\n",
                      data->bytes()[0],
                      data->bytes()[1],
                      data->bytes()[2],
                      data->bytes()[3]);
        return false;
    }
    *outIfdOffset = get_endian_int(data->bytes() + 4, *outLittleEndian);
    return true;
}

std::unique_ptr<SkTiffImageFileDirectory> SkTiffImageFileDirectory::MakeFromOffset(
        sk_sp<SkData> data, bool littleEndian, uint32_t ifdOffset) {
    uint16_t numEntries = 0;
    uint32_t nextOffset = 0;
    if (!validate_ifd(data.get(), littleEndian, ifdOffset, &numEntries, &nextOffset)) {
        SkCodecPrintf("Failed to validate IFD.\n");
        return nullptr;
    }
    return std::unique_ptr<SkTiffImageFileDirectory>(new SkTiffImageFileDirectory(
            std::move(data), littleEndian, ifdOffset, numEntries, nextOffset));
}

SkTiffImageFileDirectory::SkTiffImageFileDirectory(sk_sp<SkData> data,
                                                   bool littleEndian,
                                                   uint32_t offset,
                                                   uint16_t numEntries,
                                                   uint32_t nextIfdOffset)
        : fData(std::move(data))
        , fLittleEndian(littleEndian)
        , fOffset(offset)
        , fNumEntries(numEntries)
        , fNextIfdOffset(nextIfdOffset) {}

uint16_t SkTiffImageFileDirectory::getEntryTag(uint16_t entryIndex) const {
    const uint8_t* entry = get_entry_address(fData.get(), fOffset, entryIndex);
    return get_endian_short(entry, fLittleEndian);
}

bool SkTiffImageFileDirectory::getEntryRawData(uint16_t entryIndex,
                                               uint16_t* outTag,
                                               uint16_t* outType,
                                               uint32_t* outCount,
                                               const uint8_t** outData,
                                               size_t* outDataSize) const {
    const uint8_t* entry = get_entry_address(fData.get(), fOffset, entryIndex);

    // Read the tag
    const uint16_t tag = get_endian_short(entry, fLittleEndian);
    entry += kSizeShort;

    // Read the type.
    const uint16_t type = get_endian_short(entry, fLittleEndian);
    entry += kSizeShort;
    if (!IsValidType(type)) {
        return false;
    }

    // Read the count.
    const uint32_t count = get_endian_int(entry, fLittleEndian);
    entry += kSizeLong;

    // If the entry fits in the remaining 4 bytes, use that.
    const size_t entryDataBytes = BytesForType(type) * count;
    const uint8_t* entryData = nullptr;
    if (entryDataBytes <= kSizeLong) {
        entryData = entry;
    } else {
        // Otherwise, the next 4 bytes specify an offset where the data can be found.
        const uint32_t entryDataOffset = get_endian_int(entry, fLittleEndian);
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

sk_sp<SkData> SkTiffImageFileDirectory::getEntryUndefinedData(uint16_t entryIndex) const {
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

bool SkTiffImageFileDirectory::getEntryValuesGeneric(uint16_t entryIndex,
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
                reinterpret_cast<uint16_t*>(values)[i] = get_endian_short(data, fLittleEndian);
                break;
            case kTypeUnsignedLong:
                reinterpret_cast<uint32_t*>(values)[i] = get_endian_int(data, fLittleEndian);
                break;
            case kTypeSignedRational: {
                uint32_t numerator = get_endian_int(data, fLittleEndian);
                uint32_t denominator = get_endian_int(data + kSizeLong, fLittleEndian);
                reinterpret_cast<float*>(values)[i] = numerator / static_cast<float>(denominator);
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

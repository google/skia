/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/codec/SkCodecPriv.h"

bool SkParseEncodedOrigin(const uint8_t* exifData, size_t data_length, uint64_t offset,
                          bool littleEndian, SkEncodedOrigin* orientation) {
    // Require that the marker is at least large enough to contain the number of entries.
    if (data_length < offset + 2) {
        return false;
    }
    uint32_t numEntries = get_endian_short(exifData + offset, littleEndian);

    // Tag (2 bytes), Datatype (2 bytes), Number of elements (4 bytes), Data (4 bytes)
    const uint32_t kEntrySize = 12;
    const auto max = SkTo<uint32_t>((data_length - offset - 2) / kEntrySize);
    numEntries = std::min(numEntries, max);

    // Advance the data to the start of the entries.
    auto data = exifData;
    data += offset + 2;

    const uint16_t kOriginTag = 0x112;
    const uint16_t kOriginType = 3; // uint16_t
    const uint16_t kSubIFDOffsetTag = 0x8769;
    const uint16_t kSubIFDOffsetType = 4; // uint32_t

    uint32_t subIFDOffset = 0;

    for (uint32_t i = 0; i < numEntries; i++, data += kEntrySize) {
        uint16_t tag = get_endian_short(data, littleEndian);
        uint16_t type = get_endian_short(data + 2, littleEndian);
        uint32_t count = get_endian_int(data + 4, littleEndian);

        if (kOriginTag == tag && kOriginType == type && 1 == count) {
            uint16_t val = get_endian_short(data + 8, littleEndian);
            if (0 < val && val <= kLast_SkEncodedOrigin) {
                *orientation = (SkEncodedOrigin)val;
                return true;
            }
        } else if (kSubIFDOffsetTag == tag && kSubIFDOffsetType == type && 1 == count) {
            uint32_t val = get_endian_int(data + 8, littleEndian);
            if (0 < val && val < data_length) {
                subIFDOffset = val;
            }
        }
    }

    if (subIFDOffset > 0) {
        return SkParseEncodedOrigin(exifData, data_length, subIFDOffset, littleEndian, orientation);
    }

    return false;
}

bool SkParseEncodedOrigin(const uint8_t* data, size_t data_length, SkEncodedOrigin* orientation) {
    SkASSERT(orientation);
    bool littleEndian;
    // We need eight bytes to read the endian marker and the offset, below.
    if (data_length < 8 || !is_valid_endian_marker(data, &littleEndian)) {
        return false;
    }

    // Get the offset from the start of the marker.
    // Though this only reads four bytes, use a larger int in case it overflows.
    uint64_t offset = get_endian_int(data + 4, littleEndian);

    // Start to read the EXIF data from the buffer.
    return SkParseEncodedOrigin(data, data_length, offset, littleEndian, orientation);
}

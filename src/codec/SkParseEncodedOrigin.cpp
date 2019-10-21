/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/codec/SkCodecPriv.h"

bool SkParseEncodedOrigin(const uint8_t* data, size_t data_length, SkEncodedOrigin* orientation) {
    SkASSERT(orientation);
    bool littleEndian;
    // We need eight bytes to read the endian marker and the offset, below.
    if (data_length < 8 || !is_valid_endian_marker(data, &littleEndian)) {
        return false;
    }

    auto getEndianInt = [](const uint8_t* data, bool littleEndian) -> uint32_t {
        if (littleEndian) {
            return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0]);
        }

        return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
    };

    // Get the offset from the start of the marker.
    // Though this only reads four bytes, use a larger int in case it overflows.
    uint64_t offset = getEndianInt(data + 4, littleEndian);

    // Require that the marker is at least large enough to contain the number of entries.
    if (data_length < offset + 2) {
        return false;
    }
    uint32_t numEntries = get_endian_short(data + offset, littleEndian);

    // Tag (2 bytes), Datatype (2 bytes), Number of elements (4 bytes), Data (4 bytes)
    const uint32_t kEntrySize = 12;
    const auto max = SkTo<uint32_t>((data_length - offset - 2) / kEntrySize);
    numEntries = SkTMin(numEntries, max);

    // Advance the data to the start of the entries.
    data += offset + 2;

    const uint16_t kOriginTag = 0x112;
    const uint16_t kOriginType = 3;
    for (uint32_t i = 0; i < numEntries; i++, data += kEntrySize) {
        uint16_t tag = get_endian_short(data, littleEndian);
        uint16_t type = get_endian_short(data + 2, littleEndian);
        uint32_t count = getEndianInt(data + 4, littleEndian);
        if (kOriginTag == tag && kOriginType == type && 1 == count) {
            uint16_t val = get_endian_short(data + 8, littleEndian);
            if (0 < val && val <= kLast_SkEncodedOrigin) {
                *orientation = (SkEncodedOrigin) val;
                return true;
            }
        }
    }

    return false;
}

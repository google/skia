/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSFNTHeader_DEFINED
#define SkSFNTHeader_DEFINED

#include "SkEndian.h"
#include "SkOTTableTypes.h"

//All SK_SFNT_ prefixed types should be considered as big endian.
typedef uint16_t SK_SFNT_USHORT;
typedef uint32_t SK_SFNT_ULONG;

#pragma pack(push, 1)

struct SkSFNTHeader {
    SK_SFNT_ULONG fontType;
    static const SK_OT_ULONG fontType_WindowsTrueType = SkTEndian_SwapBE32(0x00010000);
    static const SK_OT_ULONG fontType_MacTrueType = SkTEndian_SwapBE32('true');
    static const SK_OT_ULONG fontType_PostScript = SkTEndian_SwapBE32('typ1');
    static const SK_OT_ULONG fontType_OpenTypeCFF = SkTEndian_SwapBE32('OTTO');

    SK_SFNT_USHORT numTables;
    SK_SFNT_USHORT searchRange;
    SK_SFNT_USHORT entrySelector;
    SK_SFNT_USHORT rangeShift;
};

struct SkSFNTTableDirectoryEntry {
    SK_SFNT_ULONG tag;
    SK_SFNT_ULONG checksum;
    SK_SFNT_ULONG offset; //From beginning of header.
    SK_SFNT_ULONG logicalLength;
};

#pragma pack(pop)


SK_COMPILE_ASSERT(sizeof(SkSFNTHeader) == 12, sizeof_SkSFNTHeader_not_12);
SK_COMPILE_ASSERT(sizeof(SkSFNTTableDirectoryEntry) == 16, sizeof_SkSFNTTableDirectoryEntry_not_16);

#endif

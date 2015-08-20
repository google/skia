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
    struct fontType_WindowsTrueType {
        static const SK_OT_CHAR TAG0 = 0;
        static const SK_OT_CHAR TAG1 = 1;
        static const SK_OT_CHAR TAG2 = 0;
        static const SK_OT_CHAR TAG3 = 0;
        static const SK_OT_ULONG TAG = SkOTTableTAG<fontType_WindowsTrueType>::value;
    };
    struct fontType_MacTrueType {
        static const SK_OT_CHAR TAG0 = 't';
        static const SK_OT_CHAR TAG1 = 'r';
        static const SK_OT_CHAR TAG2 = 'u';
        static const SK_OT_CHAR TAG3 = 'e';
        static const SK_OT_ULONG TAG = SkOTTableTAG<fontType_MacTrueType>::value;
    };
    struct fontType_PostScript {
        static const SK_OT_CHAR TAG0 = 't';
        static const SK_OT_CHAR TAG1 = 'y';
        static const SK_OT_CHAR TAG2 = 'p';
        static const SK_OT_CHAR TAG3 = '1';
        static const SK_OT_ULONG TAG = SkOTTableTAG<fontType_PostScript>::value;
    };
    struct fontType_OpenTypeCFF {
        static const SK_OT_CHAR TAG0 = 'O';
        static const SK_OT_CHAR TAG1 = 'T';
        static const SK_OT_CHAR TAG2 = 'T';
        static const SK_OT_CHAR TAG3 = 'O';
        static const SK_OT_ULONG TAG = SkOTTableTAG<fontType_OpenTypeCFF>::value;
    };

    SK_SFNT_USHORT numTables;
    SK_SFNT_USHORT searchRange;
    SK_SFNT_USHORT entrySelector;
    SK_SFNT_USHORT rangeShift;

    struct TableDirectoryEntry {
        SK_SFNT_ULONG tag;
        SK_SFNT_ULONG checksum;
        SK_SFNT_ULONG offset; //From beginning of header.
        SK_SFNT_ULONG logicalLength;
    }; //tableDirectoryEntries[numTables]
};

#pragma pack(pop)


static_assert(sizeof(SkSFNTHeader) == 12, "sizeof_SkSFNTHeader_not_12");
static_assert(sizeof(SkSFNTHeader::TableDirectoryEntry) == 16, "sizeof_SkSFNTHeader_TableDirectoryEntry_not_16");

#endif

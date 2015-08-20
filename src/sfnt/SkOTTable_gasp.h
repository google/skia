/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_gasp_DEFINED
#define SkOTTable_gasp_DEFINED

#include "SkEndian.h"
#include "SkOTTableTypes.h"
#include "SkTypedEnum.h"

#pragma pack(push, 1)

struct SkOTTableGridAndScanProcedure {
    static const SK_OT_CHAR TAG0 = 'g';
    static const SK_OT_CHAR TAG1 = 'a';
    static const SK_OT_CHAR TAG2 = 's';
    static const SK_OT_CHAR TAG3 = 'p';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableGridAndScanProcedure>::value;

    SK_OT_USHORT version;
    static const SK_OT_USHORT version0 = SkTEndian_SwapBE16(0);
    static const SK_OT_USHORT version1 = SkTEndian_SwapBE16(1);

    SK_OT_USHORT numRanges;

    struct GaspRange {
        SK_OT_USHORT maxPPEM;
        union behavior {
            struct Field {
                //8-15
                SK_OT_BYTE_BITFIELD(
                    Reserved08,
                    Reserved09,
                    Reserved10,
                    Reserved11,
                    Reserved12,
                    Reserved13,
                    Reserved14,
                    Reserved15)
                //0-7
                SK_OT_BYTE_BITFIELD(
                    Gridfit,
                    DoGray,
                    SymmetricGridfit,  // Version 1
                    SymmetricSmoothing,  // Version 1
                    Reserved04,
                    Reserved05,
                    Reserved06,
                    Reserved07)
            } field;
            struct Raw {
                static const SK_OT_USHORT GridfitMask = SkTEndian_SwapBE16(1 << 0);
                static const SK_OT_USHORT DoGrayMask = SkTEndian_SwapBE16(1 << 1);
                static const SK_OT_USHORT SymmetricGridfitMask = SkTEndian_SwapBE16(1 << 2);
                static const SK_OT_USHORT SymmetricSmoothingMask = SkTEndian_SwapBE16(1 << 3);
                SK_OT_USHORT value;
            } raw;
        } flags;
    }; //gaspRange[numRanges]
};

#pragma pack(pop)


#include <stddef.h>
static_assert(offsetof(SkOTTableGridAndScanProcedure, numRanges) == 2, "SkOTTableGridAndScanProcedure_numRanges_not_at_2");
static_assert(sizeof(SkOTTableGridAndScanProcedure) == 4, "sizeof_SkOTTableGridAndScanProcedure_not_4");

#endif

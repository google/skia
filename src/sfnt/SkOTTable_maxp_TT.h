/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_maxp_TT_DEFINED
#define SkOTTable_maxp_TT_DEFINED

#include "SkEndian.h"
#include "SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableMaximumProfile_TT {
    SK_OT_Fixed version;
    static const SK_OT_Fixed VERSION = SkTEndian_SwapBE32(0x00010000);

    SK_OT_USHORT numGlyphs;
    SK_OT_USHORT maxPoints;
    SK_OT_USHORT maxContours;
    SK_OT_USHORT maxCompositePoints;
    SK_OT_USHORT maxCompositeContours;
    struct MaxZones {
        enum Value : SK_OT_USHORT {
            DoesNotUseTwilightZone = SkTEndian_SwapBE16(1),
            UsesTwilightZone = SkTEndian_SwapBE16(2),
        } value;
    } maxZones;
    SK_OT_USHORT maxTwilightPoints;
    SK_OT_USHORT maxStorage;
    SK_OT_USHORT maxFunctionDefs;
    SK_OT_USHORT maxInstructionDefs;
    SK_OT_USHORT maxStackElements;
    SK_OT_USHORT maxSizeOfInstructions;
    SK_OT_USHORT maxComponentElements;
    SK_OT_USHORT maxComponentDepth;
};

#pragma pack(pop)


#include <stddef.h>
static_assert(offsetof(SkOTTableMaximumProfile_TT, maxComponentDepth) == 30, "SkOTTableMaximumProfile_TT_maxComponentDepth_not_at_30");
static_assert(sizeof(SkOTTableMaximumProfile_TT) == 32, "sizeof_SkOTTableMaximumProfile_TT_not_32");

#endif

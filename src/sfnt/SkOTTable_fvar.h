/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_fvar_DEFINED
#define SkOTTable_fvar_DEFINED

#include "src/base/SkEndian.h"
#include "src/sfnt/SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableFontVariations {
    static const SK_OT_CHAR TAG0 = 'f';
    static const SK_OT_CHAR TAG1 = 'v';
    static const SK_OT_CHAR TAG2 = 'a';
    static const SK_OT_CHAR TAG3 = 'r';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableFontVariations>::value;

    SK_OT_USHORT majorVersion;
    SK_OT_USHORT minorVersion;
    SK_OT_USHORT offsetToAxesArray;
    SK_OT_USHORT reserved;
    SK_OT_USHORT axisCount;
    SK_OT_USHORT axisSize; // Must be 0x0014 in v1.0
    SK_OT_USHORT instanceCount;
    SK_OT_USHORT instanceSize; // Must be axisCount * sizeof(Fixed) + (4 | 6)

    struct VariationAxisRecord {
        SK_OT_ULONG axisTag;
        SK_OT_Fixed minValue;
        SK_OT_Fixed defaultValue;
        SK_OT_Fixed maxValue;
        SK_OT_USHORT flags; // Must be 0
        SK_OT_USHORT axisNameID;
    }; // axes[axisCount];

    template <size_t AxisCount> struct InstanceRecord {
        SK_OT_USHORT subfamilyNameID;
        SK_OT_USHORT flags; // Must be 0
        SK_OT_Fixed coordinates[AxisCount];
        SK_OT_USHORT postScriptNameID;
    }; // instances[instanceCount];
};

#pragma pack(pop)


#include <stddef.h>
static_assert(offsetof(SkOTTableFontVariations, instanceSize) == 14, "SkOTTableFontVariations_instanceSize_not_at_14");
static_assert(sizeof(SkOTTableFontVariations) == 16, "sizeof_SkOTTableFontVariations_not_16");

#endif

/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_EBSC_DEFINED
#define SkOTTable_EBSC_DEFINED

#include "SkEndian.h"
#include "SkOTTable_EBLC.h"
#include "SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableEmbeddedBitmapScaling {
    static const SK_OT_CHAR TAG0 = 'E';
    static const SK_OT_CHAR TAG1 = 'S';
    static const SK_OT_CHAR TAG2 = 'B';
    static const SK_OT_CHAR TAG3 = 'C';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableEmbeddedBitmapScaling>::value;

    SK_OT_Fixed version;
    static const SK_OT_Fixed version_initial = SkTEndian_SwapBE32(0x00020000);

    SK_OT_ULONG numSizes;

    struct BitmapScaleTable {
        SkOTTableEmbeddedBitmapLocation::SbitLineMetrics hori;
        SkOTTableEmbeddedBitmapLocation::SbitLineMetrics vert;
        SK_OT_BYTE ppemX; //target horizontal pixels per EM
        SK_OT_BYTE ppemY; //target vertical pixels per EM
        SK_OT_BYTE substitutePpemX; //use bitmaps of this size
        SK_OT_BYTE substitutePpemY; //use bitmaps of this size
    }; //bitmapScaleTable[numSizes];
};

#pragma pack(pop)

#endif

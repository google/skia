/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_EBDT_DEFINED
#define SkOTTable_EBDT_DEFINED

#include "SkEndian.h"
#include "SkOTTableTypes.h"
#include "SkOTTable_head.h"
#include "SkOTTable_loca.h"
#include "SkTypedEnum.h"

#pragma pack(push, 1)

struct SkOTTableEmbeddedBitmapData {
    static const SK_OT_CHAR TAG0 = 'E';
    static const SK_OT_CHAR TAG1 = 'B';
    static const SK_OT_CHAR TAG2 = 'D';
    static const SK_OT_CHAR TAG3 = 'T';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableEmbeddedBitmapData>::value;

    SK_OT_Fixed version;
    static const SK_OT_Fixed version_initial = SkTEndian_SwapBE32(0x00020000);

    struct BigGlyphMetrics {
        SK_OT_BYTE height;
        SK_OT_BYTE width;
        SK_OT_CHAR horiBearingX;
        SK_OT_CHAR horiBearingY;
        SK_OT_BYTE horiAdvance;
        SK_OT_CHAR vertBearingX;
        SK_OT_CHAR vertBearingY;
        SK_OT_BYTE vertAdvance;
    };

    struct SmallGlyphMetrics {
        SK_OT_BYTE height;
        SK_OT_BYTE width;
        SK_OT_CHAR bearingX;
        SK_OT_CHAR bearingY;
        SK_OT_BYTE advance;
    };

    // Small metrics, byte-aligned data.
    struct Format1 {
        SmallGlyphMetrics smallGlyphMetrics;
        //SK_OT_BYTE[] byteAlignedBitmap;
    };

    // Small metrics, bit-aligned data.
    struct Format2 {
        SmallGlyphMetrics smallGlyphMetrics;
        //SK_OT_BYTE[] bitAlignedBitmap;
    };

    // Format 3 is not used.

    // EBLC metrics (IndexSubTable::header::indexFormat 2 or 5), compressed data.
    // Only used on Mac.
    struct Format4 {
        SK_OT_ULONG whiteTreeOffset;
        SK_OT_ULONG blackTreeOffset;
        SK_OT_ULONG glyphDataOffset;
    };

    // EBLC metrics (IndexSubTable::header::indexFormat 2 or 5), bit-aligned data.
    struct Format5 {
        //SK_OT_BYTE[] bitAlignedBitmap;
    };

    // Big metrics, byte-aligned data.
    struct Format6 {
        BigGlyphMetrics bigGlyphMetrics;
        //SK_OT_BYTE[] byteAlignedBitmap;
    };

    // Big metrics, bit-aligned data.
    struct Format7 {
        BigGlyphMetrics bigGlyphMetrics;
        //SK_OT_BYTE[] bitAlignedBitmap;
    };

    struct EBDTComponent {
        SK_OT_USHORT glyphCode; // Component glyph code
        SK_OT_CHAR xOffset; // Position of component left
        SK_OT_CHAR yOffset; // Position of component top
    };

    struct Format8 {
        SmallGlyphMetrics smallMetrics; // Metrics information for the glyph
        SK_OT_BYTE pad; // Pad to short boundary
        SK_OT_USHORT numComponents; // Number of components
        //EBDTComponent componentArray[numComponents]; // Glyph code, offset array
    };

    struct Format9 {
        BigGlyphMetrics bigMetrics; // Metrics information for the glyph
        SK_OT_USHORT numComponents; // Number of components
        //EBDTComponent componentArray[numComponents]; // Glyph code, offset array
    };
};

#pragma pack(pop)

#endif

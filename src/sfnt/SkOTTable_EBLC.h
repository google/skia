/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_EBLC_DEFINED
#define SkOTTable_EBLC_DEFINED

#include "SkEndian.h"
#include "SkOTTable_EBDT.h"
#include "SkOTTableTypes.h"
#include "SkTypedEnum.h"

#pragma pack(push, 1)

struct SkOTTableEmbeddedBitmapLocation {
    static const SK_OT_CHAR TAG0 = 'E';
    static const SK_OT_CHAR TAG1 = 'B';
    static const SK_OT_CHAR TAG2 = 'L';
    static const SK_OT_CHAR TAG3 = 'C';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableEmbeddedBitmapLocation>::value;

    SK_OT_Fixed version;
    static const SK_OT_Fixed version_initial = SkTEndian_SwapBE32(0x00020000);

    SK_OT_ULONG numSizes;

    struct SbitLineMetrics {
        SK_OT_CHAR ascender;
        SK_OT_CHAR descender;
        SK_OT_BYTE widthMax;
        SK_OT_CHAR caretSlopeNumerator;
        SK_OT_CHAR caretSlopeDenominator;
        SK_OT_CHAR caretOffset;
        SK_OT_CHAR minOriginSB;
        SK_OT_CHAR minAdvanceSB;
        SK_OT_CHAR maxBeforeBL;
        SK_OT_CHAR minAfterBL;
        SK_OT_CHAR pad1;
        SK_OT_CHAR pad2;
    };

    struct BitmapSizeTable {
        SK_OT_ULONG indexSubTableArrayOffset; //offset to indexSubtableArray from beginning of EBLC.
        SK_OT_ULONG indexTablesSize; //number of bytes in corresponding index subtables and array
        SK_OT_ULONG numberOfIndexSubTables; //an index subtable for each range or format change
        SK_OT_ULONG colorRef; //not used; set to 0.
        SbitLineMetrics hori; //line metrics for text rendered horizontally
        SbitLineMetrics vert; //line metrics for text rendered vertically
        SK_OT_USHORT startGlyphIndex; //lowest glyph index for this size
        SK_OT_USHORT endGlyphIndex; //highest glyph index for this size
        SK_OT_BYTE ppemX; //horizontal pixels per Em
        SK_OT_BYTE ppemY; //vertical pixels per Em
        struct BitDepth {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((BW, 1))
                ((Gray4, 2))
                ((Gray16, 4))
                ((Gray256, 8))
                SK_SEQ_END,
            SK_SEQ_END)
            SK_OT_BYTE value;
        } bitDepth; //the Microsoft rasterizer v.1.7 or greater supports
        union Flags {
            struct Field {
                //0-7
                SK_OT_BYTE_BITFIELD(
                    Horizontal, // Horizontal small glyph metrics
                    Vertical,  // Vertical small glyph metrics
                    Reserved02,
                    Reserved03,
                    Reserved04,
                    Reserved05,
                    Reserved06,
                    Reserved07)
            } field;
            struct Raw {
                static const SK_OT_CHAR Horizontal = 1u << 0;
                static const SK_OT_CHAR Vertical = 1u << 1;
                SK_OT_CHAR value;
            } raw;
        } flags;
    }; //bitmapSizeTable[numSizes];

    struct IndexSubTableArray {
        SK_OT_USHORT firstGlyphIndex; //first glyph code of this range
        SK_OT_USHORT lastGlyphIndex; //last glyph code of this range (inclusive)
        SK_OT_ULONG additionalOffsetToIndexSubtable; //add to BitmapSizeTable::indexSubTableArrayOffset to get offset from beginning of 'EBLC'
    }; //indexSubTableArray[BitmapSizeTable::numberOfIndexSubTables];

    struct IndexSubHeader {
        SK_OT_USHORT indexFormat; //format of this indexSubTable
        SK_OT_USHORT imageFormat; //format of 'EBDT' image data
        SK_OT_ULONG imageDataOffset; //offset to image data in 'EBDT' table
    };

    // Variable metrics glyphs with 4 byte offsets
    struct IndexSubTable1 {
        IndexSubHeader header;
        //SK_OT_ULONG offsetArray[lastGlyphIndex - firstGlyphIndex + 1 + 1]; //last element points to one past end of last glyph
        //glyphData = offsetArray[glyphIndex - firstGlyphIndex] + imageDataOffset
    };

    // All Glyphs have identical metrics
    struct IndexSubTable2 {
        IndexSubHeader header;
        SK_OT_ULONG imageSize; // all glyphs are of the same size
        SkOTTableEmbeddedBitmapData::BigGlyphMetrics bigMetrics; // all glyphs have the same metrics; glyph data may be compressed, byte-aligned, or bit-aligned
    };

    // Variable metrics glyphs with 2 byte offsets
    struct IndexSubTable3 {
        IndexSubHeader header;
        //SK_OT_USHORT offsetArray[lastGlyphIndex - firstGlyphIndex + 1 + 1]; //last element points to one past end of last glyph, may have extra element to force even number of elements
        //glyphData = offsetArray[glyphIndex - firstGlyphIndex] + imageDataOffset
    };

    // Variable metrics glyphs with sparse glyph codes
    struct IndexSubTable4 {
        IndexSubHeader header;
        SK_OT_ULONG numGlyphs;
        struct CodeOffsetPair {
            SK_OT_USHORT glyphCode;
            SK_OT_USHORT offset; //location in EBDT
        }; //glyphArray[numGlyphs+1]
    };

    // Constant metrics glyphs with sparse glyph codes
    struct IndexSubTable5 {
        IndexSubHeader header;
        SK_OT_ULONG imageSize; //all glyphs have the same data size
        SkOTTableEmbeddedBitmapData::BigGlyphMetrics bigMetrics; //all glyphs have the same metrics
        SK_OT_ULONG numGlyphs;
        //SK_OT_USHORT glyphCodeArray[numGlyphs] //must have even number of entries (set pad to 0)
    };

    union IndexSubTable {
        IndexSubHeader header;
        IndexSubTable1 format1;
        IndexSubTable2 format2;
        IndexSubTable3 format3;
        IndexSubTable4 format4;
        IndexSubTable5 format5;
    };

};

#pragma pack(pop)

#endif

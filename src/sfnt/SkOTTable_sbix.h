/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_sbix_DEFINED
#define SkOTTable_sbix_DEFINED

#include "include/private/base/SkTemplates.h"
#include "src/base/SkUtils.h"
#include "src/sfnt/SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableGlyphData;

struct SkOTTableStandardBitmapGraphics {
    static const SK_OT_CHAR TAG0 = 's';
    static const SK_OT_CHAR TAG1 = 'b';
    static const SK_OT_CHAR TAG2 = 'i';
    static const SK_OT_CHAR TAG3 = 'x';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableStandardBitmapGraphics>::value;

    SK_OT_USHORT version; // 1
    SK_OT_USHORT flags;	// Bit 0: 1. Bit 1: Draw outlines. Bits 2 to 15: reserved (set to 0)
    SK_OT_ULONG numStrikes;
    //SK_OT_ULONG strikeOffsets[/*numStrikes*/]; // offset from sbix table to Strike
    SK_OT_ULONG strikeOffset(int strikeIndex) {
        return sk_unaligned_load<SK_OT_ULONG>(
            SkTAddOffset<void*>(&numStrikes, sizeof(numStrikes)+sizeof(SK_OT_ULONG)*strikeIndex));
    }

    struct Strike {
        SK_OT_USHORT ppem; // pixels for em
        SK_OT_USHORT ppi; // design pixel density
        //SK_OT_ULONG glyphDataOffsets[/*numGlyphs+1*/]; // offset from Strike to GlyphData
        SK_OT_ULONG glyphDataOffset(int glyphId) {
            return sk_unaligned_load<SK_OT_ULONG>(
                SkTAddOffset<void*>(&ppi, sizeof(ppi)+sizeof(SK_OT_ULONG)*glyphId));
        }
    };

    struct GlyphData {
        SK_OT_SHORT originOffsetX; // x offset of bitmap in pixels
        SK_OT_SHORT originOffsetY; // y offset of bitmap in pixels (y-up)
        SK_OT_ULONG graphicType; // 'jpg ', 'png ', 'tiff', or 'dupe'
        //SK_OT_BYTE data[]; // length is to next glyphDataOffsets entry
        SK_OT_BYTE* data() { return SkTAfter<SK_OT_BYTE>(&graphicType); }
        const SK_OT_BYTE* data() const { return SkTAfter<const SK_OT_BYTE>(&graphicType); }
    };
};

#pragma pack(pop)

#endif

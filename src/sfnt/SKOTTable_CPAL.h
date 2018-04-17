/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_CPAL_DEFINED
#define SkOTTable_CPAL_DEFINED

#include "SkEndian.h"
#include "SkColor.h"
#include "SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableColorPalette {
    static const SK_OT_CHAR TAG0 = 'C';
    static const SK_OT_CHAR TAG1 = 'P';
    static const SK_OT_CHAR TAG2 = 'A';
    static const SK_OT_CHAR TAG3 = 'L';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableColorPalette>::value;

  SK_OT_USHORT version;
  SK_OT_USHORT numPaletteEntries;
  SK_OT_USHORT numPalettes;
  SK_OT_USHORT numColorRecords;
  SK_OT_ULONG  offsetFirstColorRecord;
  SK_OT_USHORT colorIndexOfFirstPalettesFirstColor;

  struct  ColorRecord  {
      SK_OT_BYTE blue;
      SK_OT_BYTE green;
      SK_OT_BYTE red;
      SK_OT_BYTE alpha;
  };
};

#pragma pack(pop)

SkColor colorForIndexFromFirstPalette(SkOTTableColorPalette* colorTable, uint16_t colorIndex) {
    SkASSERT(colorIndex < colorTable->numPaletteEntries);
    SkOTTableColorPalette::ColorRecord* firstColorRecord =
            (SkOTTableColorPalette::
                     ColorRecord*)((char*)colorTable +
                                   SkEndian_SwapBE32(colorTable->offsetFirstColorRecord) +
                                   (SkEndian_SwapBE16(
                                            colorTable->colorIndexOfFirstPalettesFirstColor) *
                                    sizeof(SkOTTableColorPalette::ColorRecord)));
    SkOTTableColorPalette::ColorRecord* indexedColorRecord = &firstColorRecord[colorIndex];
    return SkColorSetARGB(indexedColorRecord->alpha, indexedColorRecord->red, indexedColorRecord->green, indexedColorRecord->blue);
};

#include <stddef.h>
static_assert(sizeof(SkOTTableColorPalette) == 14, "sizeof_SkOTTableColorPalette_Not_14");

#endif

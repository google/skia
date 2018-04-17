/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_COLR_DEFINED
#define SkOTTable_COLR_DEFINED

#include "SkEndian.h"
#include "SkOTTableTypes.h"
#include "stdlib.h"

#pragma pack(push, 1)

struct SkOTTableColor {
    static const SK_OT_CHAR TAG0 = 'C';
    static const SK_OT_CHAR TAG1 = 'O';
    static const SK_OT_CHAR TAG2 = 'L';
    static const SK_OT_CHAR TAG3 = 'R';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableColor>::value;

    SK_OT_USHORT version;
    SK_OT_USHORT numBaseGlyphRecords;
    SK_OT_ULONG  baseGlyphRecordsOffset;
    SK_OT_ULONG  layerRecordsOffset;
    SK_OT_USHORT numLayerRecords;

    struct BaseGlyphRecord {
        SK_OT_USHORT gID;
        SK_OT_USHORT firstLayerIndex;
        SK_OT_USHORT numLayers;
    };

    struct LayerRecord {
        SK_OT_USHORT gID;
        SK_OT_USHORT paletteIndex;
    };
};

#pragma pack(pop)

static int compareGlyphIds(const void* glyphId, const void* baseGlyphRecord) {
    return (int)*reinterpret_cast<const SkGlyphID*>(glyphId) -
           SkEndian_SwapBE16(
                   (reinterpret_cast<const SkOTTableColor::BaseGlyphRecord*>(baseGlyphRecord))
                           ->gID);
};

size_t findColorLayersForGlyphId(SkOTTableColor* colorTable, SkGlyphID glyphId,
                                 SkOTTableColor::LayerRecord* layerRecordBegin, size_t* layerRecordCount) {
  SkOTTableColor::BaseGlyphRecord* baseGlyphRecord =
          reinterpret_cast<SkOTTableColor::BaseGlyphRecord*>(
                  bsearch(&glyphId,
                          reinterpret_cast<char*>(colorTable) +
                                  SkEndian_SwapBE32(colorTable->baseGlyphRecordsOffset),
                          SkEndian_SwapBE16(colorTable->numBaseGlyphRecords),
                          sizeof(SkOTTableColor::BaseGlyphRecord), compareGlyphIds));
  if (!baseGlyphRecord) return 0;
  if (layerRecordBegin && layerRecordCount) {
    for (size_t i = 0; i < SkEndian_SwapBE16(baseGlyphRecord->numLayers); ++i) {
        size_t offset = SkEndian_SwapBE32(colorTable->layerRecordsOffset) +
                        ((SkEndian_SwapBE16(baseGlyphRecord->firstLayerIndex) + i) *
                         sizeof(SkOTTableColor::LayerRecord));
        SkOTTableColor::LayerRecord* layerRecord =
            (SkOTTableColor::LayerRecord*)(reinterpret_cast<char*>(colorTable) + offset);                                               ;
        layerRecordBegin[i].gID = SkEndian_SwapBE16(layerRecord->gID);
        layerRecordBegin[i].paletteIndex = SkEndian_SwapBE16(layerRecord->paletteIndex);
    }
    *layerRecordCount = SkEndian_SwapBE16(baseGlyphRecord->numLayers);
  }
  return SkEndian_SwapBE16(baseGlyphRecord->numLayers);
};

#include <stddef.h>
static_assert(sizeof(SkOTTableColor) == 14, "sizeof_SkOTTableColor_Not_14");

#endif

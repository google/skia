/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_hmtx_DEFINED
#define SkOTTable_hmtx_DEFINED

#include "src/base/SkEndian.h"
#include "src/sfnt/SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableHorizontalMetrics {
    static const SK_OT_CHAR TAG0 = 'h';
    static const SK_OT_CHAR TAG1 = 'm';
    static const SK_OT_CHAR TAG2 = 't';
    static const SK_OT_CHAR TAG3 = 'x';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableHorizontalMetrics>::value;

    struct FullMetric {
        SK_OT_USHORT advanceWidth;
        SK_OT_SHORT lsb;
    } longHorMetric[1/*hhea::numberOfHMetrics*/];
    struct ShortMetric {
        SK_OT_SHORT lsb;
    }; /* maxp::numGlyphs - hhea::numberOfHMetrics */
};

#pragma pack(pop)

#endif

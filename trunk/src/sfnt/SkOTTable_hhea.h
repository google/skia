/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_hhea_DEFINED
#define SkOTTable_hhea_DEFINED

#include "SkEndian.h"
#include "SkOTTableTypes.h"
#include "SkTypedEnum.h"

#pragma pack(push, 1)

struct SkOTTableHorizontalHeader {
    static const SK_OT_CHAR TAG0 = 'h';
    static const SK_OT_CHAR TAG1 = 'h';
    static const SK_OT_CHAR TAG2 = 'e';
    static const SK_OT_CHAR TAG3 = 'a';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableHorizontalHeader>::value;

    SK_OT_Fixed version;
    static const SK_OT_Fixed version1 = SkTEndian_SwapBE32(0x00010000);
    SK_OT_FWORD Ascender;
    SK_OT_FWORD Descender;
    SK_OT_FWORD LineGap;
    SK_OT_UFWORD advanceWidthMax;
    SK_OT_FWORD minLeftSideBearing;
    SK_OT_FWORD minRightSideBearing;
    SK_OT_FWORD xMaxExtent;
    SK_OT_SHORT caretSlopeRise;
    SK_OT_SHORT caretSlopeRun;
    SK_OT_SHORT caretOffset;
    SK_OT_SHORT Reserved24;
    SK_OT_SHORT Reserved26;
    SK_OT_SHORT Reserved28;
    SK_OT_SHORT Reserved30;
    struct MetricDataFormat {
        SK_TYPED_ENUM(Value, SK_OT_SHORT,
            ((CurrentFormat, SkTEndian_SwapBE16(0)))
            SK_SEQ_END,
        (value)SK_SEQ_END)
    } metricDataFormat;
    SK_OT_USHORT numberOfHMetrics;
};

#pragma pack(pop)


#include <stddef.h>
SK_COMPILE_ASSERT(offsetof(SkOTTableHorizontalHeader, numberOfHMetrics) == 34, SkOTTableHorizontalHeader_numberOfHMetrics_not_at_34);
SK_COMPILE_ASSERT(sizeof(SkOTTableHorizontalHeader) == 36, sizeof_SkOTTableHorizontalHeader_not_36);

#endif

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_post_DEFINED
#define SkOTTable_post_DEFINED

#include "src/core/SkEndian.h"
#include "src/sfnt/SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTablePostScript {
    static const SK_OT_CHAR TAG0 = 'p';
    static const SK_OT_CHAR TAG1 = 'o';
    static const SK_OT_CHAR TAG2 = 's';
    static const SK_OT_CHAR TAG3 = 't';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTablePostScript>::value;

    struct Format {
        enum Value : SK_OT_Fixed {
            version1 = SkTEndian_SwapBE32(0x00010000),
            version2 = SkTEndian_SwapBE32(0x00020000),
            version2_5 = SkTEndian_SwapBE32(0x00025000),
            version3 = SkTEndian_SwapBE32(0x00030000),
            version4 = SkTEndian_SwapBE32(0x00040000),
        };
        SK_OT_Fixed value;
    } format;
    SK_OT_Fixed italicAngle;
    SK_OT_FWORD underlinePosition;
    SK_OT_FWORD underlineThickness;
    SK_OT_ULONG isFixedPitch;
    SK_OT_ULONG minMemType42;
    SK_OT_ULONG maxMemType42;
    SK_OT_ULONG minMemType1;
    SK_OT_ULONG maxMemType1;
};

#pragma pack(pop)


#include <stddef.h>
static_assert(offsetof(SkOTTablePostScript, maxMemType1) == 28, "SkOTTablePostScript_maxMemType1_not_at_28");
static_assert(sizeof(SkOTTablePostScript) == 32, "sizeof_SkOTTablePostScript_not_32");

#endif

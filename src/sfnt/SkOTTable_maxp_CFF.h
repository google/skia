/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_maxp_CFF_DEFINED
#define SkOTTable_maxp_CFF_DEFINED

#include "SkEndian.h"
#include "SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableMaximumProfile_CFF {
    SK_OT_Fixed version;
    static const SK_OT_Fixed VERSION = SkTEndian_SwapBE32(0x00005000);

    SK_OT_USHORT numGlyphs;
};

#pragma pack(pop)


#include <stddef.h>
SK_COMPILE_ASSERT(offsetof(SkOTTableMaximumProfile_CFF, numGlyphs) == 4, SkOTTableMaximumProfile_CFF_numGlyphs_not_at_4);
SK_COMPILE_ASSERT(sizeof(SkOTTableMaximumProfile_CFF) == 6, sizeof_SkOTTableMaximumProfile_CFF_not_6);

#endif

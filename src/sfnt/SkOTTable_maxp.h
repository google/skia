/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_maxp_DEFINED
#define SkOTTable_maxp_DEFINED

#include "SkOTTableTypes.h"
#include "SkOTTable_maxp_CFF.h"
#include "SkOTTable_maxp_TT.h"

#pragma pack(push, 1)

struct SkOTTableMaximumProfile {
    static const SK_OT_CHAR TAG0 = 'm';
    static const SK_OT_CHAR TAG1 = 'a';
    static const SK_OT_CHAR TAG2 = 'x';
    static const SK_OT_CHAR TAG3 = 'p';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableMaximumProfile>::value;

    union Version {
        SK_OT_Fixed version;

        struct CFF : SkOTTableMaximumProfile_CFF { } cff;
        struct TT : SkOTTableMaximumProfile_TT { } tt;
    } version;
};

#pragma pack(pop)

#endif

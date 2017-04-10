/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_cvt_DEFINED
#define SkOTTable_cvt_DEFINED

#include "SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableControlValueTable {
    static const SK_OT_CHAR TAG0 = 'c';
    static const SK_OT_CHAR TAG1 = 'v';
    static const SK_OT_CHAR TAG2 = 't';
    static const SK_OT_CHAR TAG3 = ' ';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableControlValueTable>::value;
};

#pragma pack(pop)

#endif

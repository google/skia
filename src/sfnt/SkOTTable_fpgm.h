/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_fpgm_DEFINED
#define SkOTTable_fpgm_DEFINED

#include "SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableFontProgram {
    static const SK_OT_CHAR TAG0 = 'f';
    static const SK_OT_CHAR TAG1 = 'p';
    static const SK_OT_CHAR TAG2 = 'g';
    static const SK_OT_CHAR TAG3 = 'm';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableFontProgram>::value;
};

#pragma pack(pop)

#endif

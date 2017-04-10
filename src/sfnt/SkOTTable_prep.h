/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_prep_DEFINED
#define SkOTTable_prep_DEFINED

#include "SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableControlValueProgram {
    static const SK_OT_CHAR TAG0 = 'p';
    static const SK_OT_CHAR TAG1 = 'r';
    static const SK_OT_CHAR TAG2 = 'e';
    static const SK_OT_CHAR TAG3 = 'p';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableControlValueProgram>::value;
};

#pragma pack(pop)

#endif

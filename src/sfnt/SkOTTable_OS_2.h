/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_OS_2_DEFINED
#define SkOTTable_OS_2_DEFINED

#include "SkOTTable_OS_2_VA.h"
#include "SkOTTable_OS_2_V0.h"
#include "SkOTTable_OS_2_V1.h"
#include "SkOTTable_OS_2_V2.h"
#include "SkOTTable_OS_2_V3.h"
#include "SkOTTable_OS_2_V4.h"

#pragma pack(push, 1)

union SkOTTableOS2 {
    //original V0 TT
    struct VA : SkOTTableOS2_VA { } vA;
    struct V0 : SkOTTableOS2_V0 { } v0;
    struct V1 : SkOTTableOS2_V1 { } v1;
    struct V2 : SkOTTableOS2_V2 { } v2;
    //makes fsType 0-3 exclusive
    struct V3 : SkOTTableOS2_V3 { } v3;
    //defines fsSelection bits 7-9
    struct V4 : SkOTTableOS2_V4 { } v4;
};

#pragma pack(pop)


SK_COMPILE_ASSERT(sizeof(SkOTTableOS2::VA) == 68, sizeof_SkOTTableOS2__VA_not_68);
SK_COMPILE_ASSERT(sizeof(SkOTTableOS2::V0) == 78, sizeof_SkOTTableOS2__V0_not_78);
SK_COMPILE_ASSERT(sizeof(SkOTTableOS2::V1) == 86, sizeof_SkOTTableOS2__V1_not_86);
SK_COMPILE_ASSERT(sizeof(SkOTTableOS2::V2) == 96, sizeof_SkOTTableOS2__V2_not_96);
SK_COMPILE_ASSERT(sizeof(SkOTTableOS2::V3) == 96, sizeof_SkOTTableOS2__V3_not_96);
SK_COMPILE_ASSERT(sizeof(SkOTTableOS2::V4) == 96, sizeof_SkOTTableOS2__V4_not_96);

#endif

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkClipOp_DEFINED
#define SkClipOp_DEFINED

#include "SkTypes.h"

// SkClipOp enum values always match the corresponding values in SkRegion::Op

#ifdef SK_SUPPORT_LEGACY_CLIPOPS_PLAIN_ENUM

enum SkClipOp {
    kDifference_SkClipOp    = 0,
    kIntersect_SkClipOp     = 1,

    // Goal: remove these, since they can grow the current clip
#ifdef SK_SUPPORT_EXOTIC_CLIPOPS
    kUnion_SkClipOp         = 2,
    kXOR_SkClipOp           = 3,
    kReverseDifference_SkClipOp = 4,
#endif
    kReplace_SkClipOp       = 5,
};

#else

enum class SkClipOp {
    kDifference    = 0,
    kIntersect     = 1,

    // Goal: remove these, since they can grow the current clip

#ifdef SK_SUPPORT_EXOTIC_CLIPOPS
    kUnion         = 2,
    kXOR           = 3,
    kReverseDifference = 4,
#endif
    kReplace       = 5,
};
#endif

#ifdef SK_SUPPORT_EXOTIC_CLIPOPS
    #define SkEXOTIC_CLIPOP_CODE(code)  code
#else
    #define SkEXOTIC_CLIPOP_CODE(code)
#endif

#endif

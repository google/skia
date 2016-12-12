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

    kUnion_SkClipOp         = 2,
    kXOR_SkClipOp           = 3,
    kReverseDifference_SkClipOp = 4,
    kReplace_SkClipOp       = 5,
};

#else

enum class SkClipOp {
    kDifference    = 0,
    kIntersect     = 1,

    // Goal: remove these, since they can grow the current clip

    kUnion         = 2,
    kXOR           = 3,
    kReverseDifference = 4,
    kReplace       = 5,
};

#endif

#endif

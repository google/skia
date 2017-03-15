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

enum class SkClipOp {
    kDifference    = 0,
    kIntersect     = 1,

    // Goal: remove these, since they can grow the current clip

    kUnion_deprecated             = 2,
    kXOR_deprecated               = 3,
    kReverseDifference_deprecated = 4,
    kReplace_deprecated           = 5,

    kMax_EnumValue = kReplace_deprecated,
};

#endif

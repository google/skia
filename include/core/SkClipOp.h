/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkClipOp_DEFINED
#define SkClipOp_DEFINED

#include "include/core/SkTypes.h"

enum class SkClipOp {
    kDifference    = 0,
    kIntersect     = 1,

#ifdef SK_SUPPORT_DEPRECATED_CLIPOPS
    kUnion_deprecated             = 2,
    kXOR_deprecated               = 3,
    kReverseDifference_deprecated = 4,
    kReplace_deprecated           = 5,

    // Used internally for validation, can only shrink to 1 when the deprecated flag is gone
    kMax_EnumValue = 5,
#else
    // Technically only kDifference and kIntersect are allowed, hence kMax_EnumValue = 1, but
    // internally we need these values defined for the deprecated clip ops.
    kExtraEnumNeedInternallyPleaseIgnoreWillGoAway2   = 2,
    kExtraEnumNeedInternallyPleaseIgnoreWillGoAway3   = 3,
    kExtraEnumNeedInternallyPleaseIgnoreWillGoAway4   = 4,
    kExtraEnumNeedInternallyPleaseIgnoreWillGoAway5   = 5,

    kMax_EnumValue = 1
#endif

};

#endif

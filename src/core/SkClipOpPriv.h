/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkClipOpPriv_DEFINED
#define SkClipOpPriv_DEFINED

#include "SkClipOp.h"

#ifndef SK_SUPPORT_LEGACY_CLIPOPS_PLAIN_ENUM
const SkClipOp kDifference_SkClipOp         = SkClipOp::kDifference;
const SkClipOp kIntersect_SkClipOp          = SkClipOp::kIntersect;

const SkClipOp kUnion_SkClipOp              = SkClipOp::kUnion_private_internal_do_not_use;
const SkClipOp kXOR_SkClipOp                = SkClipOp::kXOR_private_internal_do_not_use;
const SkClipOp kReverseDifference_SkClipOp  = SkClipOp::kReverseDifference_private_internal_do_not_use;
const SkClipOp kReplace_SkClipOp            = SkClipOp::kReplace_private_internal_do_not_use;
#endif

#endif

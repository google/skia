/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkClipOpPriv_DEFINED
#define SkClipOpPriv_DEFINED

#include "include/core/SkClipOp.h"

const SkClipOp kDifference_SkClipOp         = SkClipOp::kDifference;
const SkClipOp kIntersect_SkClipOp          = SkClipOp::kIntersect;

const SkClipOp kUnion_SkClipOp              = (SkClipOp)2;
const SkClipOp kXOR_SkClipOp                = (SkClipOp)3;
const SkClipOp kReverseDifference_SkClipOp  = (SkClipOp)4;
const SkClipOp kReplace_SkClipOp            = (SkClipOp)5;

#endif

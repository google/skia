/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOp.h"

int32_t GrOp::gCurrOpClassID = GrOp::kIllegalOpID;

int32_t GrOp::gCurrOpUniqueID = GrOp::kIllegalOpID;

GrOp::GrOp(uint32_t classID)
        : fClassID(classID)
        , fUniqueID(kIllegalOpID) {
    SkASSERT(classID == SkToU32(fClassID));
    SkDEBUGCODE(fBoundsFlags = kUninitialized_BoundsFlag);
}

GrOp::~GrOp() {}

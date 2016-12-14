/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOp.h"

#include <cstdlib>

int32_t GrOp::gCurrOpClassID = GrOp::kIllegalOpID;

int32_t GrOp::gCurrOpUniqueID = GrOp::kIllegalOpID;

void* GrOp::operator new(size_t size) {
    return malloc(size);
}

void GrOp::operator delete(void* target) {
    free(target);
}

GrOp::GrOp(uint32_t classID)
    : fClassID(classID)
    , fUniqueID(kIllegalOpID) {
    SkASSERT(classID == SkToU32(fClassID));
    SkDEBUGCODE(fUsed = false;)
    SkDEBUGCODE(fBoundsFlags = kUninitialized_BoundsFlag);
}

GrOp::~GrOp() {}

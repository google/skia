/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOp.h"

int32_t GrOp::gCurrOpClassID = GrOp::kIllegalOpID;

int32_t GrOp::gCurrOpUniqueID = GrOp::kIllegalOpID;

#ifdef SK_DEBUG
void* GrOp::operator new(size_t size) {
    // All GrOp-derived class should be allocated in a GrMemoryPool
    SkASSERT(0);
    return ::operator new(size);
}

void GrOp::operator delete(void* target) {
    // All GrOp-derived class should be released from their owning GrMemoryPool
    SkASSERT(0);
    ::operator delete(target);
}
#endif

GrOp::GrOp(uint32_t classID) : fClassID(classID) {
    SkASSERT(classID == SkToU32(fClassID));
    SkASSERT(classID);
    SkDEBUGCODE(fBoundsFlags = kUninitialized_BoundsFlag);
}

GrOp::~GrOp() {}

GrOp::CombineResult GrOp::combineIfPossible(GrOp* that, const GrCaps& caps) {
    if (this->classID() != that->classID()) {
        return CombineResult::kCannotCombine;
    }
    SkDEBUGCODE(bool thatWasChained = that->isChained());
    auto result = this->onCombineIfPossible(that, caps);
    // Merging a chained 'that' would cause problems given the way op lists currently manage chains.
    SkASSERT(!(thatWasChained && result == CombineResult::kMerged));
    if (fChainHead) {
        fChainHead->joinBounds(*that);
    }
    return result;
}

void GrOp::setNextInChain(GrOp* next) {
    SkASSERT(next);
    SkASSERT(this->classID() == next->classID());
    // Each op begins life as a 1 element list. We assume lists are appended only with
    SkASSERT(this->isChainTail());
    SkASSERT(!next->isChained());
    if (!fChainHead) {
        // We were using null to mark 'this' as unchained. Now 'this' is the head of the new chain.
        fChainHead = this;
    }
    fNextInChain = next;
    fChainHead->joinBounds(*next);
    next->fChainHead = this->fChainHead;
}

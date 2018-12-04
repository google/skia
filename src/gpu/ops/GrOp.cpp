/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOp.h"

std::atomic<uint32_t> GrOp::gCurrOpClassID {GrOp::kIllegalOpID + 1};
std::atomic<uint32_t> GrOp::gCurrOpUniqueID{GrOp::kIllegalOpID + 1};

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

GrOp::CombineResult GrOp::combineIfPossible(GrOp* that, const GrCaps& caps) {
    SkASSERT(this != that);
    if (this->classID() != that->classID()) {
        return CombineResult::kCannotCombine;
    }
    auto result = this->onCombineIfPossible(that, caps);
    if (result == CombineResult::kMerged) {
        this->joinBounds(*that);
    }
    return result;
}

void GrOp::chainConcat(std::unique_ptr<GrOp> next) {
    SkASSERT(next);
    SkASSERT(this->classID() == next->classID());
    SkASSERT(this->isChainTail());
    SkASSERT(next->isChainHead());
    fNextInChain = std::move(next);
    fNextInChain->fPrevInChain = this;
}

std::unique_ptr<GrOp> GrOp::cutChain() {
    if (fNextInChain) {
        fNextInChain->fPrevInChain = nullptr;
        return std::move(fNextInChain);
    }
    return nullptr;
}

#ifdef SK_DEBUG
void GrOp::validateChain(GrOp* expectedTail) const {
    SkASSERT(this->isChainHead());
    uint32_t classID = this->classID();
    const GrOp* op = this;
    while (op) {
        SkASSERT(op == this || (op->prevInChain() && op->prevInChain()->nextInChain() == op));
        SkASSERT(classID == op->classID());
        if (op->nextInChain()) {
            SkASSERT(op->nextInChain()->prevInChain() == op);
            SkASSERT(op != expectedTail);
        } else {
            SkASSERT(!expectedTail || op == expectedTail);
        }
        op = op->nextInChain();
    }
}
#endif

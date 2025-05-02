/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/ops/GrOp.h"

std::atomic<uint32_t> GrOp::gCurrOpClassID {GrOp::kIllegalOpID + 1};
std::atomic<uint32_t> GrOp::gCurrOpUniqueID{GrOp::kIllegalOpID + 1};

GrOp::GrOp(uint32_t classID) : fClassID(classID) {
    SkASSERT(classID == SkToU32(fClassID));
    SkASSERT(classID);
    SkDEBUGCODE(fBoundsFlags = kUninitialized_BoundsFlag);
}

GrOp::CombineResult GrOp::combineIfPossible(GrOp* that, SkArenaAlloc* alloc, const GrCaps& caps) {
    SkASSERT(this != that);
    if (this->classID() != that->classID()) {
        return CombineResult::kCannotCombine;
    }
    auto result = this->onCombineIfPossible(that, alloc, caps);
    if (result == CombineResult::kMerged) {
        this->joinBounds(*that);
    }
    return result;
}

void GrOp::chainConcat(GrOp::Owner next) {
    SkASSERT(next);
    SkASSERT(this->classID() == next->classID());
    SkASSERT(this->isChainTail());
    SkASSERT(next->isChainHead());
    fNextInChain = std::move(next);
    fNextInChain->fPrevInChain = this;
}

GrOp::Owner GrOp::cutChain() {
    if (fNextInChain) {
        fNextInChain->fPrevInChain = nullptr;
        return std::move(fNextInChain);
    }
    return nullptr;
}

void GrOp::prePrepare(GrRecordingContext* context, const GrSurfaceProxyView& dstView,
                      GrAppliedClip* clip, const GrDstProxyView& dstProxyView,
                      GrXferBarrierFlags renderPassXferBarriers, GrLoadOp colorLoadOp) {
    TRACE_EVENT0_ALWAYS("skia.gpu", TRACE_STR_STATIC(name()));
    this->onPrePrepare(context, dstView, clip, dstProxyView, renderPassXferBarriers,
                       colorLoadOp);
}

void GrOp::prepare(GrOpFlushState* state) {
    TRACE_EVENT0_ALWAYS("skia.gpu", TRACE_STR_STATIC(name()));
    this->onPrepare(state);
}

void GrOp::execute(GrOpFlushState* state, const SkRect& chainBounds) {
    TRACE_EVENT0_ALWAYS("skia.gpu", TRACE_STR_STATIC(name()));
    this->onExecute(state, chainBounds);
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

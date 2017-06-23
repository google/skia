/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetOpList.h"
#include "GrAuditTrail.h"
#include "GrCaps.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrRect.h"
#include "GrRenderTargetContext.h"
#include "instanced/InstancedRendering.h"
#include "ops/GrClearOp.h"
#include "ops/GrCopySurfaceOp.h"

using gr_instanced::InstancedRendering;

////////////////////////////////////////////////////////////////////////////////

// Experimentally we have found that most combining occurs within the first 10 comparisons.
static const int kMaxOpLookback = 10;
static const int kMaxOpLookahead = 10;

GrRenderTargetOpList::GrRenderTargetOpList(GrRenderTargetProxy* proxy, GrGpu* gpu,
                                           GrAuditTrail* auditTrail)
        : INHERITED(gpu->getContext()->resourceProvider(), proxy, auditTrail)
        , fLastClipStackGenID(SK_InvalidUniqueID)
        SkDEBUGCODE(, fNumClips(0)) {
    if (GrCaps::InstancedSupport::kNone != gpu->caps()->instancedSupport()) {
        fInstancedRendering.reset(gpu->createInstancedRendering());
    }
}

GrRenderTargetOpList::~GrRenderTargetOpList() {
}

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void GrRenderTargetOpList::dump() const {
    INHERITED::dump();

    SkDebugf("ops (%d):\n", fRecordedOps.count());
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        SkDebugf("*******************************\n");
        if (!fRecordedOps[i].fOp) {
            SkDebugf("%d: <combined forward>\n", i);
        } else {
            SkDebugf("%d: %s\n", i, fRecordedOps[i].fOp->name());
            SkString str = fRecordedOps[i].fOp->dumpInfo();
            SkDebugf("%s\n", str.c_str());
            const SkRect& bounds = fRecordedOps[i].fOp->bounds();
            SkDebugf("ClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", bounds.fLeft,
                     bounds.fTop, bounds.fRight, bounds.fBottom);
        }
    }
}
#endif

void GrRenderTargetOpList::prepareOps(GrOpFlushState* flushState) {
    SkASSERT(fTarget.get()->priv().peekRenderTarget());
    SkASSERT(this->isClosed());

    // Loop over the ops that haven't yet been prepared.
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i].fOp) {
            GrOpFlushState::DrawOpArgs opArgs = {
                fTarget.get()->priv().peekRenderTarget(),
                fRecordedOps[i].fAppliedClip,
                fRecordedOps[i].fDstProxy
            };

            flushState->setDrawOpArgs(&opArgs);
            fRecordedOps[i].fOp->prepare(flushState);
            flushState->setDrawOpArgs(nullptr);
        }
    }

    if (fInstancedRendering) {
        fInstancedRendering->beginFlush(flushState->resourceProvider());
    }
}

static std::unique_ptr<GrGpuCommandBuffer> create_command_buffer(GrGpu* gpu) {
    static const GrGpuCommandBuffer::LoadAndStoreInfo kBasicLoadStoreInfo {
        GrGpuCommandBuffer::LoadOp::kLoad,
        GrGpuCommandBuffer::StoreOp::kStore,
        GrColor_ILLEGAL
    };

    std::unique_ptr<GrGpuCommandBuffer> buffer(
                            gpu->createCommandBuffer(kBasicLoadStoreInfo,   // Color
                                                     kBasicLoadStoreInfo)); // Stencil
    return buffer;
}

static inline void finish_command_buffer(GrGpuCommandBuffer* buffer) {
    if (!buffer) {
        return;
    }

    buffer->end();
    buffer->submit();
}

// TODO: this is where GrOp::renderTarget is used (which is fine since it
// is at flush time). However, we need to store the RenderTargetProxy in the
// Ops and instantiate them here.
bool GrRenderTargetOpList::executeOps(GrOpFlushState* flushState) {
    if (0 == fRecordedOps.count()) {
        return false;
    }

    SkASSERT(fTarget.get()->priv().peekRenderTarget());

    std::unique_ptr<GrGpuCommandBuffer> commandBuffer = create_command_buffer(flushState->gpu());
    flushState->setCommandBuffer(commandBuffer.get());

    // Draw all the generated geometry.
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (!fRecordedOps[i].fOp) {
            continue;
        }

        if (fRecordedOps[i].fOp->needsCommandBufferIsolation()) {
            // This op is a special snowflake and must occur between command buffers
            // TODO: make this go through the command buffer
            finish_command_buffer(commandBuffer.get());

            commandBuffer.reset();
            flushState->setCommandBuffer(commandBuffer.get());
        } else if (!commandBuffer) {
            commandBuffer = create_command_buffer(flushState->gpu());
            flushState->setCommandBuffer(commandBuffer.get());
        }

        GrOpFlushState::DrawOpArgs opArgs {
            fTarget.get()->priv().peekRenderTarget(),
            fRecordedOps[i].fAppliedClip,
            fRecordedOps[i].fDstProxy
        };

        flushState->setDrawOpArgs(&opArgs);
        fRecordedOps[i].fOp->execute(flushState);
        flushState->setDrawOpArgs(nullptr);
    }

    finish_command_buffer(commandBuffer.get());
    flushState->setCommandBuffer(nullptr);

    return true;
}

void GrRenderTargetOpList::reset() {
    fLastFullClearOp = nullptr;
    fLastClipStackGenID = SK_InvalidUniqueID;
    fRecordedOps.reset();
    if (fInstancedRendering) {
        fInstancedRendering->endFlush();
        fInstancedRendering = nullptr;
    }

    INHERITED::reset();
}

void GrRenderTargetOpList::abandonGpuResources() {
    if (fInstancedRendering) {
        fInstancedRendering->resetGpuResources(InstancedRendering::ResetType::kAbandon);
    }
}

void GrRenderTargetOpList::freeGpuResources() {
    if (fInstancedRendering) {
        fInstancedRendering->resetGpuResources(InstancedRendering::ResetType::kDestroy);
    }
}

void GrRenderTargetOpList::fullClear(const GrCaps& caps, GrColor color) {
    // Currently this just inserts or updates the last clear op. However, once in MDB this can
    // remove all the previously recorded ops and change the load op to clear with supplied
    // color.
    if (fLastFullClearOp) {
        // As currently implemented, fLastFullClearOp should be the last op because we would
        // have cleared it when another op was recorded.
        SkASSERT(fRecordedOps.back().fOp.get() == fLastFullClearOp);
        GrOP_INFO("opList: %d Fusing clears (opID: %d Color: 0x%08x -> 0x%08x)\n",
                  this->uniqueID(),
                  fLastFullClearOp->uniqueID(),
                  fLastFullClearOp->color(), color);
        fLastFullClearOp->setColor(color);
        return;
    }
    std::unique_ptr<GrClearOp> op(GrClearOp::Make(GrFixedClip::Disabled(), color, fTarget.get()));
    if (!op) {
        return;
    }

    if (GrOp* clearOp = this->recordOp(std::move(op), caps)) {
        // This is either the clear op we just created or another one that it combined with.
        fLastFullClearOp = static_cast<GrClearOp*>(clearOp);
    }
}

////////////////////////////////////////////////////////////////////////////////

// This closely parallels GrTextureOpList::copySurface but renderTargetOpLists
// also store the applied clip and dest proxy with the op
bool GrRenderTargetOpList::copySurface(const GrCaps& caps,
                                       GrSurfaceProxy* dst,
                                       GrSurfaceProxy* src,
                                       const SkIRect& srcRect,
                                       const SkIPoint& dstPoint) {
    SkASSERT(dst->asRenderTargetProxy() == fTarget.get());
    std::unique_ptr<GrOp> op = GrCopySurfaceOp::Make(dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }
#ifdef ENABLE_MDB
    this->addDependency(src);
#endif

    this->recordOp(std::move(op), caps);
    return true;
}

static inline bool can_reorder(const SkRect& a, const SkRect& b) { return !GrRectsOverlap(a, b); }

bool GrRenderTargetOpList::combineIfPossible(const RecordedOp& a, GrOp* b,
                                             const GrAppliedClip* bClip,
                                             const DstProxy* bDstProxy,
                                             const GrCaps& caps) {
    if (a.fAppliedClip) {
        if (!bClip) {
            return false;
        }
        if (*a.fAppliedClip != *bClip) {
            return false;
        }
    } else if (bClip) {
        return false;
    }
    if (bDstProxy) {
        if (a.fDstProxy != *bDstProxy) {
            return false;
        }
    } else if (a.fDstProxy.proxy()) {
        return false;
    }
    return a.fOp->combineIfPossible(b, caps);
}

GrOp* GrRenderTargetOpList::recordOp(std::unique_ptr<GrOp> op,
                                     const GrCaps& caps,
                                     GrAppliedClip* clip,
                                     const DstProxy* dstProxy) {
    SkASSERT(fTarget.get());

    // A closed GrOpList should never receive new/more ops
    SkASSERT(!this->isClosed());

    // Check if there is an op we can combine with by linearly searching back until we either
    // 1) check every op
    // 2) intersect with something
    // 3) find a 'blocker'
    GR_AUDIT_TRAIL_ADD_OP(fAuditTrail, op.get(), fTarget.get()->uniqueID());
    GrOP_INFO("opList: %d Recording (%s, opID: %u)\n"
              "\tBounds [L: %.2f, T: %.2f R: %.2f B: %.2f]\n",
               this->uniqueID(),
               op->name(),
               op->uniqueID(),
               op->bounds().fLeft, op->bounds().fTop,
               op->bounds().fRight, op->bounds().fBottom);
    GrOP_INFO(SkTabString(op->dumpInfo(), 1).c_str());
    GrOP_INFO("\tOutcome:\n");
    int maxCandidates = SkTMin(kMaxOpLookback, fRecordedOps.count());
    // If we don't have a valid destination render target then we cannot reorder.
    if (maxCandidates) {
        int i = 0;
        while (true) {
            const RecordedOp& candidate = fRecordedOps.fromBack(i);

            if (this->combineIfPossible(candidate, op.get(), clip, dstProxy, caps)) {
                GrOP_INFO("\t\tBackward: Combining with (%s, opID: %u)\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                GrOP_INFO("\t\t\tBackward: Combined op info:\n");
                GrOP_INFO(SkTabString(candidate.fOp->dumpInfo(), 4).c_str());
                GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(fAuditTrail, candidate.fOp.get(), op.get());
                return candidate.fOp.get();
            }
            // Stop going backwards if we would cause a painter's order violation.
            if (!can_reorder(fRecordedOps.fromBack(i).fOp->bounds(), op->bounds())) {
                GrOP_INFO("\t\tBackward: Intersects with (%s, opID: %u)\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                break;
            }
            ++i;
            if (i == maxCandidates) {
                GrOP_INFO("\t\tBackward: Reached max lookback or beginning of op array %d\n", i);
                break;
            }
        }
    } else {
        GrOP_INFO("\t\tBackward: FirstOp\n");
    }
    GR_AUDIT_TRAIL_OP_RESULT_NEW(fAuditTrail, op);
    if (clip) {
        clip = fClipAllocator.make<GrAppliedClip>(std::move(*clip));
        SkDEBUGCODE(fNumClips++;)
    }
    fRecordedOps.emplace_back(std::move(op), clip, dstProxy);
    fRecordedOps.back().fOp->wasRecorded(this);
    fLastFullClearOp = nullptr;
    return fRecordedOps.back().fOp.get();
}

void GrRenderTargetOpList::forwardCombine(const GrCaps& caps) {
    SkASSERT(!this->isClosed());

    GrOP_INFO("opList: %d ForwardCombine %d ops:\n", this->uniqueID(), fRecordedOps.count());

    for (int i = 0; i < fRecordedOps.count() - 1; ++i) {
        GrOp* op = fRecordedOps[i].fOp.get();

        int maxCandidateIdx = SkTMin(i + kMaxOpLookahead, fRecordedOps.count() - 1);
        int j = i + 1;
        while (true) {
            const RecordedOp& candidate = fRecordedOps[j];

            if (this->combineIfPossible(fRecordedOps[i], candidate.fOp.get(),
                                        candidate.fAppliedClip, &candidate.fDstProxy, caps)) {
                GrOP_INFO("\t\t%d: (%s opID: %u) -> Combining with (%s, opID: %u)\n",
                          i, op->name(), op->uniqueID(),
                          candidate.fOp->name(), candidate.fOp->uniqueID());
                GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(fAuditTrail, op, candidate.fOp.get());
                fRecordedOps[j].fOp = std::move(fRecordedOps[i].fOp);
                break;
            }
            // Stop traversing if we would cause a painter's order violation.
            if (!can_reorder(fRecordedOps[j].fOp->bounds(), op->bounds())) {
                GrOP_INFO("\t\t%d: (%s opID: %u) -> Intersects with (%s, opID: %u)\n",
                          i, op->name(), op->uniqueID(),
                          candidate.fOp->name(), candidate.fOp->uniqueID());
                break;
            }
            ++j;
            if (j > maxCandidateIdx) {
                GrOP_INFO("\t\t%d: (%s opID: %u) -> Reached max lookahead or end of array\n",
                          i, op->name(), op->uniqueID());
                break;
            }
        }
    }
}


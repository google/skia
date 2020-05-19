/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrClearOp.h"

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"

static bool contains_scissor(const GrScissorState& a, const GrScissorState& b) {
    return !a.enabled() || (b.enabled() && a.rect().contains(b.rect()));
}

std::unique_ptr<GrClearOp> GrClearOp::MakeColor(GrRecordingContext* context,
                                                const GrScissorState& scissor,
                                                const SkPMColor4f& color) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();
    return pool->allocate<GrClearOp>(Buffer::kColor, scissor, color, false);
}

std::unique_ptr<GrClearOp> GrClearOp::MakeStencilClip(GrRecordingContext* context,
                                                      const GrScissorState& scissor,
                                                      bool insideMask) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();
    return pool->allocate<GrClearOp>(Buffer::kStencilClip, scissor, SkPMColor4f(), insideMask);
}

GrClearOp::GrClearOp(Buffer buffer, const GrScissorState& scissor,
                     const SkPMColor4f& color, bool insideMask)
        : INHERITED(ClassID())
        , fScissor(scissor)
        , fColor(color)
        , fStencilInsideMask(insideMask)
        , fBuffer(buffer) {
    this->setBounds(SkRect::Make(scissor.rect()), HasAABloat::kNo, IsHairline::kNo);
}

GrOp::CombineResult GrClearOp::onCombineIfPossible(GrOp* t, GrRecordingContext::Arenas*,
                                                   const GrCaps& caps) {
    GrClearOp* cb = t->cast<GrClearOp>();

    if (cb->fBuffer == fBuffer) {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and they clear
        // to the same value (color or stencil mask depending on target).
        if (contains_scissor(cb->fScissor, fScissor)) {
            fScissor = cb->fScissor;
            fColor = cb->fColor;
            fStencilInsideMask = cb->fStencilInsideMask;
            return CombineResult::kMerged;
        } else if (cb->fColor == fColor && cb->fStencilInsideMask == fStencilInsideMask &&
                  contains_scissor(fScissor, cb->fScissor)) {
            return CombineResult::kMerged;
        }
    } else if (cb->fScissor == fScissor) {
        // When the scissors are the exact same but the buffers are different, we can combine and
        // clear both stencil and clear together in onExecute().
        if (cb->clearsColor()) {
            fColor = cb->fColor;
        }
        if (cb->clearsStencilClip()) {
            fStencilInsideMask = cb->fStencilInsideMask;
        }
        fBuffer = Buffer::kBoth;
    }
    return CombineResult::kCannotCombine;
}

void GrClearOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->opsRenderPass());
    if (this->clearsColor()) {
        state->opsRenderPass()->clear(fScissor, fColor);
    }

    if (this->clearsStencilClip()) {
        state->opsRenderPass()->clearStencilClip(fScissor, fStencilInsideMask);
    }
}

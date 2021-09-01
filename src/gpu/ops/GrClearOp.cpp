/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrClearOp.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"

static bool contains_scissor(const GrScissorState& a, const GrScissorState& b) {
    return !a.enabled() || (b.enabled() && a.rect().contains(b.rect()));
}

GrOp::Owner GrClearOp::MakeColor(GrRecordingContext* context,
                                 const GrScissorState& scissor,
                                 std::array<float, 4> color) {
    return GrOp::Make<GrClearOp>(context, Buffer::kColor, scissor, color, false);
}

GrOp::Owner GrClearOp::MakeStencilClip(GrRecordingContext* context,
                                       const GrScissorState& scissor,
                                       bool insideMask) {
    return GrOp::Make<GrClearOp>(context,
                                 Buffer::kStencilClip,
                                 scissor,
                                 std::array<float, 4>(),
                                 insideMask);
}

GrClearOp::GrClearOp(Buffer buffer,
                     const GrScissorState& scissor,
                     std::array<float, 4> color,
                     bool insideMask)
        : INHERITED(ClassID())
        , fScissor(scissor)
        , fColor(color)
        , fStencilInsideMask(insideMask)
        , fBuffer(buffer) {
    this->setBounds(SkRect::Make(scissor.rect()), HasAABloat::kNo, IsHairline::kNo);
}

GrOp::CombineResult GrClearOp::onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) {
    GrClearOp* other = t->cast<GrClearOp>();

    if (other->fBuffer == fBuffer) {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and they clear
        // to the same value (color or stencil mask depending on target).
        if (contains_scissor(other->fScissor, fScissor)) {
            fScissor = other->fScissor;
            fColor = other->fColor;
            fStencilInsideMask = other->fStencilInsideMask;
            return CombineResult::kMerged;
        } else if (other->fColor == fColor && other->fStencilInsideMask == fStencilInsideMask &&
                   contains_scissor(fScissor, other->fScissor)) {
            return CombineResult::kMerged;
        }
    } else if (other->fScissor == fScissor) {
        // When the scissors are the exact same but the buffers are different, we can combine and
        // clear both stencil and clear together in onExecute().
        if (other->fBuffer & Buffer::kColor) {
            fColor = other->fColor;
        }
        if (other->fBuffer & Buffer::kStencilClip) {
            fStencilInsideMask = other->fStencilInsideMask;
        }
        fBuffer = Buffer::kBoth;
        return CombineResult::kMerged;
    }
    return CombineResult::kCannotCombine;
}

void GrClearOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->opsRenderPass());
    if (fBuffer & Buffer::kColor) {
        state->opsRenderPass()->clear(fScissor, fColor);
    }

    if (fBuffer & Buffer::kStencilClip) {
        state->opsRenderPass()->clearStencilClip(fScissor, fStencilInsideMask);
    }
}

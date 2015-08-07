/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrReorderCommandBuilder.h"

template <class Left, class Right>
static bool intersect(const Left& a, const Right& b) {
    SkASSERT(a.fLeft <= a.fRight && a.fTop <= a.fBottom &&
             b.fLeft <= b.fRight && b.fTop <= b.fBottom);
    return a.fLeft < b.fRight && b.fLeft < a.fRight &&
           a.fTop < b.fBottom && b.fTop < a.fBottom;
}

GrTargetCommands::Cmd* GrReorderCommandBuilder::recordDrawBatch(State* state, GrBatch* batch) {
    // Check if there is a Batch Draw we can batch with by linearly searching back until we either
    // 1) check every draw
    // 2) intersect with something
    // 3) find a 'blocker'
    // Experimentally we have found that most batching occurs within the first 10 comparisons.
    static const int kMaxLookback = 10;
    int i = 0;
    batch->setPipeline(state->getPipeline());
    GrRenderTarget* rt = state->getPipeline()->getRenderTarget();

    GrBATCH_INFO("Re-Recording (%s, B%u)\n"
                 "\tRenderTarget %p\n"
                 "\tBounds (%f, %f, %f, %f)\n",
                 batch->name(),
                 batch->uniqueID(), rt,
                 batch->bounds().fLeft, batch->bounds().fRight,
                 batch->bounds().fTop, batch->bounds().fBottom);
#if GR_BATCH_SPEW
    SkDebugf("\tColorStages:\n");
    for (int i = 0; i < state->getPipeline()->numColorFragmentStages(); i++) {
        SkDebugf("\t\t%s\n", state->getPipeline()->getColorStage(i).processor()->name());
    }
    SkDebugf("\tCoverageStages:\n");
    for (int i = 0; i < state->getPipeline()->numCoverageFragmentStages(); i++) {
        SkDebugf("\t\t%s\n", state->getPipeline()->getCoverageStage(i).processor()->name());
    }
    SkDebugf("\tXP: %s\n", state->getPipeline()->getXferProcessor()->name());
#endif
    GrBATCH_INFO("\tOutcome:\n");
    if (!this->cmdBuffer()->empty()) {
        GrTargetCommands::CmdBuffer::ReverseIter reverseIter(*this->cmdBuffer());

        do {
            if (Cmd::kDrawBatch_CmdType == reverseIter->type()) {
                DrawBatch* previous = static_cast<DrawBatch*>(reverseIter.get());

                if (previous->fBatch->pipeline()->getRenderTarget() != rt) {
                    GrBATCH_INFO("\t\tBreaking because of (%s, B%u) Rendertarget\n",
                                 previous->fBatch->name(), previous->fBatch->uniqueID());
                    break;
                }
                // We cannot continue to search backwards if the render target changes
                if (previous->fBatch->combineIfPossible(batch)) {
                    GrBATCH_INFO("\t\tCombining with (%s, B%u)\n",
                                 previous->fBatch->name(), previous->fBatch->uniqueID());
                    return NULL;
                }

                if (intersect(previous->fBatch->bounds(), batch->bounds())) {
                    GrBATCH_INFO("\t\tIntersects with (%s, B%u)\n",
                                 previous->fBatch->name(), previous->fBatch->uniqueID());
                    break;
                }
            } else if (Cmd::kClear_CmdType == reverseIter->type()) {
                Clear* previous = static_cast<Clear*>(reverseIter.get());

                // We cannot continue to search backwards if the render target changes
                if (previous->renderTarget() != rt) {
                    GrBATCH_INFO("\t\tBreaking because of Clear's Rendertarget change\n");
                    break;
                }

                // We set the color to illegal if we are doing a discard.
                if (previous->fColor == GrColor_ILLEGAL ||
                    intersect(batch->bounds(), previous->fRect)) {
                    GrBATCH_INFO("\t\tBreaking because of Clear intersection\n");
                    break;
                }
            } else {
                GrBATCH_INFO("\t\tBreaking because of other %08x\n", reverseIter->type());
                // TODO temporary until we can navigate the other types of commands
                break;
            }
        } while (reverseIter.previous() && ++i < kMaxLookback);
#if GR_BATCH_SPEW
        if (!reverseIter.get()) {
            GrBATCH_INFO("\t\tNo more commands to try and batch with\n");
        } else if (i >= kMaxLookback) {
            GrBATCH_INFO("\t\tReached max lookback %d\n", i);
        }
#endif
    }
#if GR_BATCH_SPEW
    else {
        GrBATCH_INFO("\t\tBreaking because empty command buffer\n");
    }
#endif

    return GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), DrawBatch, (state, batch,
                                                                    this->batchTarget()));
}

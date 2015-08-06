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
    if (!this->cmdBuffer()->empty()) {
        GrTargetCommands::CmdBuffer::ReverseIter reverseIter(*this->cmdBuffer());

        do {
            if (Cmd::kDrawBatch_CmdType == reverseIter->type()) {
                DrawBatch* previous = static_cast<DrawBatch*>(reverseIter.get());

                // We cannot continue to search backwards if the render target changes
                if (previous->fBatch->pipeline()->getRenderTarget() != rt) {
                    break;
                }

                if (previous->fBatch->combineIfPossible(batch)) {
                    return NULL;
                }

                if (intersect(previous->fBatch->bounds(), batch->bounds())) {
                    break;
                }
            } else if (Cmd::kClear_CmdType == reverseIter->type()) {
                Clear* previous = static_cast<Clear*>(reverseIter.get());

                // We cannot continue to search backwards if the render target changes
                if (previous->renderTarget() != rt) {
                    break;
                }

                // We set the color to illegal if we are doing a discard.
                // If we can ignore the rect, then we do a full clear
                if (previous->fColor == GrColor_ILLEGAL ||
                    previous->fCanIgnoreRect ||
                    intersect(batch->bounds(), previous->fRect)) {
                    break;
                }
            } else {
                // TODO temporary until we can navigate the other types of commands
                break;
            }
        } while (reverseIter.previous() && ++i < kMaxLookback);
    }

    return GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), DrawBatch, (state, batch,
                                                                    this->batchTarget()));
}

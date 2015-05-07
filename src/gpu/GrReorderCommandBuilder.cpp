/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrReorderCommandBuilder.h"

static bool intersect(const SkRect& a, const SkRect& b) {
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
    if (!this->cmdBuffer()->empty()) {
        GrTargetCommands::CmdBuffer::ReverseIter reverseIter(*this->cmdBuffer());

        do {
            if (Cmd::kDrawBatch_CmdType == reverseIter->type()) {
                DrawBatch* previous = static_cast<DrawBatch*>(reverseIter.get());

                if (previous->fState->getPipeline()->isEqual(*state->getPipeline()) &&
                    previous->fBatch->combineIfPossible(batch)) {
                    return NULL;
                }

                if (intersect(previous->fBatch->bounds(), batch->bounds())) {
                    break;
                }
            } else {
                // TODO temporary until we can navigate the other types of commands
                break;
            }
        } while (reverseIter.previous());
    }

    return GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), DrawBatch, (state, batch,
                                                                    this->batchTarget()));
}

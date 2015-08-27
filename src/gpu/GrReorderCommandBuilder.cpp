/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrReorderCommandBuilder.h"
#include "SkStringUtils.h"

template <class Left, class Right>
static bool intersect(const Left& a, const Right& b) {
    SkASSERT(a.fLeft <= a.fRight && a.fTop <= a.fBottom &&
             b.fLeft <= b.fRight && b.fTop <= b.fBottom);
    return a.fLeft < b.fRight && b.fLeft < a.fRight &&
           a.fTop < b.fBottom && b.fTop < a.fBottom;
}

GrTargetCommands::Cmd* GrReorderCommandBuilder::recordDrawBatch(GrBatch* batch,
                                                                const GrCaps& caps) {
    // Check if there is a Batch Draw we can batch with by linearly searching back until we either
    // 1) check every draw
    // 2) intersect with something
    // 3) find a 'blocker'
    // Experimentally we have found that most batching occurs within the first 10 comparisons.
    static const int kMaxLookback = 10;
    int i = 0;

    GrBATCH_INFO("Re-Recording (%s, B%u)\n"
                 "\tBounds (%f, %f, %f, %f)\n",
                 batch->name(),
                 batch->uniqueID(),
                 batch->bounds().fLeft, batch->bounds().fRight,
                 batch->bounds().fTop, batch->bounds().fBottom);
    GrBATCH_INFO(SkTabString(batch->dumpInfo(), 1).c_str());
    GrBATCH_INFO("\tOutcome:\n");
    if (!this->cmdBuffer()->empty()) {
        GrTargetCommands::CmdBuffer::ReverseIter reverseIter(*this->cmdBuffer());

        do {
            if (Cmd::kDrawBatch_CmdType == reverseIter->type()) {
                DrawBatch* previous = static_cast<DrawBatch*>(reverseIter.get());

                if (previous->batch()->renderTargetUniqueID() != batch->renderTargetUniqueID()) {
                    GrBATCH_INFO("\t\tBreaking because of (%s, B%u) Rendertarget\n",
                                 previous->batch()->name(), previous->batch()->uniqueID());
                    break;
                }
                // We cannot continue to search backwards if the render target changes
                if (previous->batch()->combineIfPossible(batch, caps)) {
                    GrBATCH_INFO("\t\tCombining with (%s, B%u)\n",
                                 previous->batch()->name(), previous->batch()->uniqueID());
                    return nullptr;
                }

                if (intersect(previous->batch()->bounds(), batch->bounds())) {
                    GrBATCH_INFO("\t\tIntersects with (%s, B%u)\n",
                                 previous->batch()->name(), previous->batch()->uniqueID());
                    break;
                }
            } else {
                GrBATCH_INFO("\t\tBreaking because of other %08x\n", reverseIter->type());
                // TODO temporary until we only have batches.
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

    return GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), DrawBatch, (batch));
}

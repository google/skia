/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTargetCommands.h"

#include "GrBatchFlushState.h"
#include "GrGpu.h"
#include "GrPathRendering.h"
#include "batches/GrDrawBatch.h"
#include "batches/GrVertexBatch.h"

GrBATCH_SPEW(int32_t GrTargetCommands::Cmd::gUniqueID = 0;)

void GrTargetCommands::reset() {
    fCmdBuffer.reset();
}

void GrTargetCommands::flush(GrGpu* gpu, GrResourceProvider* resourceProvider) {
    GrBATCH_INFO("Flushing\n");
    if (fCmdBuffer.empty()) {
        return;
    }
    GrBatchFlushState flushState(gpu, resourceProvider, fLastFlushToken);
    // Loop over all batches and generate geometry
    CmdBuffer::Iter genIter(fCmdBuffer);
    while (genIter.next()) {
        if (Cmd::kDrawBatch_CmdType == genIter->type()) {
            DrawBatch* db = reinterpret_cast<DrawBatch*>(genIter.get());
            db->batch()->prepare(&flushState);
        }
    }

    flushState.preIssueDraws();

    CmdBuffer::Iter iter(fCmdBuffer);
    while (iter.next()) {
        iter->execute(&flushState);
    }
    fLastFlushToken = flushState.lastFlushedToken();
}

void GrTargetCommands::DrawBatch::execute(GrBatchFlushState* state) {
    fBatch->draw(state);
}

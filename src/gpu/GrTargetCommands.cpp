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
            // TODO: encapsulate the specialization of GrVertexBatch in GrVertexBatch so that we can
            // remove this cast. Currently all GrDrawBatches are in fact GrVertexBatch.
            GrVertexBatch* vertexBatch = static_cast<GrVertexBatch*>(db->batch());

            vertexBatch->prepareDraws(&flushState);
        }
    }

    flushState.preIssueDraws();

    CmdBuffer::Iter iter(fCmdBuffer);
    while (iter.next()) {
        iter->execute(&flushState);
    }
    fLastFlushToken = flushState.lastFlushedToken();
}

void GrTargetCommands::StencilPath::execute(GrBatchFlushState* state) {
    GrPathRendering::StencilPathArgs args(fUseHWAA, fRenderTarget.get(), &fViewMatrix, &fScissor,
                                          &fStencil);
    state->gpu()->pathRendering()->stencilPath(args, this->path());
}

void GrTargetCommands::DrawPath::execute(GrBatchFlushState* state) {
    if (!fState->fCompiled) {
        state->gpu()->buildProgramDesc(&fState->fDesc, *fState->fPrimitiveProcessor,
                                       *fState->getPipeline(), fState->fBatchTracker);
        fState->fCompiled = true;
    }
    GrPathRendering::DrawPathArgs args(fState->fPrimitiveProcessor.get(), fState->getPipeline(),
                                       &fState->fDesc, &fState->fBatchTracker, &fStencilSettings);
    state->gpu()->pathRendering()->drawPath(args, this->path());
}

void GrTargetCommands::DrawPaths::execute(GrBatchFlushState* state) {
    if (!fState->fCompiled) {
        state->gpu()->buildProgramDesc(&fState->fDesc, *fState->fPrimitiveProcessor,
                                       *fState->getPipeline(), fState->fBatchTracker);
        fState->fCompiled = true;
    }
    GrPathRendering::DrawPathArgs args(fState->fPrimitiveProcessor.get(), fState->getPipeline(),
                                       &fState->fDesc, &fState->fBatchTracker, &fStencilSettings);
    state->gpu()->pathRendering()->drawPaths(args, this->pathRange(), fIndices, fIndexType,
                                             fTransforms, fTransformType, fCount);
}

void GrTargetCommands::DrawBatch::execute(GrBatchFlushState* state) {
    // TODO: encapsulate the specialization of GrVertexBatch in GrVertexBatch so that we can
    // remove this cast. Currently all GrDrawBatches are in fact GrVertexBatch.
    GrVertexBatch* vertexBatch = static_cast<GrVertexBatch*>(fBatch.get());
    vertexBatch->issueDraws(state);
}


void GrTargetCommands::Clear::execute(GrBatchFlushState* state) {
    if (GrColor_ILLEGAL == fColor) {
        state->gpu()->discard(this->renderTarget());
    } else {
        state->gpu()->clear(fRect, fColor, this->renderTarget());
    }
}

void GrTargetCommands::ClearStencilClip::execute(GrBatchFlushState* state) {
    state->gpu()->clearStencilClip(fRect, fInsideClip, this->renderTarget());
}

void GrTargetCommands::CopySurface::execute(GrBatchFlushState* state) {
    state->gpu()->copySurface(this->dst(), this->src(), fSrcRect, fDstPoint);
}

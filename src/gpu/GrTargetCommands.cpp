/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTargetCommands.h"

#include "GrBufferedDrawTarget.h"

GrBATCH_SPEW(int32_t GrTargetCommands::Cmd::gUniqueID = 0;)

void GrTargetCommands::reset() {
    fCmdBuffer.reset();
    fBatchTarget.reset();
}

void GrTargetCommands::flush(GrBufferedDrawTarget* bufferedDrawTarget) {
    GrBATCH_INFO("Flushing\n");
    if (fCmdBuffer.empty()) {
        return;
    }

    GrGpu* gpu = bufferedDrawTarget->getGpu();

    // Loop over all batches and generate geometry
    CmdBuffer::Iter genIter(fCmdBuffer);
    while (genIter.next()) {
        if (Cmd::kDrawBatch_CmdType == genIter->type()) {
            DrawBatch* db = reinterpret_cast<DrawBatch*>(genIter.get());
            fBatchTarget.resetNumberOfDraws();
            // TODO: encapsulate the specialization of GrVertexBatch in GrVertexBatch so that we can
            // remove this cast. Currently all GrDrawBatches are in fact GrVertexBatch.
            GrVertexBatch* vertexBatch = static_cast<GrVertexBatch*>(db->batch());

            vertexBatch->generateGeometry(&fBatchTarget);
            vertexBatch->setNumberOfDraws(fBatchTarget.numberOfDraws());
        }
    }

    fBatchTarget.preFlush();

    CmdBuffer::Iter iter(fCmdBuffer);

    while (iter.next()) {
        iter->execute(gpu);
    }

    fBatchTarget.postFlush();
}

void GrTargetCommands::StencilPath::execute(GrGpu* gpu) {
    GrPathRendering::StencilPathArgs args(fUseHWAA, fRenderTarget.get(), &fViewMatrix, &fScissor,
                                          &fStencil);
    gpu->pathRendering()->stencilPath(args, this->path());
}

void GrTargetCommands::DrawPath::execute(GrGpu* gpu) {
    if (!fState->fCompiled) {
        gpu->buildProgramDesc(&fState->fDesc, *fState->fPrimitiveProcessor, *fState->getPipeline(),
                              fState->fBatchTracker);
        fState->fCompiled = true;
    }
    GrPathRendering::DrawPathArgs args(fState->fPrimitiveProcessor.get(), fState->getPipeline(),
                                       &fState->fDesc, &fState->fBatchTracker, &fStencilSettings);
    gpu->pathRendering()->drawPath(args, this->path());
}

void GrTargetCommands::DrawPaths::execute(GrGpu* gpu) {
    if (!fState->fCompiled) {
        gpu->buildProgramDesc(&fState->fDesc, *fState->fPrimitiveProcessor, *fState->getPipeline(),
                              fState->fBatchTracker);
        fState->fCompiled = true;
    }
    GrPathRendering::DrawPathArgs args(fState->fPrimitiveProcessor.get(), fState->getPipeline(),
                                       &fState->fDesc, &fState->fBatchTracker, &fStencilSettings);
    gpu->pathRendering()->drawPaths(args, this->pathRange(), fIndices, fIndexType, fTransforms,
                                    fTransformType, fCount);
}

void GrTargetCommands::DrawBatch::execute(GrGpu* gpu) {
    // TODO: encapsulate the specialization of GrVertexBatch in GrVertexBatch so that we can
    // remove this cast. Currently all GrDrawBatches are in fact GrVertexBatch.
    const GrVertexBatch* vertexBatch = static_cast<const GrVertexBatch*>(fBatch.get());
    fBatchTarget->flushNext(vertexBatch->numberOfDraws());
}

void GrTargetCommands::Clear::execute(GrGpu* gpu) {
    if (GrColor_ILLEGAL == fColor) {
        gpu->discard(this->renderTarget());
    } else {
        gpu->clear(fRect, fColor, this->renderTarget());
    }
}

void GrTargetCommands::ClearStencilClip::execute(GrGpu* gpu) {
    gpu->clearStencilClip(fRect, fInsideClip, this->renderTarget());
}

void GrTargetCommands::CopySurface::execute(GrGpu* gpu) {
    gpu->copySurface(this->dst(), this->src(), fSrcRect, fDstPoint);
}

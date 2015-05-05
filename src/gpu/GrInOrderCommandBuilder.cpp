/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrInOrderCommandBuilder.h"

#include "GrColor.h"
#include "GrInOrderDrawBuffer.h"
#include "GrTemplates.h"
#include "SkPoint.h"

static bool path_fill_type_is_winding(const GrStencilSettings& pathStencilSettings) {
    static const GrStencilSettings::Face pathFace = GrStencilSettings::kFront_Face;
    bool isWinding = kInvert_StencilOp != pathStencilSettings.passOp(pathFace);
    if (isWinding) {
        // Double check that it is in fact winding.
        SkASSERT(kIncClamp_StencilOp == pathStencilSettings.passOp(pathFace));
        SkASSERT(kIncClamp_StencilOp == pathStencilSettings.failOp(pathFace));
        SkASSERT(0x1 != pathStencilSettings.writeMask(pathFace));
        SkASSERT(!pathStencilSettings.isTwoSided());
    }
    return isWinding;
}

GrTargetCommands::Cmd* GrInOrderCommandBuilder::recordDrawBatch(State* state, GrBatch* batch) {
    // Check if there is a Batch Draw we can batch with
    if (!this->cmdBuffer()->empty() &&
        Cmd::kDrawBatch_CmdType == this->cmdBuffer()->back().type()) {
        DrawBatch* previous = static_cast<DrawBatch*>(&this->cmdBuffer()->back());
        if (previous->fState == state && previous->fBatch->combineIfPossible(batch)) {
            return NULL;
        }
    }

    return GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), DrawBatch, (state, batch,
                                                                    this->batchTarget()));
}

GrTargetCommands::Cmd*
GrInOrderCommandBuilder::recordStencilPath(const GrPipelineBuilder& pipelineBuilder,
                                           const GrPathProcessor* pathProc,
                                           const GrPath* path,
                                           const GrScissorState& scissorState,
                                           const GrStencilSettings& stencilSettings) {
    StencilPath* sp = GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), StencilPath,
                                               (path, pipelineBuilder.getRenderTarget()));

    sp->fScissor = scissorState;
    sp->fUseHWAA = pipelineBuilder.isHWAntialias();
    sp->fViewMatrix = pathProc->viewMatrix();
    sp->fStencil = stencilSettings;
    return sp;
}

GrTargetCommands::Cmd*
GrInOrderCommandBuilder::recordDrawPath(State* state,
                                        const GrPathProcessor* pathProc,
                                        const GrPath* path,
                                        const GrStencilSettings& stencilSettings) {
    DrawPath* dp = GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), DrawPath, (state, path));
    dp->fStencilSettings = stencilSettings;
    return dp;
}

GrTargetCommands::Cmd*
GrInOrderCommandBuilder::recordDrawPaths(State* state,
                                         GrInOrderDrawBuffer* iodb,
                                         const GrPathProcessor* pathProc,
                                         const GrPathRange* pathRange,
                                         const void* indexValues,
                                         GrDrawTarget::PathIndexType indexType,
                                         const float transformValues[],
                                         GrDrawTarget::PathTransformType transformType,
                                         int count,
                                         const GrStencilSettings& stencilSettings,
                                         const GrDrawTarget::PipelineInfo& pipelineInfo) {
    SkASSERT(pathRange);
    SkASSERT(indexValues);
    SkASSERT(transformValues);

    char* savedIndices;
    float* savedTransforms;

    iodb->appendIndicesAndTransforms(indexValues, indexType,
                                     transformValues, transformType,
                                     count, &savedIndices, &savedTransforms);

    if (!this->cmdBuffer()->empty() &&
        Cmd::kDrawPaths_CmdType == this->cmdBuffer()->back().type()) {
        // The previous command was also DrawPaths. Try to collapse this call into the one
        // before. Note that stenciling all the paths at once, then covering, may not be
        // equivalent to two separate draw calls if there is overlap. Blending won't work,
        // and the combined calls may also cancel each other's winding numbers in some
        // places. For now the winding numbers are only an issue if the fill is even/odd,
        // because DrawPaths is currently only used for glyphs, and glyphs in the same
        // font tend to all wind in the same direction.
        DrawPaths* previous = static_cast<DrawPaths*>(&this->cmdBuffer()->back());
        if (pathRange == previous->pathRange() &&
            indexType == previous->fIndexType &&
            transformType == previous->fTransformType &&
            stencilSettings == previous->fStencilSettings &&
            path_fill_type_is_winding(stencilSettings) &&
            !pipelineInfo.willBlendWithDst(pathProc) &&
            previous->fState == state) {
                const int indexBytes = GrPathRange::PathIndexSizeInBytes(indexType);
                const int xformSize = GrPathRendering::PathTransformSize(transformType);
                if (&previous->fIndices[previous->fCount*indexBytes] == savedIndices &&
                    (0 == xformSize ||
                     &previous->fTransforms[previous->fCount*xformSize] == savedTransforms)) {
                    // Fold this DrawPaths call into the one previous.
                    previous->fCount += count;
                    return NULL;
                }
        }
    }

    DrawPaths* dp = GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), DrawPaths, (state, pathRange));
    dp->fIndices = savedIndices;
    dp->fIndexType = indexType;
    dp->fTransforms = savedTransforms;
    dp->fTransformType = transformType;
    dp->fCount = count;
    dp->fStencilSettings = stencilSettings;
    return dp;
}

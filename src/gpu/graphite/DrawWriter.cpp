/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawWriter.h"

#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawCommands.h"

namespace skgpu::graphite {

DrawWriter::DrawWriter(DrawPassCommands::List* commandList, DrawBufferManager* bufferManager)
        : fCommandList(commandList)
        , fManager(bufferManager)
        , fRenderState(RenderStateFlags::kNone)
        , fPrimitiveType(PrimitiveType::kTriangles)
        , fVertexStride(0)
        , fInstanceStride(0)
        , fVertices()
        , fIndices()
        , fInstances()
        , fBoundVertices()
        , fBoundIndices()
        , fBoundInstances()
        , fTemplateCount(0)
        , fPendingCount(0) {
    SkASSERT(commandList && bufferManager);
}

void DrawWriter::setTemplate(BindBufferInfo vertices,
                             BindBufferInfo indices,
                             BindBufferInfo instances,
                             unsigned int templateCount) {
    if (fPendingCount == 0) {
        // A pendingCount of zero indicates that a either a newPipelineState() or dynamicState()
        // triggered a flush; thus we want to update the member buffers to the incoming buffers.
        fVertices = vertices;
        fIndices = indices;
        fInstances = instances;
        fTemplateCount = templateCount;
    } else {
        // If there is non-zero pending count, we haven't called newPipelineState() yet, and we
        // aren't expecting RenderSteps to append data with different buffers but the same pipeline.
        SkASSERT(fVertices == vertices && fInstances == instances && fIndices == indices);
        SkASSERT((templateCount == 0 &&
                  (fRenderState & RenderStateFlags::kAppendDynamicInstances)) ||
                 fTemplateCount == templateCount);
    }
    // NOTE: In the case that a new buffer is acquired in reserve() BUT a subsequent setTemplate is
    // called without a newPipelineState() or dynamicState() call, the pendingCount will be zero,
    // and thus the above assert will be safe.
}

void DrawWriter::flush() {
    // Skip flush if no items appended, or dynamic instances resolved to zero count.
    if (fPendingCount == 0 ||
        ((fRenderState & RenderStateFlags::kAppendDynamicInstances) && fTemplateCount == 0)) {
        return;
    }

    // Calculate base offsets from buffer info for the draw commands.
    // - If a valid bufferFoo exists and  matches the current stride, use pendingBaseFoo as a
    //   pseudo-alias for offset and reset the offset and size before assigning to boundBufferFoo.
    // - If a valid bufferFoo does not exist, or is not stride aligned, then draw from the start of
    //   the offset with pendingBaseFoo = 0 and assign the current buffer.
    bool rebindDrawBuffers = false;
    auto bind = [&](const BindBufferInfo& buffer,
                    uint32_t stride,
                    BindBufferInfo& boundBuffer,
                    uint32_t& pendingBase) {
        if (buffer.fBuffer) {
            BindBufferInfo newBinding = buffer;
            if (buffer.fOffset % stride == 0) {
                pendingBase = buffer.fOffset / stride;
                newBinding = {buffer.fBuffer, 0, SkTo<uint32_t>(buffer.fBuffer->size())};
            }
            rebindDrawBuffers |= boundBuffer != newBinding;
            boundBuffer = newBinding;
        }
    };

    uint32_t pendingBaseIndices = 0;
    uint32_t pendingBaseVertex = 0;
    uint32_t pendingBaseInstance = 0;
    bind(fVertices, fVertexStride, fBoundVertices, pendingBaseVertex);
    bind(fInstances, fInstanceStride, fBoundInstances, pendingBaseInstance);
    bind(fIndices, sizeof(uint16_t), fBoundIndices, pendingBaseIndices);

    if (rebindDrawBuffers) {
        fCommandList->bindDrawBuffers(fBoundVertices, fBoundInstances, fBoundIndices, {});
    }

    // Before any draw commands are added, check if the DrawWriter has an assigned barrier type
    // to issue prior to draw calls.
    if (fBarrierToIssueBeforeDraws.has_value()) {
        fCommandList->addBarrier(fBarrierToIssueBeforeDraws.value());
    }

    // Issue the appropriate draw call (instanced vs. non-instanced) based on the current
    // fTemplateCount. Because of the initial AppendDynamicInstances && fTemplateCount check, any
    // DynamicInstance render step must have valid (positive) templateCount data at this point; thus
    // this check covers RenderStateFlags:: AppendInstances, AppendDynamicInstances, and Fixed.
    if (fTemplateCount) {
        SkASSERT((pendingBaseInstance + fPendingCount)*fInstanceStride <= fBoundInstances.fSize);
        if (fIndices) {
            // It's not possible to validate that the indices stored in fIndices access only valid
            // data within fVertices. Simply validate that fIndices holds enough data for the
            // vertex count that's drawn.
            SkASSERT(fTemplateCount*sizeof(uint16_t) <= fBoundIndices.fSize);
            fCommandList->drawIndexedInstanced(fPrimitiveType,
                                               pendingBaseIndices,
                                               fTemplateCount,
                                               pendingBaseVertex,
                                               pendingBaseInstance,
                                               fPendingCount);

        } else {
            SkASSERT(fTemplateCount*fVertexStride <= fBoundVertices.fSize);
            fCommandList->drawInstanced(fPrimitiveType, pendingBaseVertex, fTemplateCount,
                                        pendingBaseInstance, fPendingCount);
        }

        // Clear instancing template state after the draw is recorded for non-Fixed RenderState
        if (fRenderState &
            (RenderStateFlags::kAppendInstances | RenderStateFlags::kAppendDynamicInstances)) {
            fInstances = {};
            if (fRenderState & RenderStateFlags::kAppendDynamicInstances) {
                fTemplateCount = 0;
            }
        }
    } else {
        // Issue non-instanced draw call (indexed or non-indexed).
        SkASSERT(!fInstances); // Should not have an instance buffer in non-instanced mode.
        if (fIndices) {
            // As before, just validate there is sufficient index data
            SkASSERT(fPendingCount*sizeof(uint16_t) <= fBoundIndices.fSize);
            fCommandList->drawIndexed(fPrimitiveType,
                                      pendingBaseIndices,
                                      fPendingCount,
                                      pendingBaseVertex);
        } else {
            SkASSERT((pendingBaseVertex + fPendingCount)*fVertexStride <= fBoundVertices.fSize);
            fCommandList->draw(fPrimitiveType, pendingBaseVertex, fPendingCount);
        }

        // Clear vertex template state after the draw is recorded for non-Fixed RenderState
        if (fRenderState & RenderStateFlags::kAppendVertices) {
            fVertices = {};
        }
    }

    // Mark all appended items as drawn.
    fPendingCount = 0;
}

} // namespace skgpu::graphite

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
        , fStaticStride(0)
        , fAppendStride(0)
        , fAppend({})
        , fStatic({})
        , fIndices({})
        , fBoundAppend({})
        , fBoundStatic({})
        , fBoundIndices({})
        , fTemplateCount(0)
        , fPendingCount(0) {
    SkASSERT(commandList && bufferManager);
}

void DrawWriter::setTemplate(BindBufferInfo staticData, BindBufferInfo indices,
                             BindBufferInfo appendData, uint32_t templateCount) {
    if (fPendingCount == 0) {
        // A pendingCount of zero indicates that a either a newPipelineState() or dynamicState()
        // triggered a flush, so we want to update the incoming member buffers.
        fAppend = appendData;
        fStatic = staticData;
        fIndices = indices;
        fTemplateCount = templateCount;
    } else {
        // IF non-zero pending count, then we must not have flushed, so we cannot have called a new
        // pipeline yet. So we know the buffers MUST be the same.
        // IF a new buffer is acquired in reserve(), it calls a flush on the previous binding. The
        // flush sets the pendingCount to zero, skipping this code path.
        SkASSERT(fAppend == appendData && fStatic == staticData && fIndices == indices);
        SkASSERT((templateCount == 0 &&
                 (fRenderState & RenderStateFlags::kAppendDynamicInstances)) ||
                 fTemplateCount == templateCount);
    }
}

void DrawWriter::flushInternal() {
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
    auto bind = [](const BindBufferInfo& buffer, uint32_t stride, BindBufferInfo& boundBuffer,
                   uint32_t& pendingBase) -> bool {
        bool shouldBind = false;
        if (buffer.fBuffer) {
            BindBufferInfo newBinding = buffer;
            if (buffer.fOffset % stride == 0) {
                pendingBase = buffer.fOffset / stride;
                newBinding = {buffer.fBuffer, 0, SkTo<uint32_t>(buffer.fBuffer->size())};
            }
            shouldBind = boundBuffer != newBinding;
            boundBuffer = newBinding;
        }
        return shouldBind;
    };

    uint32_t pendingBaseAppend = 0;
    uint32_t pendingBaseStatic = 0;
    uint32_t pendingBaseIndices = 0;
    if (bind(fAppend, fAppendStride, fBoundAppend, pendingBaseAppend)) {
        fCommandList->bindAppendDataBuffer(fBoundAppend);
    }
    if (bind(fStatic, fStaticStride, fBoundStatic, pendingBaseStatic)) {
        fCommandList->bindStaticDataBuffer(fBoundStatic);
    }
    if (bind(fIndices, sizeof(uint16_t), fBoundIndices, pendingBaseIndices)) {
        fCommandList->bindIndexBuffer(fBoundIndices);
    }

    // Before any draw commands are added, check if the DrawWriter has an assigned barrier type
    // to issue prior to draw calls.
    if (fBarrierToIssueBeforeDraws.has_value()) {
        fCommandList->addBarrier(fBarrierToIssueBeforeDraws.value());
    }

    // Issue the appropriate draw call (instanced vs. non-instanced) based on the current
    // fTemplateCount. Because of the initial AppendDynamicInstances && fTemplateCount check, any
    // DynamicInstance render step must have valid (non-zero) templateCount data at this point.
    if (fTemplateCount) {
        SkASSERT((pendingBaseAppend + fPendingCount)*fAppendStride <= fBoundAppend.fSize);
        if (fIndices) {
            // It's not possible to validate that the indices stored in fIndices access only valid
            // data within fVertices. Simply validate that fIndices holds enough data for the
            // vertex count that's drawn.
            SkASSERT(fTemplateCount*sizeof(uint16_t) <= fIndices.fSize);
            fCommandList->drawIndexedInstanced(fPrimitiveType,
                                               pendingBaseIndices,
                                               fTemplateCount,
                                               pendingBaseStatic,
                                               pendingBaseAppend,
                                               fPendingCount);

        } else {
            SkASSERT(fTemplateCount*fStaticStride <= fStatic.fSize);
            fCommandList->drawInstanced(fPrimitiveType, pendingBaseStatic, fTemplateCount,
                                        pendingBaseAppend, fPendingCount);
        }

        // Clear instancing template state after the draw is recorded for non-Fixed RenderState
        if (fRenderState &
            (RenderStateFlags::kAppendInstances | RenderStateFlags::kAppendDynamicInstances)) {
            fAppend = {};
            if (fRenderState & RenderStateFlags::kAppendDynamicInstances) {
                fTemplateCount = 0;
            }
        }
    } else {
        // Issue non-instanced draw call (indexed or non-indexed).
        if (fIndices) {
            // As before, just validate there is sufficient index data
            SkASSERT(fPendingCount*sizeof(uint16_t) <= fIndices.fSize);
            fCommandList->drawIndexed(fPrimitiveType,
                                      pendingBaseIndices,
                                      fPendingCount,
                                      pendingBaseAppend);
        } else {
            SkASSERT((pendingBaseAppend + fPendingCount)*fStaticStride <= fBoundAppend.fSize);
            fCommandList->draw(fPrimitiveType, pendingBaseAppend, fPendingCount);
        }

        // Clear vertex template state after the draw is recorded for non-Fixed RenderState
        if (fRenderState & RenderStateFlags::kAppendVertices) {
            fAppend = {};
        }
    }

    // Mark all appended items as drawn.
    fPendingCount = 0;
}

} // namespace skgpu::graphite

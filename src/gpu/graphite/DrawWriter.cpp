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
        , fPrimitiveType(PrimitiveType::kTriangles)
        , fVertexStride(0)
        , fInstanceStride(0)
        , fVertices()
        , fIndices()
        , fInstances()
        , fTemplateCount(0)
        , fPendingCount(0)
        , fPendingBase(0)
        , fPendingBufferBinds(true) {
    SkASSERT(commandList && bufferManager);
}

void DrawWriter::setTemplate(BindBufferInfo vertices,
                             BindBufferInfo indices,
                             BindBufferInfo instances,
                             int templateCount) {
    if (vertices != fVertices || instances != fInstances || fIndices != indices) {
        if (fPendingCount > 0) {
            this->flush();
        }

        bool willAppendVertices = templateCount == 0;
        bool isAppendingVertices = fTemplateCount == 0;
        if (willAppendVertices != isAppendingVertices ||
            (isAppendingVertices && fVertices != vertices) ||
            (!isAppendingVertices && fInstances != instances)) {
            // The buffer binding target for appended data is changing, so reset the base offset
            fPendingBase = 0;
        }

        fVertices = vertices;
        fInstances = instances;
        fIndices = indices;

        fTemplateCount = templateCount;

        fPendingBufferBinds = true;
    } else if ((templateCount >= 0 && templateCount != fTemplateCount) || // vtx or reg. instances
               (templateCount < 0 && fTemplateCount >= 0)) {              // dynamic index instances
        if (fPendingCount > 0) {
            this->flush();
        }
        if ((templateCount == 0) != (fTemplateCount == 0)) {
            // Switching from appending vertices to instances, or vice versa, so the pending
            // base vertex for appended data is invalid
            fPendingBase = 0;
        }
        fTemplateCount = templateCount;
    }

    SkASSERT(fVertices  == vertices);
    SkASSERT(fInstances == instances);
    SkASSERT(fIndices   == indices);
    // NOTE: This allows 'fTemplateCount' to update across multiple DynamicInstances as long
    // as they have the same vertex and index buffers.
    SkASSERT((fTemplateCount < 0) == (templateCount < 0));
    SkASSERT(fTemplateCount < 0 || fTemplateCount == templateCount);
}

void DrawWriter::flush() {
    // If nothing was appended, or the only appended data was through dynamic instances and the
    // final vertex count per instance is 0 (-1 in the sign encoded field), nothing should be drawn.
    if (fPendingCount == 0 || fTemplateCount == -1) {
        return;
    }
    if (fPendingBufferBinds) {
        fCommandList->bindDrawBuffers(fVertices, fInstances, fIndices, {});
        fPendingBufferBinds = false;
    }

    if (fTemplateCount) {
        // Instanced drawing
        unsigned int realVertexCount;
        if (fTemplateCount < 0) {
            realVertexCount = -fTemplateCount - 1;
            fTemplateCount = -1; // reset to re-accumulate max index account for next flush
        } else {
            realVertexCount = fTemplateCount;
        }

        if (fIndices) {
            fCommandList->drawIndexedInstanced(fPrimitiveType, 0, realVertexCount, 0,
                                               fPendingBase, fPendingCount);
        } else {
            fCommandList->drawInstanced(fPrimitiveType, 0, realVertexCount,
                                        fPendingBase, fPendingCount);
        }
    } else {
        SkASSERT(!fInstances);
        if (fIndices) {
            fCommandList->drawIndexed(fPrimitiveType, 0, fPendingCount, fPendingBase);
        } else {
            fCommandList->draw(fPrimitiveType, fPendingBase, fPendingCount);
        }
    }

    fPendingBase += fPendingCount;
    fPendingCount = 0;
}

} // namespace skgpu::graphite

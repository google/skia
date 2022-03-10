/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawWriter.h"

#include "experimental/graphite/src/DrawBufferManager.h"
#include "src/gpu/BufferWriter.h"

namespace skgpu {

DrawWriter::DrawWriter(DrawDispatcher* dispatcher, DrawBufferManager* bufferManager)
        : DrawWriter(dispatcher, bufferManager, PrimitiveType::kTriangles, 0, 0) {}

DrawWriter::DrawWriter(DrawDispatcher* dispatcher,
                       DrawBufferManager* bufferManager,
                       PrimitiveType primitiveType,
                       size_t vertexStride,
                       size_t instanceStride)
        : fDispatcher(dispatcher)
        , fManager(bufferManager)
        , fPrimitiveType(primitiveType)
        , fVertexStride(vertexStride)
        , fInstanceStride(instanceStride)
        , fVertices()
        , fIndices()
        , fInstances()
        , fTemplateCount(0)
        , fPendingCount(0)
        , fPendingBase(0)
        , fPendingBufferBinds(true) {
    SkASSERT(dispatcher && bufferManager);
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
    if (fPendingCount == 0) {
        return;
    }
    if (fPendingBufferBinds) {
        fDispatcher->bindDrawBuffers(fVertices, fInstances, fIndices);
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
            fDispatcher->drawIndexedInstanced(fPrimitiveType, 0, realVertexCount, 0,
                                              fPendingBase, fPendingCount);
        } else {
            fDispatcher->drawInstanced(fPrimitiveType, 0, realVertexCount,
                                       fPendingBase, fPendingCount);
        }
    } else {
        SkASSERT(!fInstances);
        if (fIndices) {
            fDispatcher->drawIndexed(fPrimitiveType, 0, fPendingCount, fPendingBase);
        } else {
            fDispatcher->draw(fPrimitiveType, fPendingBase, fPendingCount);
        }
    }

    fPendingBase += fPendingCount;
    fPendingCount = 0;
}

} // namespace skgpu

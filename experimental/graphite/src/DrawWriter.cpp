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
        , fInstanceStride(instanceStride) {
    SkASSERT(dispatcher && bufferManager);
}

void DrawWriter::setTemplateInternal(BindBufferInfo vertices,
                                     BindBufferInfo indices,
                                     unsigned int count,
                                     bool drawPendingVertices) {
    SkASSERT(!vertices || fVertexStride > 0);
    if (vertices != fFixedVertexBuffer ||
        indices != fFixedIndexBuffer ||
        count != fFixedVertexCount) {
        // Issue any accumulated data that referred to the old template.
        if (drawPendingVertices) {
            this->drawPendingVertices();
        }

        fFixedBuffersDirty = true;

        fFixedVertexBuffer = vertices;
        fFixedIndexBuffer = indices;
        fFixedVertexCount = count;
    }
}

void DrawWriter::drawInternal(BindBufferInfo instances,
                              unsigned int base,
                              unsigned int instanceCount) {
    // Draw calls that are only 1 instance and have no extra instance data get routed to
    // the simpler draw APIs.
    // TODO: Is there any benefit to this? Does it help hint to drivers? Avoid more bugs?
    // Or should we always call drawInstanced and drawIndexedInstanced?
    const bool useInstancedDraw = fInstanceStride > 0 || instanceCount > 1;
    SkASSERT(useInstancedDraw ||
             (fInstanceStride == 0 && instanceCount == 1 && !SkToBool(instances)));

    // Issue new buffer binds only as necessary
    // TODO: Should this instead be the responsibility of the CB or DrawDispatcher to remember
    // what was last bound?
    if (fFixedBuffersDirty || instances != fLastInstanceBuffer) {
        fDispatcher->bindDrawBuffers(fFixedVertexBuffer, instances, fFixedIndexBuffer);
        fFixedBuffersDirty = false;
        fLastInstanceBuffer = instances;
    }

    if (useInstancedDraw) {
        // 'base' offsets accumulated instance data (or is 0 for a direct instanced draw). It is
        // assumed that any base vertex and index have been folded into the BindBufferInfos already.
        if (fFixedIndexBuffer) {
            fDispatcher->drawIndexedInstanced(fPrimitiveType, 0, fFixedVertexCount, 0,
                                              base, instanceCount);
        } else {
            fDispatcher->drawInstanced(fPrimitiveType, 0, fFixedVertexCount, base, instanceCount);
        }
    } else {
        if (fFixedIndexBuffer) {
            // Should only get here from a direct draw, in which case base should be 0 and any
            // offset needs to be embedded in the BindBufferInfo by caller.
            SkASSERT(base == 0);
            fDispatcher->drawIndexed(fPrimitiveType, 0, fFixedVertexCount, 0);
        } else {
            // 'base' offsets accumulated vertex data from another DrawWriter across a state change.
            fDispatcher->draw(fPrimitiveType, base, fFixedVertexCount);
        }
    }
}

void DrawWriter::drawPendingVertices() {
    if (fPendingCount > 0) {
        if (fPendingMode == VertexMode::kInstances) {
            // This uses instanced draws, so 'base' will be interpreted in instance units.
            this->drawInternal(fPendingAttrs, fPendingBaseVertex, fPendingCount);
        } else {
            // This triggers a non-instanced draw call so 'base' passed to drawInternal is
            // interpreted in vertex units.
            this->setTemplateInternal(fPendingAttrs, {}, fPendingCount, /*drawPending=*/false);
            this->drawInternal({}, fPendingBaseVertex, 1);
        }

        fPendingCount = 0;
        fPendingBaseVertex = 0;
        fPendingAttrs = {};
    }
}

VertexWriter DrawWriter::appendData(VertexMode mode, size_t stride, unsigned int count) {
    if (fPendingMode != mode) {
        // Switched between accumulating vertices and instances, so issue draws for the old data.
        this->drawPendingVertices();
        fPendingMode = mode;
    }

    auto [writer, nextChunk] = fManager->getVertexWriter(count * stride);
    // Check if next chunk's data is contiguous with what's previously been appended
    if (nextChunk.fBuffer == fPendingAttrs.fBuffer &&
        fPendingAttrs.fOffset + (fPendingBaseVertex + fPendingCount) * stride
                == nextChunk.fOffset) {
        // It is, so the next chunk's vertices that will be written can be folded into the next draw
        fPendingCount += count;
    } else {
        // Alignment mismatch, or the old buffer filled up
        this->drawPendingVertices();
        fPendingCount = count;
        fPendingBaseVertex = 0;
        fPendingAttrs = nextChunk;
    }

    return std::move(writer);
}

void DrawWriter::newDynamicState() {
    // Remember where we left off after we draw, since drawPendingVertices() resets all pending data
    BindBufferInfo base = fPendingAttrs;
    unsigned int baseVertex = fPendingBaseVertex + fPendingCount;
    // Draw anything that used the previous dynamic state
    this->drawPendingVertices();

    fPendingAttrs = base;
    fPendingBaseVertex = baseVertex;
}

void DrawWriter::newPipelineState(PrimitiveType type,
                                  size_t vertexStride,
                                  size_t instanceStride) {
    // Draw anything that used the previous pipeline
    this->drawPendingVertices();

    // For simplicity, if there's a new pipeline, just forget about any previous buffer bindings,
    // in which case the new writer only needs to use the dispatcher and buffer manager.
    this->setTemplateInternal({}, {}, 0, false);
    fLastInstanceBuffer = {};

    fPrimitiveType = type;
    fVertexStride = vertexStride;
    fInstanceStride = instanceStride;
}

} // namespace skgpu

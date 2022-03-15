/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawWriter_DEFINED
#define skgpu_DrawWriter_DEFINED

#include "experimental/graphite/src/DrawBufferManager.h"
#include "experimental/graphite/src/DrawTypes.h"
#include "src/gpu/BufferWriter.h"

namespace skgpu {

class DrawDispatcher; // Forward declaration, handles virtual dispatch of binds/draws

/**
 * DrawWriter is a helper around recording draws (to a temporary buffer or directly to a
 * CommandBuffer), particularly when the number of draws is not known ahead of time, or the vertex
 * and instance data is computed at record time and does not have a known size.
 *
 * To use, construct the DrawWriter with the current pipeline layout or call newPipelineState() on
 * an existing DrawWriter and then bind that matching pipeline. When other dynamic state needs to
 * change between draw calls, notify the DrawWriter using newDynamicState() before recording the
 * modifications. See the listing below for how to append dynamic data or draw with existing buffers
 *
 * CommandBuffer::draw(vertices)
 *  - dynamic vertex data     -> DrawWriter::Vertices(writer) verts;
 *                               verts.append(n) << ...;
 *  - fixed vertex data       -> writer.draw(vertices, {}, vertexCount)
 *
 * CommandBuffer::drawIndexed(vertices, indices)
 *  - dynamic vertex data     -> unsupported
 *  - fixed vertex,index data -> writer.drawIndexed(vertices, indices, indexCount)
 *
 * CommandBuffer::drawInstances(vertices, instances)
 *  - dynamic instance data + fixed vertex data        ->
 *        DrawWriter::Instances instances(writer, vertices, {}, vertexCount);
 *        instances.append(n) << ...;
 *  - fixed vertex and instance data                   ->
 *        writer.drawInstanced(vertices, vertexCount, instances, instanceCount)
 *
 * CommandBuffer::drawIndexedInstanced(vertices, indices, instances)
 *  - dynamic instance data + fixed vertex, index data ->
 *        DrawWriter::Instances instances(writer, vertices, indices, indexCount);
 *        instances.append(n) << ...;
 *  - fixed vertex, index, and instance data           ->
 *        writer.drawIndexedInstanced(vertices, indices, indexCount, instances, instanceCount)
 */
class DrawWriter {
public:
    // NOTE: This constructor creates a writer that defaults 0 vertex and instance stride, so
    // 'newPipelineState()' must be called once the pipeline properties are known before it's used.
    DrawWriter(DrawDispatcher*, DrawBufferManager*);

    DrawWriter(DrawDispatcher*, DrawBufferManager*,
               PrimitiveType type, size_t vertexStride, size_t instanceStride);

    // Cannot move or copy
    DrawWriter(const DrawWriter&) = delete;
    DrawWriter(DrawWriter&&) = delete;

    // flush() should be called before the writer is destroyed
    ~DrawWriter() { SkASSERT(fPendingCount == 0); }

    DrawBufferManager* bufferManager() { return fManager; }

    // Issue draw calls for any pending vertex and instance data collected by the writer.
    // Use either flush() or newDynamicState() based on context and readability.
    void flush();
    void newDynamicState() { this->flush(); }

    // Notify the DrawWriter that a new pipeline needs to be bound, providing the primitive type and
    // attribute strides of that pipeline. This issues draw calls for pending data that relied on
    // the old pipeline, so this must be called *before* binding the new pipeline.
    void newPipelineState(PrimitiveType type, size_t vertexStride, size_t instanceStride) {
        this->flush();
        fPrimitiveType = type;
        fVertexStride = vertexStride;
        fInstanceStride = instanceStride;

        // NOTE: resetting pending base is sufficient to redo bindings for vertex/instance data that
        // is later appended but doesn't invalidate bindings for fixed buffers that might not need
        // to change between pipelines.
        fPendingBase = 0;
        SkASSERT(fPendingCount == 0);
    }

#ifdef SK_DEBUG
    // Query current pipeline state for validation
    size_t        instanceStride() const { return fInstanceStride; }
    size_t        vertexStride()   const { return fVertexStride;   }
    PrimitiveType primitiveType()  const { return fPrimitiveType;  }
#endif

    // Collects new vertex data for a call to CommandBuffer::draw(). Automatically accumulates
    // vertex data into a buffer, issuing draw and bind calls as needed when a new buffer is
    // required, so that it is seamless to the caller. The draws do not use instances or indices.
    //
    // Usage (assuming writer has already had 'newPipelineState()' called with correct strides):
    //    DrawWriter::Vertices verts{writer};
    //    verts.append(n) << x << y << ...;
    //
    // This should not be used when the vertex stride is 0.
    class Vertices;

    // Collects new instance data for a call to CommandBuffer::drawInstanced() or
    // drawIndexedInstanced(). The specific draw call that's issued depends on if a non-null index
    // buffer is provided for the template. Like DrawWriter::Vertices, this automatically merges
    // the appended data into as few buffer binds and draw calls as possible, while remaining
    // seamless to the caller.
    //
    // Usage for drawInstanced (assuming writer has correct strides):
    //    DrawWriter::Instances instances{writer, fixedVerts, {}, fixedVertexCount};
    //    instances.append(n) << foo << bar << ...;
    //
    // Usage for drawIndexedInstanced:
    //    DrawWriter::Instances instances{writer, fixedVerts, fixedIndices, fixedIndexCount};
    //    instances.append(n) << foo << bar << ...;
    //
    // This should not be used when the instance stride is 0. However, the fixed vertex buffer can
    // be null (or have a stride of 0) if the vertex shader only relies on the vertex ID and no
    // other per-vertex data.
    class Instances;

    // Collects new instance data for a call to CommandBuffer::drawInstanced() or
    // drawIndexedInstanced() (depending on presence of index data in the template). Unlike the
    // Instances mode, the template's index or vertex count is not provided at the time of creation.
    // Instead, DynamicInstances can be used with pipeline programs that can have a flexible number
    // of vertices per instance. Appended instances specify the minimum index/vertex count they
    // must be drawn with, but if they are later batched with instances that would use more, the
    // pipeline's vertex shader knows how to handle it.
    //
    // Usage for drawInstanced (fixedIndices == {}) or drawIndexedInstanced:
    //    DrawWriter::DynamicInstances instances(writer, fixedVerts, fixedIndices);
    //    instances.append(minIndexCount1, n1) << ...;
    //    instances.append(minIndexCount2, n2) << ...;
    //
    // In this example, if the two sets of instances were contiguous, a single draw call with
    // (n1 + n2) instances would still be made using max(minIndexCount1, minIndexCount2) as the
    // index/vertex count. If the available vertex data from the DrawBufferManager forced a flush
    // after the first, then the second would use minIndexCount2 unless a subsequent compatible
    // DynamicInstances template appended more contiguous data.
    class DynamicInstances;

    // Issues a draws with fully specified data. This can be used when all instance data has already
    // been written to known buffers, or when the vertex shader only depends on the vertex or
    // instance IDs. To keep things simple, these helpers do not accept parameters for base vertices
    // or instances; if needed, this can be accounted for in the BindBufferInfos provided.
    //
    // This will not merge with any already appended instance or vertex data, pending data is issued
    // in its own draw call first.
    void draw(BindBufferInfo vertices, unsigned int vertexCount) {
        this->bindAndFlush(vertices, {}, {}, 0, vertexCount);
    }
    void drawIndexed(BindBufferInfo vertices, BindBufferInfo indices, unsigned int indexCount) {
        this->bindAndFlush(vertices, indices, {}, 0, indexCount);
    }
    void drawInstanced(BindBufferInfo vertices, unsigned int vertexCount,
                       BindBufferInfo instances, unsigned int instanceCount) {
        SkASSERT(vertexCount > 0);
        this->bindAndFlush(vertices, {}, instances, vertexCount, instanceCount);
    }
    void drawIndexedInstanced(BindBufferInfo vertices, BindBufferInfo indices,
                              unsigned int indexCount, BindBufferInfo instances,
                              unsigned int instanceCount) {
        SkASSERT(indexCount > 0);
        this->bindAndFlush(vertices, indices, instances, indexCount, instanceCount);
    }

private:
    // Both of these pointers must outlive the DrawWriter.
    DrawDispatcher*    fDispatcher;
    DrawBufferManager* fManager;

    // Pipeline state matching currently bound pipeline
    PrimitiveType fPrimitiveType;
    size_t fVertexStride;
    size_t fInstanceStride;

    /// Draw buffer binding state for pending draws
    BindBufferInfo fVertices;
    BindBufferInfo fIndices;
    BindBufferInfo fInstances;
    // Vertex/index count for [pseudo]-instanced rendering:
    // == 0 is vertex-only drawing; > 0 is regular instanced drawing; < 0 is dynamic index count
    // instanced drawing, where real index count = max(-fTemplateCount-1)
    int fTemplateCount;

    unsigned int fPendingCount; // # of vertices or instances (depending on mode) to be drawn
    unsigned int fPendingBase; // vertex/instance offset (depending on mode) applied to buffer
    bool fPendingBufferBinds; // true if {fVertices,fIndices,fInstances} has changed since last draw

    void setTemplate(BindBufferInfo vertices, BindBufferInfo indices, BindBufferInfo instances,
                     int templateCount);
    // NOTE: bindAndFlush's templateCount is unsigned because dynamic index count instancing
    // isn't applicable.
    void bindAndFlush(BindBufferInfo vertices, BindBufferInfo indices, BindBufferInfo instances,
                      unsigned int templateCount, unsigned int drawCount) {
        SkASSERT(drawCount > 0);
        this->setTemplate(vertices, indices, instances, SkTo<int>(templateCount));
        fPendingBase = 0;
        fPendingCount = drawCount;
        this->flush();
    }

    // RAII - Sets the DrawWriter's template and marks the writer in append mode (disabling direct
    // draws until the Appender is destructed).
    class Appender;
    SkDEBUGCODE(const Appender* fAppender = nullptr;)
};

// Mirrors the CommandBuffer API, since a DrawWriter is meant to aggregate and then map onto
// CommandBuffer commands, although these are virtual to allow for recording to intermediate
// storage before a CommandBuffer is available.
class DrawDispatcher {
public:
    virtual ~DrawDispatcher() = default;

    virtual void bindDrawBuffers(BindBufferInfo vertexAttribs,
                                 BindBufferInfo instanceAttribs,
                                 BindBufferInfo indices) = 0;

    virtual void draw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) = 0;
    virtual void drawIndexed(PrimitiveType type, unsigned int baseIndex,
                             unsigned int indexCount, unsigned int baseVertex) = 0;
    virtual void drawInstanced(PrimitiveType type,
                               unsigned int baseVertex, unsigned int vertexCount,
                               unsigned int baseInstance, unsigned int instanceCount) = 0;
    virtual void drawIndexedInstanced(PrimitiveType type,
                                      unsigned int baseIndex, unsigned int indexCount,
                                      unsigned int baseVertex, unsigned int baseInstance,
                                      unsigned int instanceCount) = 0;
};

// Appender implementations for DrawWriter that set the template on creation and provide a
// template-specific API to accumulate vertex/instance data.
class DrawWriter::Appender {
public:
    enum class Target { kVertices, kInstances };

    Appender(DrawWriter& w, Target target)
            : fDrawer(w)
            , fTarget(target == Target::kVertices ? w.fVertices     : w.fInstances)
            , fStride(target == Target::kVertices ? w.fVertexStride : w.fInstanceStride)
            , fReservedCount(0)
            , fNextWriter() {
        SkASSERT(fStride > 0);
        SkASSERT(!w.fAppender);
        SkDEBUGCODE(w.fAppender = this;)
    }

    ~Appender() {
        if (fReservedCount > 0) {
            fDrawer.fManager->returnVertexBytes(fReservedCount * fStride);
        }
        SkASSERT(fDrawer.fAppender == this);
        SkDEBUGCODE(fDrawer.fAppender = nullptr;)
    }

protected:
    DrawWriter&     fDrawer;
    BindBufferInfo& fTarget;
    size_t          fStride;

    unsigned int fReservedCount; // in target stride units
    VertexWriter fNextWriter;    // writing to the target buffer binding

    void reserve(unsigned int count) {
        if (fReservedCount >= count) {
            return;
        } else if (fReservedCount > 0) {
            // Have contiguous bytes that can't satisfy request, so return them in the event the
            // DBM has additional contiguous bytes after the prior reserved range.
            fDrawer.fManager->returnVertexBytes(fReservedCount * fStride);
        }

        fReservedCount = count;
        // NOTE: Cannot bind tuple directly to fNextWriter, compilers don't produce the right
        // move assignment.
        auto [writer, reservedChunk] = fDrawer.fManager->getVertexWriter(count * fStride);
        if (reservedChunk.fBuffer != fTarget.fBuffer ||
            reservedChunk.fOffset !=
                    (fTarget.fOffset + (fDrawer.fPendingBase + fDrawer.fPendingCount) * fStride)) {
            // Not contiguous, so flush and update binding to 'reservedChunk'
            fDrawer.flush();

            fTarget = reservedChunk;
            fDrawer.fPendingBase = 0;
            fDrawer.fPendingBufferBinds = true;
        }
        fNextWriter = std::move(writer);
    }

    VertexWriter append(unsigned int count) {
        SkASSERT(count > 0);
        this->reserve(count);

        SkASSERT(fReservedCount >= count);
        fReservedCount -= count;
        fDrawer.fPendingCount += count;
        return std::exchange(fNextWriter, fNextWriter.makeOffset(count * fStride));
    }
};

class DrawWriter::Vertices : private DrawWriter::Appender {
public:
    Vertices(DrawWriter& w) : Appender(w, Target::kVertices) {
        w.setTemplate(w.fVertices, {}, {}, 0);
    }

    using Appender::reserve;
    using Appender::append;
};

class DrawWriter::Instances : private DrawWriter::Appender {
public:
    Instances(DrawWriter& w,
              BindBufferInfo vertices,
              BindBufferInfo indices,
              unsigned int vertexCount)
            : Appender(w, Target::kInstances) {
        w.setTemplate(vertices, indices, w.fInstances, SkTo<int>(vertexCount));
    }

    using Appender::reserve;
    using Appender::append;
};

class DrawWriter::DynamicInstances : private DrawWriter::Appender {
public:
    DynamicInstances(DrawWriter& w,
                     BindBufferInfo vertices,
                     BindBufferInfo indices)
            : Appender(w, Target::kInstances) {
        w.setTemplate(vertices, indices, w.fInstances, -1);
    }

    using Appender::reserve;

    VertexWriter append(unsigned int indexCount, unsigned int instanceCount) {
        SkASSERT(indexCount > 0);
        VertexWriter w = this->Appender::append(instanceCount);
        // Record index count after appending instance data in case the append triggered a flush
        // or earlier dynamic instances and the max index count is reset. This ensures that the
        // just appended instances will be flushed with a template count at least 'indexCount'.
        fDrawer.fTemplateCount = std::min(fDrawer.fTemplateCount, -SkTo<int>(indexCount) - 1);
        return w;
    }
};

} // namespace skgpu

#endif // skgpu_DrawWriter_DEFINED

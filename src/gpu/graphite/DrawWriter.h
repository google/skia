/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawWriter_DEFINED
#define skgpu_graphite_DrawWriter_DEFINED

#include "src/base/SkAutoMalloc.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/DrawTypes.h"

#include <optional>

namespace skgpu::graphite {

namespace DrawPassCommands {
class List;
}

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
 *
 * NOTE: DrawWriter automatically handles failures to find or create a GPU buffer or map it to
 * be writable. All returned VertexWriters will have a non-null pointer to write to, even if it will
 * be discarded due to GPU failure at Recorder::snap() time.
 */
class DrawWriter {
public:
    // NOTE: This constructor creates a writer that defaults 0 vertex and instance stride, so
    // 'newPipelineState()' must be called once the pipeline properties are known before it's used.
    DrawWriter(DrawPassCommands::List*, DrawBufferManager*);

    // Cannot move or copy
    DrawWriter(const DrawWriter&) = delete;
    DrawWriter(DrawWriter&&) = delete;

    // flush() should be called before the writer is destroyed
    ~DrawWriter() { SkASSERT(fPendingCount == 0); }

    DrawBufferManager* bufferManager() { return fManager; }

    // Issue draw calls for any pending vertex and instance data collected by the writer.
    // Use either flush() or newDynamicState() based on context and readability.
    void flush() {
        this->padAndZero();
        this->flushInternal();
    }
    void newDynamicState() { this->flush(); }

    // Notify the DrawWriter that a new pipeline needs to be bound, providing the primitive type,
    // attribute strides, and render state of the new pipeline. This issues draw calls for pending
    // data that relied on the old pipeline, so this must be called *before* binding new pipeline.
    void newPipelineState(PrimitiveType type,
                          size_t staticStride,
                          size_t appendStride,
                          SkEnumBitMask<RenderStateFlags> newRenderState,
                          std::optional<BarrierType> barrierType) {
        this->flush();

        // Once flushed, any pending data must have been drawn.
        SkASSERT(fPendingCount == 0);

        fPrimitiveType = type;
        fStaticStride = staticStride;
        fAppendStride = appendStride;
        fRenderState = newRenderState;

        // ARM hardware b/399631317: The initial offset when appending vertices must be 4-count
        // aligned, regardless of the previous render state.
        fShouldAlign4 = SkToBool(newRenderState & RenderStateFlags::kAppendVertices);

        // Assign the (optional) barrier type. If a valid value, then the DrawWriter will append
        // AddBarrier commands of the indicated type prior to appending any draw commands used with
        // this pipeline.
        fBarrierToIssueBeforeDraws = barrierType;
    }

#ifdef SK_DEBUG
    // Query current pipeline state for validation
    size_t        appendStride()  const { return fAppendStride;  }
    size_t        staticStride()  const { return fStaticStride;  }
    PrimitiveType primitiveType() const { return fPrimitiveType; }
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
    // of vertices per instance. Appended instances specify a proxy object that can be converted
    // to the minimum index/vertex count they must be drawn with; but if they are later batched with
    // instances that would use more, the pipeline's vertex shader knows how to handle it.
    //
    // The proxy object serves as a useful point of indirection when the actual index count is
    // expensive to compute, but can be derived from correlated geometric properties. The proxy
    // can store those properties and accumulate a "worst-case" and then calculate the index count
    // when DrawWriter has to flush.
    //
    // The VertexCountProxy type must provide:
    //  - a default constructor and copy assignment, where the initial value represents the minimum
    //    supported vertex count.
    //  - an 'unsigned int' operator that converts the proxy to the actual index count that is
    //    needed in order to dispatch a draw call.
    //  - operator <<(const V&) where V is any type the caller wants to pass to append() that
    //    represents the proxy for the about-to-be-written instances. This operator then updates its
    //    internal state to represent the worst case between what had previously been recorded and
    //    the latest V value.
    //
    // Usage for drawInstanced (fixedIndices == {}) or drawIndexedInstanced:
    //    DrawWriter::DynamicInstances<ProxyType> instances(writer, fixedVerts, fixedIndices);
    //    instances.append(minIndexProxy1, n1) << ...;
    //    instances.append(minIndexProxy2, n2) << ...;
    //
    // In this example, if the two sets of instances were contiguous, a single draw call with
    // (n1 + n2) instances would still be made using max(minIndexCount1, minIndexCount2) as the
    // index/vertex count, 'minIndexCountX' was derived from 'minIndexProxyX'. If the available
    // vertex data from the DrawBufferManager forced a flush after the first, then the second would
    // use minIndexCount2 unless a subsequent compatible DynamicInstances template appended more
    // contiguous data.
    template <typename VertexCountProxy>
    class DynamicInstances;

    // Issues draws with fully specified data. This can be used when all instance data has already
    // been written to known buffers, or when the vertex shader only depends on the vertex or
    // instance IDs.
    //
    // This will not merge with any already appended instance or vertex data, pending data is issued
    // in its own draw call first. These are currently unused.
    void draw(BindBufferInfo vertices, unsigned int vertexCount) {
        this->bindAndFlush({}, {}, vertices, 0, vertexCount);
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

#if defined(GPU_TEST_UTILS)
    BindBufferInfo getLastAppendedBuffer() { return fAppend; }
#endif

private:
    // Both of these pointers must outlive the DrawWriter.
    DrawPassCommands::List* fCommandList;
    DrawBufferManager* fManager;

    SkAutoMalloc fFailureStorage; // storage address for VertexWriter when GPU buffer mapping fails

    // Current operating mode of the DrawWriter, dictating how draw data is provided and
    // interpreted. Determines whether fPendingCount refers to vertices or instances, and which
    // buffer (fVertices or fInstances) serves as the append target. Set via newPipelineState().
    SkEnumBitMask<RenderStateFlags> fRenderState;
    PrimitiveType fPrimitiveType;
    uint32_t fStaticStride;
    uint32_t fAppendStride;

    // - fAppend: Holds buffer information for data that is generated and appended during the
    //            drawPass. The data can be either vertex (kAppendVertices) or instance
    //            (kAppendInstances/kAppendDynamicInstances) data.
    // - fStatic: Holds buffer information that does not change between invocations of a renderstep.
    //            Currently this only holds vertex data, but this could change in the future.
    // - Indices: Defines the (for now static) buffer used for any kind of index drawing. A
    //            renderstep with a valid index buffer implies that it will be performing indexed
    //            drawing.
    BindBufferInfo fAppend;
    BindBufferInfo fStatic;
    BindBufferInfo fIndices;
    // These track the buffers *last bound* by the command list. Used to ensure minimal binding.
    BindBufferInfo fBoundAppend;
    BindBufferInfo fBoundStatic;
    BindBufferInfo fBoundIndices;

    // Per-instance count for instanced draws (vertex count if no index buffer, index count
    // otherwise).
    // - For fixed instancing (kAppendInstances): Represents the constant vertex/index count per
    //   instance.
    // - For dynamic instancing (kAppendDynamicInstances): Represents the *maximum* vertex/index
    //   count required across the currently accumulated batch of instances (updated via max()).
    // - Not used (remains 0) for non-instanced draws (kAppendVertices) or direct draw calls.
    uint32_t fTemplateCount;

    // Number of items (vertices or instances, depending on fRenderState) that have been appended
    // via an Appender (Vertices, Instances, DynamicInstances) but not yet issued in a draw call.
    // Reset to 0 after a flush().
    uint32_t fPendingCount;

    // ARM hardware b/399631317: Track whenever a newPipelineState occurs with appending vertices,
    // to let the next reserve() call know that we need a 4 count aligned offset.
    bool fShouldAlign4;

    std::optional<BarrierType> fBarrierToIssueBeforeDraws = std::nullopt;

    void flushInternal();

    // ARM hardware b/399631317: Unreferenced vertices in sequential indexes of 4 will be
    // speculatively executed. To work around this, we pad the buffer by requesting additional
    // space, and then ensure valid, minimally deleterious data by memsetting the padding to zero.
    void padAndZero() {
        if (fPendingCount && (fRenderState & RenderStateFlags::kAppendVertices)) {
            const uint32_t alignedCount = SkAlign4(fPendingCount);
            if (alignedCount > fPendingCount) {
                const uint32_t byteDiff = (alignedCount - fPendingCount) * fAppendStride;
                SkASSERT(!fManager->willVertexOverflow(byteDiff, 1, 1));
                auto[zWriter, zBuff] = fManager->getVertexWriter(byteDiff, 1, 1);
                if (zWriter) {
                    zWriter.zeroBytes(byteDiff);
                }
            }
        }
    }

    void setTemplate(BindBufferInfo staticData, BindBufferInfo indices, BindBufferInfo appendData,
                     uint32_t templateCount);

    void bindAndFlush(BindBufferInfo staticData, BindBufferInfo indices, BindBufferInfo appendData,
                      uint32_t templateCount, unsigned int drawCount) {
        SkASSERT(drawCount > 0);
        SkASSERT(!fAppender); // Shouldn't be appending and manually drawing at the same time.
        SkASSERT(fPendingCount == 0); // Any prior appends must have been flushed by now.
        // CAUTION: If appending vertices, we make NO checks here to ensure that the initial offset
        // is four count aligned or that the data is padded. Caller MUST ensure any unaligned data
        // is safe.
        this->setTemplate(staticData, indices, appendData, templateCount);
        fPendingCount = drawCount;
        this->flush();
    }

    // RAII - Sets the DrawWriter's template and marks the writer in append mode (disabling direct
    // draws until the Appender is destructed).
    class Appender;
    SkDEBUGCODE(const Appender* fAppender = nullptr;)
};

// Appender implementations for DrawWriter that set the template on creation and provide a
// template-specific API to accumulate vertex/instance data.
class DrawWriter::Appender {
public:
    Appender(DrawWriter& w, SkEnumBitMask<RenderStateFlags> renderState)
            : fDrawer(w)
            , fReservedCount(0)
            , fNextWriter() {
        SkASSERT(w.fAppendStride > 0);
        SkASSERT(!w.fAppender);
        SkASSERT(w.fRenderState == renderState);
        SkDEBUGCODE(w.fAppender = this;)
    }

    virtual ~Appender() {
        if (fReservedCount > 0) {
            fDrawer.fManager->returnVertexBytes(fReservedCount * fDrawer.fAppendStride);
        }
        SkASSERT(fDrawer.fAppender == this);
        SkDEBUGCODE(fDrawer.fAppender = nullptr;)
    }

protected:
    DrawWriter&     fDrawer;
    uint32_t        fReservedCount; // in target stride units
    VertexWriter    fNextWriter;    // writing to the target buffer binding

    virtual void prepareFlush() {}

    // Reserves 'count' elements, managing potential re-allocation and buffer contiguity.
    // For vertex appends (AppendVerts), addresses ARM hardware issue (b/399631317) by:
    //  1. Requesting 4-count aligned space (for necessary padding).
    //  2. Ensuring the initial buffer offset is 4-count stride aligned after a newPipelineState().
    //  3. Checking whether a new reservation will overflow to a new buffer, and if it will, safely
    //     padding the current buffer.
    // If current reservation is insufficient, any existing reserved (but unused) contiguous
    // bytes are returned to the manager before attempting a new allocation.
    // If the newly allocated chunk isn't contiguous with the current target buffer,
    // pending draws are flushed, and the target is updated to this new chunk.
    template<bool AppendVerts>
    void reserve(unsigned int count) {
        SkASSERT(AppendVerts == SkToBool(fDrawer.fRenderState & RenderStateFlags::kAppendVertices));

        uint32_t alignedCount = count;
        uint32_t alignedStride = fDrawer.fAppendStride;
        if constexpr (AppendVerts) {
            alignedCount = SkAlign4(fDrawer.fPendingCount + count) - fDrawer.fPendingCount;
            if (fDrawer.fShouldAlign4) {
                alignedStride *= 4;
                fDrawer.fShouldAlign4 = false;
            }
        }

        if (fReservedCount >= alignedCount) {
            return;
        } else if (fReservedCount > 0) {
            SkASSERT(SkTFitsIn<uint32_t>((uint64_t)fReservedCount*(uint64_t)fDrawer.fAppendStride));
            const uint32_t returnedBytes = fReservedCount * fDrawer.fAppendStride;
            SkASSERT(fDrawer.fAppend.fSize >= returnedBytes);
            fDrawer.fManager->returnVertexBytes(returnedBytes);
            fDrawer.fAppend.fSize -= returnedBytes;
            fReservedCount = 0;
        }

        // If we are appending verts, we need to check if our requested allocation will overflow the
        // current buffer
        if constexpr (AppendVerts) {
            // If it does, we need to get a vertex writer to pad and zero out the old buffer
            // *before* we get our new one and lose bufferWriter's mapping to the transfer buffer.
            if (fDrawer.fManager->willVertexOverflow(
                        alignedCount, fDrawer.fAppendStride, alignedStride)) {
                fDrawer.padAndZero();
                // When we overflow the buffer, the previous alignedCount will be incorrect, since
                // it accounted for the about-to-be-flushed fPendingCount, which will not apply to
                // writing to the new buffer.
                alignedCount = SkAlign4(count);
            }
        }

        // NOTE: Cannot bind tuple directly to fNextWriter, compilers don't produce the right
        // move assignment.
        auto [writer, reservedChunk] =
            fDrawer.fManager->getVertexWriter(alignedCount, fDrawer.fAppendStride, alignedStride);
        if (writer) {
            fReservedCount = alignedCount;
            if (reservedChunk.fBuffer != fDrawer.fAppend.fBuffer ||
                reservedChunk.fOffset !=
                        fDrawer.fAppend.fOffset + fDrawer.fPendingCount * fDrawer.fAppendStride) {
                // Not contiguous, so flush and update binding to 'reservedChunk'
                this->prepareFlush();
                fDrawer.flushInternal();
                fDrawer.fAppend = reservedChunk;
            } else {
                fDrawer.fAppend.fSize += reservedChunk.fSize;
            }
        }

        fNextWriter = std::move(writer);
    }

    template<bool AppendVerts>
    VertexWriter append(unsigned int count) {
        SkASSERT(count > 0);
        this->reserve<AppendVerts>(count);

        if (!fNextWriter) SK_UNLIKELY {
            // If the GPU mapped buffer failed, ensure we have a sufficiently large CPU address to
            // write to so that RenderSteps don't have to worry about error handling. The Recording
            // will fail since the map failure is tracked by BufferManager.
            // Since one of the reasons for GPU mapping failure is that count*stride does not fit
            // in 32-bits, we calculate the CPU-side size carefully.
            uint64_t size = (uint64_t)count * (uint64_t)fDrawer.fAppendStride;
            if (!SkTFitsIn<size_t>(size)) {
                sk_report_container_overflow_and_die();
            }
            return VertexWriter(fDrawer.fFailureStorage.reset(size, SkAutoMalloc::kReuse_OnShrink),
                                SkTo<size_t>(size));
        }

        SkASSERT(fReservedCount >= count);
        fReservedCount -= count;
        fDrawer.fPendingCount += count;
        // Since we have a writer, we know count*stride is valid.
        return std::exchange(fNextWriter, fNextWriter.makeOffset(count * fDrawer.fAppendStride));
    }
};

class DrawWriter::Vertices : private DrawWriter::Appender {
public:
    Vertices(DrawWriter& w) :
    Appender(w, RenderStateFlags::kAppendVertices) {
        w.setTemplate({}, {}, w.fAppend, 0);
    }

    VertexWriter append(unsigned int count)  { return this->Appender::append<true>(count);  }
    void         reserve(unsigned int count) { return this->Appender::reserve<true>(count); }
};

class DrawWriter::Instances : private DrawWriter::Appender {
public:
    Instances(DrawWriter& w,
              BindBufferInfo vertices,
              BindBufferInfo indices,
              unsigned int vertexCount)
            : Appender(w, RenderStateFlags::kAppendInstances) {
        SkASSERT(vertexCount > 0);
        w.setTemplate(vertices, indices, w.fAppend, vertexCount);
    }

    VertexWriter append(unsigned int count)  { return this->Appender::append<false>(count);  }
    void         reserve(unsigned int count) { return this->Appender::reserve<false>(count); }
};

template <typename VertexCountProxy>
class DrawWriter::DynamicInstances : private DrawWriter::Appender {
public:
    DynamicInstances(DrawWriter& w,
                     BindBufferInfo vertices,
                     BindBufferInfo indices)
            : Appender(w, RenderStateFlags::kAppendDynamicInstances) {
        w.setTemplate(vertices, indices, w.fAppend, 0);
    }

    ~DynamicInstances() override {
        // See updateTemplateCount() "Destructor Case"
        this->updateTemplateCount();
    }

    void reserve(unsigned int count) { return this->Appender::reserve<false>(count); }

    template <typename V>
    VertexWriter append(const V& vertexCount, unsigned int instanceCount) {
        VertexWriter w = this->Appender::append<false>(instanceCount);
        // Record index count after appending instance data in case the append triggered a flush
        // and the max index count is reset. However, the contents of 'w' will not have been flushed
        // so 'fProxy' will account for 'vertexCount' when it is actually drawn.
        fProxy << vertexCount;
        return w;
    }

private:
    // updateTemplateCount() is called in two places:
    // 1. When reserve() acquires a new buffer:
    //    This ensures data from the *previous* buffer is included in the ensuing flush.
    //    The count needs updating to signal that the prior buffer holds complete data.
    //
    // 2. In the DrawWriter::DynamicInstances destructor:
    //    This occurs after all data appending for the dynamic instances is finished. The update
    //    makes the final index count for these instances visible for the flush or combines the
    //    count with the next draw call's DynamicInstances object if there was no pipeline change
    //    between calls to RenderStep::writeVertices
    //    - max() is used to allow batches of multiple dynamic instance appends.
    //    - Since index data gets aligned to the largest count in a batch, we use max()
    //      to ensure the recorded count matches this alignment.
    void updateTemplateCount() {
        fDrawer.fTemplateCount = std::max(fDrawer.fTemplateCount, static_cast<uint32_t>(fProxy));
        // By resetting the proxy after updating the template count, the next batch will start over
        // with the minimum required vertex count and grow from there.
        fProxy = {};
    }

    // See updateTemplateCount() "Reserve Case"
    void prepareFlush() override {
        this->updateTemplateCount();
    }

    VertexCountProxy fProxy = {};
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawWriter_DEFINED

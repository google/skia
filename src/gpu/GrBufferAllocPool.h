/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBufferAllocPool_DEFINED
#define GrBufferAllocPool_DEFINED

#include "GrCpuBuffer.h"
#include "GrNonAtomicRef.h"
#include "GrTypesPriv.h"
#include "SkNoncopyable.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkTypes.h"

class GrGpu;

/**
 * A pool of geometry buffers tied to a GrGpu.
 *
 * The pool allows a client to make space for geometry and then put back excess
 * space if it over allocated. When a client is ready to draw from the pool
 * it calls unmap on the pool ensure buffers are ready for drawing. The pool
 * can be reset after drawing is completed to recycle space.
 *
 * At creation time a minimum per-buffer size can be specified. Additionally,
 * a number of buffers to preallocate can be specified. These will
 * be allocated at the min size and kept around until the pool is destroyed.
 */
class GrBufferAllocPool : SkNoncopyable {
public:
    static constexpr size_t kDefaultBufferSize = 1 << 15;

    /**
     * A cache object that can be shared by multiple GrBufferAllocPool instances. It caches
     * cpu buffer allocations to avoid reallocating them.
     */
    class CpuBufferCache : public GrNonAtomicRef<CpuBufferCache> {
    public:
        static sk_sp<CpuBufferCache> Make(int maxBuffersToCache);

        sk_sp<GrCpuBuffer> makeBuffer(size_t size, bool mustBeInitialized);
        void releaseAll();

    private:
        CpuBufferCache(int maxBuffersToCache);

        struct Buffer {
            sk_sp<GrCpuBuffer> fBuffer;
            bool fCleared = false;
        };
        std::unique_ptr<Buffer[]> fBuffers;
        int fMaxBuffersToCache = 0;
    };

    /**
     * Ensures all buffers are unmapped and have all data written to them.
     * Call before drawing using buffers from the pool.
     */
    void unmap();

    /**
     *  Invalidates all the data in the pool, unrefs non-preallocated buffers.
     */
    void reset();

    /**
     * Frees data from makeSpaces in LIFO order.
     */
    void putBack(size_t bytes);

protected:
    /**
     * Constructor
     *
     * @param gpu                   The GrGpu used to create the buffers.
     * @param bufferType            The type of buffers to create.
     * @param cpuBufferCache        If non-null a cache for client side array buffers
     *                              or staging buffers used before data is uploaded to
     *                              GPU buffer objects.
     */
    GrBufferAllocPool(GrGpu* gpu, GrGpuBufferType bufferType, sk_sp<CpuBufferCache> cpuBufferCache);

    virtual ~GrBufferAllocPool();

    /**
     * Returns a block of memory to hold data. A buffer designated to hold the
     * data is given to the caller. The buffer may or may not be locked. The
     * returned ptr remains valid until any of the following:
     *      *makeSpace is called again.
     *      *unmap is called.
     *      *reset is called.
     *      *this object is destroyed.
     *
     * Once unmap on the pool is called the data is guaranteed to be in the
     * buffer at the offset indicated by offset. Until that time it may be
     * in temporary storage and/or the buffer may be locked.
     *
     * @param size         the amount of data to make space for
     * @param alignment    alignment constraint from start of buffer
     * @param buffer       returns the buffer that will hold the data.
     * @param offset       returns the offset into buffer of the data.
     * @return pointer to where the client should write the data.
     */
    void* makeSpace(size_t size, size_t alignment, sk_sp<const GrBuffer>* buffer, size_t* offset);

    /**
     * Returns a block of memory to hold data. A buffer designated to hold the
     * data is given to the caller. The buffer may or may not be locked. The
     * returned ptr remains valid until any of the following:
     *      *makeSpace is called again.
     *      *unmap is called.
     *      *reset is called.
     *      *this object is destroyed.
     *
     * Once unmap on the pool is called the data is guaranteed to be in the
     * buffer at the offset indicated by offset. Until that time it may be
     * in temporary storage and/or the buffer may be locked.
     *
     * The caller requests a minimum number of bytes, but the block may be (much)
     * larger. Assuming that a new block must be allocated, it will be fallbackSize bytes.
     * The actual block size is returned in actualSize.
     *
     * @param minSize        the minimum amount of data to make space for
     * @param fallbackSize   the amount of data to make space for if a new block is needed
     * @param alignment      alignment constraint from start of buffer
     * @param buffer         returns the buffer that will hold the data.
     * @param offset         returns the offset into buffer of the data.
     * @param actualSize     returns the capacity of the block
     * @return pointer to where the client should write the data.
     */
    void* makeSpaceAtLeast(size_t minSize,
                           size_t fallbackSize,
                           size_t alignment,
                           sk_sp<const GrBuffer>* buffer,
                           size_t* offset,
                           size_t* actualSize);

    sk_sp<GrBuffer> getBuffer(size_t size);

private:
    struct BufferBlock {
        size_t fBytesFree;
        sk_sp<GrBuffer> fBuffer;
    };

    bool createBlock(size_t requestSize);
    void destroyBlock();
    void deleteBlocks();
    void flushCpuData(const BufferBlock& block, size_t flushSize);
    void resetCpuData(size_t newSize);
#ifdef SK_DEBUG
    void validate(bool unusedBlockAllowed = false) const;
#endif
    size_t fBytesInUse = 0;

    SkTArray<BufferBlock> fBlocks;
    sk_sp<CpuBufferCache> fCpuBufferCache;
    sk_sp<GrCpuBuffer> fCpuStagingBuffer;
    GrGpu* fGpu;
    GrGpuBufferType fBufferType;
    void* fBufferPtr = nullptr;
};

/**
 * A GrBufferAllocPool of vertex buffers
 */
class GrVertexBufferAllocPool : public GrBufferAllocPool {
public:
    /**
     * Constructor
     *
     * @param gpu                   The GrGpu used to create the vertex buffers.
     * @param cpuBufferCache        If non-null a cache for client side array buffers
     *                              or staging buffers used before data is uploaded to
     *                              GPU buffer objects.
     */
    GrVertexBufferAllocPool(GrGpu* gpu, sk_sp<CpuBufferCache> cpuBufferCache);

    /**
     * Returns a block of memory to hold vertices. A buffer designated to hold
     * the vertices given to the caller. The buffer may or may not be locked.
     * The returned ptr remains valid until any of the following:
     *      *makeSpace is called again.
     *      *unmap is called.
     *      *reset is called.
     *      *this object is destroyed.
     *
     * Once unmap on the pool is called the vertices are guaranteed to be in
     * the buffer at the offset indicated by startVertex. Until that time they
     * may be in temporary storage and/or the buffer may be locked.
     *
     * @param vertexSize   specifies size of a vertex to allocate space for
     * @param vertexCount  number of vertices to allocate space for
     * @param buffer       returns the vertex buffer that will hold the
     *                     vertices.
     * @param startVertex  returns the offset into buffer of the first vertex.
     *                     In units of the size of a vertex from layout param.
     * @return pointer to first vertex.
     */
    void* makeSpace(size_t vertexSize,
                    int vertexCount,
                    sk_sp<const GrBuffer>* buffer,
                    int* startVertex);

    /**
     * Returns a block of memory to hold vertices. A buffer designated to hold
     * the vertices given to the caller. The buffer may or may not be locked.
     * The returned ptr remains valid until any of the following:
     *      *makeSpace is called again.
     *      *unmap is called.
     *      *reset is called.
     *      *this object is destroyed.
     *
     * Once unmap on the pool is called the vertices are guaranteed to be in
     * the buffer at the offset indicated by startVertex. Until that time they
     * may be in temporary storage and/or the buffer may be locked.
     *
     * The caller requests a minimum number of vertices, but the block may be (much)
     * larger. Assuming that a new block must be allocated, it will be sized to hold
     * fallbackVertexCount vertices. The actual block size (in vertices) is returned in
     * actualVertexCount.
     *
     * @param vertexSize           specifies size of a vertex to allocate space for
     * @param minVertexCount       minimum number of vertices to allocate space for
     * @param fallbackVertexCount  number of vertices to allocate space for if a new block is needed
     * @param buffer               returns the vertex buffer that will hold the vertices.
     * @param startVertex          returns the offset into buffer of the first vertex.
     *                             In units of the size of a vertex from layout param.
     * @param actualVertexCount    returns the capacity of the block (in vertices)
     * @return pointer to first vertex.
     */
    void* makeSpaceAtLeast(size_t vertexSize,
                           int minVertexCount,
                           int fallbackVertexCount,
                           sk_sp<const GrBuffer>* buffer,
                           int* startVertex,
                           int* actualVertexCount);

private:
    typedef GrBufferAllocPool INHERITED;
};

/**
 * A GrBufferAllocPool of index buffers
 */
class GrIndexBufferAllocPool : public GrBufferAllocPool {
public:
    /**
     * Constructor
     *
     * @param gpu                   The GrGpu used to create the index buffers.
     * @param cpuBufferCache        If non-null a cache for client side array buffers
     *                              or staging buffers used before data is uploaded to
     *                              GPU buffer objects.
     */
    GrIndexBufferAllocPool(GrGpu* gpu, sk_sp<CpuBufferCache> cpuBufferCache);

    /**
     * Returns a block of memory to hold indices. A buffer designated to hold
     * the indices is given to the caller. The buffer may or may not be locked.
     * The returned ptr remains valid until any of the following:
     *      *makeSpace is called again.
     *      *unmap is called.
     *      *reset is called.
     *      *this object is destroyed.
     *
     * Once unmap on the pool is called the indices are guaranteed to be in the
     * buffer at the offset indicated by startIndex. Until that time they may be
     * in temporary storage and/or the buffer may be locked.
     *
     * @param indexCount   number of indices to allocate space for
     * @param buffer       returns the index buffer that will hold the indices.
     * @param startIndex   returns the offset into buffer of the first index.
     * @return pointer to first index.
     */
    void* makeSpace(int indexCount, sk_sp<const GrBuffer>* buffer, int* startIndex);

    /**
     * Returns a block of memory to hold indices. A buffer designated to hold
     * the indices is given to the caller. The buffer may or may not be locked.
     * The returned ptr remains valid until any of the following:
     *      *makeSpace is called again.
     *      *unmap is called.
     *      *reset is called.
     *      *this object is destroyed.
     *
     * Once unmap on the pool is called the indices are guaranteed to be in the
     * buffer at the offset indicated by startIndex. Until that time they may be
     * in temporary storage and/or the buffer may be locked.
     *
     * The caller requests a minimum number of indices, but the block may be (much)
     * larger. Assuming that a new block must be allocated, it will be sized to hold
     * fallbackIndexCount indices. The actual block size (in indices) is returned in
     * actualIndexCount.
     *
     * @param minIndexCount        minimum number of indices to allocate space for
     * @param fallbackIndexCount   number of indices to allocate space for if a new block is needed
     * @param buffer               returns the index buffer that will hold the indices.
     * @param startIndex           returns the offset into buffer of the first index.
     * @param actualIndexCount     returns the capacity of the block (in indices)
     * @return pointer to first index.
     */
    void* makeSpaceAtLeast(int minIndexCount,
                           int fallbackIndexCount,
                           sk_sp<const GrBuffer>* buffer,
                           int* startIndex,
                           int* actualIndexCount);

private:
    typedef GrBufferAllocPool INHERITED;
};

#endif

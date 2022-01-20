/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMeshDrawTarget_DEFINED
#define GrMeshDrawTarget_DEFINED

#include "src/gpu/GrDrawIndirectCommand.h"
#include "src/gpu/GrSimpleMesh.h"

class GrAtlasManager;
class GrStrikeCache;
class GrThreadSafeCache;

namespace skgpu {
    namespace v1 { class SmallPathAtlasMgr; }

    struct IndexWriter;
    struct VertexWriter;
} // namespace skgpu

/*
 * Abstract interface that supports creating vertices, indices, and meshes, as well as
 * invoking GPU draw operations.
 */
class GrMeshDrawTarget {
public:
    virtual ~GrMeshDrawTarget() {}

    /** Adds a draw of a mesh. 'primProcProxies' must have
     * GrGeometryProcessor::numTextureSamplers() entries. Can be null if no samplers.
     */
    virtual void recordDraw(const GrGeometryProcessor*,
                            const GrSimpleMesh[],
                            int meshCnt,
                            const GrSurfaceProxy* const primProcProxies[],
                            GrPrimitiveType) = 0;

    /**
     * Helper for drawing GrSimpleMesh(es) with zero primProc textures.
     */
    void recordDraw(const GrGeometryProcessor* gp,
                    const GrSimpleMesh meshes[],
                    int meshCnt,
                    GrPrimitiveType primitiveType) {
        this->recordDraw(gp, meshes, meshCnt, nullptr, primitiveType);
    }

    /**
     * Makes space for vertex data. The returned pointer is the location where vertex data
     * should be written. On return the buffer that will hold the data as well as an offset into
     * the buffer (in 'vertexSize' units) where the data will be placed.
     */
    virtual void* makeVertexSpace(size_t vertexSize, int vertexCount, sk_sp<const GrBuffer>*,
                                  int* startVertex) = 0;

    /**
     * Makes space for index data. The returned pointer is the location where index data
     * should be written. On return the buffer that will hold the data as well as an offset into
     * the buffer (in uint16_t units) where the data will be placed.
     */
    virtual uint16_t* makeIndexSpace(int indexCount, sk_sp<const GrBuffer>*, int* startIndex) = 0;

    /**
     * This is similar to makeVertexSpace. It allows the caller to use up to 'actualVertexCount'
     * vertices in the returned pointer, which may exceed 'minVertexCount'.
     * 'fallbackVertexCount' is the maximum number of vertices that should be allocated if a new
     * buffer is allocated on behalf of this request.
     */
    virtual void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount,
                                         int fallbackVertexCount, sk_sp<const GrBuffer>*,
                                         int* startVertex, int* actualVertexCount) = 0;

    /**
     * This is similar to makeIndexSpace. It allows the caller to use up to 'actualIndexCount'
     * indices in the returned pointer, which may exceed 'minIndexCount'.
     * 'fallbackIndexCount' is the maximum number of indices that should be allocated if a new
     * buffer is allocated on behalf of this request.
     */
    virtual uint16_t* makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount,
                                            sk_sp<const GrBuffer>*, int* startIndex,
                                            int* actualIndexCount) = 0;

    /**
     * Makes space for elements in a draw-indirect buffer. Upon success, the returned pointer is a
     * CPU mapping where the data should be written.
     */
    virtual GrDrawIndirectWriter makeDrawIndirectSpace(int drawCount, sk_sp<const GrBuffer>* buffer,
                                                       size_t* offsetInBytes) = 0;

    /**
     * Makes space for elements in a draw-indexed-indirect buffer. Upon success, the returned
     * pointer is a CPU mapping where the data should be written.
     */
    virtual GrDrawIndexedIndirectWriter makeDrawIndexedIndirectSpace(int drawCount,
                                                                     sk_sp<const GrBuffer>*,
                                                                     size_t* offsetInBytes) = 0;

    /** Helpers for ops that only need to use the VertexWriter to fill the data directly. */
    skgpu::VertexWriter makeVertexWriter(size_t vertexSize, int vertexCount,
                                         sk_sp<const GrBuffer>*, int* startVertex);
    skgpu::IndexWriter makeIndexWriter(int indexCount, sk_sp<const GrBuffer>*, int* startIndex);
    skgpu::VertexWriter makeVertexWriterAtLeast(size_t vertexSize, int minVertexCount,
                                                int fallbackVertexCount, sk_sp<const GrBuffer>*,
                                                int* startVertex, int* actualVertexCount);
    skgpu::IndexWriter makeIndexWriterAtLeast(int minIndexCount, int fallbackIndexCount,
                                              sk_sp<const GrBuffer>*, int* startIndex,
                                              int* actualIndexCount);

    /** Helpers for ops which over-allocate and then return excess data to the pool. */
    virtual void putBackIndices(int indices) = 0;
    virtual void putBackVertices(int vertices, size_t vertexStride) = 0;
    virtual void putBackIndirectDraws(int count) = 0;
    virtual void putBackIndexedIndirectDraws(int count) = 0;

    GrSimpleMesh* allocMesh() { return this->allocator()->make<GrSimpleMesh>(); }
    GrSimpleMesh* allocMeshes(int n) { return this->allocator()->makeArray<GrSimpleMesh>(n); }
    const GrSurfaceProxy** allocPrimProcProxyPtrs(int n) {
        return this->allocator()->makeArray<const GrSurfaceProxy*>(n);
    }

    virtual GrRenderTargetProxy* rtProxy() const = 0;
    virtual const GrSurfaceProxyView& writeView() const = 0;

    virtual const GrAppliedClip* appliedClip() const = 0;
    virtual GrAppliedClip detachAppliedClip() = 0;

    virtual const GrDstProxyView& dstProxyView() const = 0;
    virtual bool usesMSAASurface() const = 0;

    virtual GrXferBarrierFlags renderPassBarriers() const = 0;

    virtual GrLoadOp colorLoadOp() const = 0;

    virtual GrThreadSafeCache* threadSafeCache() const = 0;
    virtual GrResourceProvider* resourceProvider() const = 0;
    uint32_t contextUniqueID() const;

    virtual GrStrikeCache* strikeCache() const = 0;
    virtual GrAtlasManager* atlasManager() const = 0;
    virtual skgpu::v1::SmallPathAtlasMgr* smallPathAtlasManager() const = 0;

    // This should be called during onPrepare of a GrOp. The caller should add any proxies to the
    // array it will use that it did not access during a call to visitProxies. This is usually the
    // case for atlases.
    virtual SkTArray<GrSurfaceProxy*, true>* sampledProxyArray() = 0;

    virtual const GrCaps& caps() const = 0;

    virtual GrDeferredUploadTarget* deferredUploadTarget() = 0;

    virtual SkArenaAlloc* allocator() = 0;
};

#endif

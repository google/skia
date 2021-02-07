/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMeshDrawOp_DEFINED
#define GrMeshDrawOp_DEFINED

#include "src/core/SkArenaAlloc.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrDrawIndirectCommand.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrSimpleMesh.h"
#include "src/gpu/ops/GrDrawOp.h"
#include <type_traits>

class GrAtlasManager;
class GrCaps;
class GrStrikeCache;
class GrOpFlushState;
class GrSmallPathAtlasMgr;

/**
 * Base class for mesh-drawing GrDrawOps.
 */
class GrMeshDrawOp : public GrDrawOp {
public:
    /** Abstract interface that represents a destination for a GrMeshDrawOp. */
    class Target;

    static bool CanUpgradeAAOnMerge(GrAAType aa1, GrAAType aa2) {
        return (aa1 == GrAAType::kNone && aa2 == GrAAType::kCoverage) ||
               (aa1 == GrAAType::kCoverage && aa2 == GrAAType::kNone);
    }

protected:
    GrMeshDrawOp(uint32_t classID);

    void createProgramInfo(const GrCaps* caps,
                           SkArenaAlloc* arena,
                           const GrSurfaceProxyView& writeView,
                           GrAppliedClip&& appliedClip,
                           const GrXferProcessor::DstProxyView& dstProxyView,
                           GrXferBarrierFlags renderPassXferBarriers,
                           GrLoadOp colorLoadOp) {
        this->onCreateProgramInfo(caps, arena, writeView, std::move(appliedClip), dstProxyView,
                                  renderPassXferBarriers, colorLoadOp);
    }

    void createProgramInfo(Target* target);

    /** Helper for rendering repeating meshes using a patterned index buffer. This class creates the
        space for the vertices and flushes the draws to the GrMeshDrawOp::Target. */
    class PatternHelper {
    public:
        PatternHelper(Target*, GrPrimitiveType, size_t vertexStride,
                      sk_sp<const GrBuffer> indexBuffer, int verticesPerRepetition,
                      int indicesPerRepetition, int repeatCount, int maxRepetitions);

        /** Called to issue draws to the GrMeshDrawOp::Target.*/
        void recordDraw(Target*, const GrGeometryProcessor*) const;
        void recordDraw(Target*, const GrGeometryProcessor*,
                        const GrSurfaceProxy* const primProcProxies[]) const;

        void* vertices() const { return fVertices; }
        GrSimpleMesh* mesh() { return fMesh; }

    protected:
        PatternHelper() = default;
        void init(Target*, GrPrimitiveType, size_t vertexStride, sk_sp<const GrBuffer> indexBuffer,
                  int verticesPerRepetition, int indicesPerRepetition, int repeatCount,
                  int maxRepetitions);

    private:
        void* fVertices = nullptr;
        GrSimpleMesh* fMesh = nullptr;
        GrPrimitiveType fPrimitiveType;
    };

    /** A specialization of InstanceHelper for quad rendering.
     *  It only draws non-antialiased indexed quads.
     */
    class QuadHelper : private PatternHelper {
    public:
        QuadHelper() = delete;
        QuadHelper(Target* target, size_t vertexStride, int quadsToDraw);

        using PatternHelper::mesh;
        using PatternHelper::recordDraw;
        using PatternHelper::vertices;

    private:
        using INHERITED = PatternHelper;
    };

    static bool CombinedQuadCountWillOverflow(GrAAType aaType,
                                              bool willBeUpgradedToAA,
                                              int combinedQuadCount) {
        bool willBeAA = (aaType == GrAAType::kCoverage) || willBeUpgradedToAA;

        return combinedQuadCount > (willBeAA ? GrResourceProvider::MaxNumAAQuads()
                                             : GrResourceProvider::MaxNumNonAAQuads());
    }

    virtual void onPrePrepareDraws(GrRecordingContext*,
                                   const GrSurfaceProxyView& writeView,
                                   GrAppliedClip*,
                                   const GrXferProcessor::DstProxyView&,
                                   GrXferBarrierFlags renderPassXferBarriers,
                                   GrLoadOp colorLoadOp);

private:
    virtual GrProgramInfo* programInfo() = 0;
    // This method is responsible for creating all the programInfos required
    // by this op.
    virtual void onCreateProgramInfo(const GrCaps*,
                                     SkArenaAlloc*,
                                     const GrSurfaceProxyView& writeView,
                                     GrAppliedClip&&,
                                     const GrXferProcessor::DstProxyView&,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) = 0;

    void onPrePrepare(GrRecordingContext* context,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip* clip,
                      const GrXferProcessor::DstProxyView& dstProxyView,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) final {
        this->onPrePrepareDraws(context, writeView, clip, dstProxyView, renderPassXferBarriers,
                                colorLoadOp);
    }
    void onPrepare(GrOpFlushState* state) final;

    virtual void onPrepareDraws(Target*) = 0;
    using INHERITED = GrDrawOp;
};

class GrMeshDrawOp::Target {
public:
    virtual ~Target() {}

    /** Adds a draw of a mesh. 'primProcProxies' must have
     * GrPrimitiveProcessor::numTextureSamplers() entries. Can be null if no samplers.
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

    virtual const GrXferProcessor::DstProxyView& dstProxyView() const = 0;

    virtual GrXferBarrierFlags renderPassBarriers() const = 0;

    virtual GrLoadOp colorLoadOp() const = 0;

    virtual GrThreadSafeCache* threadSafeCache() const = 0;
    virtual GrResourceProvider* resourceProvider() const = 0;
    uint32_t contextUniqueID() const { return this->resourceProvider()->contextUniqueID(); }

    virtual GrStrikeCache* strikeCache() const = 0;
    virtual GrAtlasManager* atlasManager() const = 0;
    virtual GrSmallPathAtlasMgr* smallPathAtlasManager() const = 0;

    // This should be called during onPrepare of a GrOp. The caller should add any proxies to the
    // array it will use that it did not access during a call to visitProxies. This is usually the
    // case for atlases.
    virtual SkTArray<GrSurfaceProxy*, true>* sampledProxyArray() = 0;

    virtual const GrCaps& caps() const = 0;

    virtual GrDeferredUploadTarget* deferredUploadTarget() = 0;

    virtual SkArenaAlloc* allocator() = 0;
};

#endif

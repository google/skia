/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpFlushState_DEFINED
#define GrOpFlushState_DEFINED

#include <utility>
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkArenaAllocList.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrBufferAllocPool.h"
#include "src/gpu/GrDeferredUpload.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/ops/GrMeshDrawOp.h"

class GrGpu;
class GrOpsRenderPass;
class GrResourceProvider;
struct GrSimpleMesh;

class Target2 {
public:
    Target2() {}

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

/** Tracks the state across all the GrOps (really just the GrDrawOps) in a GrOpsTask flush. */
class GrOpFlushState final : public GrDeferredUploadTarget, public Target2 {
public:
    // vertexSpace and indexSpace may either be null or an alloation of size
    // GrBufferAllocPool::kDefaultBufferSize. If the latter, then CPU memory is only allocated for
    // vertices/indices when a buffer larger than kDefaultBufferSize is required.
    GrOpFlushState(GrGpu*, GrResourceProvider*, GrTokenTracker*,
                   sk_sp<GrBufferAllocPool::CpuBufferCache> = nullptr);

    ~GrOpFlushState() final { this->reset(); }

    /** This is called after each op has a chance to prepare its draws and before the draws are
        executed. */
    void preExecuteDraws();

    /** Called to upload data to a texture using the GrDeferredTextureUploadFn. If the uploaded
        surface needs to be prepared for being sampled in a draw after the upload, the caller
        should pass in true for shouldPrepareSurfaceForSampling. This feature is needed for Vulkan
        when doing inline uploads to reset the image layout back to sampled. */
    void doUpload(GrDeferredTextureUploadFn&, bool shouldPrepareSurfaceForSampling = false);

    /** Called as ops are executed. Must be called in the same order as the ops were prepared. */
    void executeDrawsAndUploadsForMeshDrawOp(const GrOp* op, const SkRect& chainBounds,
                                             const GrPipeline*, const GrUserStencilSettings*);

    GrOpsRenderPass* opsRenderPass() { return fOpsRenderPass; }
    void setOpsRenderPass(GrOpsRenderPass* renderPass) { fOpsRenderPass = renderPass; }

    GrGpu* gpu() { return fGpu; }

    void reset();

    /** Additional data required on a per-op basis when executing GrOps. */
    struct OpArgs {
        // TODO: why does OpArgs have the op we're going to pass it to as a member? Remove it.
        explicit OpArgs(GrOp* op, const GrSurfaceProxyView& surfaceView, bool usesMSAASurface,
                        GrAppliedClip* appliedClip, const GrDstProxyView& dstProxyView,
                        GrXferBarrierFlags renderPassXferBarriers, GrLoadOp colorLoadOp)
                : fOp(op)
                , fSurfaceView(surfaceView)
                , fRenderTargetProxy(surfaceView.asRenderTargetProxy())
                , fUsesMSAASurface(usesMSAASurface)
                , fAppliedClip(appliedClip)
                , fDstProxyView(dstProxyView)
                , fRenderPassXferBarriers(renderPassXferBarriers)
                , fColorLoadOp(colorLoadOp) {
            SkASSERT(surfaceView.asRenderTargetProxy());
        }

        GrOp* op() { return fOp; }
        const GrSurfaceProxyView& writeView() const { return fSurfaceView; }
        GrRenderTargetProxy* rtProxy() const { return fRenderTargetProxy; }
        // True if the op under consideration belongs to an opsTask that renders to an MSAA buffer.
        bool usesMSAASurface() const { return fUsesMSAASurface; }
        GrAppliedClip* appliedClip() { return fAppliedClip; }
        const GrAppliedClip* appliedClip() const { return fAppliedClip; }
        const GrDstProxyView& dstProxyView() const { return fDstProxyView; }
        GrXferBarrierFlags renderPassBarriers() const { return fRenderPassXferBarriers; }
        GrLoadOp colorLoadOp() const { return fColorLoadOp; }

#ifdef SK_DEBUG
        void validate() const {
            SkASSERT(fOp);
            SkASSERT(fSurfaceView);
        }
#endif

    private:
        GrOp*                         fOp;
        const GrSurfaceProxyView&     fSurfaceView;
        GrRenderTargetProxy*          fRenderTargetProxy;
        bool                          fUsesMSAASurface;
        GrAppliedClip*                fAppliedClip;
        GrDstProxyView                fDstProxyView;   // TODO: do we still need the dst proxy here?
        GrXferBarrierFlags            fRenderPassXferBarriers;
        GrLoadOp                      fColorLoadOp;
    };

    void setOpArgs(OpArgs* opArgs) { fOpArgs = opArgs; }

    const OpArgs& drawOpArgs() const {
        SkASSERT(fOpArgs);
        SkDEBUGCODE(fOpArgs->validate());
        return *fOpArgs;
    }

    void setSampledProxyArray(SkTArray<GrSurfaceProxy*, true>* sampledProxies) {
        fSampledProxies = sampledProxies;
    }

    SkTArray<GrSurfaceProxy*, true>* sampledProxyArray() override {
        return fSampledProxies;
    }

    /** Overrides of GrDeferredUploadTarget. */

    const GrTokenTracker* tokenTracker() final { return fTokenTracker; }
    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) final;
    GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&&) final;

    /** Overrides of GrMeshDrawOp::Target. */
    void recordDraw(const GrGeometryProcessor*,
                    const GrSimpleMesh[],
                    int meshCnt,
                    const GrSurfaceProxy* const primProcProxies[],
                    GrPrimitiveType) final;
    void* makeVertexSpace(size_t vertexSize, int vertexCount, sk_sp<const GrBuffer>*,
                          int* startVertex) final;
    uint16_t* makeIndexSpace(int indexCount, sk_sp<const GrBuffer>*, int* startIndex) final;
    void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount, int fallbackVertexCount,
                                 sk_sp<const GrBuffer>*, int* startVertex,
                                 int* actualVertexCount) final;
    uint16_t* makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount,
                                    sk_sp<const GrBuffer>*, int* startIndex,
                                    int* actualIndexCount) final;
    GrDrawIndirectWriter makeDrawIndirectSpace(int drawCount, sk_sp<const GrBuffer>* buffer,
                                               size_t* offset) override {
        return fDrawIndirectPool.makeSpace(drawCount, buffer, offset);
    }
    GrDrawIndexedIndirectWriter makeDrawIndexedIndirectSpace(int drawCount,
                                                             sk_sp<const GrBuffer>* buffer,
                                                             size_t* offset) override {
        return fDrawIndirectPool.makeIndexedSpace(drawCount, buffer, offset);
    }
    void putBackIndices(int indexCount) final;
    void putBackVertices(int vertices, size_t vertexStride) final;
    void putBackIndirectDraws(int drawCount) final { fDrawIndirectPool.putBack(drawCount); }
    void putBackIndexedIndirectDraws(int drawCount) final {
        fDrawIndirectPool.putBackIndexed(drawCount);
    }
    const GrSurfaceProxyView& writeView() const final { return this->drawOpArgs().writeView(); }
    GrRenderTargetProxy* rtProxy() const final { return this->drawOpArgs().rtProxy(); }
    bool usesMSAASurface() const final { return this->drawOpArgs().usesMSAASurface(); }
    const GrAppliedClip* appliedClip() const final { return this->drawOpArgs().appliedClip(); }
    const GrAppliedHardClip& appliedHardClip() const {
        return (fOpArgs->appliedClip()) ?
                fOpArgs->appliedClip()->hardClip() : GrAppliedHardClip::Disabled();
    }
    GrAppliedClip detachAppliedClip() final;
    const GrDstProxyView& dstProxyView() const final {
        return this->drawOpArgs().dstProxyView();
    }

    GrXferBarrierFlags renderPassBarriers() const final {
        return this->drawOpArgs().renderPassBarriers();
    }

    GrLoadOp colorLoadOp() const final {
        return this->drawOpArgs().colorLoadOp();
    }

    GrDeferredUploadTarget* deferredUploadTarget() final { return this; }
    const GrCaps& caps() const final;
    GrThreadSafeCache* threadSafeCache() const final;
    GrResourceProvider* resourceProvider() const final { return fResourceProvider; }

    GrStrikeCache* strikeCache() const final;

    // At this point we know we're flushing so full access to the GrAtlasManager and
    // GrSmallPathAtlasMgr is required (and permissible).
    GrAtlasManager* atlasManager() const final;
    GrSmallPathAtlasMgr* smallPathAtlasManager() const final;

    /** GrMeshDrawOp::Target override. */
    SkArenaAlloc* allocator() override { return &fArena; }

    // This is a convenience method that binds the given pipeline, and then, if our applied clip has
    // a scissor, sets the scissor rect from the applied clip.
    void bindPipelineAndScissorClip(const GrProgramInfo& programInfo, const SkRect& drawBounds) {
        SkASSERT((programInfo.pipeline().isScissorTestEnabled()) ==
                 (this->appliedClip() && this->appliedClip()->scissorState().enabled()));
        this->bindPipeline(programInfo, drawBounds);
        if (programInfo.pipeline().isScissorTestEnabled()) {
            this->setScissorRect(this->appliedClip()->scissorState().rect());
        }
    }

    // This is a convenience method for when the primitive processor has exactly one texture. It
    // binds one texture for the primitive processor, and any others for FPs on the pipeline.
    void bindTextures(const GrGeometryProcessor& geomProc,
                      const GrSurfaceProxy& singleGeomProcTexture,
                      const GrPipeline& pipeline) {
        SkASSERT(geomProc.numTextureSamplers() == 1);
        const GrSurfaceProxy* ptr = &singleGeomProcTexture;
        this->bindTextures(geomProc, &ptr, pipeline);
    }

    // Makes the appropriate bindBuffers() and draw*() calls for the provided mesh.
    void drawMesh(const GrSimpleMesh& mesh);

    // Pass-through methods to GrOpsRenderPass.
    void bindPipeline(const GrProgramInfo& programInfo, const SkRect& drawBounds) {
        fOpsRenderPass->bindPipeline(programInfo, drawBounds);
    }
    void setScissorRect(const SkIRect& scissorRect) {
        fOpsRenderPass->setScissorRect(scissorRect);
    }
    void bindTextures(const GrGeometryProcessor& geomProc,
                      const GrSurfaceProxy* const geomProcTextures[],
                      const GrPipeline& pipeline) {
        fOpsRenderPass->bindTextures(geomProc, geomProcTextures, pipeline);
    }
    void bindBuffers(sk_sp<const GrBuffer> indexBuffer, sk_sp<const GrBuffer> instanceBuffer,
                     sk_sp<const GrBuffer> vertexBuffer,
                     GrPrimitiveRestart primitiveRestart = GrPrimitiveRestart::kNo) {
        fOpsRenderPass->bindBuffers(std::move(indexBuffer), std::move(instanceBuffer),
                                    std::move(vertexBuffer), primitiveRestart);
    }
    void draw(int vertexCount, int baseVertex) {
        fOpsRenderPass->draw(vertexCount, baseVertex);
    }
    void drawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue, uint16_t maxIndexValue,
                     int baseVertex) {
        fOpsRenderPass->drawIndexed(indexCount, baseIndex, minIndexValue, maxIndexValue,
                                    baseVertex);
    }
    void drawInstanced(int instanceCount, int baseInstance, int vertexCount, int baseVertex) {
        fOpsRenderPass->drawInstanced(instanceCount, baseInstance, vertexCount, baseVertex);
    }
    void drawIndexedInstanced(int indexCount, int baseIndex, int instanceCount, int baseInstance,
                              int baseVertex) {
        fOpsRenderPass->drawIndexedInstanced(indexCount, baseIndex, instanceCount, baseInstance,
                                             baseVertex);
    }
    void drawIndirect(const GrBuffer* drawIndirectBuffer, size_t offset, int drawCount) {
        fOpsRenderPass->drawIndirect(drawIndirectBuffer, offset, drawCount);
    }
    void drawIndexedIndirect(const GrBuffer* drawIndirectBuffer, size_t offset, int drawCount) {
        fOpsRenderPass->drawIndexedIndirect(drawIndirectBuffer, offset, drawCount);
    }
    void drawIndexPattern(int patternIndexCount, int patternRepeatCount,
                          int maxPatternRepetitionsInIndexBuffer, int patternVertexCount,
                          int baseVertex) {
        fOpsRenderPass->drawIndexPattern(patternIndexCount, patternRepeatCount,
                                         maxPatternRepetitionsInIndexBuffer, patternVertexCount,
                                         baseVertex);
    }

private:
    struct InlineUpload {
        InlineUpload(GrDeferredTextureUploadFn&& upload, GrDeferredUploadToken token)
                : fUpload(std::move(upload)), fUploadBeforeToken(token) {}
        GrDeferredTextureUploadFn fUpload;
        GrDeferredUploadToken fUploadBeforeToken;
    };

    // A set of contiguous draws that share a draw token, geometry processor, and pipeline. The
    // meshes for the draw are stored in the fMeshes array. The reason for coalescing meshes
    // that share a geometry processor into a Draw is that it allows the Gpu object to setup
    // the shared state once and then issue draws for each mesh.
    struct Draw {
        ~Draw();
        // The geometry processor is always forced to be in an arena allocation or appears on
        // the stack (for CCPR). In either case this object does not need to manage its
        // lifetime.
        const GrGeometryProcessor* fGeometryProcessor = nullptr;
        // Must have GrGeometryProcessor::numTextureSamplers() entries. Can be null if no samplers.
        const GrSurfaceProxy* const* fGeomProcProxies = nullptr;
        const GrSimpleMesh* fMeshes = nullptr;
        const GrOp* fOp = nullptr;
        int fMeshCnt = 0;
        GrPrimitiveType fPrimitiveType;
    };

    // Storage for ops' pipelines, draws, and inline uploads.
    SkArenaAllocWithReset fArena{sizeof(GrPipeline) * 100};

    // Store vertex and index data on behalf of ops that are flushed.
    GrVertexBufferAllocPool fVertexPool;
    GrIndexBufferAllocPool fIndexPool;
    GrDrawIndirectBufferAllocPool fDrawIndirectPool;

    // Data stored on behalf of the ops being flushed.
    SkArenaAllocList<GrDeferredTextureUploadFn> fASAPUploads;
    SkArenaAllocList<InlineUpload> fInlineUploads;
    SkArenaAllocList<Draw> fDraws;

    // All draws we store have an implicit draw token. This is the draw token for the first draw
    // in fDraws.
    GrDeferredUploadToken fBaseDrawToken = GrDeferredUploadToken::AlreadyFlushedToken();

    // Info about the op that is currently preparing or executing using the flush state or null if
    // an op is not currently preparing of executing.
    OpArgs* fOpArgs = nullptr;

    // This field is only transiently set during flush. Each GrOpsTask will set it to point to an
    // array of proxies it uses before call onPrepare and onExecute.
    SkTArray<GrSurfaceProxy*, true>* fSampledProxies;

    GrGpu* fGpu;
    GrResourceProvider* fResourceProvider;
    GrTokenTracker* fTokenTracker;
    GrOpsRenderPass* fOpsRenderPass = nullptr;

    // Variables that are used to track where we are in lists as ops are executed
    SkArenaAllocList<Draw>::Iter fCurrDraw;
    SkArenaAllocList<InlineUpload>::Iter fCurrUpload;
};

#endif

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
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/ops/GrMeshDrawOp.h"

class GrGpu;
class GrOpsRenderPass;
class GrResourceProvider;

/** Tracks the state across all the GrOps (really just the GrDrawOps) in a GrOpsTask flush. */
class GrOpFlushState final : public GrDeferredUploadTarget, public GrMeshDrawOp::Target {
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
    void executeDrawsAndUploadsForMeshDrawOp(
            const GrOp* op, const SkRect& chainBounds, GrProcessorSet&&,
            GrPipeline::InputFlags = GrPipeline::InputFlags::kNone,
            const GrUserStencilSettings* = &GrUserStencilSettings::kUnused);

    GrOpsRenderPass* opsRenderPass() { return fOpsRenderPass; }
    void setOpsRenderPass(GrOpsRenderPass* renderPass) { fOpsRenderPass = renderPass; }

    GrGpu* gpu() { return fGpu; }

    void reset();

    /** Additional data required on a per-op basis when executing GrOps. */
    struct OpArgs {
        explicit OpArgs(GrOp* op, GrRenderTargetProxy* proxy, GrAppliedClip* appliedClip,
                        const GrXferProcessor::DstProxy& dstProxy)
            : fOp(op)
            , fProxy(proxy)
            , fAppliedClip(appliedClip)
            , fDstProxy(dstProxy) {
        }

        int numSamples() const { return fProxy->numSamples(); }
        GrSurfaceOrigin origin() const { return fProxy->origin(); }
        GrSwizzle outputSwizzle() const { return fProxy->outputSwizzle(); }

        GrOp* op() { return fOp; }
        GrRenderTargetProxy* proxy() const { return fProxy; }
        GrRenderTarget* renderTarget() const { return fProxy->peekRenderTarget(); }
        GrAppliedClip* appliedClip() { return fAppliedClip; }
        const GrAppliedClip* appliedClip() const { return fAppliedClip; }
        const GrXferProcessor::DstProxy& dstProxy() const { return fDstProxy; }

#ifdef SK_DEBUG
        void validate() const {
            SkASSERT(fOp);
            SkASSERT(fProxy);
        }
#endif

    private:
        GrOp*                     fOp;
        GrRenderTargetProxy*      fProxy;
        GrAppliedClip*            fAppliedClip;
        GrXferProcessor::DstProxy fDstProxy;     // TODO: do we still need the dst proxy here?
    };

    void setOpArgs(OpArgs* opArgs) { fOpArgs = opArgs; }

    const OpArgs& drawOpArgs() const {
        SkASSERT(fOpArgs);
        SkDEBUGCODE(fOpArgs->validate());
        return *fOpArgs;
    }

    void setSampledProxyArray(SkTArray<GrTextureProxy*, true>* sampledProxies) {
        fSampledProxies = sampledProxies;
    }

    SkTArray<GrTextureProxy*, true>* sampledProxyArray() override {
        return fSampledProxies;
    }

    /** Overrides of GrDeferredUploadTarget. */

    const GrTokenTracker* tokenTracker() final { return fTokenTracker; }
    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) final;
    GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&&) final;

    /** Overrides of GrMeshDrawOp::Target. */
    void recordDraw(sk_sp<const GrGeometryProcessor>, const GrMesh[], int meshCnt,
                    const GrPipeline::FixedDynamicState*,
                    const GrPipeline::DynamicStateArrays*) final;
    void* makeVertexSpace(size_t vertexSize, int vertexCount, sk_sp<const GrBuffer>*,
                          int* startVertex) final;
    uint16_t* makeIndexSpace(int indexCount, sk_sp<const GrBuffer>*, int* startIndex) final;
    void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount, int fallbackVertexCount,
                                 sk_sp<const GrBuffer>*, int* startVertex,
                                 int* actualVertexCount) final;
    uint16_t* makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount,
                                    sk_sp<const GrBuffer>*, int* startIndex,
                                    int* actualIndexCount) final;
    void putBackIndices(int indexCount) final;
    void putBackVertices(int vertices, size_t vertexStride) final;
    GrRenderTargetProxy* proxy() const final { return fOpArgs->proxy(); }
    const GrAppliedClip* appliedClip() final { return fOpArgs->appliedClip(); }
    GrAppliedClip detachAppliedClip() final;
    const GrXferProcessor::DstProxy& dstProxy() const final { return fOpArgs->dstProxy(); }
    GrDeferredUploadTarget* deferredUploadTarget() final { return this; }
    const GrCaps& caps() const final;
    GrResourceProvider* resourceProvider() const final { return fResourceProvider; }

    GrStrikeCache* glyphCache() const final;

    // At this point we know we're flushing so full access to the GrAtlasManager is required (and
    // permissible).
    GrAtlasManager* atlasManager() const final;

    /** GrMeshDrawOp::Target override. */
    SkArenaAlloc* allocator() override { return &fArena; }

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
        sk_sp<const GrGeometryProcessor> fGeometryProcessor;
        const GrPipeline::FixedDynamicState* fFixedDynamicState;
        const GrPipeline::DynamicStateArrays* fDynamicStateArrays;
        const GrMesh* fMeshes = nullptr;
        const GrOp* fOp = nullptr;
        int fMeshCnt = 0;
    };

    // Storage for ops' pipelines, draws, and inline uploads.
    SkArenaAlloc fArena{sizeof(GrPipeline) * 100};

    // Store vertex and index data on behalf of ops that are flushed.
    GrVertexBufferAllocPool fVertexPool;
    GrIndexBufferAllocPool fIndexPool;

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
    SkTArray<GrTextureProxy*, true>* fSampledProxies;

    GrGpu* fGpu;
    GrResourceProvider* fResourceProvider;
    GrTokenTracker* fTokenTracker;
    GrOpsRenderPass* fOpsRenderPass = nullptr;

    // Variables that are used to track where we are in lists as ops are executed
    SkArenaAllocList<Draw>::Iter fCurrDraw;
    SkArenaAllocList<InlineUpload>::Iter fCurrUpload;
};

#endif

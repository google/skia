/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpFlushState_DEFINED
#define GrOpFlushState_DEFINED

#include <utility>
#include "GrAppliedClip.h"
#include "GrBufferAllocPool.h"
#include "GrDeferredUpload.h"
#include "SkArenaAlloc.h"
#include "ops/GrMeshDrawOp.h"

class GrGpu;
class GrGpuCommandBuffer;
class GrGpuRTCommandBuffer;
class GrResourceProvider;

/** Tracks the state across all the GrOps (really just the GrDrawOps) in a GrOpList flush. */
class GrOpFlushState final : public GrDeferredUploadTarget, public GrMeshDrawOp::Target {
public:
    GrOpFlushState(GrGpu*, GrResourceProvider*);

    ~GrOpFlushState() final { this->reset(); }

    /** This is called after each op has a chance to prepare its draws and before the draws are
        issued. */
    void preIssueDraws();

    void doUpload(GrDeferredTextureUploadFn&);

    void executeDrawsAndUploadsForOp(uint32_t opID, const SkRect& opBounds);

    GrGpuCommandBuffer* commandBuffer() { return fCommandBuffer; }
    // Helper function used by Ops that are only called via RenderTargetOpLists
    GrGpuRTCommandBuffer* rtCommandBuffer();
    void setCommandBuffer(GrGpuCommandBuffer* buffer) { fCommandBuffer = buffer; }

    GrGpu* gpu() { return fGpu; }

    void reset();

    /** Additional data required on a per-op basis when executing GrOps. */
    struct OpArgs {
        GrRenderTarget* renderTarget() const { return fProxy->priv().peekRenderTarget(); }

        GrOp* fOp;
        // TODO: do we still need the dst proxy here?
        GrRenderTargetProxy* fProxy;
        GrAppliedClip* fAppliedClip;
        GrXferProcessor::DstProxy fDstProxy;
    };

    void setOpArgs(OpArgs* opArgs) { fOpArgs = opArgs; }

    const OpArgs& drawOpArgs() const {
        SkASSERT(fOpArgs);
        SkASSERT(fOpArgs->fOp);
        return *fOpArgs;
    }

    /** Expose base class methods for incrementing the last flushed and next draw token. */

    void flushToken() { this->GrDeferredUploadTarget::flushToken(); }

    GrDeferredUploadToken issueDrawToken() {
        return this->GrDeferredUploadTarget::issueDrawToken();
    }

    /** Overrides of GrDeferredUploadTarget. */

    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) final;
    GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&&) final;

    /** Overrides of GrMeshDrawOp::Target. */

    void draw(const GrGeometryProcessor*, const GrPipeline*, const GrMesh&) final;
    void* makeVertexSpace(size_t vertexSize, int vertexCount, const GrBuffer**,
                          int* startVertex) final;
    uint16_t* makeIndexSpace(int indexCount, const GrBuffer**, int* startIndex) final;
    void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount, int fallbackVertexCount,
                                 const GrBuffer**, int* startVertex, int* actualVertexCount) final;
    uint16_t* makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount, const GrBuffer**,
                                    int* startIndex, int* actualIndexCount) final;
    void putBackIndices(int indexCount) final;
    void putBackVertices(int vertices, size_t vertexStride) final;
    GrRenderTargetProxy* proxy() const final { return fOpArgs->fProxy; }
    GrAppliedClip detachAppliedClip() final;
    const GrXferProcessor::DstProxy& dstProxy() const final { return fOpArgs->fDstProxy; }
    GrDeferredUploadTarget* deferredUploadTarget() final { return this; }
    const GrCaps& caps() const final;
    GrResourceProvider* resourceProvider() const final { return fResourceProvider; }

private:
    /** GrMeshDrawOp::Target override. */
    SkArenaAlloc* pipelineArena() override { return &fArena; }

    GrGpu* fGpu;
    GrResourceProvider* fResourceProvider;
    GrGpuCommandBuffer* fCommandBuffer;
    GrVertexBufferAllocPool fVertexPool;
    GrIndexBufferAllocPool fIndexPool;

    OpArgs* fOpArgs;
    SkArenaAlloc fArena{sizeof(GrPipeline) * 100};

    struct InlineUpload {
        InlineUpload(GrDeferredTextureUploadFn&& upload, GrDeferredUploadToken token)
                : fUpload(std::move(upload)), fUploadBeforeToken(token) {}
        GrDeferredTextureUploadFn fUpload;
        GrDeferredUploadToken fUploadBeforeToken;
    };

    // A set of contiguous draws that share a draw token and primitive processor. The draws all use
    // the op's pipeline. The meshes for the draw are stored in the fMeshes array and each
    // Queued draw uses fMeshCnt meshes from the fMeshes array. The reason for coallescing meshes
    // that share a primitive processor into a QueuedDraw is that it allows the Gpu object to setup
    // the shared state once and then issue draws for each mesh.
    struct Draw {
        int fMeshCnt = 0;
        GrPendingProgramElement<const GrGeometryProcessor> fGeometryProcessor;
        const GrPipeline* fPipeline;
        uint32_t fOpID;
    };

    template <typename T> class List {
    private:
        struct Node {
            template<typename... Args> Node(Args... args) : fT(std::forward<Args>(args)...) {}
            T fT;
            Node* fNext = nullptr;
        };
        Node* fHead = nullptr;
        Node* fTail = nullptr;

    public:
        List() = default;

        void reset() { fHead = fTail = nullptr; }

        template <typename... Args> T& append(SkArenaAlloc* arena, Args... args) {
            SkASSERT(!fHead == !fTail);
            auto* n = arena->make<Node>(std::forward<Args>(args)...);
            if (!fTail) {
                fHead = fTail = n;
            } else {
                fTail = fTail->fNext = n;
            }
            return fTail->fT;
        }

        class Iter {
        public:
            explicit Iter(List& list) : fCurr(list.fHead) {}
            Iter() = default;
            Iter& operator++() {
                fCurr = fCurr->fNext;
                return *this;
            }
            T& operator*() const { return fCurr->fT; }
            T* operator->() const { return &fCurr->fT; }
            bool operator==(const Iter& that) const { return fCurr == that.fCurr; }
            bool operator!=(const Iter& that) const { return !(*this == that); }

        private:
            Node* fCurr = nullptr;
        };

        Iter begin() { return Iter(*this); }
        Iter end() { return Iter(); }
    };

    List<GrDeferredTextureUploadFn> fAsapUploads;
    List<InlineUpload> fInlineUploads;
    List<Draw> fDraws;

    // All draws in all the GrMeshDrawOps have implicit tokens based on the order they are enqueued
    // globally across all ops. This is the offset of the first entry in fQueuedDraws.
    // fQueuedDraws[i]'s token is fBaseDrawToken + i.
    GrDeferredUploadToken fBaseDrawToken = GrDeferredUploadToken::AlreadyFlushedToken();

    // TODO: These should go in the arena. However, GrGpuCommandBuffer and other classes currently
    // accept contiguous arrays of meshes.
    SkSTArray<16, GrMesh> fMeshes;

    /** Stuff for execution time. */
    List<Draw>::Iter fCurrDraw;
    int fCurrMesh;
    List<InlineUpload>::Iter fCurrUpload;
};

#endif

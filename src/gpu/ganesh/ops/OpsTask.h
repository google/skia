/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef OpsTask_DEFINED
#define OpsTask_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "include/private/base/SkTypeTraits.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrDstProxyView.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/ops/GrOp.h"

#include <array>
#include <cstdint>

class GrAppliedClip;
class GrArenas;
class GrAuditTrail;
class GrCaps;
class GrDrawingManager;
class GrOpFlushState;
class GrRecordingContext;
class GrResourceAllocator;
class GrSurfaceProxyView;
class GrTextureResolveManager;
class OpsTaskTestingAccess;
class SkArenaAlloc;
class SkString;
enum GrSurfaceOrigin : int;

namespace skgpu::ganesh {

class SurfaceDrawContext;

class OpsTask : public GrRenderTask {
public:
    // Manage the arenas life time by maintaining are reference to it.
    OpsTask(GrDrawingManager*, GrSurfaceProxyView, GrAuditTrail*, sk_sp<GrArenas>);
    ~OpsTask() override;

    OpsTask* asOpsTask() override { return this; }

    bool isEmpty() const { return fOpChains.empty(); }
    bool usesMSAASurface() const { return fUsesMSAASurface; }
    GrXferBarrierFlags renderPassXferBarriers() const { return fRenderPassXferBarriers; }

    /**
     * Empties the draw buffer of any queued up draws.
     */
    void endFlush(GrDrawingManager*) override;

    void onPrePrepare(GrRecordingContext*) override;
    /**
     * Together these two functions flush all queued up draws to GrCommandBuffer. The return value
     * of onExecute() indicates whether any commands were actually issued to the GPU.
     */
    void onPrepare(GrOpFlushState* flushState) override;
    bool onExecute(GrOpFlushState* flushState) override;

    void addSampledTexture(GrSurfaceProxy* proxy) {
        // This function takes a GrSurfaceProxy because all subsequent uses of the proxy do not
        // require the specifics of GrTextureProxy, so this avoids a number of unnecessary virtual
        // asTextureProxy() calls. However, sampling the proxy implicitly requires that the proxy
        // be a texture. Eventually, when proxies are a unified type with flags, this can just
        // assert that capability.
        SkASSERT(proxy->asTextureProxy());
        fSampledProxies.push_back(proxy);
    }

    void addOp(GrDrawingManager*, GrOp::Owner, GrTextureResolveManager, const GrCaps&);

    void addDrawOp(GrDrawingManager*, GrOp::Owner, bool usesMSAA, const GrProcessorSet::Analysis&,
                   GrAppliedClip&&, const GrDstProxyView&, GrTextureResolveManager, const GrCaps&);

    void discard();

    enum class CanDiscardPreviousOps : bool {
        kYes = true,
        kNo = false
    };

    // Perform book-keeping for a fullscreen clear, regardless of how the clear is implemented later
    // (i.e. setColorLoadOp(), adding a ClearOp, or adding a FillRectOp that covers the device).
    // Returns true if the clear can be converted into a load op (barring device caps).
    bool resetForFullscreenClear(CanDiscardPreviousOps);

    // Must only be called if native color buffer clearing is enabled.
    void setColorLoadOp(GrLoadOp op, std::array<float, 4> color = {0, 0, 0, 0});

    // Returns whether the given opsTask can be appended at the end of this one.
    bool canMerge(const OpsTask*) const;

    // Merge as many opsTasks as possible from the head of 'tasks'. They should all be
    // renderPass compatible. Return the number of tasks merged into 'this'.
    int mergeFrom(SkSpan<const sk_sp<GrRenderTask>> tasks);

#ifdef SK_DEBUG
    int numClips() const override { return fNumClips; }
    void visitProxies_debugOnly(const GrVisitProxyFunc&) const override;
#endif

#if defined(GPU_TEST_UTILS)
    void dump(const SkString& label,
              SkString indent,
              bool printDependencies,
              bool close) const override;
    const char* name() const final { return "Ops"; }
    int numOpChains() const { return fOpChains.size(); }
    const GrOp* getChain(int index) const { return fOpChains[index].head(); }
#endif

protected:
    enum class StencilContent {
        kDontCare,
        kUserBitsCleared,  // User bits: cleared
                           // Clip bit: don't care (Ganesh always pre-clears the clip bit.)
        kPreserved
    };

    // Lets the caller specify what the content of the stencil buffer should be at the beginning
    // of the render pass.
    //
    // When requesting kClear: Tilers will load the stencil buffer with a "clear" op; non-tilers
    // will clear the stencil on first load, and then preserve it on subsequent loads. (Preserving
    // works because SurfaceDrawContexts are required to leave the user bits in a cleared state
    // once finished.)
    //
    // NOTE: initialContent must not be kClear if caps.performStencilClearsAsDraws() is true.
    void setInitialStencilContent(StencilContent initialContent) {
        fInitialStencilContent = initialContent;
    }

    void recordOp(GrOp::Owner, bool usesMSAA, GrProcessorSet::Analysis, GrAppliedClip*,
                  const GrDstProxyView*, const GrCaps&);

    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect* targetUpdateBounds) override;

private:
    bool isColorNoOp() const {
        // TODO: GrLoadOp::kDiscard (i.e., storing a discard) should also be grounds for skipping
        // execution. We currently don't because of Vulkan. See skbug.com/40040696.
        return fOpChains.empty() && GrLoadOp::kLoad == fColorLoadOp;
    }

    void deleteOps();

    // If a surfaceDrawContext splits its opsTask, it uses this method to guarantee stencil values
    // get preserved across its split tasks.
    void setMustPreserveStencil() { fMustPreserveStencil = true; }

    // Prevents this opsTask from merging backward. This is used by DMSAA when a non-multisampled
    // opsTask cannot be promoted to MSAA, or when we split a multisampled opsTask in order to
    // resolve its texture.
    void setCannotMergeBackward() { fCannotMergeBackward = true; }

    class OpChain {
    public:
        OpChain(GrOp::Owner, GrProcessorSet::Analysis, GrAppliedClip*, const GrDstProxyView*);
        ~OpChain() {
            // The ops are stored in a GrMemoryPool and must be explicitly deleted via the pool.
            SkASSERT(fList.empty());
        }

        OpChain(const OpChain&) = delete;
        OpChain& operator=(const OpChain&) = delete;
        OpChain(OpChain&&) = default;
        OpChain& operator=(OpChain&&) = default;

        void visitProxies(const GrVisitProxyFunc&) const;

        GrOp* head() const { return fList.head(); }

        GrAppliedClip* appliedClip() const { return fAppliedClip; }
        const GrDstProxyView& dstProxyView() const { return fDstProxyView; }
        const SkRect& bounds() const { return fBounds; }

        // Deletes all the ops in the chain.
        void deleteOps();

        // Attempts to move the ops from the passed chain to this chain at the head. Also attempts
        // to merge ops between the chains. Upon success the passed chain is empty.
        // Fails when the chains aren't of the same op type, have different clips or dst proxies.
        bool prependChain(OpChain*, const GrCaps&, SkArenaAlloc* opsTaskArena, GrAuditTrail*);

        // Attempts to add 'op' to this chain either by merging or adding to the tail. Returns
        // 'op' to the caller upon failure, otherwise null. Fails when the op and chain aren't of
        // the same op type, have different clips or dst proxies.
        GrOp::Owner appendOp(GrOp::Owner op, GrProcessorSet::Analysis, const GrDstProxyView*,
                             const GrAppliedClip*, const GrCaps&, SkArenaAlloc* opsTaskArena,
                             GrAuditTrail*);

        bool shouldExecute() const {
            return SkToBool(this->head());
        }

        using sk_is_trivially_relocatable = std::true_type;

    private:
        class List {
        public:
            List() = default;
            List(GrOp::Owner);
            List(List&&);
            List& operator=(List&& that);

            bool empty() const { return !SkToBool(fHead); }
            GrOp* head() const { return fHead.get(); }
            GrOp* tail() const { return fTail; }

            GrOp::Owner popHead();
            GrOp::Owner removeOp(GrOp* op);
            void pushHead(GrOp::Owner op);
            void pushTail(GrOp::Owner);

            void validate() const;

            using sk_is_trivially_relocatable = std::true_type;

        private:
            GrOp::Owner fHead{nullptr};
            GrOp* fTail{nullptr};

            static_assert(::sk_is_trivially_relocatable<decltype(fHead)>::value);
            static_assert(::sk_is_trivially_relocatable<decltype(fTail)>::value);
        };

        void validate() const;

        bool tryConcat(List*, GrProcessorSet::Analysis, const GrDstProxyView&, const GrAppliedClip*,
                       const SkRect& bounds, const GrCaps&, SkArenaAlloc* opsTaskArena,
                       GrAuditTrail*);
        static List DoConcat(List, List, const GrCaps&, SkArenaAlloc* opsTaskArena, GrAuditTrail*);

        List fList;
        GrProcessorSet::Analysis fProcessorAnalysis;
        GrDstProxyView fDstProxyView;
        GrAppliedClip* fAppliedClip;
        SkRect fBounds;

        static_assert(::sk_is_trivially_relocatable<decltype(fProcessorAnalysis)>::value);
        static_assert(::sk_is_trivially_relocatable<decltype(fDstProxyView)>::value);
        static_assert(::sk_is_trivially_relocatable<decltype(fAppliedClip)>::value);
        static_assert(::sk_is_trivially_relocatable<decltype(fBounds)>::value);
    };

    void onMakeSkippable() override;

    bool onIsUsed(GrSurfaceProxy*) const override;

    void gatherProxyIntervals(GrResourceAllocator*) const override;

    void forwardCombine(const GrCaps&);

    // Remove all ops, proxies, etc. Used in the merging algorithm when tasks can be skipped.
    void reset();

    friend class ::OpsTaskTestingAccess;

    // The SDC and OpsTask have to work together to handle buffer clears. In most cases, buffer
    // clearing can be done natively, in which case the op list's load ops are sufficient. In other
    // cases, draw ops must be used, which makes the SDC the best place for those decisions. This,
    // however, requires that the SDC be able to coordinate with the op list to achieve similar ends
    friend class skgpu::ganesh::SurfaceDrawContext;

    GrAuditTrail* fAuditTrail;

    bool fUsesMSAASurface;
    skgpu::Swizzle fTargetSwizzle;
    GrSurfaceOrigin fTargetOrigin;

    GrLoadOp fColorLoadOp = GrLoadOp::kLoad;
    std::array<float, 4> fLoadClearColor = {0, 0, 0, 0};
    StencilContent fInitialStencilContent = StencilContent::kDontCare;
    bool fMustPreserveStencil = false;
    bool fCannotMergeBackward = false;

    uint32_t fLastClipStackGenID = SK_InvalidUniqueID;
    SkIRect fLastDevClipBounds;
    int fLastClipNumAnalyticElements;

    GrXferBarrierFlags fRenderPassXferBarriers = GrXferBarrierFlags::kNone;

    // For ops/opsTask we have mean: 5 stdDev: 28
    skia_private::STArray<25, OpChain> fOpChains;

    sk_sp<GrArenas> fArenas;
    SkDEBUGCODE(int fNumClips;)

    // TODO: We could look into this being a set if we find we're adding a lot of duplicates that is
    // causing slow downs.
    skia_private::TArray<GrSurfaceProxy*, true> fSampledProxies;

    SkRect fTotalBounds = SkRect::MakeEmpty();
    SkIRect fClippedContentBounds = SkIRect::MakeEmpty();
};

}  // namespace skgpu::ganesh

#endif // OpsTask_DEFINED

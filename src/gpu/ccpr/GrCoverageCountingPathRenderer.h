/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverageCountingPathRenderer_DEFINED
#define GrCoverageCountingPathRenderer_DEFINED

#include "GrAllocator.h"
#include "GrOnFlushResourceProvider.h"
#include "GrPathRenderer.h"
#include "SkTInternalLList.h"
#include "ccpr/GrCCPRAtlas.h"
#include "ccpr/GrCCPRCoverageOpsBuilder.h"
#include "ops/GrDrawOp.h"
#include <map>

/**
 * This is a path renderer that draws antialiased paths by counting coverage in an offscreen
 * buffer. (See GrCCPRCoverageProcessor, GrCCPRPathProcessor)
 *
 * It also serves as the per-render-target tracker for pending path draws, and at the start of
 * flush, it compiles GPU buffers and renders a "coverage count atlas" for the upcoming paths.
 */
class GrCoverageCountingPathRenderer
    : public GrPathRenderer
    , public GrOnFlushCallbackObject {

    struct RTPendingOps;

public:
    static bool IsSupported(const GrCaps&);
    static sk_sp<GrCoverageCountingPathRenderer> CreateIfSupported(const GrCaps&);

    // GrPathRenderer overrides.
    StencilSupport onGetStencilSupport(const GrShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }
    bool onCanDrawPath(const CanDrawPathArgs& args) const override;
    bool onDrawPath(const DrawPathArgs&) final;

    // GrOnFlushCallbackObject overrides.
    void preFlush(GrOnFlushResourceProvider*, const uint32_t* opListIDs, int numOpListIDs,
                  SkTArray<sk_sp<GrRenderTargetContext>>* results) override;
    void postFlush() override;

    // This is the Op that ultimately draws a path into its final destination, using the atlas we
    // generate at flush time.
    class DrawPathsOp : public GrDrawOp {
    public:
        DEFINE_OP_CLASS_ID
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(DrawPathsOp);

        DrawPathsOp(GrCoverageCountingPathRenderer*, const DrawPathArgs&, GrColor);

        // GrDrawOp overrides.
        const char* name() const override { return "GrCoverageCountingPathRenderer::DrawPathsOp"; }
        FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
        RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override;
        void wasRecorded(GrRenderTargetOpList*) override;
        bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override;
        void onPrepare(GrOpFlushState*) override {}
        void onExecute(GrOpFlushState*) override;

    private:
        SkPath::FillType getFillType() const {
            SkASSERT(fDebugInstanceCount >= 1);
            return fHeadDraw.fPath.getFillType();
        }

        struct SingleDraw  {
            using ScissorMode = GrCCPRCoverageOpsBuilder::ScissorMode;
            SkIRect       fClipBounds;
            ScissorMode   fScissorMode;
            SkMatrix      fMatrix;
            SkPath        fPath;
            GrColor       fColor;
            SingleDraw*   fNext = nullptr;
        };

        SingleDraw& getOnlyPathDraw() {
            SkASSERT(&fHeadDraw == fTailDraw);
            SkASSERT(1 == fDebugInstanceCount);
            return fHeadDraw;
        }

        struct AtlasBatch {
            const GrCCPRAtlas*   fAtlas;
            int                  fEndInstanceIdx;
        };

        void addAtlasBatch(const GrCCPRAtlas* atlas, int endInstanceIdx) {
            SkASSERT(endInstanceIdx > fBaseInstance);
            SkASSERT(fAtlasBatches.empty() ||
                     endInstanceIdx > fAtlasBatches.back().fEndInstanceIdx);
            fAtlasBatches.push_back() = {atlas, endInstanceIdx};
        }

        GrCoverageCountingPathRenderer* const   fCCPR;
        const uint32_t                          fSRGBFlags;
        GrProcessorSet                          fProcessors;
        SingleDraw                              fHeadDraw;
        SingleDraw*                             fTailDraw;
        RTPendingOps*                           fOwningRTPendingOps;
        int                                     fBaseInstance;
        SkDEBUGCODE(int                         fDebugInstanceCount;)
        SkSTArray<1, AtlasBatch, true>          fAtlasBatches;

        friend class GrCoverageCountingPathRenderer;

        typedef GrDrawOp INHERITED;
    };

private:
    GrCoverageCountingPathRenderer() = default;

    struct RTPendingOps {
        SkTInternalLList<DrawPathsOp>                 fOpList;
        GrCCPRCoverageOpsBuilder::MaxBufferItems      fMaxBufferItems;
        GrSTAllocator<256, DrawPathsOp::SingleDraw>   fDrawsAllocator;
    };

    // Map from render target ID to the individual render target's pending path ops.
    std::map<uint32_t, RTPendingOps>   fRTPendingOpsMap;

    sk_sp<GrBuffer>                    fPerFlushIndexBuffer;
    sk_sp<GrBuffer>                    fPerFlushVertexBuffer;
    sk_sp<GrBuffer>                    fPerFlushInstanceBuffer;
    GrSTAllocator<4, GrCCPRAtlas>      fPerFlushAtlases;
    SkDEBUGCODE(bool                   fFlushing = false;)
};

#endif

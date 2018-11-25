/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverageCountingPathRenderer_DEFINED
#define GrCoverageCountingPathRenderer_DEFINED

#include <map>
#include "GrAllocator.h"
#include "GrOnFlushResourceProvider.h"
#include "GrPathRenderer.h"
#include "SkTInternalLList.h"
#include "ccpr/GrCCAtlas.h"
#include "ccpr/GrCCPathParser.h"
#include "ccpr/GrCCPathProcessor.h"
#include "ops/GrDrawOp.h"

/**
 * This is a path renderer that draws antialiased paths by counting coverage in an offscreen
 * buffer. (See GrCCCoverageProcessor, GrCCPathProcessor)
 *
 * It also serves as the per-render-target tracker for pending path draws, and at the start of
 * flush, it compiles GPU buffers and renders a "coverage count atlas" for the upcoming paths.
 */
class GrCoverageCountingPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
    struct RTPendingPaths;

public:
    static bool IsSupported(const GrCaps&);
    static sk_sp<GrCoverageCountingPathRenderer> CreateIfSupported(const GrCaps&,
                                                                   bool drawCachablePaths);

    ~GrCoverageCountingPathRenderer() override {
        // Ensure no Ops exist that could have a dangling pointer back into this class.
        SkASSERT(fRTPendingPathsMap.empty());
        SkASSERT(0 == fPendingDrawOpsCount);
    }

    // This is the Op that ultimately draws a path into its final destination, using the atlas we
    // generate at flush time.
    class DrawPathsOp : public GrDrawOp {
    public:
        DEFINE_OP_CLASS_ID
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(DrawPathsOp);

        DrawPathsOp(GrCoverageCountingPathRenderer*, const DrawPathArgs&, GrColor);
        ~DrawPathsOp() override;

        struct SingleDraw {
            SkIRect fClipIBounds;
            SkMatrix fMatrix;
            SkPath fPath;
            GrColor fColor;
            SingleDraw* fNext = nullptr;
        };

        const SingleDraw* head() const {
            SkASSERT(fInstanceCount >= 1);
            return &fHeadDraw;
        }

        SkDEBUGCODE(int numSkippedInstances_debugOnly() const { return fNumSkippedInstances; })

        // GrDrawOp overrides.
        const char* name() const override { return "GrCoverageCountingPathRenderer::DrawPathsOp"; }
        FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
        RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*,
                                    GrPixelConfigIsClamped) override;
        void wasRecorded(GrRenderTargetOpList*) override;
        bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override;
        void visitProxies(const VisitProxyFunc& func) const override {
            fProcessors.visitProxies(func);
        }
        void onPrepare(GrOpFlushState*) override {}
        void onExecute(GrOpFlushState*) override;

        int setupResources(GrOnFlushResourceProvider*,
                           GrCCPathProcessor::Instance* pathInstanceData, int pathInstanceIdx);

    private:
        SkPath::FillType getFillType() const {
            SkASSERT(fInstanceCount >= 1);
            return fHeadDraw.fPath.getFillType();
        }

        struct AtlasBatch {
            const GrCCAtlas* fAtlas;
            int fEndInstanceIdx;
        };

        void addAtlasBatch(const GrCCAtlas* atlas, int endInstanceIdx) {
            SkASSERT(endInstanceIdx > fBaseInstance);
            SkASSERT(fAtlasBatches.empty() ||
                     endInstanceIdx > fAtlasBatches.back().fEndInstanceIdx);
            fAtlasBatches.push_back() = {atlas, endInstanceIdx};
        }

        GrCoverageCountingPathRenderer* const fCCPR;
        const uint32_t fSRGBFlags;
        GrProcessorSet fProcessors;
        SingleDraw fHeadDraw;
        SingleDraw* fTailDraw;
        RTPendingPaths* fOwningRTPendingPaths;
        int fBaseInstance;
        SkDEBUGCODE(int fInstanceCount);
        SkDEBUGCODE(int fNumSkippedInstances);
        SkSTArray<1, AtlasBatch, true> fAtlasBatches;

        typedef GrDrawOp INHERITED;
    };

    // GrPathRenderer overrides.
    StencilSupport onGetStencilSupport(const GrShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }
    CanDrawPath onCanDrawPath(const CanDrawPathArgs& args) const override;
    bool onDrawPath(const DrawPathArgs&) final;

    // These are keyed by SkPath generation ID, and store which device-space paths are accessed and
    // where by clip FPs in a given opList. A single ClipPath can be referenced by multiple FPs. At
    // flush time their coverage count masks are packed into atlas(es) alongside normal DrawPathOps.
    class ClipPath {
    public:
        ClipPath() = default;
        ClipPath(const ClipPath&) = delete;

        ~ClipPath() {
            // Ensure no clip FPs exist with a dangling pointer back into this class.
            SkASSERT(!fAtlasLazyProxy || fAtlasLazyProxy->isUnique_debugOnly());
            // Ensure no lazy proxy callbacks exist with a dangling pointer back into this class.
            SkASSERT(fHasAtlasTransform);
        }

        bool isUninitialized() const { return !fAtlasLazyProxy; }
        void init(GrProxyProvider* proxyProvider,
                  const SkPath& deviceSpacePath, const SkIRect& accessRect,
                  int rtWidth, int rtHeight);
        void addAccess(const SkIRect& accessRect) {
            SkASSERT(!this->isUninitialized());
            fAccessRect.join(accessRect);
        }

        GrTextureProxy* atlasLazyProxy() const {
            SkASSERT(!this->isUninitialized());
            return fAtlasLazyProxy.get();
        }
        const SkPath& deviceSpacePath() const {
            SkASSERT(!this->isUninitialized());
            return fDeviceSpacePath;
        }
        const SkIRect& pathDevIBounds() const {
            SkASSERT(!this->isUninitialized());
            return fPathDevIBounds;
        }
        void placePathInAtlas(GrCoverageCountingPathRenderer*, GrOnFlushResourceProvider*,
                              GrCCPathParser*);

        const SkVector& atlasScale() const {
            SkASSERT(fHasAtlasTransform);
            return fAtlasScale;
        }
        const SkVector& atlasTranslate() const {
            SkASSERT(fHasAtlasTransform);
            return fAtlasTranslate;
        }

    private:
        sk_sp<GrTextureProxy> fAtlasLazyProxy;
        SkPath fDeviceSpacePath;
        SkIRect fPathDevIBounds;
        SkIRect fAccessRect;

        const GrCCAtlas* fAtlas = nullptr;
        int16_t fAtlasOffsetX;
        int16_t fAtlasOffsetY;
        SkDEBUGCODE(bool fHasAtlas = false);

        SkVector fAtlasScale;
        SkVector fAtlasTranslate;
        SkDEBUGCODE(bool fHasAtlasTransform = false);
    };

    bool canMakeClipProcessor(const SkPath& deviceSpacePath) const;

    std::unique_ptr<GrFragmentProcessor> makeClipProcessor(GrProxyProvider*, uint32_t oplistID,
                                                           const SkPath& deviceSpacePath,
                                                           const SkIRect& accessRect,
                                                           int rtWidth, int rtHeight);

    // GrOnFlushCallbackObject overrides.
    void preFlush(GrOnFlushResourceProvider*, const uint32_t* opListIDs, int numOpListIDs,
                  SkTArray<sk_sp<GrRenderTargetContext>>* results) override;
    void postFlush(GrDeferredUploadToken, const uint32_t* opListIDs, int numOpListIDs) override;

private:
    GrCoverageCountingPathRenderer(bool drawCachablePaths)
            : fDrawCachablePaths(drawCachablePaths) {}

    GrCCAtlas* placeParsedPathInAtlas(GrOnFlushResourceProvider*, const SkIRect& accessRect,
                                      const SkIRect& pathIBounds, int16_t* atlasOffsetX,
                                      int16_t* atlasOffsetY);

    struct RTPendingPaths {
        ~RTPendingPaths() {
            // Ensure all DrawPathsOps in this opList have been deleted.
            SkASSERT(fDrawOps.isEmpty());
        }

        SkTInternalLList<DrawPathsOp> fDrawOps;
        std::map<uint32_t, ClipPath> fClipPaths;
        GrSTAllocator<256, DrawPathsOp::SingleDraw> fDrawsAllocator;
    };

    // A map from render target ID to the individual render target's pending paths.
    std::map<uint32_t, RTPendingPaths> fRTPendingPathsMap;
    SkDEBUGCODE(int fPendingDrawOpsCount = 0);

    sk_sp<const GrBuffer> fPerFlushIndexBuffer;
    sk_sp<const GrBuffer> fPerFlushVertexBuffer;
    sk_sp<GrBuffer> fPerFlushInstanceBuffer;
    sk_sp<GrCCPathParser> fPerFlushPathParser;
    GrSTAllocator<4, GrCCAtlas> fPerFlushAtlases;
    bool fPerFlushResourcesAreValid;
    SkDEBUGCODE(bool fFlushing = false);

    const bool fDrawCachablePaths;
};

#endif

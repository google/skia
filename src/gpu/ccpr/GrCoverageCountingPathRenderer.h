/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverageCountingPathRenderer_DEFINED
#define GrCoverageCountingPathRenderer_DEFINED

#include "GrPathRenderer.h"
#include "GrRenderTargetOpList.h"
#include "SkArenaAlloc.h"
#include "SkTInternalLList.h"
#include "ccpr/GrCCClipPath.h"
#include "ccpr/GrCCDrawPathsOp.h"
#include <map>

class GrCCPerFlushResources;

/**
 * Tracks all the paths in a single render target that will be drawn at next flush.
 */
struct GrCCRTPendingPaths {
    ~GrCCRTPendingPaths() {
        // Ensure there are no surviving DrawPathsOps with a dangling pointer into this class.
        if (!fDrawOps.isEmpty()) {
            SK_ABORT("GrCCDrawPathsOp(s) not deleted during flush");
        }
        // Clip lazy proxies also reference this class from their callbacks, but those callbacks
        // are only invoked at flush time while we are still alive. (Unlike DrawPathsOps, that
        // unregister themselves upon destruction.) So it shouldn't matter if any clip proxies
        // are still around.
    }

    SkTInternalLList<GrCCDrawPathsOp> fDrawOps;
    std::map<uint32_t, GrCCClipPath> fClipPaths;
    SkSTArenaAlloc<10 * 1024> fAllocator{10 * 1024 * 2};
};

/**
 * This is a path renderer that draws antialiased paths by counting coverage in an offscreen
 * buffer. (See GrCCCoverageProcessor, GrCCPathProcessor.)
 *
 * It also serves as the per-render-target tracker for pending path draws, and at the start of
 * flush, it compiles GPU buffers and renders a "coverage count atlas" for the upcoming paths.
 */
class GrCoverageCountingPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
public:
    static bool IsSupported(const GrCaps&);
    static sk_sp<GrCoverageCountingPathRenderer> CreateIfSupported(const GrCaps&,
                                                                   bool drawCachablePaths);
    ~GrCoverageCountingPathRenderer() override;

    // GrPathRenderer overrides.
    StencilSupport onGetStencilSupport(const GrShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }
    CanDrawPath onCanDrawPath(const CanDrawPathArgs& args) const override;
    bool onDrawPath(const DrawPathArgs&) override;

    std::unique_ptr<GrFragmentProcessor> makeClipProcessor(GrProxyProvider*, uint32_t oplistID,
                                                           const SkPath& deviceSpacePath,
                                                           const SkIRect& accessRect,
                                                           int rtWidth, int rtHeight);

    // GrOnFlushCallbackObject overrides.
    void preFlush(GrOnFlushResourceProvider*, const uint32_t* opListIDs, int numOpListIDs,
                  SkTArray<sk_sp<GrRenderTargetContext>>* atlasDraws) override;
    void postFlush(GrDeferredUploadToken, const uint32_t* opListIDs, int numOpListIDs) override;

#ifdef SK_DEBUG
    bool isFlushing_debugOnly() const { return fFlushing; }
    void incrDrawOpCount_debugOnly() { ++fNumOutstandingDrawOps; }
    void decrDrawOpCount_debugOnly() { --fNumOutstandingDrawOps; }
#endif

private:
    GrCoverageCountingPathRenderer(bool drawCachablePaths);

    GrCCRTPendingPaths* lookupRTPendingPaths(GrRenderTargetOpList* opList) {
        SkASSERT(!fFlushing);
        return &fRTPendingPathsMap[opList->uniqueID()];
    }

    const GrCCPerFlushResources* getPerFlushResources() const {
        SkASSERT(fFlushing);
        return fPerFlushResources.get();
    }

    std::map<uint32_t, GrCCRTPendingPaths> fRTPendingPathsMap;
    SkSTArray<4, std::map<uint32_t, GrCCRTPendingPaths>::iterator> fFlushingRTPathIters;
    std::unique_ptr<GrCCPerFlushResources> fPerFlushResources;
    SkDEBUGCODE(bool fFlushing = false);
    SkDEBUGCODE(int fNumOutstandingDrawOps = 0);

    const bool fDrawCachablePaths;

    friend void GrCCDrawPathsOp::wasRecorded(GrRenderTargetOpList*);  // For lookupRTPendingPaths.
    friend void GrCCDrawPathsOp::onExecute(GrOpFlushState*);  // For getPerFlushResources.
};

#endif

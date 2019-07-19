/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverageCountingPathRenderer_DEFINED
#define GrCoverageCountingPathRenderer_DEFINED

#include <map>
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrRenderTargetOpList.h"
#include "src/gpu/ccpr/GrCCPerFlushResources.h"
#include "src/gpu/ccpr/GrCCPerOpListPaths.h"

class GrCCDrawPathsOp;
class GrCCPathCache;

/**
 * This is a path renderer that draws antialiased paths by counting coverage in an offscreen
 * buffer. (See GrCCCoverageProcessor, GrCCPathProcessor.)
 *
 * It also serves as the per-render-target tracker for pending path draws, and at the start of
 * flush, it compiles GPU buffers and renders a "coverage count atlas" for the upcoming paths.
 */
class GrCoverageCountingPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
public:
    using CoverageType = GrCCAtlas::CoverageType;

    static bool IsSupported(const GrCaps&, CoverageType* = nullptr);

    enum class AllowCaching : bool {
        kNo = false,
        kYes = true
    };

    static sk_sp<GrCoverageCountingPathRenderer> CreateIfSupported(
            const GrCaps&, AllowCaching, uint32_t contextUniqueID);

    CoverageType coverageType() const { return fCoverageType; }

    using PendingPathsMap = std::map<uint32_t, sk_sp<GrCCPerOpListPaths>>;

    // In DDL mode, Ganesh needs to be able to move the pending GrCCPerOpListPaths to the DDL object
    // (detachPendingPaths) and then return them upon replay (mergePendingPaths).
    PendingPathsMap detachPendingPaths() { return std::move(fPendingPaths); }

    void mergePendingPaths(const PendingPathsMap& paths) {
#ifdef SK_DEBUG
        // Ensure there are no duplicate opList IDs between the incoming path map and ours.
        // This should always be true since opList IDs are globally unique and these are coming
        // from different DDL recordings.
        for (const auto& it : paths) {
            SkASSERT(!fPendingPaths.count(it.first));
        }
#endif

        fPendingPaths.insert(paths.begin(), paths.end());
    }

    std::unique_ptr<GrFragmentProcessor> makeClipProcessor(
            uint32_t oplistID, const SkPath& deviceSpacePath, const SkIRect& accessRect,
            const GrCaps&);

    // GrOnFlushCallbackObject overrides.
    void preFlush(GrOnFlushResourceProvider*, const uint32_t* opListIDs, int numOpListIDs,
                  SkTArray<sk_sp<GrRenderTargetContext>>* out) override;
    void postFlush(GrDeferredUploadToken, const uint32_t* opListIDs, int numOpListIDs) override;

    void purgeCacheEntriesOlderThan(GrProxyProvider*, const GrStdSteadyClock::time_point&);

    // If a path spans more pixels than this, we need to crop it or else analytic AA can run out of
    // fp32 precision.
    static constexpr float kPathCropThreshold = 1 << 16;

    static void CropPath(const SkPath&, const SkIRect& cropbox, SkPath* out);

    // Maximum inflation of path bounds due to stroking (from width, miter, caps). Strokes wider
    // than this will be converted to fill paths and drawn by the CCPR filler instead.
    static constexpr float kMaxBoundsInflationFromStroke = 4096;

    static float GetStrokeDevWidth(const SkMatrix&, const SkStrokeRec&,
                                   float* inflationRadius = nullptr);

private:
    GrCoverageCountingPathRenderer(CoverageType, AllowCaching, uint32_t contextUniqueID);

    // GrPathRenderer overrides.
    StencilSupport onGetStencilSupport(const GrShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;
    bool onDrawPath(const DrawPathArgs&) override;

    GrCCPerOpListPaths* lookupPendingPaths(uint32_t opListID);
    void recordOp(std::unique_ptr<GrCCDrawPathsOp>, const DrawPathArgs&);

    const CoverageType fCoverageType;

    // fPendingPaths holds the GrCCPerOpListPaths objects that have already been created, but not
    // flushed, and those that are still being created. All GrCCPerOpListPaths objects will first
    // reside in fPendingPaths, then be moved to fFlushingPaths during preFlush().
    PendingPathsMap fPendingPaths;

    // fFlushingPaths holds the GrCCPerOpListPaths objects that are currently being flushed.
    // (It will only contain elements when fFlushing is true.)
    SkSTArray<4, sk_sp<GrCCPerOpListPaths>> fFlushingPaths;

    std::unique_ptr<GrCCPathCache> fPathCache;

    SkDEBUGCODE(bool fFlushing = false);

public:
    void testingOnly_drawPathDirectly(const DrawPathArgs&);
    const GrCCPerFlushResources* testingOnly_getCurrentFlushResources();
    const GrCCPathCache* testingOnly_getPathCache() const;
};

#endif

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
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/ccpr/GrCCPerFlushResources.h"
#include "src/gpu/ccpr/GrCCPerOpsTaskPaths.h"

/**
 * This is a path renderer that draws antialiased paths by counting coverage in an offscreen
 * buffer. (See GrCCCoverageProcessor, GrCCPathProcessor.)
 *
 * It also serves as the per-render-target tracker for pending path draws, and at the start of
 * flush, it compiles GPU buffers and renders a "coverage count atlas" for the upcoming paths.
 */
class GrCoverageCountingPathRenderer : public GrOnFlushCallbackObject {
public:
    static bool IsSupported(const GrCaps&);

    static std::unique_ptr<GrCoverageCountingPathRenderer> CreateIfSupported(const GrCaps&);

    using PendingPathsMap = std::map<uint32_t, sk_sp<GrCCPerOpsTaskPaths>>;

    // In DDL mode, Ganesh needs to be able to move the pending GrCCPerOpsTaskPaths to the DDL
    // object (detachPendingPaths) and then return them upon replay (mergePendingPaths).
    PendingPathsMap detachPendingPaths() { return std::move(fPendingPaths); }

    void mergePendingPaths(const PendingPathsMap& paths) {
#ifdef SK_DEBUG
        // Ensure there are no duplicate opsTask IDs between the incoming path map and ours.
        // This should always be true since opsTask IDs are globally unique and these are coming
        // from different DDL recordings.
        for (const auto& it : paths) {
            SkASSERT(!fPendingPaths.count(it.first));
        }
#endif

        fPendingPaths.insert(paths.begin(), paths.end());
    }

    // The atlas can take up a lot of memory. We should only use clip processors for small paths.
    // Large clip paths should consider a different method, like MSAA stencil.
    constexpr static int64_t kMaxClipPathArea = 256 * 256;

    std::unique_ptr<GrFragmentProcessor> makeClipProcessor(
            std::unique_ptr<GrFragmentProcessor> inputFP, uint32_t opsTaskID,
            const SkPath& deviceSpacePath, const SkIRect& accessRect, const GrCaps& caps);

    // GrOnFlushCallbackObject overrides.
    void preFlush(GrOnFlushResourceProvider*, SkSpan<const uint32_t> taskIDs) override;
    void postFlush(GrDeferredUploadToken, SkSpan<const uint32_t> taskIDs) override;

    // If a path spans more pixels than this, we need to crop it or else analytic AA can run out of
    // fp32 precision.
    static constexpr float kPathCropThreshold = 1 << 16;

    static void CropPath(const SkPath&, const SkIRect& cropbox, SkPath* out);

    // Maximum inflation of path bounds due to stroking (from width, miter, caps). Strokes wider
    // than this will be converted to fill paths and drawn by the CCPR filler instead.
    static constexpr float kMaxBoundsInflationFromStroke = 4096;

    static constexpr int kDoCopiesThreshold = 100;

private:
    GrCCPerOpsTaskPaths* lookupPendingPaths(uint32_t opsTaskID);

    // fPendingPaths holds the GrCCPerOpsTaskPaths objects that have already been created, but not
    // flushed, and those that are still being created. All GrCCPerOpsTaskPaths objects will first
    // reside in fPendingPaths, then be moved to fFlushingPaths during preFlush().
    PendingPathsMap fPendingPaths;

    // fFlushingPaths holds the GrCCPerOpsTaskPaths objects that are currently being flushed.
    // (It will only contain elements when fFlushing is true.)
    SkSTArray<4, sk_sp<GrCCPerOpsTaskPaths>> fFlushingPaths;

    std::unique_ptr<GrCCPerFlushResources> fPerFlushResources;

    SkDEBUGCODE(bool fFlushing = false);
};

#endif

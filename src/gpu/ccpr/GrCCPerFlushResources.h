/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPerFlushResources_DEFINED
#define GrCCPerFlushResources_DEFINED

#include "src/gpu/GrNonAtomicRef.h"
#include "src/gpu/ccpr/GrCCAtlas.h"

class GrOnFlushResourceProvider;

/**
 * This class wraps all the GPU resources that CCPR builds at flush time. It is allocated in CCPR's
 * preFlush() method, and referenced by all the GrCCPerOpsTaskPaths objects that are being flushed.
 * It is deleted in postFlush() once all the flushing GrCCPerOpsTaskPaths objects are deleted.
 */
class GrCCPerFlushResources : public GrNonAtomicRef<GrCCPerFlushResources> {
public:
    GrCCPerFlushResources(GrOnFlushResourceProvider*, const GrCCAtlas::Specs&);

    // Renders a path into an atlas.
    const GrCCAtlas* renderDeviceSpacePathInAtlas(
            GrOnFlushResourceProvider*, const SkIRect& clipIBounds, const SkPath& devPath,
            const SkIRect& devPathIBounds, GrFillRule fillRule, SkIVector* devToAtlasOffset);

    // Finishes off the GPU buffers and renders the atlas(es).
    bool finalize(GrOnFlushResourceProvider*);

    // Accessors used by draw calls, once the resources have been finalized.
    void placeRenderedPathInAtlas(
            GrOnFlushResourceProvider*, const SkIRect& clippedPathIBounds, GrScissorTest,
            SkIVector* devToAtlasOffset);

    // Enqueues the given path to be rendered during the next call to flushRenderedPaths().
    void enqueueRenderedPath(const SkPath&, GrFillRule, const SkIRect& clippedDevIBounds,
                             const SkMatrix& pathToAtlasMatrix, GrScissorTest enableScissorInAtlas,
                             SkIVector devToAtlasOffset);

    // Renders all enqueued paths into the given atlas and clears our path queue.
    void flushRenderedPaths(GrOnFlushResourceProvider*, GrCCAtlas*);

    GrCCAtlasStack fRenderedAtlasStack;

    // Paths to be rendered in the atlas we are currently building.
    struct AtlasPaths {
        SkPath fUberPath;  // Contains all contours from all non-scissored paths.
        SkSTArray<32, std::tuple<SkPath, SkIRect>> fScissoredPaths;
    };
    static_assert((int)GrFillRule::kNonzero == 0);
    static_assert((int)GrFillRule::kEvenOdd == 1);
    AtlasPaths fAtlasPaths[2];  // One for "nonzero" fill rule and one for "even-odd".
};

#endif

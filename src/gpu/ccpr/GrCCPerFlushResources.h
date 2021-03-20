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
    //
    // If the return value is non-null, it means the given path did not fit in the then-current
    // atlas, so it was retired and a new one was added to the stack. The return value is the
    // newly-retired atlas. (*NOT* the atlas the path was just drawn into.) The caller must call
    // assignAtlasTexture on all GrCCClipPaths that will use the retired atlas.
    std::unique_ptr<GrCCAtlas> renderDeviceSpacePathInAtlas(
            GrOnFlushResourceProvider*, const SkIRect& clipIBounds, const SkPath& devPath,
            const SkIRect& devPathIBounds, GrFillRule fillRule, SkIVector* devToAtlasOffset);

    // Finishes off the GPU buffers and renders the atlas(es).
    std::unique_ptr<GrCCAtlas> finalize(GrOnFlushResourceProvider*);

private:
    // If the return value is non-null, it means the given path did not fit in the then-current
    // atlas, so it was retired and a new one was added to the stack. The return value is the
    // newly-retired atlas. (*NOT* the atlas the path was just drawn into.) The caller must call
    // assignAtlasTexture on all GrCCClipPaths that will use the retired atlas.
    std::unique_ptr<GrCCAtlas> placeRenderedPathInAtlas(
            GrOnFlushResourceProvider*, const SkIRect& clippedPathIBounds, GrScissorTest,
            SkIVector* devToAtlasOffset);

    // Enqueues the given path to be rendered during the next call to flushRenderedPaths().
    void enqueueRenderedPath(const SkPath&, GrFillRule, const SkIRect& clippedDevIBounds,
                             const SkMatrix& pathToAtlasMatrix, GrScissorTest enableScissorInAtlas,
                             SkIVector devToAtlasOffset);

    // Renders all enqueued paths into the given atlas and clears our path queue.
    void flushRenderedPaths(GrOnFlushResourceProvider*);

    const GrCCAtlas::Specs fAtlasSpecs;

    // Paths to be rendered in the atlas we are currently building.
    struct AtlasPaths {
        SkPath fUberPath;  // Contains all contours from all non-scissored paths.
        SkSTArray<32, std::tuple<SkPath, SkIRect>> fScissoredPaths;
    };
    static_assert((int)GrFillRule::kNonzero == 0);
    static_assert((int)GrFillRule::kEvenOdd == 1);
    AtlasPaths fAtlasPaths[2];  // One for "nonzero" fill rule and one for "even-odd".

    std::unique_ptr<GrCCAtlas> fAtlas;
};

#endif

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPerFlushResources_DEFINED
#define GrCCPerFlushResources_DEFINED

#include "src/gpu/GrNonAtomicRef.h"
#include "src/gpu/ccpr/GrAutoMapVertexBuffer.h"
#include "src/gpu/ccpr/GrCCAtlas.h"
#include "src/gpu/ccpr/GrCCFiller.h"
#include "src/gpu/ccpr/GrStencilAtlasOp.h"

class GrCCPathCache;
class GrCCPathCacheEntry;
class GrOctoBounds;
class GrOnFlushResourceProvider;
class GrStyledShape;

/**
 * This struct counts values that help us preallocate buffers for rendered path geometry.
 */
struct GrCCRenderedPathStats {
    int fMaxPointsPerPath = 0;
    int fNumTotalSkPoints = 0;
    int fNumTotalSkVerbs = 0;
    int fNumTotalConicWeights = 0;

    void statPath(const SkPath&);
};

/**
 * This struct encapsulates the minimum and desired requirements for the GPU resources required by
 * CCPR in a given flush.
 */
struct GrCCPerFlushResourceSpecs {
    int fNumClipPaths = 0;
    GrCCRenderedPathStats fRenderedPathStats;
    GrCCAtlas::Specs fRenderedAtlasSpecs;

    bool isEmpty() const {
        return 0 == fNumClipPaths;
    }
};

/**
 * This class wraps all the GPU resources that CCPR builds at flush time. It is allocated in CCPR's
 * preFlush() method, and referenced by all the GrCCPerOpsTaskPaths objects that are being flushed.
 * It is deleted in postFlush() once all the flushing GrCCPerOpsTaskPaths objects are deleted.
 */
class GrCCPerFlushResources : public GrNonAtomicRef<GrCCPerFlushResources> {
public:
    GrCCPerFlushResources(
            GrOnFlushResourceProvider*,const GrCCPerFlushResourceSpecs&);

    bool isMapped() const { return fStencilResolveBuffer.isMapped(); }

    // Renders a path into an atlas.
    const GrCCAtlas* renderDeviceSpacePathInAtlas(
            const SkIRect& clipIBounds, const SkPath& devPath, const SkIRect& devPathIBounds,
            GrFillRule fillRule, SkIVector* devToAtlasOffset);

    // Finishes off the GPU buffers and renders the atlas(es).
    bool finalize(GrOnFlushResourceProvider*);

    // Accessors used by draw calls, once the resources have been finalized.
    const GrCCFiller& filler() const { SkASSERT(!this->isMapped()); return fFiller; }
    sk_sp<const GrGpuBuffer> stencilResolveBuffer() const {
        SkASSERT(!this->isMapped());
        return fStencilResolveBuffer.gpuBuffer();
    }

private:
    void placeRenderedPathInAtlas(
            const SkIRect& clippedPathIBounds, GrScissorTest, SkIVector* devToAtlasOffset);

    // In MSAA mode we record an additional instance per path that draws a rectangle on top of its
    // corresponding path in the atlas and resolves stencil winding values to coverage.
    void recordStencilResolveInstance(
            const SkIRect& clippedPathIBounds, const SkIVector& devToAtlasOffset, GrFillRule);

    GrCCFiller fFiller;
    GrCCAtlasStack fRenderedAtlasStack;

    // Used in MSAA mode make an intermediate draw that resolves stencil winding values to coverage.
    GrTAutoMapVertexBuffer<GrStencilAtlasOp::ResolveRectInstance> fStencilResolveBuffer;
    int fNextStencilResolveInstanceIdx = 0;
    SkDEBUGCODE(int fEndStencilResolveInstance);

public:
#ifdef SK_DEBUG
    void debugOnly_didReuseRenderedPath() {
        --fEndStencilResolveInstance;
    }
#endif
    const GrTexture* testingOnly_frontRenderedAtlasTexture() const;
};

inline void GrCCRenderedPathStats::statPath(const SkPath& path) {
    fMaxPointsPerPath = std::max(fMaxPointsPerPath, path.countPoints());
    fNumTotalSkPoints += path.countPoints();
    fNumTotalSkVerbs += path.countVerbs();
    fNumTotalConicWeights += SkPathPriv::ConicWeightCnt(path);
}

#endif

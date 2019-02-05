/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPerFlushResources_DEFINED
#define GrCCPerFlushResources_DEFINED

#include "GrNonAtomicRef.h"
#include "ccpr/GrCCAtlas.h"
#include "ccpr/GrCCFiller.h"
#include "ccpr/GrCCStroker.h"
#include "ccpr/GrCCPathProcessor.h"

class GrCCPathCache;
class GrCCPathCacheEntry;
class GrOnFlushResourceProvider;
class GrShape;

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
    static constexpr int kFillIdx = 0;
    static constexpr int kStrokeIdx = 1;

    int fNumCachedPaths = 0;

    int fNumCopiedPaths[2] = {0, 0};
    GrCCRenderedPathStats fCopyPathStats[2];
    GrCCAtlas::Specs fCopyAtlasSpecs;

    int fNumRenderedPaths[2] = {0, 0};
    int fNumClipPaths = 0;
    GrCCRenderedPathStats fRenderedPathStats[2];
    GrCCAtlas::Specs fRenderedAtlasSpecs;

    bool isEmpty() const {
        return 0 == fNumCachedPaths + fNumCopiedPaths[kFillIdx] + fNumCopiedPaths[kStrokeIdx] +
                    fNumRenderedPaths[kFillIdx] + fNumRenderedPaths[kStrokeIdx] + fNumClipPaths;
    }
    // Converts the copies to normal cached draws.
    void cancelCopies();
};

/**
 * This class wraps all the GPU resources that CCPR builds at flush time. It is allocated in CCPR's
 * preFlush() method, and referenced by all the GrCCPerOpListPaths objects that are being flushed.
 * It is deleted in postFlush() once all the flushing GrCCPerOpListPaths objects are deleted.
 */
class GrCCPerFlushResources : public GrNonAtomicRef<GrCCPerFlushResources> {
public:
    GrCCPerFlushResources(GrOnFlushResourceProvider*, const GrCCPerFlushResourceSpecs&);

    bool isMapped() const { return SkToBool(fPathInstanceData); }

    // Copies a coverage-counted path out of the given texture proxy, and into a cached, 8-bit,
    // literal coverage atlas. Updates the cache entry to reference the new atlas.
    void upgradeEntryToLiteralCoverageAtlas(GrCCPathCache*, GrOnFlushResourceProvider*,
                                            GrCCPathCacheEntry*, GrCCPathProcessor::DoEvenOddFill);

    // These two methods render a path into a temporary coverage count atlas. See
    // GrCCPathProcessor::Instance for a description of the outputs.
    //
    // strokeDevWidth must be 0 for fills, 1 for hairlines, or the stroke width in device-space
    // pixels for non-hairline strokes (implicitly requiring a rigid-body transform).
    GrCCAtlas* renderShapeInAtlas(const SkIRect& clipIBounds, const SkMatrix&, const GrShape&,
                                  float strokeDevWidth, SkRect* devBounds, SkRect* devBounds45,
                                  SkIRect* devIBounds, SkIVector* devToAtlasOffset);
    const GrCCAtlas* renderDeviceSpacePathInAtlas(const SkIRect& clipIBounds, const SkPath& devPath,
                                                  const SkIRect& devPathIBounds,
                                                  SkIVector* devToAtlasOffset);

    // Returns the index in instanceBuffer() of the next instance that will be added by
    // appendDrawPathInstance().
    int nextPathInstanceIdx() const { return fNextPathInstanceIdx; }

    // Appends an instance to instanceBuffer() that will draw a path to the destination render
    // target. The caller is responsible to call set() on the returned instance, to keep track of
    // its atlas and index (see nextPathInstanceIdx()), and to issue the actual draw call.
    GrCCPathProcessor::Instance& appendDrawPathInstance() {
        SkASSERT(this->isMapped());
        SkASSERT(fNextPathInstanceIdx < fEndPathInstance);
        return fPathInstanceData[fNextPathInstanceIdx++];
    }

    // Finishes off the GPU buffers and renders the atlas(es).
    bool finalize(GrOnFlushResourceProvider*, SkTArray<sk_sp<GrRenderTargetContext>>* out);

    // Accessors used by draw calls, once the resources have been finalized.
    const GrCCFiller& filler() const { SkASSERT(!this->isMapped()); return fFiller; }
    const GrCCStroker& stroker() const { SkASSERT(!this->isMapped()); return fStroker; }
    sk_sp<const GrGpuBuffer> refIndexBuffer() const {
        SkASSERT(!this->isMapped());
        return fIndexBuffer;
    }
    sk_sp<const GrGpuBuffer> refVertexBuffer() const {
        SkASSERT(!this->isMapped());
        return fVertexBuffer;
    }
    sk_sp<const GrGpuBuffer> refInstanceBuffer() const {
        SkASSERT(!this->isMapped());
        return fInstanceBuffer;
    }

private:
    void recordCopyPathInstance(const GrCCPathCacheEntry&, const SkIVector& newAtlasOffset,
                                GrCCPathProcessor::DoEvenOddFill, sk_sp<GrTextureProxy> srcProxy);
    bool placeRenderedPathInAtlas(const SkIRect& clipIBounds, const SkIRect& pathIBounds,
                                  GrScissorTest*, SkIRect* clippedPathIBounds,
                                  SkIVector* devToAtlasOffset);

    const SkAutoSTArray<32, SkPoint> fLocalDevPtsBuffer;
    GrCCFiller fFiller;
    GrCCStroker fStroker;
    GrCCAtlasStack fCopyAtlasStack;
    GrCCAtlasStack fRenderedAtlasStack;

    const sk_sp<const GrGpuBuffer> fIndexBuffer;
    const sk_sp<const GrGpuBuffer> fVertexBuffer;
    const sk_sp<GrGpuBuffer> fInstanceBuffer;

    GrCCPathProcessor::Instance* fPathInstanceData = nullptr;
    int fNextCopyInstanceIdx;
    SkDEBUGCODE(int fEndCopyInstance);
    int fNextPathInstanceIdx;
    SkDEBUGCODE(int fEndPathInstance);

    // Represents a range of copy-path instances that all share the same source proxy. (i.e. Draw
    // instances that copy a path mask from a 16-bit coverage count atlas into an 8-bit literal
    // coverage atlas.)
    struct CopyPathRange {
        CopyPathRange() = default;
        CopyPathRange(sk_sp<GrTextureProxy> srcProxy, int count)
                : fSrcProxy(std::move(srcProxy)), fCount(count) {}
        sk_sp<GrTextureProxy> fSrcProxy;
        int fCount;
    };

    SkSTArray<4, CopyPathRange> fCopyPathRanges;
    int fCurrCopyAtlasRangesIdx = 0;

    // This is a list of coverage count atlas textures that have been invalidated due to us copying
    // their paths into new 8-bit literal coverage atlases. Since copying is finished by the time
    // we begin rendering new atlases, we can recycle these textures for the rendered atlases rather
    // than allocating new texture objects upon instantiation.
    SkSTArray<2, sk_sp<GrTexture>> fRecyclableAtlasTextures;

public:
    const GrTexture* testingOnly_frontCopyAtlasTexture() const;
    const GrTexture* testingOnly_frontRenderedAtlasTexture() const;
};

inline void GrCCRenderedPathStats::statPath(const SkPath& path) {
    fMaxPointsPerPath = SkTMax(fMaxPointsPerPath, path.countPoints());
    fNumTotalSkPoints += path.countPoints();
    fNumTotalSkVerbs += path.countVerbs();
    fNumTotalConicWeights += SkPathPriv::ConicWeightCnt(path);
}

#endif

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
#include "ccpr/GrCCPathParser.h"
#include "ccpr/GrCCPathProcessor.h"

class GrCCPathCacheEntry;
class GrOnFlushResourceProvider;

/**
 * This struct encapsulates the minimum and desired requirements for the GPU resources required by
 * CCPR in a given flush.
 */
struct GrCCPerFlushResourceSpecs {
    int fNumCachedPaths = 0;

    int fNumCopiedPaths = 0;
    GrCCPathParser::PathStats fCopyPathStats;
    GrCCAtlas::Specs fCopyAtlasSpecs;

    int fNumRenderedPaths = 0;
    int fNumClipPaths = 0;
    GrCCPathParser::PathStats fRenderedPathStats;
    GrCCAtlas::Specs fRenderedAtlasSpecs;

    bool isEmpty() const {
        return 0 == fNumCachedPaths + fNumCopiedPaths + fNumRenderedPaths + fNumClipPaths;
    }
    void convertCopiesToRenders();
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

    // Copies a path out of the the previous flush's stashed mainline coverage count atlas, and into
    // a cached, 8-bit, literal-coverage atlas. The actual source texture to copy from will be
    // provided at the time finalize() is called.
    GrCCAtlas* copyPathToCachedAtlas(const GrCCPathCacheEntry&, GrCCPathProcessor::DoEvenOddFill,
                                     SkIVector* newAtlasOffset);

    // These two methods render a path into a temporary coverage count atlas. See GrCCPathParser for
    // a description of the arguments. The returned atlases are "const" to prevent the caller from
    // assigning a unique key.
    const GrCCAtlas* renderPathInAtlas(const SkIRect& clipIBounds, const SkMatrix&, const SkPath&,
                                       SkRect* devBounds, SkRect* devBounds45, SkIRect* devIBounds,
                                       SkIVector* devToAtlasOffset);
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

    // Finishes off the GPU buffers and renders the atlas(es). 'stashedAtlasProxy', if provided, is
    // the mainline coverage count atlas from the previous flush. It will be used as the source
    // texture for any copies setup by copyStashedPathToAtlas().
    bool finalize(GrOnFlushResourceProvider*, sk_sp<GrTextureProxy> stashedAtlasProxy,
                  SkTArray<sk_sp<GrRenderTargetContext>>* out);

    // Accessors used by draw calls, once the resources have been finalized.
    const GrCCPathParser& pathParser() const { SkASSERT(!this->isMapped()); return fPathParser; }
    const GrBuffer* indexBuffer() const { SkASSERT(!this->isMapped()); return fIndexBuffer.get(); }
    const GrBuffer* vertexBuffer() const { SkASSERT(!this->isMapped()); return fVertexBuffer.get();}
    GrBuffer* instanceBuffer() const { SkASSERT(!this->isMapped()); return fInstanceBuffer.get(); }

    // Returns the mainline coverage count atlas that the client may stash for next flush, if any.
    // The caller is responsible to call getOrAssignUniqueKey() on this atlas if they wish to
    // actually stash it in order to copy paths into cached atlases.
    GrCCAtlas* nextAtlasToStash() {
        return fRenderedAtlasStack.empty() ? nullptr : &fRenderedAtlasStack.front();
    }

    // Returs true if the client has called getOrAssignUniqueKey() on our nextAtlasToStash().
    bool hasStashedAtlas() const {
        return !fRenderedAtlasStack.empty() && fRenderedAtlasStack.front().uniqueKey().isValid();
    }
    const GrUniqueKey& stashedAtlasKey() const  {
        SkASSERT(this->hasStashedAtlas());
        return fRenderedAtlasStack.front().uniqueKey();
    }

private:
    bool placeParsedPathInAtlas(const SkIRect& clipIBounds, const SkIRect& pathIBounds,
                                SkIVector* devToAtlasOffset);

    GrCCPathParser fPathParser;
    GrCCAtlasStack fCopyAtlasStack;
    GrCCAtlasStack fRenderedAtlasStack;

    const sk_sp<const GrBuffer> fIndexBuffer;
    const sk_sp<const GrBuffer> fVertexBuffer;
    const sk_sp<GrBuffer> fInstanceBuffer;

    GrCCPathProcessor::Instance* fPathInstanceData = nullptr;
    int fNextCopyInstanceIdx;
    SkDEBUGCODE(int fEndCopyInstance);
    int fNextPathInstanceIdx;
    SkDEBUGCODE(int fEndPathInstance);
};

#endif

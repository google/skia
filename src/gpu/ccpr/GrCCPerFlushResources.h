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

/**
 * This struct encapsulates the minimum and desired requirements for the GPU resources required by
 * CCPR in a given flush.
 */
struct GrCCPerFlushResourceSpecs {
    int fNumRenderedPaths = 0;
    int fNumClipPaths = 0;
    GrCCPathParser::PathStats fParsingPathStats;
    GrCCAtlas::Specs fAtlasSpecs;

    bool isEmpty() const { return 0 == fNumRenderedPaths + fNumClipPaths; }
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

    // Renders a path into a temporary atlas. See GrCCPathParser for a description of the arguments.
    const GrCCAtlas* renderPathInAtlas(const SkIRect& clipIBounds, const SkMatrix&, const SkPath&,
                                       SkRect* devBounds, SkRect* devBounds45,
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

    // Finishes off the GPU buffers and renders the atlas(es).
    bool finalize(GrOnFlushResourceProvider*, SkTArray<sk_sp<GrRenderTargetContext>>* out);

    // Accessors used by draw calls, once the resources have been finalized.
    const GrCCPathParser& pathParser() const { SkASSERT(!this->isMapped()); return fPathParser; }
    const GrBuffer* indexBuffer() const { SkASSERT(!this->isMapped()); return fIndexBuffer.get(); }
    const GrBuffer* vertexBuffer() const { SkASSERT(!this->isMapped()); return fVertexBuffer.get();}
    GrBuffer* instanceBuffer() const { SkASSERT(!this->isMapped()); return fInstanceBuffer.get(); }

private:
    bool placeParsedPathInAtlas(const SkIRect& clipIBounds, const SkIRect& pathIBounds,
                                SkIVector* devToAtlasOffset);

    GrCCPathParser fPathParser;
    GrCCAtlasStack fAtlasStack;

    const sk_sp<const GrBuffer> fIndexBuffer;
    const sk_sp<const GrBuffer> fVertexBuffer;
    const sk_sp<GrBuffer> fInstanceBuffer;

    GrCCPathProcessor::Instance* fPathInstanceData = nullptr;
    int fNextPathInstanceIdx = 0;
    SkDEBUGCODE(int fEndPathInstance);
};

#endif

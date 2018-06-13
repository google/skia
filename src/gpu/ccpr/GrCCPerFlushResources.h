/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPerFlushResources_DEFINED
#define GrCCPerFlushResources_DEFINED

#include "GrAllocator.h"
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

    GrCCAtlas* renderPathInAtlas(const SkIRect& clipIBounds, const SkMatrix&, const SkPath&,
                                 SkRect* devBounds, SkRect* devBounds45, int16_t* offsetX,
                                 int16_t* offsetY);
    GrCCAtlas* renderDeviceSpacePathInAtlas(const SkIRect& clipIBounds, const SkPath& devPath,
                                            const SkIRect& devPathIBounds, int16_t* atlasOffsetX,
                                            int16_t* atlasOffsetY);

    GrCCPathProcessor::Instance& appendDrawPathInstance() {
        SkASSERT(this->isMapped());
        SkASSERT(fNextPathInstanceIdx < fPathInstanceBufferCount);
        return fPathInstanceData[fNextPathInstanceIdx++];
    }
    int nextPathInstanceIdx() const { return fNextPathInstanceIdx; }

    bool finalize(GrOnFlushResourceProvider*, SkTArray<sk_sp<GrRenderTargetContext>>* atlasDraws);

    const GrBuffer* indexBuffer() const { SkASSERT(!this->isMapped()); return fIndexBuffer.get(); }
    const GrBuffer* vertexBuffer() const { SkASSERT(!this->isMapped()); return fVertexBuffer.get();}
    GrBuffer* instanceBuffer() const { SkASSERT(!this->isMapped()); return fInstanceBuffer.get(); }

private:
    GrCCAtlas* placeParsedPathInAtlas(const SkIRect& clipIBounds, const SkIRect& pathIBounds,
                                      int16_t* atlasOffsetX, int16_t* atlasOffsetY);

    const sk_sp<GrCCPathParser> fPathParser;
    const GrCCAtlas::Specs fAtlasSpecs;

    sk_sp<const GrBuffer> fIndexBuffer;
    sk_sp<const GrBuffer> fVertexBuffer;
    sk_sp<GrBuffer> fInstanceBuffer;

    GrCCPathProcessor::Instance* fPathInstanceData = nullptr;
    int fNextPathInstanceIdx = 0;
    SkDEBUGCODE(int fPathInstanceBufferCount);

    GrSTAllocator<4, GrCCAtlas> fAtlases;
};

#endif

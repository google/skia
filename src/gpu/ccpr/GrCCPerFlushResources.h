/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPerFlushResources_DEFINED
#define GrCCPerFlushResources_DEFINED

#include "GrAllocator.h"
#include "ccpr/GrCCAtlas.h"
#include "ccpr/GrCCPathParser.h"
#include "ccpr/GrCCPathProcessor.h"

/**
 * This class wraps all the GPU resources that CCPR builds at flush time.
 */
class GrCCPerFlushResources {
public:
    GrCCPerFlushResources(GrOnFlushResourceProvider*, int numPathDraws, int numClipPaths,
                          const GrCCPathParser::PathStats&);

    bool isMapped() const { return SkToBool(fPathInstanceData); }

    GrCCAtlas* addPathToAtlas(const GrCaps&, const SkIRect& clipIBounds, const SkMatrix&,
                              const SkPath&, SkRect* devBounds, SkRect* devBounds45,
                              int16_t* offsetX, int16_t* offsetY);
    GrCCAtlas* addDeviceSpacePathToAtlas(const GrCaps&, const SkIRect& clipIBounds,
                                         const SkPath& devPath, const SkIRect& devPathIBounds,
                                         int16_t* atlasOffsetX, int16_t* atlasOffsetY);

    // See GrCCPathProcessor::Instance.
    int appendDrawPathInstance(const SkRect& devBounds, const SkRect& devBounds45,
                               const std::array<float, 4>& viewMatrix,
                               const std::array<float, 2>& viewTranslate,
                               const std::array<int16_t, 2>& atlasOffset, uint32_t color) {
        SkASSERT(this->isMapped());
        SkASSERT(fPathInstanceCount < fPathInstanceBufferCount);
        fPathInstanceData[fPathInstanceCount] = {devBounds, devBounds45, viewMatrix, viewTranslate,
                                                 atlasOffset, color};
        return fPathInstanceCount++;
    }
    int pathInstanceCount() const { return fPathInstanceCount; }

    bool finalize(GrOnFlushResourceProvider*,
                  SkTArray<sk_sp<GrRenderTargetContext>>* atlasDraws);

    const GrBuffer* indexBuffer() const { SkASSERT(!this->isMapped()); return fIndexBuffer.get(); }
    const GrBuffer* vertexBuffer() const { SkASSERT(!this->isMapped()); return fVertexBuffer.get();}
    GrBuffer* instanceBuffer() const { SkASSERT(!this->isMapped()); return fInstanceBuffer.get(); }

private:
    GrCCAtlas* placeParsedPathInAtlas(const GrCaps&, const SkIRect& clipIBounds,
                                      const SkIRect& pathIBounds, int16_t* atlasOffsetX,
                                      int16_t* atlasOffsetY);

    const sk_sp<GrCCPathParser> fPathParser;

    sk_sp<const GrBuffer> fIndexBuffer;
    sk_sp<const GrBuffer> fVertexBuffer;
    sk_sp<GrBuffer> fInstanceBuffer;

    GrCCPathProcessor::Instance* fPathInstanceData = nullptr;
    int fPathInstanceCount = 0;
    SkDEBUGCODE(int fPathInstanceBufferCount);

    GrSTAllocator<4, GrCCAtlas> fAtlases;
};

#endif

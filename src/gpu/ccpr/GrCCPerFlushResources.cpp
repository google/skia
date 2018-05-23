/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPerFlushResources.h"

#include "GrOnFlushResourceProvider.h"
#include "GrRenderTargetContext.h"
#include "SkIPoint16.h"

using PathInstance = GrCCPathProcessor::Instance;

GrCCPerFlushResources::GrCCPerFlushResources(GrOnFlushResourceProvider* onFlushRP,
                                             int numPathDraws, int numClipPaths,
                                             const GrCCPathParser::PathStats& pathStats)
        : fPathParser(sk_make_sp<GrCCPathParser>(numPathDraws + numClipPaths, pathStats))
        , fIndexBuffer(GrCCPathProcessor::FindIndexBuffer(onFlushRP))
        , fVertexBuffer(GrCCPathProcessor::FindVertexBuffer(onFlushRP))
        , fInstanceBuffer(onFlushRP->makeBuffer(kVertex_GrBufferType,
                                                numPathDraws * sizeof(PathInstance))) {
    if (!fIndexBuffer) {
        SkDebugf("WARNING: failed to allocate CCPR index buffer. No paths will be drawn.\n");
        return;
    }
    if (!fVertexBuffer) {
        SkDebugf("WARNING: failed to allocate CCPR vertex buffer. No paths will be drawn.\n");
        return;
    }
    if (!fInstanceBuffer) {
        SkDebugf("WARNING: failed to allocate CCPR instance buffer. No paths will be drawn.\n");
        return;
    }
    fPathInstanceData = static_cast<PathInstance*>(fInstanceBuffer->map());
    SkASSERT(fPathInstanceData);
    SkDEBUGCODE(fPathInstanceBufferCount = numPathDraws);
}

GrCCAtlas* GrCCPerFlushResources::renderPathInAtlas(const GrCaps& caps, const SkIRect& clipIBounds,
                                                    const SkMatrix& m, const SkPath& path,
                                                    SkRect* devBounds, SkRect* devBounds45,
                                                    int16_t* atlasOffsetX, int16_t* atlasOffsetY) {
    SkASSERT(this->isMapped());
    SkIRect devIBounds;
    fPathParser->parsePath(m, path, devBounds, devBounds45);
    devBounds->roundOut(&devIBounds);
    return this->placeParsedPathInAtlas(caps, clipIBounds, devIBounds, atlasOffsetX, atlasOffsetY);
}

GrCCAtlas* GrCCPerFlushResources::renderDeviceSpacePathInAtlas(const GrCaps& caps,
                                                               const SkIRect& clipIBounds,
                                                               const SkPath& devPath,
                                                               const SkIRect& devPathIBounds,
                                                               int16_t* atlasOffsetX,
                                                               int16_t* atlasOffsetY) {
    SkASSERT(this->isMapped());
    fPathParser->parseDeviceSpacePath(devPath);
    return this->placeParsedPathInAtlas(caps, clipIBounds, devPathIBounds, atlasOffsetX,
                                        atlasOffsetY);
}

GrCCAtlas* GrCCPerFlushResources::placeParsedPathInAtlas(const GrCaps& caps,
                                                         const SkIRect& clipIBounds,
                                                         const SkIRect& pathIBounds,
                                                         int16_t* atlasOffsetX,
                                                         int16_t* atlasOffsetY) {
    using ScissorMode = GrCCPathParser::ScissorMode;
    ScissorMode scissorMode;
    SkIRect clippedPathIBounds;
    if (clipIBounds.contains(pathIBounds)) {
        clippedPathIBounds = pathIBounds;
        scissorMode = ScissorMode::kNonScissored;
    } else if (clippedPathIBounds.intersect(clipIBounds, pathIBounds)) {
        scissorMode = ScissorMode::kScissored;
    } else {
        fPathParser->discardParsedPath();
        return nullptr;
    }

    SkIPoint16 atlasLocation;
    int h = clippedPathIBounds.height(), w = clippedPathIBounds.width();
    if (fAtlases.empty() || !fAtlases.back().addRect(w, h, &atlasLocation)) {
        if (!fAtlases.empty()) {
            // The atlas is out of room and can't grow any bigger.
            auto coverageCountBatchID = fPathParser->closeCurrentBatch();
            fAtlases.back().setCoverageCountBatchID(coverageCountBatchID);
        }
        fAtlases.emplace_back(caps, SkTMax(w, h));
        SkAssertResult(fAtlases.back().addRect(w, h, &atlasLocation));
    }

    *atlasOffsetX = atlasLocation.x() - static_cast<int16_t>(clippedPathIBounds.left());
    *atlasOffsetY = atlasLocation.y() - static_cast<int16_t>(clippedPathIBounds.top());
    fPathParser->saveParsedPath(scissorMode, clippedPathIBounds, *atlasOffsetX, *atlasOffsetY);

    return &fAtlases.back();
}

bool GrCCPerFlushResources::finalize(GrOnFlushResourceProvider* onFlushRP,
                                     SkTArray<sk_sp<GrRenderTargetContext>>* atlasDraws) {
    SkASSERT(this->isMapped());
    fInstanceBuffer->unmap();
    fPathInstanceData = nullptr;

    if (!fAtlases.empty()) {
        auto coverageCountBatchID = fPathParser->closeCurrentBatch();
        fAtlases.back().setCoverageCountBatchID(coverageCountBatchID);
    }

    if (!fPathParser->finalize(onFlushRP)) {
        SkDebugf("WARNING: failed to allocate GPU buffers for CCPR. No paths will be drawn.\n");
        return false;
    }

    // Draw the atlas(es).
    GrTAllocator<GrCCAtlas>::Iter atlasIter(&fAtlases);
    while (atlasIter.next()) {
        if (auto rtc = atlasIter.get()->finalize(onFlushRP, fPathParser)) {
            atlasDraws->push_back(std::move(rtc));
        }
    }

    return true;
}

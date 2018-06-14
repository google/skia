/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPerFlushResources.h"

#include "GrClip.h"
#include "GrMemoryPool.h"
#include "GrOnFlushResourceProvider.h"
#include "GrSurfaceContextPriv.h"
#include "GrRenderTargetContext.h"
#include "SkMakeUnique.h"

using CoverageCountBatchID = GrCCPathParser::CoverageCountBatchID;
using PathInstance = GrCCPathProcessor::Instance;

namespace {

// Renders coverage counts to a CCPR atlas using the resources' pre-filled GrCCPathParser.
class RenderAtlasOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          sk_sp<const GrCCPerFlushResources> resources,
                                          CoverageCountBatchID batchID, const SkISize& drawBounds) {
        return std::unique_ptr<GrDrawOp>(new RenderAtlasOp(std::move(resources), batchID,
                                                           drawBounds));
    }

    // GrDrawOp interface.
    const char* name() const override { return "RenderAtlasOp (CCPR)"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*,
                                GrPixelConfigIsClamped) override { return RequiresDstTexture::kNo; }
    bool onCombineIfPossible(GrOp* other, const GrCaps&) override { return false; }
    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* flushState) override {
        fResources->pathParser().drawCoverageCount(flushState, fBatchID, fDrawBounds);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    RenderAtlasOp(sk_sp<const GrCCPerFlushResources> resources, CoverageCountBatchID batchID,
                  const SkISize& drawBounds)
            : GrDrawOp(ClassID())
            , fResources(std::move(resources))
            , fBatchID(batchID)
            , fDrawBounds(SkIRect::MakeWH(drawBounds.width(), drawBounds.height())) {
        this->setBounds(SkRect::MakeIWH(fDrawBounds.width(), fDrawBounds.height()),
                        GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
    }

    const sk_sp<const GrCCPerFlushResources> fResources;
    const CoverageCountBatchID fBatchID;
    const SkIRect fDrawBounds;
};

}

GrCCPerFlushResources::GrCCPerFlushResources(GrOnFlushResourceProvider* onFlushRP,
                                             const GrCCPerFlushResourceSpecs& specs)
        : fPathParser(specs.fNumRenderedPaths + specs.fNumClipPaths, specs.fParsingPathStats)
        , fAtlasStack(specs.fAtlasSpecs)
        , fIndexBuffer(GrCCPathProcessor::FindIndexBuffer(onFlushRP))
        , fVertexBuffer(GrCCPathProcessor::FindVertexBuffer(onFlushRP))
        , fInstanceBuffer(onFlushRP->makeBuffer(kVertex_GrBufferType,
                                                specs.fNumRenderedPaths * sizeof(PathInstance))) {
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
    SkDEBUGCODE(fEndPathInstance = specs.fNumRenderedPaths);
}

const GrCCAtlas* GrCCPerFlushResources::renderPathInAtlas(const SkIRect& clipIBounds,
                                                          const SkMatrix& m, const SkPath& path,
                                                          SkRect* devBounds, SkRect* devBounds45,
                                                          SkIVector* atlasOffset) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx < fEndPathInstance);

    fPathParser.parsePath(m, path, devBounds, devBounds45);

    SkIRect devIBounds;
    devBounds->roundOut(&devIBounds);

    if (!this->placeParsedPathInAtlas(clipIBounds, devIBounds, atlasOffset)) {
        SkDEBUGCODE(--fEndPathInstance);
        return nullptr;  // Path was degenerate or clipped away.
    }
    return &fAtlasStack.current();
}

const GrCCAtlas* GrCCPerFlushResources::renderDeviceSpacePathInAtlas(
        const SkIRect& clipIBounds, const SkPath& devPath, const SkIRect& devPathIBounds,
        SkIVector* atlasOffset) {
    SkASSERT(this->isMapped());
    fPathParser.parseDeviceSpacePath(devPath);
    if (!this->placeParsedPathInAtlas(clipIBounds, devPathIBounds, atlasOffset)) {
        return nullptr;
    }
    return &fAtlasStack.current();
}

bool GrCCPerFlushResources::placeParsedPathInAtlas(const SkIRect& clipIBounds,
                                                   const SkIRect& pathIBounds,
                                                   SkIVector* atlasOffset) {
    using ScissorMode = GrCCPathParser::ScissorMode;
    ScissorMode scissorMode;
    SkIRect clippedPathIBounds;
    if (clipIBounds.contains(pathIBounds)) {
        clippedPathIBounds = pathIBounds;
        scissorMode = ScissorMode::kNonScissored;
    } else if (clippedPathIBounds.intersect(clipIBounds, pathIBounds)) {
        scissorMode = ScissorMode::kScissored;
    } else {
        fPathParser.discardParsedPath();
        return false;
    }

    if (GrCCAtlas* retiredAtlas = fAtlasStack.addRect(clippedPathIBounds, atlasOffset)) {
        // We did not fit in the previous coverage count atlas, so it was retired. Close off of the
        // path parser's current batch and the retired atlas will render it.
        retiredAtlas->setRenderToken(fPathParser.closeCurrentBatch());
    }
    fPathParser.saveParsedPath(scissorMode, clippedPathIBounds, *atlasOffset);
    return true;
}

bool GrCCPerFlushResources::finalize(GrOnFlushResourceProvider* onFlushRP,
                                     SkTArray<sk_sp<GrRenderTargetContext>>* out) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx == fEndPathInstance);

    fInstanceBuffer->unmap();
    fPathInstanceData = nullptr;

    if (!fAtlasStack.empty()) {
        fAtlasStack.current().setRenderToken(fPathParser.closeCurrentBatch());
    }

    // Build the GPU buffers to render path coverage counts. (This must not happen until after the
    // final call to fPathParser.closeCurrentBatch().)
    if (!fPathParser.finalize(onFlushRP)) {
        SkDebugf("WARNING: failed to allocate GPU buffers for CCPR. No paths will be drawn.\n");
        return false;
    }

    // Render the atlas(es).
    for (GrCCAtlasStack::Iter atlas(fAtlasStack); atlas.next();) {
        if (auto rtc = atlas->makeClearedTextureProxy(onFlushRP, kAlpha_half_GrPixelConfig)) {
            auto op = RenderAtlasOp::Make(rtc->surfPriv().getContext(), sk_ref_sp(this),
                                          atlas->renderToken(), atlas->drawBounds());
            rtc->addDrawOp(GrNoClip(), std::move(op));
            out->push_back(std::move(rtc));
        }
    }

    return true;
}

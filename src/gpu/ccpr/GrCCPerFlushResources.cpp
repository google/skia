/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCPerFlushResources.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrFillRectOp.h"

GrCCPerFlushResources::GrCCPerFlushResources(GrOnFlushResourceProvider* onFlushRP,
                                             const GrCCAtlas::Specs& specs)
        : fAtlasSpecs(specs) {
}

std::unique_ptr<GrCCAtlas> GrCCPerFlushResources::renderDeviceSpacePathInAtlas(
        GrOnFlushResourceProvider* onFlushRP, const SkIRect& clipIBounds, const SkPath& devPath,
        const SkIRect& devPathIBounds, GrFillRule fillRule, SkIVector* devToAtlasOffset) {
    SkASSERT(!devPath.isEmpty());
    GrScissorTest enableScissorInAtlas;
    SkIRect clippedPathIBounds;
    if (clipIBounds.contains(devPathIBounds)) {
        clippedPathIBounds = devPathIBounds;
        enableScissorInAtlas = GrScissorTest::kDisabled;
    } else {
        SkAssertResult(clippedPathIBounds.intersect(clipIBounds, devPathIBounds));
        enableScissorInAtlas = GrScissorTest::kEnabled;
    }

    auto retiredAtlas = this->placeRenderedPathInAtlas(onFlushRP, clippedPathIBounds,
                                                       enableScissorInAtlas, devToAtlasOffset);

    SkMatrix atlasMatrix = SkMatrix::Translate(devToAtlasOffset->fX, devToAtlasOffset->fY);
    this->enqueueRenderedPath(devPath, fillRule, clippedPathIBounds, atlasMatrix,
                              enableScissorInAtlas, *devToAtlasOffset);

    return retiredAtlas;
}

std::unique_ptr<GrCCAtlas> GrCCPerFlushResources::placeRenderedPathInAtlas(
        GrOnFlushResourceProvider* onFlushRP, const SkIRect& clippedPathIBounds,
        GrScissorTest scissorTest, SkIVector* devToAtlasOffset) {
    std::unique_ptr<GrCCAtlas> retiredAtlas;
    SkIPoint16 location;
    if (!fAtlas ||
        !fAtlas->addRect(clippedPathIBounds.width(), clippedPathIBounds.height(), &location)) {
        // The retired atlas is out of room and can't grow any bigger.
        if (fAtlas) {
            this->flushRenderedPaths(onFlushRP);
            retiredAtlas = std::move(fAtlas);
        }
        fAtlas = std::make_unique<GrCCAtlas>(fAtlasSpecs, *onFlushRP->caps());
        SkASSERT(clippedPathIBounds.width() <= fAtlasSpecs.fMinWidth);
        SkASSERT(clippedPathIBounds.height() <= fAtlasSpecs.fMinHeight);
        SkAssertResult(fAtlas->addRect(clippedPathIBounds.width(), clippedPathIBounds.height(),
                                       &location));
    }
    devToAtlasOffset->set(location.x() - clippedPathIBounds.left(),
                          location.y() - clippedPathIBounds.top());
    return retiredAtlas;
}

void GrCCPerFlushResources::enqueueRenderedPath(const SkPath& path, GrFillRule fillRule,
                                                const SkIRect& clippedDevIBounds,
                                                const SkMatrix& pathToAtlasMatrix,
                                                GrScissorTest enableScissorInAtlas,
                                                SkIVector devToAtlasOffset) {
    SkPath* atlasPath;
    if (enableScissorInAtlas == GrScissorTest::kDisabled) {
        atlasPath = &fAtlasPaths[(int)fillRule].fUberPath;
    } else {
        auto& [scissoredPath, scissor] = fAtlasPaths[(int)fillRule].fScissoredPaths.push_back();
        scissor = clippedDevIBounds.makeOffset(devToAtlasOffset);
        atlasPath = &scissoredPath;
    }
    auto origin = clippedDevIBounds.topLeft() + devToAtlasOffset;
    atlasPath->moveTo(origin.fX, origin.fY);  // Implicit moveTo(0,0).
    atlasPath->addPath(path, pathToAtlasMatrix);
}

static void draw_stencil_to_coverage(GrOnFlushResourceProvider* onFlushRP,
                                     GrSurfaceDrawContext* surfaceDrawContext, SkRect&& rect) {
    auto aaType = GrAAType::kMSAA;
    auto fillRectFlags = GrSimpleMeshDrawOpHelper::InputFlags::kNone;

    // This will be the final op in the surfaceDrawContext. So if Ganesh is planning to discard the
    // stencil values anyway, then we might not actually need to reset the stencil values back to 0.
    bool mustResetStencil = !onFlushRP->caps()->discardStencilValuesAfterRenderPass();

    if (surfaceDrawContext->numSamples() == 1) {
        // We are mixed sampled. We need to either enable conservative raster (preferred) or disable
        // MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for the cover
        // geometry, the stencil test is still multisampled and will still produce smooth results.)
        if (onFlushRP->caps()->conservativeRasterSupport()) {
            fillRectFlags |= GrSimpleMeshDrawOpHelper::InputFlags::kConservativeRaster;
        } else {
            aaType = GrAAType::kNone;
        }
        mustResetStencil = true;
    }

    const GrUserStencilSettings* stencil;
    if (mustResetStencil) {
        constexpr static GrUserStencilSettings kTestAndResetStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kZero,
                GrUserStencilOp::kKeep,
                0xffff>());

        // Outset the cover rect in case there are T-junctions in the path bounds.
        rect.outset(1, 1);
        stencil = &kTestAndResetStencil;
    } else {
        constexpr static GrUserStencilSettings kTestStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kKeep,
                GrUserStencilOp::kKeep,
                0xffff>());

        stencil = &kTestStencil;
    }

    GrPaint paint;
    paint.setColor4f(SK_PMColor4fWHITE);
    GrQuad coverQuad(rect);
    DrawQuad drawQuad{coverQuad, coverQuad, GrQuadAAFlags::kAll};
    auto coverOp = GrFillRectOp::Make(surfaceDrawContext->recordingContext(), std::move(paint),
                                      aaType, &drawQuad, stencil, fillRectFlags);
    surfaceDrawContext->addDrawOp(nullptr, std::move(coverOp));
}

void GrCCPerFlushResources::flushRenderedPaths(GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(fAtlas);
    auto surfaceDrawContext = fAtlas->instantiate(onFlushRP);
    if (!surfaceDrawContext) {
        for (int i = 0; i < (int)SK_ARRAY_COUNT(fAtlasPaths); ++i) {
            fAtlasPaths[i].fUberPath.reset();
            fAtlasPaths[i].fScissoredPaths.reset();
        }
        return;
    }

    for (int i = 0; i < (int)SK_ARRAY_COUNT(fAtlasPaths); ++i) {
        SkPathFillType fillType = (i == (int)GrFillRule::kNonzero) ? SkPathFillType::kWinding
                                                                   : SkPathFillType::kEvenOdd;
        SkPath& uberPath = fAtlasPaths[i].fUberPath;
        if (!uberPath.isEmpty()) {
            uberPath.setIsVolatile(true);
            uberPath.setFillType(fillType);
            surfaceDrawContext->stencilPath(nullptr, GrAA::kYes, SkMatrix::I(), uberPath);
            uberPath.reset();
        }
        for (auto& [scissoredPath, scissor] : fAtlasPaths[i].fScissoredPaths) {
            GrFixedClip fixedClip(
                    surfaceDrawContext->asRenderTargetProxy()->backingStoreDimensions(), scissor);
            scissoredPath.setIsVolatile(true);
            scissoredPath.setFillType(fillType);
            surfaceDrawContext->stencilPath(&fixedClip, GrAA::kYes, SkMatrix::I(), scissoredPath);
        }
        fAtlasPaths[i].fScissoredPaths.reset();
    }

    draw_stencil_to_coverage(onFlushRP, surfaceDrawContext.get(),
                             SkRect::MakeSize(SkSize::Make(fAtlas->drawBounds())));

    if (surfaceDrawContext->asSurfaceProxy()->requiresManualMSAAResolve()) {
        onFlushRP->addTextureResolveTask(sk_ref_sp(surfaceDrawContext->asTextureProxy()),
                                         GrSurfaceProxy::ResolveFlags::kMSAA);
    }
}

std::unique_ptr<GrCCAtlas> GrCCPerFlushResources::finalize(GrOnFlushResourceProvider* onFlushRP) {
    if (fAtlas) {
        this->flushRenderedPaths(onFlushRP);
    }
#ifdef SK_DEBUG
    // These paths should have been rendered and reset to empty by this point.
    for (size_t i = 0; i < SK_ARRAY_COUNT(fAtlasPaths); ++i) {
        SkASSERT(fAtlasPaths[i].fUberPath.isEmpty());
        SkASSERT(fAtlasPaths[i].fScissoredPaths.empty());
    }
#endif
    return std::move(fAtlas);
}

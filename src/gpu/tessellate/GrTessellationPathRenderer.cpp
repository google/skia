/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

#include "include/private/SkVx.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/tessellate/GrAtlasRenderTask.h"
#include "src/gpu/tessellate/GrDrawAtlasPathOp.h"
#include "src/gpu/tessellate/GrPathInnerTriangulateOp.h"
#include "src/gpu/tessellate/GrPathStencilCoverOp.h"
#include "src/gpu/tessellate/GrPathTessellateOp.h"
#include "src/gpu/tessellate/GrStrokeTessellateOp.h"
#include "src/gpu/tessellate/shaders/GrModulateAtlasCoverageFP.h"

constexpr static auto kAtlasAlpha8Type = GrColorType::kAlpha_8;
constexpr static int kAtlasInitialSize = 512;

// The atlas is only used for small-area paths, which means at least one dimension of every path is
// guaranteed to be quite small. So if we transpose tall paths, then every path will have a small
// height, which lends very well to efficient pow2 atlas packing.
constexpr static auto kAtlasAlgorithm = GrDynamicAtlas::RectanizerAlgorithm::kPow2;

// Ensure every path in the atlas falls in or below the 128px high rectanizer band.
constexpr static int kAtlasMaxPathHeight = 128;

bool GrTessellationPathRenderer::IsSupported(const GrCaps& caps) {
    return !caps.avoidStencilBuffers() &&
           caps.drawInstancedSupport() &&
           caps.shaderCaps()->vertexIDSupport() &&
           !caps.disableTessellationPathRenderer();
}

GrTessellationPathRenderer::GrTessellationPathRenderer(GrRecordingContext* rContext) {
    const GrCaps& caps = *rContext->priv().caps();
    auto atlasFormat = caps.getDefaultBackendFormat(kAtlasAlpha8Type, GrRenderable::kYes);
    if (rContext->asDirectContext() &&  // The atlas doesn't support DDL yet.
        caps.internalMultisampleCount(atlasFormat) > 1) {
#if GR_TEST_UTILS
        fAtlasMaxSize = rContext->priv().options().fMaxTextureAtlasSize;
#else
        fAtlasMaxSize = 2048;
#endif
        fAtlasMaxSize = SkPrevPow2(std::min(fAtlasMaxSize, caps.maxPreferredRenderTargetSize()));
        fAtlasInitialSize = SkNextPow2(std::min(kAtlasInitialSize, fAtlasMaxSize));
    }
}

GrPathRenderer::StencilSupport GrTessellationPathRenderer::onGetStencilSupport(
        const GrStyledShape& shape) const {
    if (!shape.style().isSimpleFill()) {
        // Don't bother with stroke stencilling yet. Skia probably shouldn't support this at all
        // since you can't clip by a stroke.
        return kNoSupport_StencilSupport;
    }
    return shape.knownToBeConvex() ? kNoRestriction_StencilSupport : kStencilOnly_StencilSupport;
}

GrPathRenderer::CanDrawPath GrTessellationPathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
    const GrStyledShape& shape = *args.fShape;
    if (args.fAAType == GrAAType::kCoverage ||
        shape.style().hasPathEffect() ||
        args.fViewMatrix->hasPerspective() ||
        shape.style().strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style ||
        shape.inverseFilled() ||
        !args.fProxy->canUseStencil(*args.fCaps)) {
        return CanDrawPath::kNo;
    }
    if (args.fHasUserStencilSettings) {
        // Non-convex paths and strokes use the stencil buffer internally, so they can't support
        // draws with stencil settings.
        if (!shape.style().isSimpleFill() || !shape.knownToBeConvex()) {
            return CanDrawPath::kNo;
        }
    }
    return CanDrawPath::kYes;
}

static GrOp::Owner make_non_convex_fill_op(GrRecordingContext* rContext,
                                           GrTessellationPathRenderer::PathFlags pathFlags,
                                           GrAAType aaType, const SkRect& pathDevBounds,
                                           const SkMatrix& viewMatrix, const SkPath& path,
                                           GrPaint&& paint) {
    SkASSERT(!path.isConvex());
    int numVerbs = path.countVerbs();
    if (numVerbs > 0) {
        // Check if the path is large and/or simple enough that we can triangulate the inner fan
        // on the CPU. This is our fastest approach. It allows us to stencil only the curves,
        // and then fill the inner fan directly to the final render target, thus drawing the
        // majority of pixels in a single render pass.
        float gpuFragmentWork = pathDevBounds.height() * pathDevBounds.width();
        float cpuTessellationWork = numVerbs * SkNextLog2(numVerbs);  // N log N.
        constexpr static float kCpuWeight = 512;
        constexpr static float kMinNumPixelsToTriangulate = 256 * 256;
        if (cpuTessellationWork * kCpuWeight + kMinNumPixelsToTriangulate < gpuFragmentWork) {
            return GrOp::Make<GrPathInnerTriangulateOp>(rContext, viewMatrix, path,
                                                        std::move(paint), aaType, pathFlags,
                                                        pathDevBounds);
        }
    }
    return GrOp::Make<GrPathStencilCoverOp>(rContext, viewMatrix, path, std::move(paint), aaType,
                                            pathFlags, pathDevBounds);
}

bool GrTessellationPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GrSurfaceDrawContext* surfaceDrawContext = args.fSurfaceDrawContext;

    SkPath path;
    args.fShape->asPath(&path);

    // Handle strokes first.
    if (!args.fShape->style().isSimpleFill()) {
        SkASSERT(args.fUserStencilSettings->isUnused());
        const SkStrokeRec& stroke = args.fShape->style().strokeRec();
        SkASSERT(stroke.getStyle() != SkStrokeRec::kStrokeAndFill_Style);
        auto op = GrOp::Make<GrStrokeTessellateOp>(args.fContext, args.fAAType, *args.fViewMatrix,
                                                   path, stroke, std::move(args.fPaint));
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
        return true;
    }

    SkRect pathDevBounds;
    args.fViewMatrix->mapRect(&pathDevBounds, args.fShape->bounds());

    if (args.fUserStencilSettings->isUnused()) {
        // See if the path is small and simple enough to atlas instead of drawing directly.
        //
        // NOTE: The atlas uses alpha8 coverage even for msaa render targets. We could theoretically
        // render the sample mask to an integer texture, but such a scheme would probably require
        // GL_EXT_post_depth_coverage, which appears to have low adoption.
        SkIRect devIBounds;
        SkIPoint16 locationInAtlas;
        bool transposedInAtlas;
        auto visitProxiesUsedByDraw = [&args](GrVisitProxyFunc visitor) {
            if (args.fPaint.hasColorFragmentProcessor()) {
                args.fPaint.getColorFragmentProcessor()->visitProxies(visitor);
            }
            if (args.fPaint.hasCoverageFragmentProcessor()) {
                args.fPaint.getCoverageFragmentProcessor()->visitProxies(visitor);
            }
        };
        if (this->tryAddPathToAtlas(args.fContext, *args.fViewMatrix, path, pathDevBounds,
                                    args.fAAType != GrAAType::kNone, &devIBounds, &locationInAtlas,
                                    &transposedInAtlas, visitProxiesUsedByDraw)) {
            auto op = GrOp::Make<GrDrawAtlasPathOp>(
                    args.fContext, surfaceDrawContext->numSamples(),
                    sk_ref_sp(fAtlasRenderTasks.back()->atlasProxy()), devIBounds, locationInAtlas,
                    transposedInAtlas, *args.fViewMatrix, std::move(args.fPaint));
            surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
            return true;
        }
    }

    // Handle convex paths only if we couldn't fit them in the atlas. We give the atlas priority in
    // an effort to reduce DMSAA triggers.
    if (args.fShape->knownToBeConvex()) {
        auto op = GrOp::Make<GrPathTessellateOp>(args.fContext, *args.fViewMatrix, path,
                                                 std::move(args.fPaint), args.fAAType,
                                                 args.fUserStencilSettings, pathDevBounds);
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
        return true;
    }

    SkASSERT(args.fUserStencilSettings->isUnused());  // See onGetStencilSupport().
    auto op = make_non_convex_fill_op(args.fContext, PathFlags::kNone, args.fAAType, pathDevBounds,
                                      *args.fViewMatrix, path, std::move(args.fPaint));
    surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
    return true;
}

void GrTessellationPathRenderer::onStencilPath(const StencilPathArgs& args) {
    SkASSERT(args.fShape->style().isSimpleFill());  // See onGetStencilSupport().

    GrSurfaceDrawContext* surfaceDrawContext = args.fSurfaceDrawContext;
    GrAAType aaType = (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;

    SkRect pathDevBounds;
    args.fViewMatrix->mapRect(&pathDevBounds, args.fShape->bounds());

    SkPath path;
    args.fShape->asPath(&path);

    if (args.fShape->knownToBeConvex()) {
        constexpr static GrUserStencilSettings kMarkStencil(
            GrUserStencilSettings::StaticInit<
                0x0001,
                GrUserStencilTest::kAlways,
                0xffff,
                GrUserStencilOp::kReplace,
                GrUserStencilOp::kKeep,
                0xffff>());

        GrPaint stencilPaint;
        stencilPaint.setXPFactory(GrDisableColorXPFactory::Get());
        auto op = GrOp::Make<GrPathTessellateOp>(args.fContext, *args.fViewMatrix, path,
                                                 std::move(stencilPaint), aaType, &kMarkStencil,
                                                 pathDevBounds);
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
        return;
    }

    auto op = make_non_convex_fill_op(args.fContext, PathFlags::kStencilOnly, aaType, pathDevBounds,
                                      *args.fViewMatrix, path, GrPaint());
    surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
}

GrFPResult GrTessellationPathRenderer::makeAtlasClipFP(GrRecordingContext* rContext,
                                                       const GrOp* opBeingClipped,
                                                       std::unique_ptr<GrFragmentProcessor> inputFP,
                                                       const SkIRect& drawBounds,
                                                       const SkMatrix& viewMatrix,
                                                       const SkPath& path, GrAA aa) {
    if (viewMatrix.hasPerspective()) {
        return GrFPFailure(std::move(inputFP));
    }
    SkIRect devIBounds;
    SkIPoint16 locationInAtlas;
    bool transposedInAtlas;
    auto visitProxiesUsedByDraw = [&opBeingClipped, &inputFP](GrVisitProxyFunc visitor) {
        opBeingClipped->visitProxies(visitor);
        if (inputFP) {
            inputFP->visitProxies(visitor);
        }
    };
    // tryAddPathToAtlas() ignores inverseness of the fill. See getAtlasUberPath().
    if (!this->tryAddPathToAtlas(rContext, viewMatrix, path, viewMatrix.mapRect(path.getBounds()),
                                 aa != GrAA::kNo, &devIBounds, &locationInAtlas,
                                 &transposedInAtlas, visitProxiesUsedByDraw)) {
        // The path is too big, or the atlas ran out of room.
        return GrFPFailure(std::move(inputFP));
    }
    SkMatrix atlasMatrix;
    auto [atlasX, atlasY] = locationInAtlas;
    if (!transposedInAtlas) {
        atlasMatrix = SkMatrix::Translate(atlasX - devIBounds.left(), atlasY - devIBounds.top());
    } else {
        atlasMatrix.setAll(0, 1, atlasX - devIBounds.top(),
                           1, 0, atlasY - devIBounds.left(),
                           0, 0, 1);
    }
    auto flags = GrModulateAtlasCoverageFP::Flags::kNone;
    if (path.isInverseFillType()) {
        flags |= GrModulateAtlasCoverageFP::Flags::kInvertCoverage;
    }
    if (!devIBounds.contains(drawBounds)) {
        flags |= GrModulateAtlasCoverageFP::Flags::kCheckBounds;
        // At this point in time we expect callers to tighten the scissor for "kIntersect" clips, as
        // opposed to us having to check the path bounds. Feel free to remove this assert if that
        // ever changes.
        SkASSERT(path.isInverseFillType());
    }
    GrSurfaceProxyView atlasView = fAtlasRenderTasks.back()->readView(*rContext->priv().caps());
    return GrFPSuccess(std::make_unique<GrModulateAtlasCoverageFP>(flags, std::move(inputFP),
                                                                   std::move(atlasView),
                                                                   atlasMatrix, devIBounds));
}

void GrTessellationPathRenderer::AtlasPathKey::set(const SkMatrix& m, bool antialias,
                                                   const SkPath& path) {
    using grvx::float2;
    fAffineMatrix[0] = m.getScaleX();
    fAffineMatrix[1] = m.getSkewX();
    fAffineMatrix[2] = m.getSkewY();
    fAffineMatrix[3] = m.getScaleY();
    float2 translate = {m.getTranslateX(), m.getTranslateY()};
    float2 subpixelPosition = translate - skvx::floor(translate);
    float2 subpixelPositionKey = skvx::trunc(subpixelPosition *
                                             GrPathTessellator::kLinearizationPrecision);
    skvx::cast<uint8_t>(subpixelPositionKey).store(fSubpixelPositionKey);
    fAntialias = antialias;
    fFillRule = (uint8_t)GrFillRuleForSkPath(path);  // Fill rule doesn't affect the path's genID.
    fPathGenID = path.getGenerationID();
}

bool GrTessellationPathRenderer::tryAddPathToAtlas(GrRecordingContext* rContext,
                                                   const SkMatrix& viewMatrix, const SkPath& path,
                                                   const SkRect& pathDevBounds, bool antialias,
                                                   SkIRect* devIBounds, SkIPoint16* locationInAtlas,
                                                   bool* transposedInAtlas,
                                                   const VisitProxiesFn& visitProxiesUsedByDraw) {
    SkASSERT(!viewMatrix.hasPerspective());  // See onCanDrawPath().

    if (!fAtlasMaxSize) {
        return false;
    }

    // The atlas is not compatible with DDL. We should only be using it on direct contexts.
    SkASSERT(rContext->asDirectContext());

    const GrCaps& caps = *rContext->priv().caps();
    if (!caps.multisampleDisableSupport() && !antialias) {
        return false;
    }

    // Transpose tall paths in the atlas. Since we limit ourselves to small-area paths, this
    // guarantees that every atlas entry has a small height, which lends very well to efficient pow2
    // atlas packing.
    pathDevBounds.roundOut(devIBounds);
    int maxDimension = devIBounds->width();
    int minDimension = devIBounds->height();
    *transposedInAtlas = minDimension > maxDimension;
    if (*transposedInAtlas) {
        std::swap(minDimension, maxDimension);
    }

    // Check if the path is too large for an atlas. Since we transpose paths in the atlas so height
    // is always "minDimension", limiting to kAtlasMaxPathHeight^2 pixels guarantees height <=
    // kAtlasMaxPathHeight, while also allowing paths that are very wide and short.
    if ((uint64_t)maxDimension * minDimension > kAtlasMaxPathHeight * kAtlasMaxPathHeight ||
        maxDimension > fAtlasMaxSize) {
        return false;
    }

    // Check if this path is already in the atlas. This is mainly for clip paths.
    AtlasPathKey atlasPathKey;
    if (!path.isVolatile()) {
        atlasPathKey.set(viewMatrix, antialias, path);
        if (const SkIPoint16* existingLocation = fAtlasPathCache.find(atlasPathKey)) {
            *locationInAtlas = *existingLocation;
            return true;
        }
    }

    if (fAtlasRenderTasks.empty() ||
        !fAtlasRenderTasks.back()->addPath(viewMatrix, path, antialias, devIBounds->topLeft(),
                                           maxDimension, minDimension, *transposedInAtlas,
                                           locationInAtlas)) {
        // We either don't have an atlas yet or the current one is full. Try to replace it.
        GrAtlasRenderTask* currentAtlasTask = (!fAtlasRenderTasks.empty())
                ? fAtlasRenderTasks.back().get() : nullptr;
        if (currentAtlasTask) {
            // Don't allow the current atlas to be replaced if the draw already uses it. Otherwise
            // the draw would use two different atlases, which breaks our guarantee that there will
            // only ever be one atlas active at a time.
            const GrSurfaceProxy* currentAtlasProxy = currentAtlasTask->atlasProxy();
            bool drawUsesCurrentAtlas = false;
            visitProxiesUsedByDraw([currentAtlasProxy, &drawUsesCurrentAtlas](GrSurfaceProxy* proxy,
                                                                              GrMipmapped) {
                if (proxy == currentAtlasProxy) {
                    drawUsesCurrentAtlas = true;
                }
            });
            if (drawUsesCurrentAtlas) {
                // The draw already uses the current atlas. Give up.
                return false;
            }
        }
        // Replace the atlas with a new one.
        auto dynamicAtlas = std::make_unique<GrDynamicAtlas>(
                kAtlasAlpha8Type, GrDynamicAtlas::InternalMultisample::kYes,
                SkISize{fAtlasInitialSize, fAtlasInitialSize}, fAtlasMaxSize,
                *rContext->priv().caps(), kAtlasAlgorithm);
        auto newAtlasTask = sk_make_sp<GrAtlasRenderTask>(rContext, rContext->priv().auditTrail(),
                                                          sk_make_sp<GrArenas>(),
                                                          std::move(dynamicAtlas));
        rContext->priv().drawingManager()->addAtlasTask(newAtlasTask, currentAtlasTask);
        SkAssertResult(newAtlasTask->addPath(viewMatrix, path, antialias, devIBounds->topLeft(),
                                             maxDimension, minDimension, *transposedInAtlas,
                                             locationInAtlas));
        fAtlasRenderTasks.push_back(std::move(newAtlasTask));
        fAtlasPathCache.reset();
    }

    // Remember this path's location in the atlas, in case it gets drawn again.
    if (!path.isVolatile()) {
        fAtlasPathCache.set(atlasPathKey, *locationInAtlas);
    }
    return true;
}

#ifdef SK_DEBUG
// Ensures the atlas dependencies are set up such that each atlas will be totally out of service
// before we render the next one in line. This means there will only ever be one atlas active at a
// time and that they can all share the same texture.
void validate_atlas_dependencies(const SkTArray<sk_sp<GrAtlasRenderTask>>& atlasTasks) {
    for (int i = atlasTasks.count() - 1; i >= 1; --i) {
        GrAtlasRenderTask* atlasTask = atlasTasks[i].get();
        GrAtlasRenderTask* previousAtlasTask = atlasTasks[i - 1].get();
        // Double check that atlasTask depends on every dependent of its previous atlas. If this
        // fires it might mean previousAtlasTask gained a new dependent after atlasTask came into
        // service (maybe by an op that hadn't yet been added to an opsTask when we registered the
        // new atlas with the drawingManager).
        for (GrRenderTask* previousAtlasUser : previousAtlasTask->dependents()) {
            SkASSERT(atlasTask->dependsOn(previousAtlasUser));
        }
    }
}
#endif

void GrTessellationPathRenderer::preFlush(GrOnFlushResourceProvider* onFlushRP,
                                          SkSpan<const uint32_t> /* taskIDs */) {
    if (fAtlasRenderTasks.empty()) {
        SkASSERT(fAtlasPathCache.count() == 0);
        return;
    }

    // Verify the atlases can all share the same texture.
    SkDEBUGCODE(validate_atlas_dependencies(fAtlasRenderTasks);)

    // Instantiate the first atlas.
    fAtlasRenderTasks[0]->instantiate(onFlushRP);

    // Instantiate the remaining atlases.
    GrTexture* firstAtlasTexture = fAtlasRenderTasks[0]->atlasProxy()->peekTexture();
    SkASSERT(firstAtlasTexture);
    for (int i = 1; i < fAtlasRenderTasks.count(); ++i) {
        GrAtlasRenderTask* atlasTask = fAtlasRenderTasks[i].get();
        if (atlasTask->atlasProxy()->backingStoreDimensions() == firstAtlasTexture->dimensions()) {
            atlasTask->instantiate(onFlushRP, sk_ref_sp(firstAtlasTexture));
        } else {
            // The atlases are expected to all be full size except possibly the final one.
            SkASSERT(i == fAtlasRenderTasks.count() - 1);
            SkASSERT(atlasTask->atlasProxy()->backingStoreDimensions().area() <
                     firstAtlasTexture->dimensions().area());
            // TODO: Recycle the larger atlas texture anyway?
            atlasTask->instantiate(onFlushRP);
        }
    }

    // Reset all atlas data.
    fAtlasRenderTasks.reset();
    fAtlasPathCache.reset();
}

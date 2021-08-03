/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrAtlasPathRenderer.h"

#include "include/private/SkVx.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/effects/GrModulateAtlasCoverageEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrDrawAtlasPathOp.h"
#include "src/gpu/tessellate/GrAtlasRenderTask.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include "src/gpu/tessellate/shaders/GrTessellationShader.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

using grvx::float2;
using grvx::int2;

constexpr static auto kAtlasAlpha8Type = GrColorType::kAlpha_8;
constexpr static int kAtlasInitialSize = 512;

// The atlas is only used for small-area paths, which means at least one dimension of every path is
// guaranteed to be quite small. So if we transpose tall paths, then every path will have a small
// height, which lends very well to efficient pow2 atlas packing.
constexpr static auto kAtlasAlgorithm = GrDynamicAtlas::RectanizerAlgorithm::kPow2;

// Ensure every path in the atlas falls in or below the 256px high rectanizer band.
constexpr static int kAtlasMaxPathHeight = 256;

// If we have MSAA to fall back on, paths are already fast enough that we really only benefit from
// atlasing when they are very small.
constexpr static int kAtlasMaxPathHeightWithMSAAFallback = 128;

// http://skbug.com/12291 -- The way GrDynamicAtlas works, a single 2048x1 path is given an entire
// 2048x2048 atlas with draw bounds of 2048x1025. Limit the max width to 1024 to avoid this landmine
// until it's resolved.
constexpr static int kAtlasMaxPathWidth = 1024;

bool GrAtlasPathRenderer::IsSupported(GrRecordingContext* rContext) {
#ifdef SK_BUILD_FOR_IOS
    // b/195095846: There is a bug with the atlas path renderer on OpenGL iOS. Disable until we can
    // investigate.
    if (rContext->backend() == GrBackendApi::kOpenGL) {
        return false;
    }
#endif
    const GrCaps& caps = *rContext->priv().caps();
    auto atlasFormat = caps.getDefaultBackendFormat(kAtlasAlpha8Type, GrRenderable::kYes);
    return rContext->asDirectContext() &&  // The atlas doesn't support DDL yet.
           caps.internalMultisampleCount(atlasFormat) > 1 &&
           // GrAtlasRenderTask currently requires tessellation. In the future it could use the
           // default path renderer when tessellation isn't available.
           GrTessellationPathRenderer::IsSupported(caps);
}

sk_sp<GrAtlasPathRenderer> GrAtlasPathRenderer::Make(GrRecordingContext* rContext) {
    return IsSupported(rContext)
            ? sk_sp<GrAtlasPathRenderer>(new GrAtlasPathRenderer(rContext->asDirectContext()))
            : nullptr;
}

GrAtlasPathRenderer::GrAtlasPathRenderer(GrDirectContext* dContext) {
    SkASSERT(IsSupported(dContext));
    const GrCaps& caps = *dContext->priv().caps();
#if GR_TEST_UTILS
    fAtlasMaxSize = dContext->priv().options().fMaxTextureAtlasSize;
#else
    fAtlasMaxSize = 2048;
#endif
    fAtlasMaxSize = SkPrevPow2(std::min(fAtlasMaxSize, (float)caps.maxPreferredRenderTargetSize()));
    fAtlasMaxPathWidth = std::min((float)kAtlasMaxPathWidth, fAtlasMaxSize);
    fAtlasInitialSize = SkNextPow2(std::min(kAtlasInitialSize, (int)fAtlasMaxSize));
}

// Returns the rect [topLeftFloor, botRightCeil], which is the rect [r] rounded out to integer
// boundaries.
static std::tuple<float2,float2> round_out(const SkRect& r) {
    return {skvx::floor(float2::Load(&r.fLeft)), skvx::ceil(float2::Load(&r.fRight))};
}

bool GrAtlasPathRenderer::pathFitsInAtlas(const SkRect& pathDevBounds,
                                          GrAAType fallbackAAType) const {
    SkASSERT(fallbackAAType != GrAAType::kNone);  // The atlas doesn't support non-AA.
    float atlasMaxPathHeight_pow2 = (fallbackAAType == GrAAType::kMSAA)
            ? kAtlasMaxPathHeightWithMSAAFallback * kAtlasMaxPathHeightWithMSAAFallback
            : kAtlasMaxPathHeight * kAtlasMaxPathHeight;
    auto [topLeftFloor, botRightCeil] = round_out(pathDevBounds);
    float2 size = botRightCeil - topLeftFloor;
    return // Ensure the path's largest dimension fits in the atlas.
           skvx::all(size <= fAtlasMaxPathWidth) &&
           // Since we will transpose tall skinny paths, limiting to atlasMaxPathHeight^2 pixels
           // guarantees heightInAtlas <= atlasMaxPathHeight, while also allowing paths that are
           // very wide and short.
           size[0] * size[1] <= atlasMaxPathHeight_pow2;
}

void GrAtlasPathRenderer::AtlasPathKey::set(const SkMatrix& m, const SkPath& path) {
    using grvx::float2;
    fPathGenID = path.getGenerationID();
    fAffineMatrix[0] = m.getScaleX();
    fAffineMatrix[1] = m.getSkewX();
    fAffineMatrix[2] = m.getSkewY();
    fAffineMatrix[3] = m.getScaleY();
    float2 translate = {m.getTranslateX(), m.getTranslateY()};
    float2 subpixelPosition = translate - skvx::floor(translate);
    float2 subpixelPositionKey = skvx::trunc(subpixelPosition *
                                             GrTessellationShader::kLinearizationPrecision);
    skvx::cast<uint8_t>(subpixelPositionKey).store(fSubpixelPositionKey);
    fFillRule = (uint16_t)GrFillRuleForSkPath(path);  // Fill rule doesn't affect the path's genID.
}

bool GrAtlasPathRenderer::addPathToAtlas(GrRecordingContext* rContext,
                                         const SkMatrix& viewMatrix,
                                         const SkPath& path,
                                         const SkRect& pathDevBounds,
                                         SkIRect* devIBounds,
                                         SkIPoint16* locationInAtlas,
                                         bool* transposedInAtlas,
                                         const DrawRefsAtlasCallback& drawRefsAtlasCallback) {
    SkASSERT(!viewMatrix.hasPerspective());  // See onCanDrawPath().

    pathDevBounds.roundOut(devIBounds);
#ifdef SK_DEBUG
    // is_visible() should have guaranteed the path's bounds were representable as ints, since clip
    // bounds within the max render target size are nowhere near INT_MAX.
    auto [topLeftFloor, botRightCeil] = round_out(pathDevBounds);
    SkASSERT(skvx::all(skvx::cast<float>(int2::Load(&devIBounds->fLeft)) == topLeftFloor));
    SkASSERT(skvx::all(skvx::cast<float>(int2::Load(&devIBounds->fRight)) == botRightCeil));
#endif

    int widthInAtlas = devIBounds->width();
    int heightInAtlas = devIBounds->height();
    // is_visible() should have guaranteed the path's bounds were non-empty.
    SkASSERT(widthInAtlas > 0 && heightInAtlas > 0);

    if (SkNextPow2(widthInAtlas) == SkNextPow2(heightInAtlas)) {
        // Both dimensions go to the same pow2 band in the atlas. Use the larger dimension as height
        // for more efficient packing.
        *transposedInAtlas = widthInAtlas > heightInAtlas;
    } else {
        // Both dimensions go to different pow2 bands in the atlas. Use the smaller pow2 band for
        // most efficient packing.
        *transposedInAtlas = heightInAtlas > widthInAtlas;
    }
    if (*transposedInAtlas) {
        std::swap(heightInAtlas, widthInAtlas);
    }
    // pathFitsInAtlas() should have guaranteed these constraints on the path size.
    SkASSERT(widthInAtlas <= (int)fAtlasMaxPathWidth);
    SkASSERT(heightInAtlas <= kAtlasMaxPathHeight);

    // Check if this path is already in the atlas. This is mainly for clip paths.
    AtlasPathKey atlasPathKey;
    if (!path.isVolatile()) {
        atlasPathKey.set(viewMatrix, path);
        if (const SkIPoint16* existingLocation = fAtlasPathCache.find(atlasPathKey)) {
            *locationInAtlas = *existingLocation;
            return true;
        }
    }

    if (fAtlasRenderTasks.empty() ||
        !fAtlasRenderTasks.back()->addPath(viewMatrix, path, devIBounds->topLeft(), widthInAtlas,
                                           heightInAtlas, *transposedInAtlas, locationInAtlas)) {
        // We either don't have an atlas yet or the current one is full. Try to replace it.
        GrAtlasRenderTask* currentAtlasTask = (!fAtlasRenderTasks.empty())
                ? fAtlasRenderTasks.back().get() : nullptr;
        if (currentAtlasTask &&
            drawRefsAtlasCallback &&
            drawRefsAtlasCallback(currentAtlasTask->atlasProxy())) {
            // The draw already refs the current atlas. Give up. Otherwise the draw would ref two
            // different atlases and they couldn't share a texture.
            return false;
        }
        // Replace the atlas with a new one.
        auto dynamicAtlas = std::make_unique<GrDynamicAtlas>(
                kAtlasAlpha8Type, GrDynamicAtlas::InternalMultisample::kYes,
                SkISize{fAtlasInitialSize, fAtlasInitialSize}, fAtlasMaxSize,
                *rContext->priv().caps(), kAtlasAlgorithm);
        auto newAtlasTask = sk_make_sp<GrAtlasRenderTask>(rContext,
                                                          sk_make_sp<GrArenas>(),
                                                          std::move(dynamicAtlas));
        rContext->priv().drawingManager()->addAtlasTask(newAtlasTask, currentAtlasTask);
        SkAssertResult(newAtlasTask->addPath(viewMatrix, path, devIBounds->topLeft(), widthInAtlas,
                                             heightInAtlas, *transposedInAtlas, locationInAtlas));
        fAtlasRenderTasks.push_back(std::move(newAtlasTask));
        fAtlasPathCache.reset();
    }

    // Remember this path's location in the atlas, in case it gets drawn again.
    if (!path.isVolatile()) {
        fAtlasPathCache.set(atlasPathKey, *locationInAtlas);
    }
    return true;
}

// Returns whether the given proxyOwner uses the atlasProxy.
template<typename T> bool refs_atlas(const T* proxyOwner, const GrSurfaceProxy* atlasProxy) {
    bool refsAtlas = false;
    auto checkForAtlasRef = [atlasProxy, &refsAtlas](GrSurfaceProxy* proxy, GrMipmapped) {
        if (proxy == atlasProxy) {
            refsAtlas = true;
        }
    };
    if (proxyOwner) {
        proxyOwner->visitProxies(checkForAtlasRef);
    }
    return refsAtlas;
}

GrPathRenderer::CanDrawPath GrAtlasPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
#ifdef SK_DEBUG
    if (!fAtlasRenderTasks.empty()) {
        // args.fPaint should NEVER reference our current atlas. If it does, it means somebody
        // intercepted a clip FP meant for a different op and will cause rendering artifacts.
        const GrSurfaceProxy* atlasProxy = fAtlasRenderTasks.back()->atlasProxy();
        SkASSERT(!refs_atlas(args.fPaint->getColorFragmentProcessor(), atlasProxy));
        SkASSERT(!refs_atlas(args.fPaint->getCoverageFragmentProcessor(), atlasProxy));
    }
    SkASSERT(!args.fHasUserStencilSettings);  // See onGetStencilSupport().
#endif
    bool canDrawPath = args.fShape->style().isSimpleFill() &&
#ifdef SK_DISABLE_ATLAS_PATH_RENDERER_WITH_COVERAGE_AA
                       // The MSAA requirement is a temporary limitation in order to preserve
                       // functionality for refactoring. TODO: Allow kCoverage AA types.
                       args.fAAType == GrAAType::kMSAA &&
#else
                       args.fAAType != GrAAType::kNone &&
#endif
                       !args.fShape->style().hasPathEffect() &&
                       !args.fViewMatrix->hasPerspective() &&
                       this->pathFitsInAtlas(args.fViewMatrix->mapRect(args.fShape->bounds()),
                                             args.fAAType);
    return canDrawPath ? CanDrawPath::kYes : CanDrawPath::kNo;
}

static bool is_visible(const SkRect& pathDevBounds, const SkIRect& clipBounds) {
    float2 pathTopLeft = float2::Load(&pathDevBounds.fLeft);
    float2 pathBotRight = float2::Load(&pathDevBounds.fRight);
    // Empty paths are never visible. Phrase this as a NOT of positive logic so we also return false
    // in the case of NaN.
    if (!skvx::all(pathTopLeft < pathBotRight)) {
        return false;
    }
    float2 clipTopLeft = skvx::cast<float>(int2::Load(&clipBounds.fLeft));
    float2 clipBotRight = skvx::cast<float>(int2::Load(&clipBounds.fRight));
    static_assert(sizeof(clipBounds) == sizeof(clipTopLeft) + sizeof(clipBotRight));
    return skvx::all(pathTopLeft < clipBotRight) && skvx::all(pathBotRight > clipTopLeft);
}

bool GrAtlasPathRenderer::onDrawPath(const DrawPathArgs& args) {
    SkPath path;
    args.fShape->asPath(&path);

    const SkRect pathDevBounds = args.fViewMatrix->mapRect(args.fShape->bounds());
    SkASSERT(this->pathFitsInAtlas(pathDevBounds, args.fAAType));

    if (!is_visible(pathDevBounds, args.fClip->getConservativeBounds())) {
        // The path is empty or outside the clip. No mask is needed.
        if (path.isInverseFillType()) {
            args.fSurfaceDrawContext->drawPaint(args.fClip, std::move(args.fPaint),
                                                *args.fViewMatrix);
        }
        return true;
    }

    SkIRect devIBounds;
    SkIPoint16 locationInAtlas;
    bool transposedInAtlas;
    SkAssertResult(this->addPathToAtlas(args.fContext, *args.fViewMatrix, path, pathDevBounds,
                                        &devIBounds, &locationInAtlas, &transposedInAtlas,
                                        nullptr/*DrawRefsAtlasCallback -- see onCanDrawPath()*/));

    const SkIRect& fillBounds = args.fShape->inverseFilled()
            ? (args.fClip
                    ? args.fClip->getConservativeBounds()
                    : args.fSurfaceDrawContext->asSurfaceProxy()->backingStoreBoundsIRect())
            : devIBounds;
    const GrCaps& caps = *args.fSurfaceDrawContext->caps();
    auto op = GrOp::Make<GrDrawAtlasPathOp>(args.fContext,
                                            args.fSurfaceDrawContext->arenaAlloc(),
                                            fillBounds, *args.fViewMatrix,
                                            std::move(args.fPaint), locationInAtlas,
                                            devIBounds, transposedInAtlas,
                                            fAtlasRenderTasks.back()->readView(caps),
                                            args.fShape->inverseFilled());
    args.fSurfaceDrawContext->addDrawOp(args.fClip, std::move(op));
    return true;
}

GrFPResult GrAtlasPathRenderer::makeAtlasClipEffect(const skgpu::v1::SurfaceDrawContext* sdc,
                                                    const GrOp* opBeingClipped,
                                                    std::unique_ptr<GrFragmentProcessor> inputFP,
                                                    const SkIRect& drawBounds,
                                                    const SkMatrix& viewMatrix,
                                                    const SkPath& path) {
    if (viewMatrix.hasPerspective()) {
        return GrFPFailure(std::move(inputFP));
    }

    const SkRect pathDevBounds = viewMatrix.mapRect(path.getBounds());
    if (!is_visible(pathDevBounds, drawBounds)) {
        // The path is empty or outside the drawBounds. No mask is needed.
        return path.isInverseFillType() ? GrFPSuccess(std::move(inputFP))
                                        : GrFPFailure(std::move(inputFP));
    }

    auto fallbackAAType = (sdc->numSamples() > 1 || sdc->canUseDynamicMSAA()) ? GrAAType::kMSAA
                                                                              : GrAAType::kCoverage;
    if (!this->pathFitsInAtlas(pathDevBounds, fallbackAAType)) {
        // The path is too big.
        return GrFPFailure(std::move(inputFP));
    }

    SkIRect devIBounds;
    SkIPoint16 locationInAtlas;
    bool transposedInAtlas;
    // Called if the atlas runs out of room, to determine if it's safe to create a new one. (Draws
    // can never access more than one atlas.)
    auto drawRefsAtlasCallback = [opBeingClipped, &inputFP](const GrSurfaceProxy* atlasProxy) {
        return refs_atlas(opBeingClipped, atlasProxy) ||
               refs_atlas(inputFP.get(), atlasProxy);
    };
    // addPathToAtlas() ignores inverseness of the fill. See GrAtlasRenderTask::getAtlasUberPath().
    if (!this->addPathToAtlas(sdc->recordingContext(), viewMatrix, path, pathDevBounds, &devIBounds,
                              &locationInAtlas, &transposedInAtlas, drawRefsAtlasCallback)) {
        // The atlas ran out of room and we were unable to start a new one.
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
    auto flags = GrModulateAtlasCoverageEffect::Flags::kNone;
    if (path.isInverseFillType()) {
        flags |= GrModulateAtlasCoverageEffect::Flags::kInvertCoverage;
    }
    if (!devIBounds.contains(drawBounds)) {
        flags |= GrModulateAtlasCoverageEffect::Flags::kCheckBounds;
        // At this point in time we expect callers to tighten the scissor for "kIntersect" clips, as
        // opposed to us having to check the path bounds. Feel free to remove this assert if that
        // ever changes.
        SkASSERT(path.isInverseFillType());
    }
    GrSurfaceProxyView atlasView = fAtlasRenderTasks.back()->readView(*sdc->caps());
    return GrFPSuccess(std::make_unique<GrModulateAtlasCoverageEffect>(flags, std::move(inputFP),
                                                                       std::move(atlasView),
                                                                       atlasMatrix, devIBounds));
}

#ifdef SK_DEBUG
// Ensures the atlas dependencies are set up such that each atlas will be totally out of service
// before we render the next one in line. This means there will only ever be one atlas active at a
// time and that they can all share the same texture.
static void validate_atlas_dependencies(const SkTArray<sk_sp<GrAtlasRenderTask>>& atlasTasks) {
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

void GrAtlasPathRenderer::preFlush(GrOnFlushResourceProvider* onFlushRP,
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

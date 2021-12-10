/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/v1/SurfaceDrawContext_v1.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkVertices.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/GrImageContext.h"
#include "include/private/SkShadowFlags.h"
#include "include/private/SkVx.h"
#include "include/utils/SkShadowUtils.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkDrawProcs.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrAttachment.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/effects/GrDisableColorXP.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/ClearOp.h"
#include "src/gpu/ops/DrawAtlasOp.h"
#include "src/gpu/ops/DrawVerticesOp.h"
#include "src/gpu/ops/DrawableOp.h"
#include "src/gpu/ops/FillRRectOp.h"
#include "src/gpu/ops/FillRectOp.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrOp.h"
#include "src/gpu/ops/GrOvalOpFactory.h"
#include "src/gpu/ops/LatticeOp.h"
#include "src/gpu/ops/RegionOp.h"
#include "src/gpu/ops/ShadowRRectOp.h"
#include "src/gpu/ops/StrokeRectOp.h"
#include "src/gpu/ops/TextureOp.h"
#include "src/gpu/text/GrSDFTControl.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/gpu/v1/PathRenderer.h"

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this->drawingManager()->getContext())
#define ASSERT_SINGLE_OWNER        GR_ASSERT_SINGLE_OWNER(this->singleOwner())
#define RETURN_IF_ABANDONED        if (fContext->abandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED  if (fContext->abandoned()) { return false; }

//////////////////////////////////////////////////////////////////////////////

namespace {

void op_bounds(SkRect* bounds, const GrOp* op) {
    *bounds = op->bounds();
    if (op->hasZeroArea()) {
        if (op->hasAABloat()) {
            bounds->outset(0.5f, 0.5f);
        } else {
            // We don't know which way the particular GPU will snap lines or points at integer
            // coords. So we ensure that the bounds is large enough for either snap.
            SkRect before = *bounds;
            bounds->roundOut(bounds);
            if (bounds->fLeft == before.fLeft) {
                bounds->fLeft -= 1;
            }
            if (bounds->fTop == before.fTop) {
                bounds->fTop -= 1;
            }
            if (bounds->fRight == before.fRight) {
                bounds->fRight += 1;
            }
            if (bounds->fBottom == before.fBottom) {
                bounds->fBottom += 1;
            }
        }
    }
}

} // anonymous namespace

namespace skgpu::v1 {

using DoSimplify = GrStyledShape::DoSimplify;

class AutoCheckFlush {
public:
    AutoCheckFlush(GrDrawingManager* drawingManager) : fDrawingManager(drawingManager) {
        SkASSERT(fDrawingManager);
    }
    ~AutoCheckFlush() { fDrawingManager->flushIfNecessary(); }

private:
    GrDrawingManager* fDrawingManager;
};

std::unique_ptr<SurfaceDrawContext> SurfaceDrawContext::Make(GrRecordingContext* rContext,
                                                             GrColorType colorType,
                                                             sk_sp<GrSurfaceProxy> proxy,
                                                             sk_sp<SkColorSpace> colorSpace,
                                                             GrSurfaceOrigin origin,
                                                             const SkSurfaceProps& surfaceProps,
                                                             bool flushTimeOpsTask) {
    if (!rContext || !proxy || colorType == GrColorType::kUnknown) {
        return nullptr;
    }

    const GrBackendFormat& format = proxy->backendFormat();
    GrSwizzle readSwizzle = rContext->priv().caps()->getReadSwizzle(format, colorType);
    GrSwizzle writeSwizzle = rContext->priv().caps()->getWriteSwizzle(format, colorType);

    GrSurfaceProxyView readView (          proxy,  origin, readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    return std::make_unique<SurfaceDrawContext>(rContext,
                                                std::move(readView),
                                                std::move(writeView),
                                                colorType,
                                                std::move(colorSpace),
                                                surfaceProps,
                                                flushTimeOpsTask);
}

std::unique_ptr<SurfaceDrawContext> SurfaceDrawContext::Make(
        GrRecordingContext* rContext,
        sk_sp<SkColorSpace> colorSpace,
        SkBackingFit fit,
        SkISize dimensions,
        const GrBackendFormat& format,
        int sampleCnt,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        GrSwizzle readSwizzle,
        GrSwizzle writeSwizzle,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted,
        const SkSurfaceProps& surfaceProps) {
    // It is probably not necessary to check if the context is abandoned here since uses of the
    // SurfaceDrawContext which need the context will mostly likely fail later on without an
    // issue. However having this hear adds some reassurance in case there is a path doesn't handle
    // an abandoned context correctly. It also lets us early out of some extra work.
    if (rContext->abandoned()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = rContext->priv().proxyProvider()->createProxy(
            format,
            dimensions,
            GrRenderable::kYes,
            sampleCnt,
            mipMapped,
            fit,
            budgeted,
            isProtected);
    if (!proxy) {
        return nullptr;
    }

    GrSurfaceProxyView readView (           proxy, origin,  readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    auto sdc = std::make_unique<SurfaceDrawContext>(rContext,
                                                    std::move(readView),
                                                    std::move(writeView),
                                                    GrColorType::kUnknown,
                                                    std::move(colorSpace),
                                                    surfaceProps);
    sdc->discard();
    return sdc;
}

std::unique_ptr<SurfaceDrawContext> SurfaceDrawContext::Make(
        GrRecordingContext* rContext,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        SkBackingFit fit,
        SkISize dimensions,
        const SkSurfaceProps& surfaceProps,
        int sampleCnt,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted) {
    if (!rContext) {
        return nullptr;
    }

    auto format = rContext->priv().caps()->getDefaultBackendFormat(colorType, GrRenderable::kYes);
    if (!format.isValid()) {
        return nullptr;
    }
    sk_sp<GrTextureProxy> proxy = rContext->priv().proxyProvider()->createProxy(format,
                                                                                dimensions,
                                                                                GrRenderable::kYes,
                                                                                sampleCnt,
                                                                                mipMapped,
                                                                                fit,
                                                                                budgeted,
                                                                                isProtected);
    if (!proxy) {
        return nullptr;
    }

    return SurfaceDrawContext::Make(rContext,
                                    colorType,
                                    std::move(proxy),
                                    std::move(colorSpace),
                                    origin,
                                    surfaceProps);
}

std::unique_ptr<SurfaceDrawContext> SurfaceDrawContext::MakeWithFallback(
        GrRecordingContext* rContext,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        SkBackingFit fit,
        SkISize dimensions,
        const SkSurfaceProps& surfaceProps,
        int sampleCnt,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted) {
    const GrCaps* caps = rContext->priv().caps();
    auto [ct, _] = caps->getFallbackColorTypeAndFormat(colorType, sampleCnt);
    if (ct == GrColorType::kUnknown) {
        return nullptr;
    }
    return SurfaceDrawContext::Make(rContext, ct, colorSpace, fit, dimensions, surfaceProps,
                                    sampleCnt, mipMapped, isProtected, origin, budgeted);
}

std::unique_ptr<SurfaceDrawContext> SurfaceDrawContext::MakeFromBackendTexture(
        GrRecordingContext* rContext,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        const GrBackendTexture& tex,
        int sampleCnt,
        GrSurfaceOrigin origin,
        const SkSurfaceProps& surfaceProps,
        sk_sp<GrRefCntedCallback> releaseHelper) {
    SkASSERT(sampleCnt > 0);
    sk_sp<GrTextureProxy> proxy(rContext->priv().proxyProvider()->wrapRenderableBackendTexture(
            tex, sampleCnt, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
            std::move(releaseHelper)));
    if (!proxy) {
        return nullptr;
    }

    return SurfaceDrawContext::Make(rContext, colorType, std::move(proxy), std::move(colorSpace),
                                    origin, surfaceProps);
}

// In MDB mode the reffing of the 'getLastOpsTask' call's result allows in-progress
// OpsTask to be picked up and added to by SurfaceDrawContexts lower in the call
// stack. When this occurs with a closed OpsTask, a new one will be allocated
// when the surfaceDrawContext attempts to use it (via getOpsTask).
SurfaceDrawContext::SurfaceDrawContext(GrRecordingContext* rContext,
                                       GrSurfaceProxyView readView,
                                       GrSurfaceProxyView writeView,
                                       GrColorType colorType,
                                       sk_sp<SkColorSpace> colorSpace,
                                       const SkSurfaceProps& surfaceProps,
                                       bool flushTimeOpsTask)
        : SurfaceFillContext(rContext,
                             std::move(readView),
                             std::move(writeView),
                             {colorType, kPremul_SkAlphaType, std::move(colorSpace)},
                             flushTimeOpsTask)
        , fSurfaceProps(surfaceProps)
        , fCanUseDynamicMSAA(
                (fSurfaceProps.flags() & SkSurfaceProps::kDynamicMSAA_Flag) &&
                rContext->priv().caps()->supportsDynamicMSAA(this->asRenderTargetProxy()))
        , fGlyphPainter(*this) {
    SkDEBUGCODE(this->validate();)
}

SurfaceDrawContext::~SurfaceDrawContext() {
    ASSERT_SINGLE_OWNER
}

void SurfaceDrawContext::willReplaceOpsTask(OpsTask* prevTask, OpsTask* nextTask) {
    if (prevTask && fNeedsStencil) {
        // Store the stencil values in memory upon completion of fOpsTask.
        prevTask->setMustPreserveStencil();
        // Reload the stencil buffer content at the beginning of newOpsTask.
        // FIXME: Could the topo sort insert a task between these two that modifies the stencil
        // values?
        nextTask->setInitialStencilContent(OpsTask::StencilContent::kPreserved);
    }
#if GR_GPU_STATS && GR_TEST_UTILS
    if (fCanUseDynamicMSAA) {
        fContext->priv().dmsaaStats().fNumRenderPasses++;
    }
#endif
}

void SurfaceDrawContext::drawGlyphRunListNoCache(const GrClip* clip,
                                                 const SkMatrixProvider& viewMatrix,
                                                 const SkGlyphRunList& glyphRunList,
                                                 const SkPaint& paint) {
    GrSDFTControl control =
            fContext->priv().getSDFTControl(fSurfaceProps.isUseDeviceIndependentFonts());
    const SkPoint drawOrigin = glyphRunList.origin();
    SkMatrix drawMatrix = viewMatrix.localToDevice();
    drawMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
    GrSubRunAllocator* const alloc = this->subRunAlloc();

    GrSubRunNoCachePainter painter{this, alloc, clip, viewMatrix, glyphRunList, paint};
    for (auto& glyphRun : glyphRunList) {
        // Make and add the text ops.
        fGlyphPainter.processGlyphRun(glyphRun,
                                      drawMatrix,
                                      paint,
                                      control,
                                      &painter);
    }
}

// choose to use the GrTextBlob cache or not.
bool gGrDrawTextNoCache = false;
void SurfaceDrawContext::drawGlyphRunList(const GrClip* clip,
                                          const SkMatrixProvider& viewMatrix,
                                          const SkGlyphRunList& glyphRunList,
                                          const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawGlyphRunList", fContext);

    // Drawing text can cause us to do inline uploads. This is not supported for wrapped vulkan
    // secondary command buffers because it would require stopping and starting a render pass which
    // we don't have access to.
    if (this->wrapsVkSecondaryCB()) {
        return;
    }

    if (gGrDrawTextNoCache || glyphRunList.blob() == nullptr) {
        // If the glyphRunList does not have an associated text blob, then it was created by one of
        // the direct draw APIs (drawGlyphs, etc.). There is no need to create a GrTextBlob just
        // build the sub run directly and place it in the op.
        this->drawGlyphRunListNoCache(clip, viewMatrix, glyphRunList, paint);
    } else {
        GrTextBlobCache* textBlobCache = fContext->priv().getTextBlobCache();
        textBlobCache->drawGlyphRunList(clip, viewMatrix, glyphRunList, paint, this);
    }
}

void SurfaceDrawContext::drawPaint(const GrClip* clip,
                                   GrPaint&& paint,
                                   const SkMatrix& viewMatrix) {
    // Start with the render target, since that is the maximum content we could possibly fill.
    // drawFilledQuad() will automatically restrict it to clip bounds for us if possible.
    if (!paint.numTotalFragmentProcessors()) {
        // The paint is trivial so we won't need to use local coordinates, so skip calculating the
        // inverse view matrix.
        SkRect r = this->asSurfaceProxy()->getBoundsRect();
        this->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(), r, r);
    } else {
        // Use the inverse view matrix to arrive at appropriate local coordinates for the paint.
        SkMatrix localMatrix;
        if (!viewMatrix.invert(&localMatrix)) {
            return;
        }
        SkIRect bounds = SkIRect::MakeSize(this->asSurfaceProxy()->dimensions());
        this->fillPixelsWithLocalMatrix(clip, std::move(paint), bounds, localMatrix);
    }
}

enum class SurfaceDrawContext::QuadOptimization {
    // The rect to draw doesn't intersect clip or render target, so no draw op should be added
    kDiscarded,
    // The rect to draw was converted to some other op and appended to the oplist, so no additional
    // op is necessary. Currently this can convert it to a clear op or a rrect op. Only valid if
    // a constColor is provided.
    kSubmitted,
    // The clip was folded into the device quad, with updated edge flags and local coords, and
    // caller is responsible for adding an appropriate op.
    kClipApplied,
    // No change to clip, but quad updated to better fit clip/render target, and caller is
    // responsible for adding an appropriate op.
    kCropped
};

SurfaceDrawContext::QuadOptimization SurfaceDrawContext::attemptQuadOptimization(
        const GrClip* clip, const GrUserStencilSettings* stencilSettings, GrAA* aa, DrawQuad* quad,
        GrPaint* paint) {
    // Optimization requirements:
    // 1. kDiscard applies when clip bounds and quad bounds do not intersect
    // 2a. kSubmitted applies when constColor and final geom is pixel aligned rect;
    //       pixel aligned rect requires rect clip and (rect quad or quad covers clip) OR
    // 2b. kSubmitted applies when constColor and rrect clip and quad covers clip
    // 4. kClipApplied applies when rect clip and (rect quad or quad covers clip)
    // 5. kCropped in all other scenarios (although a crop may be a no-op)
    const SkPMColor4f* constColor = nullptr;
    SkPMColor4f paintColor;
    if (!stencilSettings && paint && !paint->hasCoverageFragmentProcessor() &&
        paint->isConstantBlendedColor(&paintColor)) {
        // Only consider clears/rrects when it's easy to guarantee 100% fill with single color
        constColor = &paintColor;
    }

    // Save the old AA flags since CropToRect will modify 'quad' and if kCropped is returned, it's
    // better to just keep the old flags instead of introducing mixed edge flags.
    GrQuadAAFlags oldFlags = quad->fEdgeFlags;

    // Use the logical size of the render target, which allows for "fullscreen" clears even if
    // the render target has an approximate backing fit
    SkRect rtRect = this->asSurfaceProxy()->getBoundsRect();

    // For historical reasons, we assume AA for exact bounds checking in IsOutsideClip.
    // TODO(michaelludwig) - Hopefully that can be revisited when the clipping optimizations are
    // refactored to work better with round rects and dmsaa.
    SkRect drawBounds = quad->fDevice.bounds();
    if (!quad->fDevice.isFinite() || drawBounds.isEmpty() ||
        GrClip::IsOutsideClip(SkIRect::MakeSize(this->dimensions()), drawBounds, GrAA::kYes)) {
        return QuadOptimization::kDiscarded;
    } else if (GrQuadUtils::WillUseHairline(quad->fDevice, GrAAType::kCoverage, quad->fEdgeFlags)) {
        // Don't try to apply the clip early if we know rendering will use hairline methods, as this
        // has an effect on the op bounds not otherwise taken into account in this function.
        return QuadOptimization::kCropped;
    }

    auto conservativeCrop = [&]() {
        static constexpr int kLargeDrawLimit = 15000;
        // Crop the quad to the render target. This doesn't change the visual results of drawing but
        // is meant to help numerical stability for excessively large draws.
        if (drawBounds.width() > kLargeDrawLimit || drawBounds.height() > kLargeDrawLimit) {
            GrQuadUtils::CropToRect(rtRect, *aa, quad, /* compute local */ !constColor);
        }
    };

    bool simpleColor = !stencilSettings && constColor;
    GrClip::PreClipResult result = clip ? clip->preApply(drawBounds, *aa)
                                        : GrClip::PreClipResult(GrClip::Effect::kUnclipped);
    switch(result.fEffect) {
        case GrClip::Effect::kClippedOut:
            return QuadOptimization::kDiscarded;
        case GrClip::Effect::kUnclipped:
            if (!simpleColor) {
                conservativeCrop();
                return QuadOptimization::kClipApplied;
            } else {
                // Update result to store the render target bounds in order and then fall
                // through to attempt the draw->native clear optimization
                result = GrClip::PreClipResult(SkRRect::MakeRect(rtRect), *aa);
            }
            break;
        case GrClip::Effect::kClipped:
            if (!result.fIsRRect || (stencilSettings && result.fAA != *aa) ||
                (!result.fRRect.isRect() && !simpleColor)) {
                // The clip and draw state are too complicated to try and reduce
                conservativeCrop();
                return QuadOptimization::kCropped;
            } // Else fall through to attempt to combine the draw and clip geometry together
            break;
        default:
            SkUNREACHABLE;
    }

    // If we reached here, we know we're an axis-aligned clip that is either a rect or a round rect,
    // so we can potentially combine it with the draw geometry so that no clipping is needed.
    SkASSERT(result.fEffect == GrClip::Effect::kClipped && result.fIsRRect);
    SkRect clippedBounds = result.fRRect.getBounds();
    clippedBounds.intersect(rtRect);
    if (!drawBounds.intersect(clippedBounds)) {
        // Our fractional bounds aren't actually inside the clip. GrClip::preApply() can sometimes
        // think in terms of rounded-out bounds. Discard the draw.
        return QuadOptimization::kDiscarded;
    }
    // Guard against the clipped draw turning into a hairline draw after intersection
    if (drawBounds.width() < 1.f || drawBounds.height() < 1.f) {
        return QuadOptimization::kCropped;
    }

    if (result.fRRect.isRect()) {
        // No rounded corners, so we might be able to become a native clear or we might be able to
        // modify geometry and edge flags to represent intersected shape of clip and draw.
        if (GrQuadUtils::CropToRect(clippedBounds, result.fAA, quad,
                                    /*compute local*/ !constColor)) {
            if (simpleColor && quad->fDevice.quadType() == GrQuad::Type::kAxisAligned) {
                // Clear optimization is possible
                drawBounds = quad->fDevice.bounds();
                if (drawBounds.contains(rtRect)) {
                    // Fullscreen clear
                    this->clear(*constColor);
                    return QuadOptimization::kSubmitted;
                } else if (GrClip::IsPixelAligned(drawBounds) &&
                           drawBounds.width() > 256 && drawBounds.height() > 256) {
                    // Scissor + clear (round shouldn't do anything since we are pixel aligned)
                    SkIRect scissorRect;
                    drawBounds.round(&scissorRect);
                    this->clear(scissorRect, *constColor);
                    return QuadOptimization::kSubmitted;
                }
            }

            // else the draw and clip were combined so just update the AA to reflect combination
            if (*aa == GrAA::kNo && result.fAA == GrAA::kYes &&
                quad->fEdgeFlags != GrQuadAAFlags::kNone) {
                // The clip was anti-aliased and now the draw needs to be upgraded to AA to
                // properly reflect the smooth edge of the clip.
                *aa = GrAA::kYes;
            }
            // We intentionally do not downgrade AA here because we don't know if we need to
            // preserve MSAA (see GrQuadAAFlags docs). But later in the pipeline, the ops can
            // use GrResolveAATypeForQuad() to turn off coverage AA when all flags are off.
            // deviceQuad is exactly the intersection of original quad and clip, so it can be
            // drawn with no clip (submitted by caller)
            return QuadOptimization::kClipApplied;
        }
    } else {
        // Rounded corners and constant filled color (limit ourselves to solid colors because
        // there is no way to use custom local coordinates with drawRRect).
        SkASSERT(simpleColor);
        if (GrQuadUtils::CropToRect(clippedBounds, result.fAA, quad,
                                    /* compute local */ false) &&
            quad->fDevice.quadType() == GrQuad::Type::kAxisAligned &&
            quad->fDevice.bounds().contains(clippedBounds)) {
            // Since the cropped quad became a rectangle which covered the bounds of the rrect,
            // we can draw the rrect directly and ignore the edge flags
            this->drawRRect(nullptr, std::move(*paint), result.fAA, SkMatrix::I(), result.fRRect,
                            GrStyle::SimpleFill());
            return QuadOptimization::kSubmitted;
        }
    }

    // The quads have been updated to better fit the clip bounds, but can't get rid of
    // the clip entirely
    quad->fEdgeFlags = oldFlags;
    return QuadOptimization::kCropped;
}

void SurfaceDrawContext::drawFilledQuad(const GrClip* clip,
                                        GrPaint&& paint,
                                        GrAA aa,
                                        DrawQuad* quad,
                                        const GrUserStencilSettings* ss) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawFilledQuad", fContext);

    AutoCheckFlush acf(this->drawingManager());

    QuadOptimization opt = this->attemptQuadOptimization(clip, ss, &aa, quad, &paint);
    if (opt >= QuadOptimization::kClipApplied) {
        // These optimizations require caller to add an op themselves
        const GrClip* finalClip = opt == QuadOptimization::kClipApplied ? nullptr : clip;
        GrAAType aaType;
        if (ss) {
            aaType = (aa == GrAA::kYes) ? GrAAType::kMSAA : GrAAType::kNone;
        } else if (fCanUseDynamicMSAA && aa == GrAA::kNo) {
            // The SkGpuDevice ensures GrAA is always kYes when using dmsaa. If the caller calls
            // into here with GrAA::kNo, trust that they know what they're doing and that the
            // rendering will be equal with or without msaa.
            aaType = GrAAType::kNone;
        } else {
            aaType = this->chooseAAType(aa);
        }
        this->addDrawOp(finalClip, FillRectOp::Make(fContext, std::move(paint), aaType,
                                                    quad, ss));
    }
    // All other optimization levels were completely handled inside attempt(), so no extra op needed
}

void SurfaceDrawContext::drawTexture(const GrClip* clip,
                                     GrSurfaceProxyView view,
                                     SkAlphaType srcAlphaType,
                                     GrSamplerState::Filter filter,
                                     GrSamplerState::MipmapMode mm,
                                     SkBlendMode blendMode,
                                     const SkPMColor4f& color,
                                     const SkRect& srcRect,
                                     const SkRect& dstRect,
                                     GrAA aa,
                                     GrQuadAAFlags edgeAA,
                                     SkCanvas::SrcRectConstraint constraint,
                                     const SkMatrix& viewMatrix,
                                     sk_sp<GrColorSpaceXform> colorSpaceXform) {
    // If we are using dmsaa then go through FillRRectOp (via fillRectToRect).
    if ((this->alwaysAntialias() || this->caps()->reducedShaderMode()) && aa == GrAA::kYes) {
        GrPaint paint;
        paint.setColor4f(color);
        std::unique_ptr<GrFragmentProcessor> fp;
        if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
            fp = GrTextureEffect::MakeSubset(view, srcAlphaType, SkMatrix::I(),
                                             GrSamplerState(filter, mm), srcRect,
                                             *this->caps());
        } else {
            fp = GrTextureEffect::Make(view, srcAlphaType, SkMatrix::I(), filter, mm);
        }
        if (colorSpaceXform) {
            fp = GrColorSpaceXformEffect::Make(std::move(fp), std::move(colorSpaceXform));
        }
        fp = GrBlendFragmentProcessor::Make(std::move(fp), nullptr, SkBlendMode::kModulate);
        paint.setColorFragmentProcessor(std::move(fp));
        if (blendMode != SkBlendMode::kSrcOver) {
            paint.setXPFactory(SkBlendMode_AsXPFactory(blendMode));
        }
        this->fillRectToRect(clip, std::move(paint), GrAA::kYes, viewMatrix, dstRect, srcRect);
        return;
    }

    const SkRect* subset = constraint == SkCanvas::kStrict_SrcRectConstraint ?
            &srcRect : nullptr;
    DrawQuad quad{GrQuad::MakeFromRect(dstRect, viewMatrix), GrQuad(srcRect), edgeAA};

    this->drawTexturedQuad(clip, std::move(view), srcAlphaType, std::move(colorSpaceXform), filter,
                           mm, color, blendMode, aa, &quad, subset);
}

void SurfaceDrawContext::drawTexturedQuad(const GrClip* clip,
                                          GrSurfaceProxyView proxyView,
                                          SkAlphaType srcAlphaType,
                                          sk_sp<GrColorSpaceXform> textureXform,
                                          GrSamplerState::Filter filter,
                                          GrSamplerState::MipmapMode mm,
                                          const SkPMColor4f& color,
                                          SkBlendMode blendMode,
                                          GrAA aa,
                                          DrawQuad* quad,
                                          const SkRect* subset) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    SkASSERT(proxyView.asTextureProxy());
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawTexturedQuad", fContext);

    AutoCheckFlush acf(this->drawingManager());

    // Functionally this is very similar to drawFilledQuad except that there's no constColor to
    // enable the kSubmitted optimizations, no stencil settings support, and its a TextureOp.
    QuadOptimization opt = this->attemptQuadOptimization(clip, nullptr/*stencil*/, &aa, quad,
                                                         nullptr/*paint*/);

    SkASSERT(opt != QuadOptimization::kSubmitted);
    if (opt != QuadOptimization::kDiscarded) {
        // And the texture op if not discarded
        const GrClip* finalClip = opt == QuadOptimization::kClipApplied ? nullptr : clip;
        GrAAType aaType = this->chooseAAType(aa);
        auto clampType = GrColorTypeClampType(this->colorInfo().colorType());
        auto saturate = clampType == GrClampType::kManual ? TextureOp::Saturate::kYes
                                                          : TextureOp::Saturate::kNo;
        // Use the provided subset, although hypothetically we could detect that the cropped local
        // quad is sufficiently inside the subset and the constraint could be dropped.
        this->addDrawOp(finalClip,
                        TextureOp::Make(fContext, std::move(proxyView), srcAlphaType,
                                        std::move(textureXform), filter, mm, color, saturate,
                                        blendMode, aaType, quad, subset));
    }
}

void SurfaceDrawContext::drawRect(const GrClip* clip,
                                  GrPaint&& paint,
                                  GrAA aa,
                                  const SkMatrix& viewMatrix,
                                  const SkRect& rect,
                                  const GrStyle* style) {
    if (!style) {
        style = &GrStyle::SimpleFill();
    }
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawRect", fContext);

    // Path effects should've been devolved to a path in SkGpuDevice
    SkASSERT(!style->pathEffect());

    AutoCheckFlush acf(this->drawingManager());

    const SkStrokeRec& stroke = style->strokeRec();
    if (stroke.getStyle() == SkStrokeRec::kFill_Style) {
        // Fills the rect, using rect as its own local coordinates
        this->fillRectToRect(clip, std::move(paint), aa, viewMatrix, rect, rect);
        return;
    } else if ((stroke.getStyle() == SkStrokeRec::kStroke_Style ||
                stroke.getStyle() == SkStrokeRec::kHairline_Style) &&
               rect.width()                                        &&
               rect.height()                                       &&
               !this->caps()->reducedShaderMode()) {
        // Only use the StrokeRectOp for non-empty rectangles. Empty rectangles will be processed by
        // GrStyledShape to handle stroke caps and dashing properly.
        //
        // http://skbug.com/12206 -- there is a double-blend issue with the bevel version of
        // AAStrokeRectOp, and if we increase the AA bloat for MSAA it becomes more pronounced.
        // Don't use the bevel version with DMSAA.
        GrAAType aaType = (fCanUseDynamicMSAA &&
                           stroke.getJoin() == SkPaint::kMiter_Join &&
                           stroke.getMiter() >= SK_ScalarSqrt2) ? GrAAType::kCoverage
                                                                : this->chooseAAType(aa);
        GrOp::Owner op = StrokeRectOp::Make(fContext, std::move(paint), aaType, viewMatrix,
                                            rect, stroke);
        // op may be null if the stroke is not supported or if using coverage aa and the view matrix
        // does not preserve rectangles.
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
    }
    assert_alive(paint);
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix,
                                     GrStyledShape(rect, *style, DoSimplify::kNo));
}

void SurfaceDrawContext::fillRectToRect(const GrClip* clip,
                                        GrPaint&& paint,
                                        GrAA aa,
                                        const SkMatrix& viewMatrix,
                                        const SkRect& rectToDraw,
                                        const SkRect& localRect) {
    DrawQuad quad{GrQuad::MakeFromRect(rectToDraw, viewMatrix), GrQuad(localRect),
                  aa == GrAA::kYes ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone};

    // If we are using dmsaa then attempt to draw the rect with FillRRectOp.
    if ((fContext->priv().caps()->reducedShaderMode() || this->alwaysAntialias()) &&
        this->caps()->drawInstancedSupport()                                      &&
        aa == GrAA::kYes) {  // If aa is kNo when using dmsaa, the rect is axis aligned. Don't use
                             // FillRRectOp because it might require dual source blending.
                             // http://skbug.com/11756
        QuadOptimization opt = this->attemptQuadOptimization(clip, nullptr/*stencil*/, &aa, &quad,
                                                             &paint);
        if (opt < QuadOptimization::kClipApplied) {
            // The optimization was completely handled inside attempt().
            return;
        }

        SkRect croppedRect, croppedLocal{};
        const GrClip* optimizedClip = clip;
        if (clip && viewMatrix.isScaleTranslate() && quad.fDevice.asRect(&croppedRect) &&
            (!paint.usesLocalCoords() || quad.fLocal.asRect(&croppedLocal))) {
            // The cropped quad is still a rect, and our view matrix preserves rects. Map it back
            // to pre-matrix space.
            SkMatrix inverse;
            if (!viewMatrix.invert(&inverse)) {
                return;
            }
            SkASSERT(inverse.rectStaysRect());
            inverse.mapRect(&croppedRect);
            if (opt == QuadOptimization::kClipApplied) {
                optimizedClip = nullptr;
            }
        } else {
            // Even if attemptQuadOptimization gave us an optimized quad, FillRRectOp needs a rect
            // in pre-matrix space, so use the original rect. Also preserve the original clip.
            croppedRect = rectToDraw;
            croppedLocal = localRect;
        }

        if (auto op = FillRRectOp::Make(fContext, this->arenaAlloc(), std::move(paint),
                                        viewMatrix, SkRRect::MakeRect(croppedRect), croppedLocal,
                                        GrAA::kYes)) {
            this->addDrawOp(optimizedClip, std::move(op));
            return;
        }
    }

    assert_alive(paint);
    this->drawFilledQuad(clip, std::move(paint), aa, &quad);
}

void SurfaceDrawContext::drawQuadSet(const GrClip* clip,
                                     GrPaint&& paint,
                                     GrAA aa,
                                     const SkMatrix& viewMatrix,
                                     const GrQuadSetEntry quads[],
                                     int cnt) {
    GrAAType aaType = this->chooseAAType(aa);

    FillRectOp::AddFillRectOps(this, clip, fContext, std::move(paint), aaType, viewMatrix,
                               quads, cnt);
}

int SurfaceDrawContext::maxWindowRectangles() const {
    return this->asRenderTargetProxy()->maxWindowRectangles(*this->caps());
}

OpsTask::CanDiscardPreviousOps SurfaceDrawContext::canDiscardPreviousOpsOnFullClear() const {
#if GR_TEST_UTILS
    if (fPreserveOpsOnFullClear_TestingOnly) {
        return OpsTask::CanDiscardPreviousOps::kNo;
    }
#endif
    // Regardless of how the clear is implemented (native clear or a fullscreen quad), all prior ops
    // would normally be overwritten. The one exception is if the render target context is marked as
    // needing a stencil buffer then there may be a prior op that writes to the stencil buffer.
    // Although the clear will ignore the stencil buffer, following draw ops may not so we can't get
    // rid of all the preceding ops. Beware! If we ever add any ops that have a side effect beyond
    // modifying the stencil buffer we will need a more elaborate tracking system (skbug.com/7002).
    return OpsTask::CanDiscardPreviousOps(!fNeedsStencil);
}

void SurfaceDrawContext::setNeedsStencil() {
    // Don't clear stencil until after we've set fNeedsStencil. This ensures we don't loop forever
    // in the event that there are driver bugs and we need to clear as a draw.
    bool hasInitializedStencil = fNeedsStencil;
    fNeedsStencil = true;
    if (!hasInitializedStencil) {
        this->asRenderTargetProxy()->setNeedsStencil();
        if (this->caps()->performStencilClearsAsDraws()) {
            // There is a driver bug with clearing stencil. We must use an op to manually clear the
            // stencil buffer before the op that required 'setNeedsStencil'.
            this->internalStencilClear(nullptr, /* inside mask */ false);
        } else {
            this->getOpsTask()->setInitialStencilContent(
                    OpsTask::StencilContent::kUserBitsCleared);
        }
    }
}

void SurfaceDrawContext::internalStencilClear(const SkIRect* scissor, bool insideStencilMask) {
    this->setNeedsStencil();

    GrScissorState scissorState(this->asSurfaceProxy()->backingStoreDimensions());
    if (scissor && !scissorState.set(*scissor)) {
        // The requested clear region is off screen, so nothing to do.
        return;
    }

    bool clearWithDraw = this->caps()->performStencilClearsAsDraws() ||
                         (scissorState.enabled() && this->caps()->performPartialClearsAsDraws());
    if (clearWithDraw) {
        const GrUserStencilSettings* ss = GrStencilSettings::SetClipBitSettings(insideStencilMask);

        // Configure the paint to have no impact on the color buffer
        GrPaint paint;
        paint.setXPFactory(GrDisableColorXPFactory::Get());
        this->addDrawOp(nullptr,
                        FillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                                  SkRect::Make(scissorState.rect()), ss));
    } else {
        this->addOp(ClearOp::MakeStencilClip(fContext, scissorState, insideStencilMask));
    }
}

bool SurfaceDrawContext::stencilPath(const GrHardClip* clip,
                                     GrAA doStencilMSAA,
                                     const SkMatrix& viewMatrix,
                                     const SkPath& path) {
    SkIRect clipBounds = clip ? clip->getConservativeBounds()
                              : SkIRect::MakeSize(this->dimensions());
    GrStyledShape shape(path, GrStyledShape::DoSimplify::kNo);

    PathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = fContext->priv().caps();
    canDrawArgs.fProxy = this->asRenderTargetProxy();
    canDrawArgs.fClipConservativeBounds = &clipBounds;
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fPaint = nullptr;
    canDrawArgs.fSurfaceProps = &fSurfaceProps;
    canDrawArgs.fAAType = (doStencilMSAA == GrAA::kYes) ? GrAAType::kMSAA : GrAAType::kNone;
    canDrawArgs.fHasUserStencilSettings = false;
    auto pr = this->drawingManager()->getPathRenderer(canDrawArgs,
                                                      false,
                                                      PathRendererChain::DrawType::kStencil);
    if (!pr) {
        SkDebugf("WARNING: No path renderer to stencil path.\n");
        return false;
    }

    PathRenderer::StencilPathArgs args;
    args.fContext = fContext;
    args.fSurfaceDrawContext = this;
    args.fClip = clip;
    args.fClipConservativeBounds = &clipBounds;
    args.fViewMatrix = &viewMatrix;
    args.fShape = &shape;
    args.fDoStencilMSAA = doStencilMSAA;
    pr->stencilPath(args);
    return true;
}

void SurfaceDrawContext::drawTextureSet(const GrClip* clip,
                                        GrTextureSetEntry set[],
                                        int cnt,
                                        int proxyRunCnt,
                                        GrSamplerState::Filter filter,
                                        GrSamplerState::MipmapMode mm,
                                        SkBlendMode mode,
                                        GrAA aa,
                                        SkCanvas::SrcRectConstraint constraint,
                                        const SkMatrix& viewMatrix,
                                        sk_sp<GrColorSpaceXform> texXform) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawTextureSet", fContext);

    // Create the minimum number of GrTextureOps needed to draw this set. Individual
    // GrTextureOps can rebind the texture between draws thus avoiding GrPaint (re)creation.
    AutoCheckFlush acf(this->drawingManager());
    GrAAType aaType = this->chooseAAType(aa);
    auto clampType = GrColorTypeClampType(this->colorInfo().colorType());
    auto saturate = clampType == GrClampType::kManual ? TextureOp::Saturate::kYes
                                                      : TextureOp::Saturate::kNo;
    TextureOp::AddTextureSetOps(this, clip, fContext, set, cnt, proxyRunCnt, filter, mm, saturate,
                                mode, aaType, constraint, viewMatrix, std::move(texXform));
}

void SurfaceDrawContext::drawVertices(const GrClip* clip,
                                      GrPaint&& paint,
                                      const SkMatrixProvider& matrixProvider,
                                      sk_sp<SkVertices> vertices,
                                      GrPrimitiveType* overridePrimType) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawVertices", fContext);

    AutoCheckFlush acf(this->drawingManager());

    SkASSERT(vertices);
    GrAAType aaType = fCanUseDynamicMSAA ? GrAAType::kMSAA : this->chooseAAType(GrAA::kNo);
    GrOp::Owner op = DrawVerticesOp::Make(fContext,
                                          std::move(paint),
                                          std::move(vertices),
                                          matrixProvider,
                                          aaType,
                                          this->colorInfo().refColorSpaceXformFromSRGB(),
                                          overridePrimType);
    this->addDrawOp(clip, std::move(op));
}

///////////////////////////////////////////////////////////////////////////////

void SurfaceDrawContext::drawAtlas(const GrClip* clip,
                                   GrPaint&& paint,
                                   const SkMatrix& viewMatrix,
                                   int spriteCount,
                                   const SkRSXform xform[],
                                   const SkRect texRect[],
                                   const SkColor colors[]) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawAtlas", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(GrAA::kNo);
    GrOp::Owner op = DrawAtlasOp::Make(fContext, std::move(paint), viewMatrix,
                                       aaType, spriteCount, xform, texRect, colors);
    this->addDrawOp(clip, std::move(op));
}

///////////////////////////////////////////////////////////////////////////////

void SurfaceDrawContext::drawRRect(const GrClip* origClip,
                                   GrPaint&& paint,
                                   GrAA aa,
                                   const SkMatrix& viewMatrix,
                                   const SkRRect& rrect,
                                   const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawRRect", fContext);

    SkASSERT(!style.pathEffect()); // this should've been devolved to a path in SkGpuDevice

    const SkStrokeRec& stroke = style.strokeRec();
    if (stroke.getStyle() == SkStrokeRec::kFill_Style && rrect.isEmpty()) {
       return;
    }

    const GrClip* clip = origClip;
    // It is not uncommon to clip to a round rect and then draw that same round rect. Since our
    // lower level clip code works from op bounds, which are SkRects, it doesn't detect that the
    // clip can be ignored. The following test attempts to mitigate the stencil clip cost but only
    // works for axis-aligned round rects. This also only works for filled rrects since the stroke
    // width outsets beyond the rrect itself.
    // TODO: skbug.com/10462 - There was mixed performance wins and regressions when this
    // optimization was turned on outside of Android Framework. I (michaelludwig) believe this is
    // do to the overhead in determining if an SkClipStack is just a rrect. Once that is improved,
    // re-enable this and see if we avoid the regressions.
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    SkRRect devRRect;
    if (clip && stroke.getStyle() == SkStrokeRec::kFill_Style &&
        rrect.transform(viewMatrix, &devRRect)) {
        GrClip::PreClipResult result = clip->preApply(devRRect.getBounds(), aa);
        switch(result.fEffect) {
            case GrClip::Effect::kClippedOut:
                return;
            case GrClip::Effect::kUnclipped:
                clip = nullptr;
                break;
            case GrClip::Effect::kClipped:
                // Currently there's no general-purpose rrect-to-rrect contains function, and if we
                // got here, we know the devRRect's bounds aren't fully contained by the clip.
                // Testing for equality between the two is a reasonable stop-gap for now.
                if (result.fIsRRect && result.fRRect == devRRect) {
                    // NOTE: On the android framework, we allow this optimization even when the clip
                    // is non-AA and the draw is AA.
                    if (result.fAA == aa || (result.fAA == GrAA::kNo && aa == GrAA::kYes)) {
                        clip = nullptr;
                    }
                }
                break;
            default:
                SkUNREACHABLE;
        }
    }
#endif

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa);

    GrOp::Owner op;
    if (aaType == GrAAType::kCoverage                          &&
        !fCanUseDynamicMSAA                                    &&
        !this->caps()->reducedShaderMode()                     &&
        rrect.isSimple()                                       &&
        rrect.getSimpleRadii().fX == rrect.getSimpleRadii().fY &&
        viewMatrix.rectStaysRect() && viewMatrix.isSimilarity()) {
        // In specific cases we use a dedicated circular round rect op to try and get better perf.
        assert_alive(paint);
        op = GrOvalOpFactory::MakeCircularRRectOp(fContext, std::move(paint), viewMatrix, rrect,
                                                  stroke, this->caps()->shaderCaps());
    }
    if (!op && style.isSimpleFill()) {
        assert_alive(paint);
        op = FillRRectOp::Make(fContext, this->arenaAlloc(), std::move(paint), viewMatrix, rrect,
                               rrect.rect(), GrAA(aaType != GrAAType::kNone));
    }
    if (!op && (aaType == GrAAType::kCoverage || fCanUseDynamicMSAA)) {
        assert_alive(paint);
        op = GrOvalOpFactory::MakeRRectOp(
                fContext, std::move(paint), viewMatrix, rrect, stroke, this->caps()->shaderCaps());
    }
    if (op) {
        this->addDrawOp(clip, std::move(op));
        return;
    }

    assert_alive(paint);
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix,
                                     GrStyledShape(rrect, style, DoSimplify::kNo));
}

///////////////////////////////////////////////////////////////////////////////

bool SurfaceDrawContext::drawFastShadow(const GrClip* clip,
                                        const SkMatrix& viewMatrix,
                                        const SkPath& path,
                                        const SkDrawShadowRec& rec) {
    ASSERT_SINGLE_OWNER
    if (fContext->abandoned()) {
        return true;
    }
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawFastShadow", fContext);

    // check z plane
    bool tiltZPlane = SkToBool(!SkScalarNearlyZero(rec.fZPlaneParams.fX) ||
                               !SkScalarNearlyZero(rec.fZPlaneParams.fY));
    bool skipAnalytic = SkToBool(rec.fFlags & SkShadowFlags::kGeometricOnly_ShadowFlag);
    if (tiltZPlane || skipAnalytic || !viewMatrix.rectStaysRect() || !viewMatrix.isSimilarity()) {
        return false;
    }

    SkRRect rrect;
    SkRect rect;
    // we can only handle rects, circles, and simple rrects with circular corners
    bool isRRect = path.isRRect(&rrect) && SkRRectPriv::IsNearlySimpleCircular(rrect) &&
                   rrect.getSimpleRadii().fX > SK_ScalarNearlyZero;
    if (!isRRect &&
        path.isOval(&rect) && SkScalarNearlyEqual(rect.width(), rect.height()) &&
        rect.width() > SK_ScalarNearlyZero) {
        rrect.setOval(rect);
        isRRect = true;
    }
    if (!isRRect && path.isRect(&rect)) {
        rrect.setRect(rect);
        isRRect = true;
    }

    if (!isRRect) {
        return false;
    }

    if (rrect.isEmpty()) {
        return true;
    }

    AutoCheckFlush acf(this->drawingManager());

    SkPoint3 devLightPos = rec.fLightPos;
    bool directional = SkToBool(rec.fFlags & kDirectionalLight_ShadowFlag);
    if (!directional) {
        // transform light
        viewMatrix.mapPoints((SkPoint*)&devLightPos.fX, 1);
    }

    // 1/scale
    SkScalar devToSrcScale = viewMatrix.isScaleTranslate() ?
        SkScalarInvert(SkScalarAbs(viewMatrix[SkMatrix::kMScaleX])) :
        sk_float_rsqrt(viewMatrix[SkMatrix::kMScaleX] * viewMatrix[SkMatrix::kMScaleX] +
                       viewMatrix[SkMatrix::kMSkewX] * viewMatrix[SkMatrix::kMSkewX]);

    SkScalar occluderHeight = rec.fZPlaneParams.fZ;
    bool transparent = SkToBool(rec.fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag);

    if (SkColorGetA(rec.fAmbientColor) > 0) {
        SkScalar devSpaceInsetWidth = SkDrawShadowMetrics::AmbientBlurRadius(occluderHeight);
        const SkScalar umbraRecipAlpha = SkDrawShadowMetrics::AmbientRecipAlpha(occluderHeight);
        const SkScalar devSpaceAmbientBlur = devSpaceInsetWidth * umbraRecipAlpha;

        // Outset the shadow rrect to the border of the penumbra
        SkScalar ambientPathOutset = devSpaceInsetWidth * devToSrcScale;
        SkRRect ambientRRect;
        SkRect outsetRect = rrect.rect().makeOutset(ambientPathOutset, ambientPathOutset);
        // If the rrect was an oval then its outset will also be one.
        // We set it explicitly to avoid errors.
        if (rrect.isOval()) {
            ambientRRect = SkRRect::MakeOval(outsetRect);
        } else {
            SkScalar outsetRad = SkRRectPriv::GetSimpleRadii(rrect).fX + ambientPathOutset;
            ambientRRect = SkRRect::MakeRectXY(outsetRect, outsetRad, outsetRad);
        }

        GrColor ambientColor = SkColorToPremulGrColor(rec.fAmbientColor);
        if (transparent) {
            // set a large inset to force a fill
            devSpaceInsetWidth = ambientRRect.width();
        }

        GrOp::Owner op = ShadowRRectOp::Make(fContext,
                                             ambientColor,
                                             viewMatrix,
                                             ambientRRect,
                                             devSpaceAmbientBlur,
                                             devSpaceInsetWidth);
        if (op) {
            this->addDrawOp(clip, std::move(op));
        }
    }

    if (SkColorGetA(rec.fSpotColor) > 0) {
        SkScalar devSpaceSpotBlur;
        SkScalar spotScale;
        SkVector spotOffset;
        if (directional) {
            SkDrawShadowMetrics::GetDirectionalParams(occluderHeight, devLightPos.fX,
                                                      devLightPos.fY, devLightPos.fZ,
                                                      rec.fLightRadius, &devSpaceSpotBlur,
                                                      &spotScale, &spotOffset);
        } else {
            SkDrawShadowMetrics::GetSpotParams(occluderHeight, devLightPos.fX, devLightPos.fY,
                                               devLightPos.fZ, rec.fLightRadius,
                                               &devSpaceSpotBlur, &spotScale, &spotOffset);
        }
        // handle scale of radius due to CTM
        const SkScalar srcSpaceSpotBlur = devSpaceSpotBlur * devToSrcScale;

        // Adjust translate for the effect of the scale.
        spotOffset.fX += spotScale*viewMatrix[SkMatrix::kMTransX];
        spotOffset.fY += spotScale*viewMatrix[SkMatrix::kMTransY];
        // This offset is in dev space, need to transform it into source space.
        SkMatrix ctmInverse;
        if (viewMatrix.invert(&ctmInverse)) {
            ctmInverse.mapPoints(&spotOffset, 1);
        } else {
            // Since the matrix is a similarity, this should never happen, but just in case...
            SkDebugf("Matrix is degenerate. Will not render spot shadow correctly!\n");
            SkASSERT(false);
        }

        // Compute the transformed shadow rrect
        SkRRect spotShadowRRect;
        SkMatrix shadowTransform;
        shadowTransform.setScaleTranslate(spotScale, spotScale, spotOffset.fX, spotOffset.fY);
        rrect.transform(shadowTransform, &spotShadowRRect);
        SkScalar spotRadius = spotShadowRRect.getSimpleRadii().fX;

        // Compute the insetWidth
        SkScalar blurOutset = srcSpaceSpotBlur;
        SkScalar insetWidth = blurOutset;
        if (transparent) {
            // If transparent, just do a fill
            insetWidth += spotShadowRRect.width();
        } else {
            // For shadows, instead of using a stroke we specify an inset from the penumbra
            // border. We want to extend this inset area so that it meets up with the caster
            // geometry. The inset geometry will by default already be inset by the blur width.
            //
            // We compare the min and max corners inset by the radius between the original
            // rrect and the shadow rrect. The distance between the two plus the difference
            // between the scaled radius and the original radius gives the distance from the
            // transformed shadow shape to the original shape in that corner. The max
            // of these gives the maximum distance we need to cover.
            //
            // Since we are outsetting by 1/2 the blur distance, we just add the maxOffset to
            // that to get the full insetWidth.
            SkScalar maxOffset;
            if (rrect.isRect()) {
                // Manhattan distance works better for rects
                maxOffset = std::max(std::max(SkTAbs(spotShadowRRect.rect().fLeft -
                                                 rrect.rect().fLeft),
                                          SkTAbs(spotShadowRRect.rect().fTop -
                                                 rrect.rect().fTop)),
                                   std::max(SkTAbs(spotShadowRRect.rect().fRight -
                                                 rrect.rect().fRight),
                                          SkTAbs(spotShadowRRect.rect().fBottom -
                                                 rrect.rect().fBottom)));
            } else {
                SkScalar dr = spotRadius - SkRRectPriv::GetSimpleRadii(rrect).fX;
                SkPoint upperLeftOffset = SkPoint::Make(spotShadowRRect.rect().fLeft -
                                                        rrect.rect().fLeft + dr,
                                                        spotShadowRRect.rect().fTop -
                                                        rrect.rect().fTop + dr);
                SkPoint lowerRightOffset = SkPoint::Make(spotShadowRRect.rect().fRight -
                                                         rrect.rect().fRight - dr,
                                                         spotShadowRRect.rect().fBottom -
                                                         rrect.rect().fBottom - dr);
                maxOffset = SkScalarSqrt(std::max(SkPointPriv::LengthSqd(upperLeftOffset),
                                                  SkPointPriv::LengthSqd(lowerRightOffset))) + dr;
            }
            insetWidth += std::max(blurOutset, maxOffset);
        }

        // Outset the shadow rrect to the border of the penumbra
        SkRect outsetRect = spotShadowRRect.rect().makeOutset(blurOutset, blurOutset);
        if (spotShadowRRect.isOval()) {
            spotShadowRRect = SkRRect::MakeOval(outsetRect);
        } else {
            SkScalar outsetRad = spotRadius + blurOutset;
            spotShadowRRect = SkRRect::MakeRectXY(outsetRect, outsetRad, outsetRad);
        }

        GrColor spotColor = SkColorToPremulGrColor(rec.fSpotColor);

        GrOp::Owner op = ShadowRRectOp::Make(fContext,
                                             spotColor,
                                             viewMatrix,
                                             spotShadowRRect,
                                             2.0f * devSpaceSpotBlur,
                                             insetWidth);
        if (op) {
            this->addDrawOp(clip, std::move(op));
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SurfaceDrawContext::drawRegion(const GrClip* clip,
                                    GrPaint&& paint,
                                    GrAA aa,
                                    const SkMatrix& viewMatrix,
                                    const SkRegion& region,
                                    const GrStyle& style,
                                    const GrUserStencilSettings* ss) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawRegion", fContext);

    if (GrAA::kYes == aa) {
        // GrRegionOp performs no antialiasing but is much faster, so here we check the matrix
        // to see whether aa is really required.
        if (!SkToBool(viewMatrix.getType() & ~(SkMatrix::kTranslate_Mask)) &&
            SkScalarIsInt(viewMatrix.getTranslateX()) &&
            SkScalarIsInt(viewMatrix.getTranslateY())) {
            aa = GrAA::kNo;
        }
    }
    bool complexStyle = !style.isSimpleFill();
    if (complexStyle || GrAA::kYes == aa) {
        SkPath path;
        region.getBoundaryPath(&path);
        path.setIsVolatile(true);

        return this->drawPath(clip, std::move(paint), aa, viewMatrix, path, style);
    }

    GrAAType aaType = (this->numSamples() > 1) ? GrAAType::kMSAA : GrAAType::kNone;
    GrOp::Owner op = RegionOp::Make(fContext, std::move(paint), viewMatrix, region, aaType, ss);
    this->addDrawOp(clip, std::move(op));
}

void SurfaceDrawContext::drawOval(const GrClip* clip,
                                  GrPaint&& paint,
                                  GrAA aa,
                                  const SkMatrix& viewMatrix,
                                  const SkRect& oval,
                                  const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawOval", fContext);

    const SkStrokeRec& stroke = style.strokeRec();

    if (oval.isEmpty() && !style.pathEffect()) {
        if (stroke.getStyle() == SkStrokeRec::kFill_Style) {
            return;
        }

        this->drawRect(clip, std::move(paint), aa, viewMatrix, oval, &style);
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa);

    GrOp::Owner op;
    if (aaType == GrAAType::kCoverage      &&
        !fCanUseDynamicMSAA                &&
        !this->caps()->reducedShaderMode() &&
        oval.width() > SK_ScalarNearlyZero &&
        oval.width() == oval.height()      &&
        viewMatrix.isSimilarity()) {
        // In specific cases we use a dedicated circle op to try and get better perf.
        assert_alive(paint);
        op = GrOvalOpFactory::MakeCircleOp(fContext, std::move(paint), viewMatrix, oval, style,
                                           this->caps()->shaderCaps());
    }
    if (!op && style.isSimpleFill()) {
        // FillRRectOp has special geometry and a fragment-shader branch to conditionally evaluate
        // the arc equation. This same special geometry and fragment branch also turn out to be a
        // substantial optimization for drawing ovals (namely, by not evaluating the arc equation
        // inside the oval's inner diamond). Given these optimizations, it's a clear win to draw
        // ovals the exact same way we do round rects.
        assert_alive(paint);
        op = FillRRectOp::Make(fContext, this->arenaAlloc(), std::move(paint), viewMatrix,
                               SkRRect::MakeOval(oval), oval, GrAA(aaType != GrAAType::kNone));
    }
    if (!op && (aaType == GrAAType::kCoverage || fCanUseDynamicMSAA)) {
        assert_alive(paint);
        op = GrOvalOpFactory::MakeOvalOp(fContext, std::move(paint), viewMatrix, oval, style,
                                         this->caps()->shaderCaps());
    }
    if (op) {
        this->addDrawOp(clip, std::move(op));
        return;
    }

    assert_alive(paint);
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix,
                                     GrStyledShape(SkRRect::MakeOval(oval), SkPathDirection::kCW, 2,
                                                   false, style, DoSimplify::kNo));
}

void SurfaceDrawContext::drawArc(const GrClip* clip,
                                 GrPaint&& paint,
                                 GrAA aa,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& oval,
                                 SkScalar startAngle,
                                 SkScalar sweepAngle,
                                 bool useCenter,
                                 const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
            GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawArc", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa);
    if (aaType == GrAAType::kCoverage) {
        const GrShaderCaps* shaderCaps = this->caps()->shaderCaps();
        GrOp::Owner op = GrOvalOpFactory::MakeArcOp(fContext,
                                                    std::move(paint),
                                                    viewMatrix,
                                                    oval,
                                                    startAngle,
                                                    sweepAngle,
                                                    useCenter,
                                                    style,
                                                    shaderCaps);
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
        assert_alive(paint);
    }
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix,
                                     GrStyledShape::MakeArc(oval, startAngle, sweepAngle, useCenter,
                                                            style, DoSimplify::kNo));
}

void SurfaceDrawContext::drawImageLattice(const GrClip* clip,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          GrSurfaceProxyView view,
                                          SkAlphaType alphaType,
                                          sk_sp<GrColorSpaceXform> csxf,
                                          GrSamplerState::Filter filter,
                                          std::unique_ptr<SkLatticeIter> iter,
                                          const SkRect& dst) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawImageLattice", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrOp::Owner op =
              LatticeOp::MakeNonAA(fContext, std::move(paint), viewMatrix, std::move(view),
                                   alphaType, std::move(csxf), filter, std::move(iter), dst);
    this->addDrawOp(clip, std::move(op));
}

void SurfaceDrawContext::drawDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
                                      const SkRect& bounds) {
    GrOp::Owner op(DrawableOp::Make(fContext, std::move(drawable), bounds));
    SkASSERT(op);
    this->addOp(std::move(op));
}

void SurfaceDrawContext::setLastClip(uint32_t clipStackGenID,
                                     const SkIRect& devClipBounds,
                                     int numClipAnalyticElements) {
    auto opsTask = this->getOpsTask();
    opsTask->fLastClipStackGenID = clipStackGenID;
    opsTask->fLastDevClipBounds = devClipBounds;
    opsTask->fLastClipNumAnalyticElements = numClipAnalyticElements;
}

bool SurfaceDrawContext::mustRenderClip(uint32_t clipStackGenID,
                                        const SkIRect& devClipBounds,
                                        int numClipAnalyticElements) {
    auto opsTask = this->getOpsTask();
    return opsTask->fLastClipStackGenID != clipStackGenID ||
           !opsTask->fLastDevClipBounds.contains(devClipBounds) ||
           opsTask->fLastClipNumAnalyticElements != numClipAnalyticElements;
}

bool SurfaceDrawContext::waitOnSemaphores(int numSemaphores,
                                          const GrBackendSemaphore waitSemaphores[],
                                          bool deleteSemaphoresAfterWait) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "waitOnSemaphores", fContext);

    AutoCheckFlush acf(this->drawingManager());

    if (numSemaphores && !this->caps()->semaphoreSupport()) {
        return false;
    }

    auto direct = fContext->asDirectContext();
    if (!direct) {
        return false;
    }

    auto resourceProvider = direct->priv().resourceProvider();

    GrWrapOwnership ownership =
            deleteSemaphoresAfterWait ? kAdopt_GrWrapOwnership : kBorrow_GrWrapOwnership;

    std::unique_ptr<std::unique_ptr<GrSemaphore>[]> grSemaphores(
            new std::unique_ptr<GrSemaphore>[numSemaphores]);
    for (int i = 0; i < numSemaphores; ++i) {
        grSemaphores[i] = resourceProvider->wrapBackendSemaphore(waitSemaphores[i],
                                                                 GrSemaphoreWrapType::kWillWait,
                                                                 ownership);
    }
    this->drawingManager()->newWaitRenderTask(this->asSurfaceProxyRef(), std::move(grSemaphores),
                                              numSemaphores);
    return true;
}

void SurfaceDrawContext::drawPath(const GrClip* clip,
                                  GrPaint&& paint,
                                  GrAA aa,
                                  const SkMatrix& viewMatrix,
                                  const SkPath& path,
                                  const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawPath", fContext);

    GrStyledShape shape(path, style, DoSimplify::kNo);
    this->drawShape(clip, std::move(paint), aa, viewMatrix, std::move(shape));
}

void SurfaceDrawContext::drawShape(const GrClip* clip,
                                   GrPaint&& paint,
                                   GrAA aa,
                                   const SkMatrix& viewMatrix,
                                   GrStyledShape&& shape) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawShape", fContext);

    if (shape.isEmpty()) {
        if (shape.inverseFilled()) {
            this->drawPaint(clip, std::move(paint), viewMatrix);
        }
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    // If we get here in drawShape(), we definitely need to use path rendering
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix, std::move(shape),
                                     /* attemptDrawSimple */ true);
}

static SkIRect get_clip_bounds(const SurfaceDrawContext* sdc, const GrClip* clip) {
    return clip ? clip->getConservativeBounds() : SkIRect::MakeWH(sdc->width(), sdc->height());
}

bool SurfaceDrawContext::drawAndStencilPath(const GrHardClip* clip,
                                            const GrUserStencilSettings* ss,
                                            SkRegion::Op op,
                                            bool invert,
                                            GrAA aa,
                                            const SkMatrix& viewMatrix,
                                            const SkPath& path) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "drawAndStencilPath", fContext);

    if (path.isEmpty() && path.isInverseFillType()) {
        GrPaint paint;
        paint.setCoverageSetOpXPFactory(op, invert);
        this->stencilRect(clip, ss, std::move(paint), GrAA::kNo, SkMatrix::I(),
                          SkRect::Make(this->dimensions()));
        return true;
    }

    AutoCheckFlush acf(this->drawingManager());

    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    GrAAType aaType = this->chooseAAType(aa);
    bool hasUserStencilSettings = !ss->isUnused();

    SkIRect clipConservativeBounds = get_clip_bounds(this, clip);

    GrPaint paint;
    paint.setCoverageSetOpXPFactory(op, invert);

    GrStyledShape shape(path, GrStyle::SimpleFill());
    PathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = this->caps();
    canDrawArgs.fProxy = this->asRenderTargetProxy();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fPaint = &paint;
    canDrawArgs.fSurfaceProps = &fSurfaceProps;
    canDrawArgs.fClipConservativeBounds = &clipConservativeBounds;
    canDrawArgs.fAAType = aaType;
    canDrawArgs.fHasUserStencilSettings = hasUserStencilSettings;

    using DrawType = PathRendererChain::DrawType;

    // Don't allow the SW renderer
    auto pr = this->drawingManager()->getPathRenderer(canDrawArgs,
                                                      false,
                                                      DrawType::kStencilAndColor);
    if (!pr) {
        return false;
    }

    PathRenderer::DrawPathArgs args{this->drawingManager()->getContext(),
                                    std::move(paint),
                                    ss,
                                    this,
                                    clip,
                                    &clipConservativeBounds,
                                    &viewMatrix,
                                    &shape,
                                    aaType,
                                    this->colorInfo().isLinearlyBlended()};
    pr->drawPath(args);
    return true;
}

SkBudgeted SurfaceDrawContext::isBudgeted() const {
    ASSERT_SINGLE_OWNER

    if (fContext->abandoned()) {
        return SkBudgeted::kNo;
    }

    SkDEBUGCODE(this->validate();)

    return this->asSurfaceProxy()->isBudgeted();
}

void SurfaceDrawContext::drawStrokedLine(const GrClip* clip,
                                         GrPaint&& paint,
                                         GrAA aa,
                                         const SkMatrix& viewMatrix,
                                         const SkPoint points[2],
                                         const SkStrokeRec& stroke) {
    ASSERT_SINGLE_OWNER

    SkASSERT(stroke.getStyle() == SkStrokeRec::kStroke_Style);
    SkASSERT(stroke.getWidth() > 0);
    // Adding support for round capping would require a
    // SurfaceDrawContext::fillRRectWithLocalMatrix entry point
    SkASSERT(SkPaint::kRound_Cap != stroke.getCap());

    const SkScalar halfWidth = 0.5f * stroke.getWidth();
    if (halfWidth <= 0.f) {
        // Prevents underflow when stroke width is epsilon > 0 (so technically not a hairline).
        // The CTM would need to have a scale near 1/epsilon in order for this to have meaningful
        // coverage (although that would likely overflow elsewhere and cause the draw to drop due
        // to non-finite bounds). At any other scale, this line is so thin, it's coverage is
        // negligible, so discarding the draw is visually equivalent.
        return;
    }

    SkVector parallel = points[1] - points[0];

    if (!SkPoint::Normalize(&parallel)) {
        parallel.fX = 1.0f;
        parallel.fY = 0.0f;
    }
    parallel *= halfWidth;

    SkVector ortho = { parallel.fY, -parallel.fX };
    if (SkPaint::kButt_Cap == stroke.getCap()) {
        // No extra extension for butt caps
        parallel = {0.f, 0.f};
    }
    // Order is TL, TR, BR, BL where arbitrarily "down" is p0 to p1 and "right" is positive
    SkPoint corners[4] = { points[0] - ortho - parallel,
                           points[0] + ortho - parallel,
                           points[1] + ortho + parallel,
                           points[1] - ortho + parallel };

    GrQuadAAFlags edgeAA = (aa == GrAA::kYes) ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
    this->fillQuadWithEdgeAA(clip, std::move(paint), aa, edgeAA, viewMatrix, corners, nullptr);
}

bool SurfaceDrawContext::drawSimpleShape(const GrClip* clip,
                                         GrPaint* paint,
                                         GrAA aa,
                                         const SkMatrix& viewMatrix,
                                         const GrStyledShape& shape) {
    if (!shape.style().hasPathEffect()) {
        GrAAType aaType = this->chooseAAType(aa);
        SkPoint linePts[2];
        SkRRect rrect;
        // We can ignore the starting point and direction since there is no path effect.
        bool inverted;
        if (shape.asLine(linePts, &inverted) && !inverted &&
            shape.style().strokeRec().getStyle() == SkStrokeRec::kStroke_Style &&
            shape.style().strokeRec().getCap() != SkPaint::kRound_Cap) {
            // The stroked line is an oriented rectangle, which looks the same or better (if
            // perspective) compared to path rendering. The exception is subpixel/hairline lines
            // that are non-AA or MSAA, in which case the default path renderer achieves higher
            // quality.
            // FIXME(michaelludwig): If the fill rect op could take an external coverage, or checks
            // for and outsets thin non-aa rects to 1px, the path renderer could be skipped.
            SkScalar coverage;
            if (aaType == GrAAType::kCoverage ||
                !SkDrawTreatAAStrokeAsHairline(shape.style().strokeRec().getWidth(), viewMatrix,
                                               &coverage)) {
                this->drawStrokedLine(clip, std::move(*paint), aa, viewMatrix, linePts,
                                      shape.style().strokeRec());
                return true;
            }
        } else if (shape.asRRect(&rrect, nullptr, nullptr, &inverted) && !inverted) {
            if (rrect.isRect()) {
                this->drawRect(clip, std::move(*paint), aa, viewMatrix, rrect.rect(),
                               &shape.style());
                return true;
            } else if (rrect.isOval()) {
                this->drawOval(clip, std::move(*paint), aa, viewMatrix, rrect.rect(),
                               shape.style());
                return true;
            }
            this->drawRRect(clip, std::move(*paint), aa, viewMatrix, rrect, shape.style());
            return true;
        } else if (GrAAType::kCoverage == aaType &&
                   shape.style().isSimpleFill()  &&
                   viewMatrix.rectStaysRect()    &&
                   !this->caps()->reducedShaderMode()) {
            // TODO: the rectStaysRect restriction could be lifted if we were willing to apply the
            // matrix to all the points individually rather than just to the rect
            SkRect rects[2];
            if (shape.asNestedRects(rects)) {
                // Concave AA paths are expensive - try to avoid them for special cases
                GrOp::Owner op = StrokeRectOp::MakeNested(fContext, std::move(*paint),
                                                          viewMatrix, rects);
                if (op) {
                    this->addDrawOp(clip, std::move(op));
                    return true;
                }
                // Fall through to let path renderer handle subpixel nested rects with unequal
                // stroke widths along X/Y.
            }
        }
    }
    return false;
}

void SurfaceDrawContext::drawShapeUsingPathRenderer(const GrClip* clip,
                                                    GrPaint&& paint,
                                                    GrAA aa,
                                                    const SkMatrix& viewMatrix,
                                                    GrStyledShape&& shape,
                                                    bool attemptDrawSimple) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "internalDrawPath", fContext);

    if (!viewMatrix.isFinite() || !shape.bounds().isFinite()) {
        return;
    }

    SkIRect clipConservativeBounds = get_clip_bounds(this, clip);

    // Always allow paths to trigger DMSAA.
    GrAAType aaType = fCanUseDynamicMSAA ? GrAAType::kMSAA : this->chooseAAType(aa);

    PathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = this->caps();
    canDrawArgs.fProxy = this->asRenderTargetProxy();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fPaint = &paint;
    canDrawArgs.fSurfaceProps = &fSurfaceProps;
    canDrawArgs.fClipConservativeBounds = &clipConservativeBounds;
    canDrawArgs.fHasUserStencilSettings = false;
    canDrawArgs.fAAType = aaType;

    constexpr static bool kDisallowSWPathRenderer = false;
    constexpr static bool kAllowSWPathRenderer = true;
    using DrawType = PathRendererChain::DrawType;

    PathRenderer* pr = nullptr;

    if (!shape.style().strokeRec().isFillStyle() && !shape.isEmpty()) {
        // Give the tessellation path renderer a chance to claim this stroke before we simplify it.
        PathRenderer* tess = this->drawingManager()->getTessellationPathRenderer();
        if (tess && tess->canDrawPath(canDrawArgs) == PathRenderer::CanDrawPath::kYes) {
            pr = tess;
        }
    }

    if (!pr) {
        // The shape isn't a stroke that can be drawn directly. Simplify if possible.
        shape.simplify();

        if (shape.isEmpty() && !shape.inverseFilled()) {
            return;
        }

        if (attemptDrawSimple || shape.simplified()) {
            // Usually we enter drawShapeUsingPathRenderer() because the shape+style was too complex
            // for dedicated draw ops. However, if GrStyledShape was able to reduce something we
            // ought to try again instead of going right to path rendering.
            if (this->drawSimpleShape(clip, &paint, aa, viewMatrix, shape)) {
                return;
            }
        }

        // Try a 1st time without applying any of the style to the geometry (and barring sw)
        pr = this->drawingManager()->getPathRenderer(canDrawArgs, kDisallowSWPathRenderer,
                                                     DrawType::kColor);
    }

    SkScalar styleScale =  GrStyle::MatrixToScaleFactor(viewMatrix);
    if (styleScale == 0.0f) {
        return;
    }

    if (!pr && shape.style().pathEffect()) {
        // It didn't work above, so try again with the path effect applied.
        shape = shape.applyStyle(GrStyle::Apply::kPathEffectOnly, styleScale);
        if (shape.isEmpty()) {
            return;
        }
        pr = this->drawingManager()->getPathRenderer(canDrawArgs, kDisallowSWPathRenderer,
                                                     DrawType::kColor);
    }
    if (!pr) {
        if (shape.style().applies()) {
            shape = shape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale);
            if (shape.isEmpty()) {
                return;
            }
            // This time, allow SW renderer
            pr = this->drawingManager()->getPathRenderer(canDrawArgs, kAllowSWPathRenderer,
                                                         DrawType::kColor);
        } else {
            pr = this->drawingManager()->getSoftwarePathRenderer();
#if GR_PATH_RENDERER_SPEW
            SkDebugf("falling back to: %s\n", pr->name());
#endif
        }
    }

    if (!pr) {
#ifdef SK_DEBUG
        SkDebugf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    PathRenderer::DrawPathArgs args{this->drawingManager()->getContext(),
                                    std::move(paint),
                                    &GrUserStencilSettings::kUnused,
                                    this,
                                    clip,
                                    &clipConservativeBounds,
                                    &viewMatrix,
                                    canDrawArgs.fShape,
                                    aaType,
                                    this->colorInfo().isLinearlyBlended()};
    pr->drawPath(args);
}

void SurfaceDrawContext::addDrawOp(const GrClip* clip,
                                   GrOp::Owner op,
                                   const std::function<WillAddOpFn>& willAddFn) {
    ASSERT_SINGLE_OWNER
    if (fContext->abandoned()) {
        return;
    }
    GrDrawOp* drawOp = (GrDrawOp*)op.get();
    SkDEBUGCODE(this->validate();)
    SkDEBUGCODE(drawOp->fAddDrawOpCalled = true;)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceDrawContext", "addDrawOp", fContext);

    // Setup clip
    SkRect bounds;
    op_bounds(&bounds, op.get());
    GrAppliedClip appliedClip(this->dimensions(), this->asSurfaceProxy()->backingStoreDimensions());
    const bool opUsesMSAA = drawOp->usesMSAA();
    bool skipDraw = false;
    if (clip) {
        // Have a complex clip, so defer to its early clip culling
        GrAAType aaType;
        if (opUsesMSAA) {
            aaType = GrAAType::kMSAA;
        } else {
            aaType = op->hasAABloat() ? GrAAType::kCoverage : GrAAType::kNone;
        }
        skipDraw = clip->apply(fContext, this, drawOp, aaType,
                               &appliedClip, &bounds) == GrClip::Effect::kClippedOut;
    } else {
        // No clipping, so just clip the bounds against the logical render target dimensions
        skipDraw = !bounds.intersect(this->asSurfaceProxy()->getBoundsRect());
    }

    if (skipDraw) {
        return;
    }

    GrClampType clampType = GrColorTypeClampType(this->colorInfo().colorType());
    GrProcessorSet::Analysis analysis = drawOp->finalize(*this->caps(), &appliedClip, clampType);

    const bool opUsesStencil = drawOp->usesStencil();

    // Always trigger DMSAA when there is stencil. This ensures stencil contents get properly
    // preserved between render passes, if needed.
    const bool drawNeedsMSAA = opUsesMSAA || (fCanUseDynamicMSAA && opUsesStencil);

    // Must be called before setDstProxyView so that it sees the final bounds of the op.
    op->setClippedBounds(bounds);

    // Determine if the Op will trigger the use of a separate DMSAA attachment that requires manual
    // resolves.
    // TODO: Currently usesAttachmentIfDMSAA checks if this is a textureProxy or not. This check is
    // really only for GL which uses a normal texture sampling when using barriers. For Vulkan it
    // is possible to use the msaa buffer as an input attachment even if this is not a texture.
    // However, support for that is not fully implemented yet in Vulkan. Once it is, this check
    // should change to a virtual caps check that returns whether we need to break up an OpsTask
    // if it has barriers and we are about to promote to MSAA.
    bool usesAttachmentIfDMSAA =
            fCanUseDynamicMSAA &&
            (!this->caps()->msaaResolvesAutomatically() || !this->asTextureProxy());
    bool opRequiresDMSAAAttachment = usesAttachmentIfDMSAA && drawNeedsMSAA;
    bool opTriggersDMSAAAttachment =
            opRequiresDMSAAAttachment && !this->getOpsTask()->usesMSAASurface();
    if (opTriggersDMSAAAttachment) {
        // This will be the op that actually triggers use of a DMSAA attachment. Texture barriers
        // can't be moved to a DMSAA attachment, so if there already are any on the current opsTask
        // then we need to split.
        if (this->getOpsTask()->renderPassXferBarriers() & GrXferBarrierFlags::kTexture) {
            SkASSERT(!this->getOpsTask()->isColorNoOp());
            this->replaceOpsTask()->setCannotMergeBackward();
        }
    }

    GrDstProxyView dstProxyView;
    if (analysis.requiresDstTexture()) {
        if (!this->setupDstProxyView(drawOp->bounds(), drawNeedsMSAA, &dstProxyView)) {
            return;
        }
#ifdef SK_DEBUG
        if (fCanUseDynamicMSAA && drawNeedsMSAA && !this->caps()->msaaResolvesAutomatically()) {
            // Since we aren't literally writing to the render target texture while using a DMSAA
            // attachment, we need to resolve that texture before sampling it. Ensure the current
            // opsTask got closed off in order to initiate an implicit resolve.
            SkASSERT(this->getOpsTask()->isEmpty());
        }
#endif
    }

    auto opsTask = this->getOpsTask();
    if (willAddFn) {
        willAddFn(op.get(), opsTask->uniqueID());
    }

    // Note if the op needs stencil. Stencil clipping already called setNeedsStencil for itself, if
    // needed.
    if (opUsesStencil) {
        this->setNeedsStencil();
    }

#if GR_GPU_STATS && GR_TEST_UTILS
    if (fCanUseDynamicMSAA && drawNeedsMSAA) {
        if (!opsTask->usesMSAASurface()) {
            fContext->priv().dmsaaStats().fNumMultisampleRenderPasses++;
        }
        fContext->priv().dmsaaStats().fTriggerCounts[op->name()]++;
    }
#endif

    opsTask->addDrawOp(this->drawingManager(), std::move(op), drawNeedsMSAA, analysis,
                       std::move(appliedClip), dstProxyView,
                       GrTextureResolveManager(this->drawingManager()), *this->caps());

#ifdef SK_DEBUG
    if (fCanUseDynamicMSAA && drawNeedsMSAA) {
        SkASSERT(opsTask->usesMSAASurface());
    }
#endif
}

bool SurfaceDrawContext::setupDstProxyView(const SkRect& opBounds,
                                           bool opRequiresMSAA,
                                           GrDstProxyView* dstProxyView) {
    // If we are wrapping a vulkan secondary command buffer, we can't make a dst copy because we
    // don't actually have a VkImage to make a copy of. Additionally we don't have the power to
    // start and stop the render pass in order to make the copy.
    if (this->asRenderTargetProxy()->wrapsVkSecondaryCB()) {
        return false;
    }

    // First get the dstSampleFlags as if we will put the draw into the current OpsTask
    auto dstSampleFlags = this->caps()->getDstSampleFlagsForProxy(
            this->asRenderTargetProxy(), this->getOpsTask()->usesMSAASurface() || opRequiresMSAA);

    // If we don't have barriers for this draw then we will definitely be breaking up the OpsTask.
    // However, if using dynamic MSAA, the new OpsTask will not have MSAA already enabled on it
    // and that may allow us to use texture barriers. So we check if we can use barriers on the new
    // ops task and then break it up if so.
    if (!(dstSampleFlags & GrDstSampleFlags::kRequiresTextureBarrier) &&
        fCanUseDynamicMSAA && this->getOpsTask()->usesMSAASurface() && !opRequiresMSAA) {
        auto newFlags =
                this->caps()->getDstSampleFlagsForProxy(this->asRenderTargetProxy(),
                                                        false/*=opRequiresMSAA*/);
        if (newFlags & GrDstSampleFlags::kRequiresTextureBarrier) {
            // We can't have an empty ops task if we are in DMSAA and the ops task already returns
            // true for usesMSAASurface.
            SkASSERT(!this->getOpsTask()->isColorNoOp());
            this->replaceOpsTask()->setCannotMergeBackward();
            dstSampleFlags = newFlags;
        }
    }

    if (dstSampleFlags & GrDstSampleFlags::kRequiresTextureBarrier) {
        // If we require a barrier to sample the dst it means we are sampling the RT itself
        // either as a texture or input attachment. In this case we don't need to break up the
        // OpsTask.
        dstProxyView->setProxyView(this->readSurfaceView());
        dstProxyView->setOffset(0, 0);
        dstProxyView->setDstSampleFlags(dstSampleFlags);
        return true;
    }
    SkASSERT(dstSampleFlags == GrDstSampleFlags::kNone);

    // We are using a different surface from the main color attachment to sample the dst from. If we
    // are in DMSAA we can just use the single sampled RT texture itself. Otherwise, we must do a
    // copy.
    // We do have to check if we ended up here becasue we don't have texture barriers but do have
    // msaaResolvesAutomatically (i.e. render-to-msaa-texture). In that case there will be no op or
    // barrier between draws to flush the render target before being used as a texture in the next
    // draw. So in that case we just fall through to doing a copy.
    if (fCanUseDynamicMSAA && opRequiresMSAA && this->asTextureProxy() &&
        !this->caps()->msaaResolvesAutomatically() &&
        this->caps()->dmsaaResolveCanBeUsedAsTextureInSameRenderPass()) {
        this->replaceOpsTaskIfModifiesColor()->setCannotMergeBackward();
        dstProxyView->setProxyView(this->readSurfaceView());
        dstProxyView->setOffset(0, 0);
        dstProxyView->setDstSampleFlags(dstSampleFlags);
        return true;
    }

    // Now we fallback to doing a copy.

    GrColorType colorType = this->colorInfo().colorType();
    // MSAA consideration: When there is support for reading MSAA samples in the shader we could
    // have per-sample dst values by making the copy multisampled.
    GrCaps::DstCopyRestrictions restrictions = this->caps()->getDstCopyRestrictions(
            this->asRenderTargetProxy(), colorType);

    SkIRect copyRect = SkIRect::MakeSize(this->asSurfaceProxy()->backingStoreDimensions());
    if (!restrictions.fMustCopyWholeSrc) {
        // If we don't need the whole source, restrict to the op's bounds. We add an extra pixel
        // of padding to account for AA bloat and the unpredictable rounding of coords near pixel
        // centers during rasterization.
        SkIRect conservativeDrawBounds = opBounds.roundOut();
        conservativeDrawBounds.outset(1, 1);
        SkAssertResult(copyRect.intersect(conservativeDrawBounds));
    }

    SkIPoint dstOffset;
    SkBackingFit fit;
    if (restrictions.fRectsMustMatch == GrSurfaceProxy::RectsMustMatch::kYes) {
        dstOffset = {0, 0};
        fit = SkBackingFit::kExact;
    } else {
        dstOffset = {copyRect.fLeft, copyRect.fTop};
        fit = SkBackingFit::kApprox;
    }
    auto copy = GrSurfaceProxy::Copy(fContext,
                                     this->asSurfaceProxyRef(),
                                     this->origin(),
                                     GrMipmapped::kNo,
                                     copyRect,
                                     fit,
                                     SkBudgeted::kYes,
                                     restrictions.fRectsMustMatch);
    SkASSERT(copy);

    dstProxyView->setProxyView({std::move(copy), this->origin(), this->readSwizzle()});
    dstProxyView->setOffset(dstOffset);
    dstProxyView->setDstSampleFlags(dstSampleFlags);
    return true;
}

OpsTask* SurfaceDrawContext::replaceOpsTaskIfModifiesColor() {
    if (!this->getOpsTask()->isColorNoOp()) {
        this->replaceOpsTask();
    }
    return this->getOpsTask();
}

} // namespace skgpu::v1

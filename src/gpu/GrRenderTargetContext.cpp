/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDrawable.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/SkShadowFlags.h"
#include "include/utils/SkShadowUtils.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpList.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/GrTextureDomain.h"
#include "src/gpu/effects/generated/GrColorMatrixFragmentProcessor.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/ops/GrClearOp.h"
#include "src/gpu/ops/GrClearStencilClipOp.h"
#include "src/gpu/ops/GrDebugMarkerOp.h"
#include "src/gpu/ops/GrDrawAtlasOp.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrDrawVerticesOp.h"
#include "src/gpu/ops/GrDrawableOp.h"
#include "src/gpu/ops/GrFillRRectOp.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/ops/GrLatticeOp.h"
#include "src/gpu/ops/GrOp.h"
#include "src/gpu/ops/GrOvalOpFactory.h"
#include "src/gpu/ops/GrRegionOp.h"
#include "src/gpu/ops/GrSemaphoreOp.h"
#include "src/gpu/ops/GrShadowRRectOp.h"
#include "src/gpu/ops/GrStencilPathOp.h"
#include "src/gpu/ops/GrStrokeRectOp.h"
#include "src/gpu/ops/GrTextureOp.h"
#include "src/gpu/text/GrTextContext.h"
#include "src/gpu/text/GrTextTarget.h"

class GrRenderTargetContext::TextTarget : public GrTextTarget {
public:
    TextTarget(GrRenderTargetContext* renderTargetContext)
            : GrTextTarget(renderTargetContext->width(), renderTargetContext->height(),
                           renderTargetContext->colorSpaceInfo())
            , fRenderTargetContext(renderTargetContext)
            , fGlyphPainter{*renderTargetContext}{}

    void addDrawOp(const GrClip& clip, std::unique_ptr<GrAtlasTextOp> op) override {
        fRenderTargetContext->addDrawOp(clip, std::move(op));
    }

    void drawShape(const GrClip& clip, const SkPaint& paint,
                  const SkMatrix& viewMatrix, const GrShape& shape) override {
        GrBlurUtils::drawShapeWithMaskFilter(fRenderTargetContext->fContext, fRenderTargetContext,
                                             clip, paint, viewMatrix, shape);
    }

    void makeGrPaint(GrMaskFormat maskFormat, const SkPaint& skPaint, const SkMatrix& viewMatrix,
                     GrPaint* grPaint) override {
        auto context = fRenderTargetContext->fContext;
        const GrColorSpaceInfo& colorSpaceInfo = fRenderTargetContext->colorSpaceInfo();
        if (kARGB_GrMaskFormat == maskFormat) {
            SkPaintToGrPaintWithPrimitiveColor(context, colorSpaceInfo, skPaint, grPaint);
        } else {
            SkPaintToGrPaint(context, colorSpaceInfo, skPaint, viewMatrix, grPaint);
        }
    }

    GrRecordingContext* getContext() override {
        return fRenderTargetContext->fContext;
    }

    SkGlyphRunListPainter* glyphPainter() override {
        return &fGlyphPainter;
    }

private:
    GrRenderTargetContext* fRenderTargetContext;
    SkGlyphRunListPainter fGlyphPainter;

};

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this->drawingManager()->getContext())
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)
#define ASSERT_SINGLE_OWNER_PRIV \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fRenderTargetContext->singleOwner());)
#define RETURN_IF_ABANDONED        if (fContext->priv().abandoned()) { return; }
#define RETURN_IF_ABANDONED_PRIV   if (fRenderTargetContext->fContext->priv().abandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED  if (fContext->priv().abandoned()) { return false; }
#define RETURN_FALSE_IF_ABANDONED_PRIV  if (fRenderTargetContext->fContext->priv().abandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED   if (fContext->priv().abandoned()) { return nullptr; }

//////////////////////////////////////////////////////////////////////////////

class AutoCheckFlush {
public:
    AutoCheckFlush(GrDrawingManager* drawingManager) : fDrawingManager(drawingManager) {
        SkASSERT(fDrawingManager);
    }
    ~AutoCheckFlush() { fDrawingManager->flushIfNecessary(); }

private:
    GrDrawingManager* fDrawingManager;
};

// In MDB mode the reffing of the 'getLastOpList' call's result allows in-progress
// GrOpLists to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpList, a new one will be allocated
// when the renderTargetContext attempts to use it (via getOpList).
GrRenderTargetContext::GrRenderTargetContext(GrRecordingContext* context,
                                             sk_sp<GrRenderTargetProxy> rtp,
                                             GrColorType colorType,
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkSurfaceProps* surfaceProps,
                                             bool managedOpList)
        : GrSurfaceContext(context, colorType, kPremul_SkAlphaType, std::move(colorSpace))
        , fRenderTargetProxy(std::move(rtp))
        , fOpList(sk_ref_sp(fRenderTargetProxy->getLastRenderTargetOpList()))
        , fSurfaceProps(SkSurfacePropsCopyOrDefault(surfaceProps))
        , fManagedOpList(managedOpList) {
    fTextTarget.reset(new TextTarget(this));
    SkDEBUGCODE(this->validate();)
}

#ifdef SK_DEBUG
void GrRenderTargetContext::validate() const {
    SkASSERT(fRenderTargetProxy);
    fRenderTargetProxy->validate(fContext);

    if (fOpList && !fOpList->isClosed()) {
        SkASSERT(fRenderTargetProxy->getLastRenderTask() == fOpList.get());
    }
}
#endif

GrRenderTargetContext::~GrRenderTargetContext() {
    ASSERT_SINGLE_OWNER
}

inline GrAAType GrRenderTargetContext::chooseAAType(GrAA aa) {
    if (GrAA::kNo == aa) {
        // On some devices we cannot disable MSAA if it is enabled so we make the AA type reflect
        // that.
        if (this->numSamples() > 1 && !this->caps()->multisampleDisableSupport()) {
            return GrAAType::kMSAA;
        }
        return GrAAType::kNone;
    }
    return (this->numSamples() > 1) ? GrAAType::kMSAA : GrAAType::kCoverage;
}

GrTextureProxy* GrRenderTargetContext::asTextureProxy() {
    return fRenderTargetProxy->asTextureProxy();
}

const GrTextureProxy* GrRenderTargetContext::asTextureProxy() const {
    return fRenderTargetProxy->asTextureProxy();
}

sk_sp<GrTextureProxy> GrRenderTargetContext::asTextureProxyRef() {
    return sk_ref_sp(fRenderTargetProxy->asTextureProxy());
}

GrMipMapped GrRenderTargetContext::mipMapped() const {
    if (const GrTextureProxy* proxy = this->asTextureProxy()) {
        return proxy->mipMapped();
    }
    return GrMipMapped::kNo;
}

GrRenderTargetOpList* GrRenderTargetContext::getRTOpList() {
    ASSERT_SINGLE_OWNER
    SkDEBUGCODE(this->validate();)

    if (!fOpList || fOpList->isClosed()) {
        fOpList = this->drawingManager()->newRTOpList(fRenderTargetProxy, fManagedOpList);
    }

    return fOpList.get();
}

GrOpList* GrRenderTargetContext::getOpList() {
    return this->getRTOpList();
}

void GrRenderTargetContext::drawGlyphRunList(
        const GrClip& clip, const SkMatrix& viewMatrix,
        const SkGlyphRunList& blob) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawGlyphRunList", fContext);

    // Drawing text can cause us to do inline uploads. This is not supported for wrapped vulkan
    // secondary command buffers because it would require stopping and starting a render pass which
    // we don't have access to.
    if (this->wrapsVkSecondaryCB()) {
        return;
    }

    GrTextContext* atlasTextContext = this->drawingManager()->getTextContext();
    atlasTextContext->drawGlyphRunList(fContext, fTextTarget.get(), clip, viewMatrix,
                                       fSurfaceProps, blob);
}

void GrRenderTargetContext::discard() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "discard", fContext);

    AutoCheckFlush acf(this->drawingManager());

    this->getRTOpList()->discard();
}

void GrRenderTargetContext::clear(const SkIRect* rect,
                                  const SkPMColor4f& color,
                                  CanClearFullscreen canClearFullscreen) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "clear", fContext);

    AutoCheckFlush acf(this->drawingManager());
    this->internalClear(rect ? GrFixedClip(*rect) : GrFixedClip::Disabled(), color,
                        canClearFullscreen);
}

void GrRenderTargetContextPriv::clear(const GrFixedClip& clip,
                                      const SkPMColor4f& color,
                                      CanClearFullscreen canClearFullscreen) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "clear",
                                   fRenderTargetContext->fContext);

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());
    fRenderTargetContext->internalClear(clip, color, canClearFullscreen);
}

static void clear_to_grpaint(const SkPMColor4f& color, GrPaint* paint) {
    paint->setColor4f(color);
    if (color.isOpaque()) {
        // Can just rely on the src-over blend mode to do the right thing
        paint->setPorterDuffXPFactory(SkBlendMode::kSrcOver);
    } else {
        // A clear overwrites the prior color, so even if it's transparent, it behaves as if it
        // were src blended
        paint->setPorterDuffXPFactory(SkBlendMode::kSrc);
    }
}

void GrRenderTargetContext::internalClear(const GrFixedClip& clip,
                                          const SkPMColor4f& color,
                                          CanClearFullscreen canClearFullscreen) {
    bool isFull = false;
    if (!clip.hasWindowRectangles()) {
        // TODO: wrt the shouldInitializeTextures path, it would be more performant to
        // only clear the entire target if we knew it had not been cleared before. As
        // is this could end up doing a lot of redundant clears.
        isFull = !clip.scissorEnabled() ||
                 (CanClearFullscreen::kYes == canClearFullscreen &&
                  (this->caps()->preferFullscreenClears() || this->caps()->shouldInitializeTextures())) ||
                 clip.scissorRect().contains(SkIRect::MakeWH(this->width(), this->height()));
    }

    if (isFull) {
        GrRenderTargetOpList* opList = this->getRTOpList();
        if (opList->resetForFullscreenClear(this->canDiscardPreviousOpsOnFullClear()) &&
            !this->caps()->performColorClearsAsDraws()) {
            // The op list was emptied and native clears are allowed, so just use the load op
            opList->setColorLoadOp(GrLoadOp::kClear, color);
            return;
        } else {
            // Will use an op for the clear, reset the load op to discard since the op will
            // blow away the color buffer contents
            opList->setColorLoadOp(GrLoadOp::kDiscard);
        }

        // Must add an op to the list (either because we couldn't use a load op, or because the
        // clear load op isn't supported)
        if (this->caps()->performColorClearsAsDraws()) {
            SkRect rtRect = SkRect::MakeWH(this->width(), this->height());
            GrPaint paint;
            clear_to_grpaint(color, &paint);
            this->addDrawOp(GrFixedClip::Disabled(),
                            GrFillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                                        rtRect));
        } else {
            this->addOp(GrClearOp::Make(
                    fContext, SkIRect::MakeEmpty(), color, /* fullscreen */ true));
        }
    } else {
        if (this->caps()->performPartialClearsAsDraws()) {
            // performPartialClearsAsDraws() also returns true if any clear has to be a draw.
            GrPaint paint;
            clear_to_grpaint(color, &paint);

            this->addDrawOp(clip,
                            GrFillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                                        SkRect::Make(clip.scissorRect())));
        } else {
            std::unique_ptr<GrOp> op(GrClearOp::Make(fContext, clip, color,
                                                     this->asSurfaceProxy()));
            // This version of the clear op factory can return null if the clip doesn't intersect
            // with the surface proxy's boundary
            if (!op) {
                return;
            }
            this->addOp(std::move(op));
        }
    }
}

void GrRenderTargetContextPriv::absClear(const SkIRect* clearRect, const SkPMColor4f& color) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "absClear",
                                   fRenderTargetContext->fContext);

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    SkIRect rtRect = SkIRect::MakeWH(fRenderTargetContext->fRenderTargetProxy->worstCaseWidth(),
                                     fRenderTargetContext->fRenderTargetProxy->worstCaseHeight());

    if (clearRect) {
        if (clearRect->contains(rtRect)) {
            clearRect = nullptr; // full screen
        } else {
            if (!rtRect.intersect(*clearRect)) {
                return;
            }
        }
    }

    // TODO: in a post-MDB world this should be handled at the OpList level.
    // This makes sure to always add an op to the list, instead of marking the clear as a load op.
    // This code follows very similar logic to internalClear() below, but critical differences are
    // highlighted in line related to absClear()'s unique behavior.
    if (clearRect) {
        if (fRenderTargetContext->caps()->performPartialClearsAsDraws()) {
            GrPaint paint;
            clear_to_grpaint(color, &paint);

            // Use the disabled clip; the rect geometry already matches the clear rectangle and
            // if it were added to a scissor, that would be intersected with the logical surface
            // bounds and not the worst case dimensions required here.
            fRenderTargetContext->addDrawOp(
                    GrFixedClip::Disabled(),
                    GrFillRectOp::MakeNonAARect(fRenderTargetContext->fContext, std::move(paint),
                                                SkMatrix::I(), SkRect::Make(rtRect)));
        } else {
            // Must use the ClearOp factory that takes a boolean (false) instead of a surface
            // proxy. The surface proxy variant would intersect the clip rect with its logical
            // bounds, which is not desired in this special case.
            fRenderTargetContext->addOp(GrClearOp::Make(
                    fRenderTargetContext->fContext, rtRect, color, /* fullscreen */ false));
        }
    } else {
        // Reset the oplist like in internalClear(), but do not rely on a load op for the clear
        fRenderTargetContext->getRTOpList()->resetForFullscreenClear(
                fRenderTargetContext->canDiscardPreviousOpsOnFullClear());
        fRenderTargetContext->getRTOpList()->setColorLoadOp(GrLoadOp::kDiscard);

        if (fRenderTargetContext->caps()->performColorClearsAsDraws()) {
            // This draws a quad covering the worst case dimensions instead of just the logical
            // width and height like in internalClear().
            GrPaint paint;
            clear_to_grpaint(color, &paint);
            fRenderTargetContext->addDrawOp(
                    GrFixedClip::Disabled(),
                    GrFillRectOp::MakeNonAARect(fRenderTargetContext->fContext, std::move(paint),
                                                SkMatrix::I(), SkRect::Make(rtRect)));
        } else {
            // Nothing special about this path in absClear compared to internalClear()
            fRenderTargetContext->addOp(GrClearOp::Make(
                    fRenderTargetContext->fContext, SkIRect::MakeEmpty(), color,
                    /* fullscreen */ true));
        }
    }
}

void GrRenderTargetContext::drawPaint(const GrClip& clip,
                                      GrPaint&& paint,
                                      const SkMatrix& viewMatrix) {
    // Start with the render target, since that is the maximum content we could possibly fill.
    // drawFilledQuad() will automatically restrict it to clip bounds for us if possible.
    SkRect r = fRenderTargetProxy->getBoundsRect();
    if (!paint.numTotalFragmentProcessors()) {
        // The paint is trivial so we won't need to use local coordinates, so skip calculating the
        // inverse view matrix.
        this->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(), r, r);
    } else {
        // Use the inverse view matrix to arrive at appropriate local coordinates for the paint.
        SkMatrix localMatrix;
        if (!viewMatrix.invert(&localMatrix)) {
            return;
        }
        this->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(), r,
                                      localMatrix);
    }
}

enum class GrRenderTargetContext::QuadOptimization {
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

static bool make_vertex_finite(float* value) {
    if (SkScalarIsNaN(*value)) {
        return false;
    }

    if (!SkScalarIsFinite(*value)) {
        // +/- infinity at this point. Don't use exactly SK_ScalarMax so that we have some precision
        // left when calculating crops.
        static constexpr float kNearInfinity = SK_ScalarMax / 4.f;
        *value = *value < 0.f ? -kNearInfinity : kNearInfinity;
    }

    return true;
}

GrRenderTargetContext::QuadOptimization GrRenderTargetContext::attemptQuadOptimization(
        const GrClip& clip, const SkPMColor4f* constColor,
        const GrUserStencilSettings* stencilSettings, GrAA* aa, GrQuadAAFlags* edgeFlags,
        GrQuad* deviceQuad, GrQuad* localQuad) {
    // Optimization requirements:
    // 1. kDiscard applies when clip bounds and quad bounds do not intersect
    // 2. kClear applies when constColor and final geom is pixel aligned rect;
    //       pixel aligned rect requires rect clip and (rect quad or quad covers clip)
    // 3. kRRect applies when constColor and rrect clip and quad covers clip
    // 4. kExplicitClip applies when rect clip and (rect quad or quad covers clip)
    // 5. kCropped applies when rect quad (currently)
    // 6. kNone always applies
    GrQuadAAFlags newFlags = *edgeFlags;

    SkRect rtRect;
    if (stencilSettings) {
        // Must use worst case bounds so that stencil buffer updates on approximately sized render
        // targets don't get corrupted.
        rtRect = SkRect::MakeWH(fRenderTargetProxy->worstCaseWidth(),
                                fRenderTargetProxy->worstCaseHeight());
    } else {
        // Use the logical size of the render target, which allows for "fullscreen" clears even if
        // the render target has an approximate backing fit
        rtRect = SkRect::MakeWH(this->width(), this->height());
    }

    SkRect drawBounds = deviceQuad->bounds();
    if (constColor) {
        // Don't bother updating local coordinates when the paint will ignore them anyways
        localQuad = nullptr;
        // If the device quad is not finite, coerce into a finite quad. This is acceptable since it
        // will be cropped to the finite 'clip' or render target and there is no local space mapping
        if (!deviceQuad->isFinite()) {
            for (int i = 0; i < 4; ++i) {
                if (!make_vertex_finite(deviceQuad->xs() + i) ||
                    !make_vertex_finite(deviceQuad->ys() + i) ||
                    !make_vertex_finite(deviceQuad->ws() + i)) {
                    // Discard if we see a nan
                    return QuadOptimization::kDiscarded;
                }
            }
            SkASSERT(deviceQuad->isFinite());
        }
    } else {
        // CropToRect requires the quads to be finite. If they are not finite and we have local
        // coordinates, the mapping from local space to device space is poorly defined so drop it
        if (!deviceQuad->isFinite()) {
            return QuadOptimization::kDiscarded;
        }
    }

    // If the quad is entirely off screen, it doesn't matter what the clip does
    if (!rtRect.intersects(drawBounds)) {
        return QuadOptimization::kDiscarded;
    }

    // Check if clip can be represented as a rounded rect (initialize as if clip fully contained
    // the render target).
    SkRRect clipRRect = SkRRect::MakeRect(rtRect);
    // We initialize clipAA to *aa when there are stencil settings so that we don't artificially
    // encounter mixed-aa edges (not allowed for stencil), but we want to start as non-AA for
    // regular draws so that if we fully cover the render target, that can stop being anti-aliased.
    GrAA clipAA = stencilSettings ? *aa : GrAA::kNo;
    bool axisAlignedClip = true;
    if (!clip.quickContains(rtRect)) {
        if (!clip.isRRect(rtRect, &clipRRect, &clipAA)) {
            axisAlignedClip = false;
        }
    }

    // If the clip rrect is valid (i.e. axis-aligned), we can potentially combine it with the
    // draw geometry so that no clip is needed when drawing.
    if (axisAlignedClip && (!stencilSettings || clipAA == *aa)) {
        // Tighten clip bounds (if clipRRect.isRect() is true, clipBounds now holds the intersection
        // of the render target and the clip rect)
        SkRect clipBounds = rtRect;
        if (!clipBounds.intersect(clipRRect.rect()) || !clipBounds.intersects(drawBounds)) {
            return QuadOptimization::kDiscarded;
        }

        if (clipRRect.isRect()) {
            // No rounded corners, so the kClear and kExplicitClip optimizations are possible
            if (GrQuadUtils::CropToRect(clipBounds, clipAA, &newFlags, deviceQuad, localQuad)) {
                if (constColor && deviceQuad->quadType() == GrQuad::Type::kAxisAligned) {
                    // Clear optimization is possible
                    drawBounds = deviceQuad->bounds();
                    if (drawBounds.contains(rtRect)) {
                        // Fullscreen clear
                        this->clear(nullptr, *constColor, CanClearFullscreen::kYes);
                        return QuadOptimization::kSubmitted;
                    } else if (GrClip::IsPixelAligned(drawBounds) &&
                               drawBounds.width() > 256 && drawBounds.height() > 256) {
                        // Scissor + clear (round shouldn't do anything since we are pixel aligned)
                        SkIRect scissorRect;
                        drawBounds.round(&scissorRect);
                        this->clear(&scissorRect, *constColor, CanClearFullscreen::kNo);
                        return QuadOptimization::kSubmitted;
                    }
                }

                // Update overall AA setting.
                *edgeFlags = newFlags;
                if (*aa == GrAA::kNo && clipAA == GrAA::kYes &&
                    newFlags != GrQuadAAFlags::kNone) {
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
            } else {
                // The quads have been updated to better fit the clip bounds, but can't get rid of
                // the clip entirely
                return QuadOptimization::kCropped;
            }
        } else if (constColor) {
            // Rounded corners and constant filled color (limit ourselves to solid colors because
            // there is no way to use custom local coordinates with drawRRect).
            if (GrQuadUtils::CropToRect(clipBounds, clipAA, &newFlags, deviceQuad, localQuad) &&
                deviceQuad->quadType() == GrQuad::Type::kAxisAligned &&
                deviceQuad->bounds().contains(clipBounds)) {
                // Since the cropped quad became a rectangle which covered the bounds of the rrect,
                // we can draw the rrect directly and ignore the edge flags
                GrPaint paint;
                clear_to_grpaint(*constColor, &paint);
                this->drawRRect(GrFixedClip::Disabled(), std::move(paint), clipAA, SkMatrix::I(),
                                clipRRect, GrStyle::SimpleFill());
                return QuadOptimization::kSubmitted;
            } else {
                // The quad has been updated to better fit clip bounds, but can't remove the clip
                return QuadOptimization::kCropped;
            }
        }
    }

    // Crop the quad to the conservative bounds of the clip.
    SkIRect clipDevBounds;
    clip.getConservativeBounds(rtRect.width(), rtRect.height(), &clipDevBounds);
    SkRect clipBounds = SkRect::Make(clipDevBounds);

    // One final check for discarding, since we may have gone here directly due to a complex clip
    if (!clipBounds.intersects(drawBounds)) {
        return QuadOptimization::kDiscarded;
    }

    // Even if this were to return true, the crop rect does not exactly match the clip, so can not
    // report explicit-clip. Since these edges aren't visible, don't update the final edge flags.
    GrQuadUtils::CropToRect(clipBounds, clipAA, &newFlags, deviceQuad, localQuad);

    return QuadOptimization::kCropped;
}

void GrRenderTargetContext::drawFilledQuad(const GrClip& clip,
                                           GrPaint&& paint,
                                           GrAA aa,
                                           GrQuadAAFlags edgeFlags,
                                           const GrQuad& deviceQuad,
                                           const GrQuad& localQuad,
                                           const GrUserStencilSettings* ss) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawFilledQuad", fContext);

    AutoCheckFlush acf(this->drawingManager());

    SkPMColor4f* constColor = nullptr;
    SkPMColor4f paintColor;
    if (!ss && !paint.numCoverageFragmentProcessors() &&
        paint.isConstantBlendedColor(&paintColor)) {
        // Only consider clears/rrects when it's easy to guarantee 100% fill with single color
        constColor = &paintColor;
    }

    GrQuad croppedDeviceQuad = deviceQuad;
    GrQuad croppedLocalQuad = localQuad;
    QuadOptimization opt = this->attemptQuadOptimization(clip, constColor, ss, &aa, &edgeFlags,
                                                         &croppedDeviceQuad, &croppedLocalQuad);
    if (opt >= QuadOptimization::kClipApplied) {
        // These optimizations require caller to add an op themselves
        const GrClip& finalClip = opt == QuadOptimization::kClipApplied ? GrFixedClip::Disabled()
                                                                        : clip;
        GrAAType aaType = ss ? (aa == GrAA::kYes ? GrAAType::kMSAA : GrAAType::kNone)
                             : this->chooseAAType(aa);
        this->addDrawOp(finalClip, GrFillRectOp::Make(fContext, std::move(paint), aaType, edgeFlags,
                                                      croppedDeviceQuad, croppedLocalQuad, ss));
    }
    // All other optimization levels were completely handled inside attempt(), so no extra op needed
}

void GrRenderTargetContext::drawTexturedQuad(const GrClip& clip,
                                             sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> textureXform,
                                             GrSamplerState::Filter filter,
                                             const SkPMColor4f& color,
                                             SkBlendMode blendMode,
                                             GrAA aa,
                                             GrQuadAAFlags edgeFlags,
                                             const GrQuad& deviceQuad,
                                             const GrQuad& localQuad,
                                             const SkRect* domain) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawTexturedQuad", fContext);

    AutoCheckFlush acf(this->drawingManager());

    // Functionally this is very similar to drawFilledQuad except that there's no constColor to
    // enable the kSubmitted optimizations, no stencil settings support, and its a GrTextureOp.
    GrQuad croppedDeviceQuad = deviceQuad;
    GrQuad croppedLocalQuad = localQuad;
    QuadOptimization opt = this->attemptQuadOptimization(clip, nullptr, nullptr, &aa, &edgeFlags,
                                                         &croppedDeviceQuad, &croppedLocalQuad);

    SkASSERT(opt != QuadOptimization::kSubmitted);
    if (opt != QuadOptimization::kDiscarded) {
        // And the texture op if not discarded
        const GrClip& finalClip = opt == QuadOptimization::kClipApplied ? GrFixedClip::Disabled()
                                                                        : clip;
        GrAAType aaType = this->chooseAAType(aa);
        // Use the provided domain, although hypothetically we could detect that the cropped local
        // quad is sufficiently inside the domain and the constraint could be dropped.
        this->addDrawOp(finalClip, GrTextureOp::Make(fContext, std::move(proxy),
                                                     std::move(textureXform), filter, color,
                                                     blendMode, aaType, edgeFlags,
                                                     croppedDeviceQuad, croppedLocalQuad, domain));
    }
}

void GrRenderTargetContext::drawRect(const GrClip& clip,
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
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawRect", fContext);

    // Path effects should've been devolved to a path in SkGpuDevice
    SkASSERT(!style->pathEffect());

    AutoCheckFlush acf(this->drawingManager());

    const SkStrokeRec& stroke = style->strokeRec();
    if (stroke.getStyle() == SkStrokeRec::kFill_Style) {
        // Fills the rect, using rect as its own local coordinates
        this->fillRectToRect(clip, std::move(paint), aa, viewMatrix, rect, rect);
        return;
    } else if (stroke.getStyle() == SkStrokeRec::kStroke_Style ||
               stroke.getStyle() == SkStrokeRec::kHairline_Style) {
        if ((!rect.width() || !rect.height()) &&
            SkStrokeRec::kHairline_Style != stroke.getStyle()) {
            SkScalar r = stroke.getWidth() / 2;
            // TODO: Move these stroke->fill fallbacks to GrShape?
            switch (stroke.getJoin()) {
                case SkPaint::kMiter_Join:
                    this->drawRect(
                            clip, std::move(paint), aa, viewMatrix,
                            {rect.fLeft - r, rect.fTop - r, rect.fRight + r, rect.fBottom + r},
                            &GrStyle::SimpleFill());
                    return;
                case SkPaint::kRound_Join:
                    // Raster draws nothing when both dimensions are empty.
                    if (rect.width() || rect.height()){
                        SkRRect rrect = SkRRect::MakeRectXY(rect.makeOutset(r, r), r, r);
                        this->drawRRect(clip, std::move(paint), aa, viewMatrix, rrect,
                                        GrStyle::SimpleFill());
                        return;
                    }
                case SkPaint::kBevel_Join:
                    if (!rect.width()) {
                        this->drawRect(clip, std::move(paint), aa, viewMatrix,
                                       {rect.fLeft - r, rect.fTop, rect.fRight + r, rect.fBottom},
                                       &GrStyle::SimpleFill());
                    } else {
                        this->drawRect(clip, std::move(paint), aa, viewMatrix,
                                       {rect.fLeft, rect.fTop - r, rect.fRight, rect.fBottom + r},
                                       &GrStyle::SimpleFill());
                    }
                    return;
                }
        }

        std::unique_ptr<GrDrawOp> op;

        GrAAType aaType = this->chooseAAType(aa);
        op = GrStrokeRectOp::Make(fContext, std::move(paint), aaType, viewMatrix, rect, stroke);
        // op may be null if the stroke is not supported or if using coverage aa and the view matrix
        // does not preserve rectangles.
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
    }
    assert_alive(paint);
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix, GrShape(rect, *style));
}

void GrRenderTargetContext::drawQuadSet(const GrClip& clip, GrPaint&& paint, GrAA aa,
                                        const SkMatrix& viewMatrix, const QuadSetEntry quads[],
                                        int cnt) {
    GrAAType aaType = this->chooseAAType(aa);
    this->addDrawOp(clip, GrFillRectOp::MakeSet(fContext, std::move(paint), aaType, viewMatrix,
                                                quads, cnt));
}

int GrRenderTargetContextPriv::maxWindowRectangles() const {
    return fRenderTargetContext->fRenderTargetProxy->maxWindowRectangles(
            *fRenderTargetContext->caps());
}

GrRenderTargetOpList::CanDiscardPreviousOps GrRenderTargetContext::canDiscardPreviousOpsOnFullClear(
        ) const {
#if GR_TEST_UTILS
    if (fPreserveOpsOnFullClear_TestingOnly) {
        return GrRenderTargetOpList::CanDiscardPreviousOps::kNo;
    }
#endif
    // Regardless of how the clear is implemented (native clear or a fullscreen quad), all prior ops
    // would normally be overwritten. The one exception is if the render target context is marked as
    // needing a stencil buffer then there may be a prior op that writes to the stencil buffer.
    // Although the clear will ignore the stencil buffer, following draw ops may not so we can't get
    // rid of all the preceding ops. Beware! If we ever add any ops that have a side effect beyond
    // modifying the stencil buffer we will need a more elaborate tracking system (skbug.com/7002).
    return GrRenderTargetOpList::CanDiscardPreviousOps(!fNumStencilSamples);
}

void GrRenderTargetContext::setNeedsStencil(bool multisampled) {
    // Don't clear stencil until after we've changed fNumStencilSamples. This ensures we don't loop
    // forever in the event that there are driver bugs and we need to clear as a draw.
    bool needsStencilClear = !fNumStencilSamples;

    int numRequiredSamples = this->numSamples();
    if (multisampled && 1 == numRequiredSamples) {
        // The caller has requested a multisampled stencil buffer on a non-MSAA render target. Use
        // mixed samples.
        SkASSERT(fRenderTargetProxy->canUseMixedSamples(*this->caps()));
        numRequiredSamples = this->caps()->internalMultisampleCount(
                this->asSurfaceProxy()->backendFormat());
    }
    SkASSERT(numRequiredSamples > 0);

    if (numRequiredSamples > fNumStencilSamples) {
        fNumStencilSamples = numRequiredSamples;
        fRenderTargetProxy->setNeedsStencil(fNumStencilSamples);
    }

    if (needsStencilClear) {
        if (this->caps()->performStencilClearsAsDraws()) {
            // There is a driver bug with clearing stencil. We must use an op to manually clear the
            // stencil buffer before the op that required 'setNeedsStencil'.
            this->internalStencilClear(GrFixedClip::Disabled(), /* inside mask */ false);
        } else {
            // Setting the clear stencil load op is preferable. On non-tilers, this lets the flush
            // code note when the instantiated stencil buffer is already clear and skip the clear
            // altogether. And on tilers, loading the stencil buffer cleared is even faster than
            // preserving the previous contents.
            this->getRTOpList()->setStencilLoadOp(GrLoadOp::kClear);
        }
    }
}

void GrRenderTargetContextPriv::clearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "clearStencilClip",
                                   fRenderTargetContext->fContext);

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    fRenderTargetContext->internalStencilClear(clip, insideStencilMask);
}

void GrRenderTargetContext::internalStencilClear(const GrFixedClip& clip, bool insideStencilMask) {
    if (this->caps()->performStencilClearsAsDraws()) {
        const GrUserStencilSettings* ss = GrStencilSettings::SetClipBitSettings(insideStencilMask);
        SkRect rtRect = SkRect::MakeWH(this->width(), this->height());

        // Configure the paint to have no impact on the color buffer
        GrPaint paint;
        paint.setXPFactory(GrDisableColorXPFactory::Get());
        this->addDrawOp(clip, GrFillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                                          rtRect, ss));
    } else {
        std::unique_ptr<GrOp> op(GrClearStencilClipOp::Make(fContext, clip, insideStencilMask,
                                                            fRenderTargetProxy.get()));
        if (!op) {
            return;
        }
        this->addOp(std::move(op));
    }
}

void GrRenderTargetContextPriv::stencilPath(const GrHardClip& clip,
                                            GrAA doStencilMSAA,
                                            const SkMatrix& viewMatrix,
                                            sk_sp<const GrPath> path) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "stencilPath",
                                   fRenderTargetContext->fContext);

    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(path);
    SkASSERT(fRenderTargetContext->caps()->shaderCaps()->pathRenderingSupport());

    // FIXME: Use path bounds instead of this WAR once
    // https://bugs.chromium.org/p/skia/issues/detail?id=5640 is resolved.
    SkRect bounds = SkRect::MakeIWH(fRenderTargetContext->width(), fRenderTargetContext->height());

    // Setup clip
    GrAppliedHardClip appliedClip;
    if (!clip.apply(fRenderTargetContext->width(), fRenderTargetContext->height(), &appliedClip,
                    &bounds)) {
        return;
    }

    std::unique_ptr<GrOp> op = GrStencilPathOp::Make(fRenderTargetContext->fContext,
                                                     viewMatrix,
                                                     GrAA::kYes == doStencilMSAA,
                                                     appliedClip.hasStencilClip(),
                                                     appliedClip.scissorState(),
                                                     std::move(path));
    if (!op) {
        return;
    }
    op->setClippedBounds(bounds);

    fRenderTargetContext->setNeedsStencil(GrAA::kYes == doStencilMSAA);
    fRenderTargetContext->addOp(std::move(op));
}

void GrRenderTargetContext::drawTextureSet(const GrClip& clip, const TextureSetEntry set[], int cnt,
                                           GrSamplerState::Filter filter, SkBlendMode mode,
                                           GrAA aa, SkCanvas::SrcRectConstraint constraint,
                                           const SkMatrix& viewMatrix,
                                           sk_sp<GrColorSpaceXform> texXform) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawTextureSet", fContext);

    if (mode != SkBlendMode::kSrcOver ||
        !fContext->priv().caps()->dynamicStateArrayGeometryProcessorTextureSupport()) {
        // Draw one at a time since the bulk API doesn't support non src-over blending, or the
        // backend can't support the bulk geometry processor yet.
        SkMatrix ctm;
        for (int i = 0; i < cnt; ++i) {
            float alpha = set[i].fAlpha;
            ctm = viewMatrix;
            if (set[i].fPreViewMatrix) {
                ctm.preConcat(*set[i].fPreViewMatrix);
            }

            GrQuad quad, srcQuad;
            if (set[i].fDstClipQuad) {
                quad = GrQuad::MakeFromSkQuad(set[i].fDstClipQuad, ctm);

                SkPoint srcPts[4];
                GrMapRectPoints(set[i].fDstRect, set[i].fSrcRect, set[i].fDstClipQuad, srcPts, 4);
                srcQuad = GrQuad::MakeFromSkQuad(srcPts, SkMatrix::I());
            } else {
                quad = GrQuad::MakeFromRect(set[i].fDstRect, ctm);
                srcQuad = GrQuad(set[i].fSrcRect);
            }

            const SkRect* domain = constraint == SkCanvas::kStrict_SrcRectConstraint
                    ? &set[i].fSrcRect : nullptr;
            this->drawTexturedQuad(clip, set[i].fProxy, texXform, filter,
                                   {alpha, alpha, alpha, alpha}, mode, aa, set[i].fAAFlags,
                                   quad, srcQuad, domain);
        }
    } else {
        // Can use a single op, avoiding GrPaint creation, and can batch across proxies
        AutoCheckFlush acf(this->drawingManager());
        GrAAType aaType = this->chooseAAType(aa);
        auto op = GrTextureOp::MakeSet(fContext, set, cnt, filter, aaType, constraint, viewMatrix,
                                       std::move(texXform));
        this->addDrawOp(clip, std::move(op));
    }
}

void GrRenderTargetContext::drawVertices(const GrClip& clip,
                                         GrPaint&& paint,
                                         const SkMatrix& viewMatrix,
                                         sk_sp<SkVertices> vertices,
                                         const SkVertices::Bone bones[],
                                         int boneCount,
                                         GrPrimitiveType* overridePrimType) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawVertices", fContext);

    AutoCheckFlush acf(this->drawingManager());

    SkASSERT(vertices);
    GrAAType aaType = this->chooseAAType(GrAA::kNo);
    std::unique_ptr<GrDrawOp> op = GrDrawVerticesOp::Make(
            fContext, std::move(paint), std::move(vertices), bones, boneCount, viewMatrix, aaType,
            this->colorSpaceInfo().refColorSpaceXformFromSRGB(), overridePrimType);
    this->addDrawOp(clip, std::move(op));
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetContext::drawAtlas(const GrClip& clip,
                                      GrPaint&& paint,
                                      const SkMatrix& viewMatrix,
                                      int spriteCount,
                                      const SkRSXform xform[],
                                      const SkRect texRect[],
                                      const SkColor colors[]) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawAtlas", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(GrAA::kNo);
    std::unique_ptr<GrDrawOp> op = GrDrawAtlasOp::Make(fContext, std::move(paint), viewMatrix,
                                                       aaType, spriteCount, xform, texRect, colors);
    this->addDrawOp(clip, std::move(op));
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetContext::drawRRect(const GrClip& origClip,
                                      GrPaint&& paint,
                                      GrAA aa,
                                      const SkMatrix& viewMatrix,
                                      const SkRRect& rrect,
                                      const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawRRect", fContext);

    const SkStrokeRec& stroke = style.strokeRec();
    if (stroke.getStyle() == SkStrokeRec::kFill_Style && rrect.isEmpty()) {
       return;
    }

    GrNoClip noclip;
    const GrClip* clip = &origClip;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // The Android framework frequently clips rrects to themselves where the clip is non-aa and the
    // draw is aa. Since our lower level clip code works from op bounds, which are SkRects, it
    // doesn't detect that the clip can be ignored (modulo antialiasing). The following test
    // attempts to mitigate the stencil clip cost but will only help when the entire clip stack
    // can be ignored. We'd prefer to fix this in the framework by removing the clips calls. This
    // only works for filled rrects since the stroke width outsets beyond the rrect itself.
    SkRRect devRRect;
    if (stroke.getStyle() == SkStrokeRec::kFill_Style && rrect.transform(viewMatrix, &devRRect) &&
        clip->quickContains(devRRect)) {
        clip = &noclip;
    }
#endif
    SkASSERT(!style.pathEffect()); // this should've been devolved to a path in SkGpuDevice

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa);

    std::unique_ptr<GrDrawOp> op;
    if (GrAAType::kCoverage == aaType && rrect.isSimple() &&
        rrect.getSimpleRadii().fX == rrect.getSimpleRadii().fY &&
        viewMatrix.rectStaysRect() && viewMatrix.isSimilarity()) {
        // In coverage mode, we draw axis-aligned circular roundrects with the GrOvalOpFactory
        // to avoid perf regressions on some platforms.
        assert_alive(paint);
        op = GrOvalOpFactory::MakeCircularRRectOp(
                fContext, std::move(paint), viewMatrix, rrect, stroke, this->caps()->shaderCaps());
    }
    if (!op && style.isSimpleFill()) {
        assert_alive(paint);
        op = GrFillRRectOp::Make(
                fContext, aaType, viewMatrix, rrect, *this->caps(), std::move(paint));
    }
    if (!op && GrAAType::kCoverage == aaType) {
        assert_alive(paint);
        op = GrOvalOpFactory::MakeRRectOp(
                fContext, std::move(paint), viewMatrix, rrect, stroke, this->caps()->shaderCaps());
    }
    if (op) {
        this->addDrawOp(*clip, std::move(op));
        return;
    }

    assert_alive(paint);
    this->drawShapeUsingPathRenderer(*clip, std::move(paint), aa, viewMatrix,
                                     GrShape(rrect, style));
}

///////////////////////////////////////////////////////////////////////////////

static SkPoint3 map(const SkMatrix& m, const SkPoint3& pt) {
    SkPoint3 result;
    m.mapXY(pt.fX, pt.fY, (SkPoint*)&result.fX);
    result.fZ = pt.fZ;
    return result;
}

bool GrRenderTargetContext::drawFastShadow(const GrClip& clip,
                                           const SkMatrix& viewMatrix,
                                           const SkPath& path,
                                           const SkDrawShadowRec& rec) {
    ASSERT_SINGLE_OWNER
    if (fContext->priv().abandoned()) {
        return true;
    }
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawFastShadow", fContext);

    // check z plane
    bool tiltZPlane = SkToBool(!SkScalarNearlyZero(rec.fZPlaneParams.fX) ||
                               !SkScalarNearlyZero(rec.fZPlaneParams.fY));
    bool skipAnalytic = SkToBool(rec.fFlags & SkShadowFlags::kGeometricOnly_ShadowFlag);
    if (tiltZPlane || skipAnalytic || !viewMatrix.rectStaysRect() || !viewMatrix.isSimilarity()) {
        return false;
    }

    SkRRect rrect;
    SkRect rect;
    // we can only handle rects, circles, and rrects with circular corners
    bool isRRect = path.isRRect(&rrect) && SkRRectPriv::IsSimpleCircular(rrect) &&
        rrect.radii(SkRRect::kUpperLeft_Corner).fX > SK_ScalarNearlyZero;
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

    // transform light
    SkPoint3 devLightPos = map(viewMatrix, rec.fLightPos);

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

        std::unique_ptr<GrDrawOp> op = GrShadowRRectOp::Make(fContext,
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
        SkDrawShadowMetrics::GetSpotParams(occluderHeight, devLightPos.fX, devLightPos.fY,
                                           devLightPos.fZ, rec.fLightRadius,
                                           &devSpaceSpotBlur, &spotScale, &spotOffset);
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
        SkScalar spotRadius = SkRRectPriv::GetSimpleRadii(spotShadowRRect).fX;

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
                maxOffset = SkTMax(SkTMax(SkTAbs(spotShadowRRect.rect().fLeft -
                                                 rrect.rect().fLeft),
                                          SkTAbs(spotShadowRRect.rect().fTop -
                                                 rrect.rect().fTop)),
                                   SkTMax(SkTAbs(spotShadowRRect.rect().fRight -
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
                maxOffset = SkScalarSqrt(SkTMax(SkPointPriv::LengthSqd(upperLeftOffset),
                                                SkPointPriv::LengthSqd(lowerRightOffset))) + dr;
            }
            insetWidth += SkTMax(blurOutset, maxOffset);
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

        std::unique_ptr<GrDrawOp> op = GrShadowRRectOp::Make(fContext,
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

bool GrRenderTargetContext::drawFilledDRRect(const GrClip& clip,
                                             GrPaint&& paint,
                                             GrAA aa,
                                             const SkMatrix& viewMatrix,
                                             const SkRRect& origOuter,
                                             const SkRRect& origInner) {
    SkASSERT(!origInner.isEmpty());
    SkASSERT(!origOuter.isEmpty());

    SkTCopyOnFirstWrite<SkRRect> inner(origInner), outer(origOuter);

    GrAAType aaType = this->chooseAAType(aa);

    if (GrAAType::kMSAA == aaType) {
        return false;
    }

    if (GrAAType::kCoverage == aaType && SkRRectPriv::IsCircle(*inner)
                                      && SkRRectPriv::IsCircle(*outer)) {
        auto outerR = outer->width() / 2.f;
        auto innerR = inner->width() / 2.f;
        auto cx = outer->getBounds().fLeft + outerR;
        auto cy = outer->getBounds().fTop + outerR;
        if (SkScalarNearlyEqual(cx, inner->getBounds().fLeft + innerR) &&
            SkScalarNearlyEqual(cy, inner->getBounds().fTop + innerR)) {
            auto avgR = (innerR + outerR) / 2.f;
            auto circleBounds = SkRect::MakeLTRB(cx - avgR, cy - avgR, cx + avgR, cy + avgR);
            SkStrokeRec stroke(SkStrokeRec::kFill_InitStyle);
            stroke.setStrokeStyle(outerR - innerR);
            auto op = GrOvalOpFactory::MakeOvalOp(fContext, std::move(paint), viewMatrix,
                                                  circleBounds, GrStyle(stroke, nullptr),
                                                  this->caps()->shaderCaps());
            if (op) {
                this->addDrawOp(clip, std::move(op));
                return true;
            }
            assert_alive(paint);
        }
    }

    GrClipEdgeType innerEdgeType, outerEdgeType;
    if (GrAAType::kCoverage == aaType) {
        innerEdgeType = GrClipEdgeType::kInverseFillAA;
        outerEdgeType = GrClipEdgeType::kFillAA;
    } else {
        innerEdgeType = GrClipEdgeType::kInverseFillBW;
        outerEdgeType = GrClipEdgeType::kFillBW;
    }

    SkMatrix inverseVM;
    if (!viewMatrix.isIdentity()) {
        if (!origInner.transform(viewMatrix, inner.writable())) {
            return false;
        }
        if (!origOuter.transform(viewMatrix, outer.writable())) {
            return false;
        }
        if (!viewMatrix.invert(&inverseVM)) {
            return false;
        }
    } else {
        inverseVM.reset();
    }

    const auto& caps = *this->caps()->shaderCaps();
    // TODO these need to be a geometry processors
    auto innerEffect = GrRRectEffect::Make(innerEdgeType, *inner, caps);
    if (!innerEffect) {
        return false;
    }

    auto outerEffect = GrRRectEffect::Make(outerEdgeType, *outer, caps);
    if (!outerEffect) {
        return false;
    }

    paint.addCoverageFragmentProcessor(std::move(innerEffect));
    paint.addCoverageFragmentProcessor(std::move(outerEffect));

    SkRect bounds = outer->getBounds();
    if (GrAAType::kCoverage == aaType) {
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);
    }

    this->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(), bounds,
                                  inverseVM);
    return true;
}

void GrRenderTargetContext::drawDRRect(const GrClip& clip,
                                       GrPaint&& paint,
                                       GrAA aa,
                                       const SkMatrix& viewMatrix,
                                       const SkRRect& outer,
                                       const SkRRect& inner) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawDRRect", fContext);

    SkASSERT(!outer.isEmpty());
    SkASSERT(!inner.isEmpty());

    AutoCheckFlush acf(this->drawingManager());

    if (this->drawFilledDRRect(clip, std::move(paint), aa, viewMatrix, outer, inner)) {
        return;
    }
    assert_alive(paint);

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(inner);
    path.addRRect(outer);
    path.setFillType(SkPath::kEvenOdd_FillType);
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix, GrShape(path));
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetContext::drawRegion(const GrClip& clip,
                                       GrPaint&& paint,
                                       GrAA aa,
                                       const SkMatrix& viewMatrix,
                                       const SkRegion& region,
                                       const GrStyle& style,
                                       const GrUserStencilSettings* ss) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawRegion", fContext);

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

    GrAAType aaType = this->chooseAAType(GrAA::kNo);
    std::unique_ptr<GrDrawOp> op = GrRegionOp::Make(fContext, std::move(paint), viewMatrix, region,
                                                    aaType, ss);
    this->addDrawOp(clip, std::move(op));
}

void GrRenderTargetContext::drawOval(const GrClip& clip,
                                     GrPaint&& paint,
                                     GrAA aa,
                                     const SkMatrix& viewMatrix,
                                     const SkRect& oval,
                                     const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawOval", fContext);

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

    std::unique_ptr<GrDrawOp> op;
    if (GrAAType::kCoverage == aaType && oval.width() > SK_ScalarNearlyZero &&
        oval.width() == oval.height() && viewMatrix.isSimilarity()) {
        // We don't draw true circles as round rects in coverage mode, because it can
        // cause perf regressions on some platforms as compared to the dedicated circle Op.
        assert_alive(paint);
        op = GrOvalOpFactory::MakeCircleOp(fContext, std::move(paint), viewMatrix, oval, style,
                                           this->caps()->shaderCaps());
    }
    if (!op && style.isSimpleFill()) {
        // GrFillRRectOp has special geometry and a fragment-shader branch to conditionally evaluate
        // the arc equation. This same special geometry and fragment branch also turn out to be a
        // substantial optimization for drawing ovals (namely, by not evaluating the arc equation
        // inside the oval's inner diamond). Given these optimizations, it's a clear win to draw
        // ovals the exact same way we do round rects.
        assert_alive(paint);
        op = GrFillRRectOp::Make(fContext, aaType, viewMatrix, SkRRect::MakeOval(oval),
                                 *this->caps(), std::move(paint));
    }
    if (!op && GrAAType::kCoverage == aaType) {
        assert_alive(paint);
        op = GrOvalOpFactory::MakeOvalOp(fContext, std::move(paint), viewMatrix, oval, style,
                                         this->caps()->shaderCaps());
    }
    if (op) {
        this->addDrawOp(clip, std::move(op));
        return;
    }

    assert_alive(paint);
    this->drawShapeUsingPathRenderer(
            clip, std::move(paint), aa, viewMatrix,
            GrShape(SkRRect::MakeOval(oval), SkPath::kCW_Direction, 2, false, style));
}

void GrRenderTargetContext::drawArc(const GrClip& clip,
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
            GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawArc", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa);
    if (GrAAType::kCoverage == aaType) {
        const GrShaderCaps* shaderCaps = this->caps()->shaderCaps();
        std::unique_ptr<GrDrawOp> op = GrOvalOpFactory::MakeArcOp(fContext,
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
    this->drawShapeUsingPathRenderer(
            clip, std::move(paint), aa, viewMatrix,
            GrShape::MakeArc(oval, startAngle, sweepAngle, useCenter, style));
}

void GrRenderTargetContext::drawImageLattice(const GrClip& clip,
                                             GrPaint&& paint,
                                             const SkMatrix& viewMatrix,
                                             sk_sp<GrTextureProxy> image,
                                             sk_sp<GrColorSpaceXform> csxf,
                                             GrSamplerState::Filter filter,
                                             std::unique_ptr<SkLatticeIter> iter,
                                             const SkRect& dst) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawImageLattice", fContext);

    AutoCheckFlush acf(this->drawingManager());

    std::unique_ptr<GrDrawOp> op =
            GrLatticeOp::MakeNonAA(fContext, std::move(paint), viewMatrix, std::move(image),
                                   std::move(csxf), filter, std::move(iter), dst);
    this->addDrawOp(clip, std::move(op));
}

void GrRenderTargetContext::drawDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
                                         const SkRect& bounds) {
    std::unique_ptr<GrOp> op(GrDrawableOp::Make(fContext, std::move(drawable), bounds));
    SkASSERT(op);
    this->addOp(std::move(op));
}

void GrRenderTargetContext::asyncRescaleAndReadPixels(
        const SkImageInfo& info, const SkIRect& srcRect, SkSurface::RescaleGamma rescaleGamma,
        SkFilterQuality rescaleQuality, ReadPixelsCallback callback, ReadPixelsContext context) {
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        callback(context, nullptr, 0);
        return;
    }
    if (fRenderTargetProxy->wrapsVkSecondaryCB()) {
        callback(context, nullptr, 0);
        return;
    }
    auto dstCT = SkColorTypeToGrColorType(info.colorType());
    bool needsRescale = srcRect.width() != info.width() || srcRect.height() != info.height();
    auto colorTypeOfFinalContext = this->colorSpaceInfo().colorType();
    auto backendFormatOfFinalContext = fRenderTargetProxy->backendFormat();
    if (needsRescale) {
        colorTypeOfFinalContext = dstCT;
        backendFormatOfFinalContext = this->caps()->getDefaultBackendFormat(dstCT,
                                                                            GrRenderable::kYes);
    }
    auto readInfo = this->caps()->supportedReadPixelsColorType(colorTypeOfFinalContext,
                                                               backendFormatOfFinalContext, dstCT);
    // Fail if we can't read from the source surface's color type.
    if (readInfo.fColorType == GrColorType::kUnknown) {
        callback(context, nullptr, 0);
        return;
    }
    // Fail if read color type does not have all of dstCT's color channels and those missing color
    // channels are in the src.
    uint32_t dstComponents = GrColorTypeComponentFlags(dstCT);
    uint32_t legalReadComponents = GrColorTypeComponentFlags(readInfo.fColorType);
    uint32_t srcComponents = GrColorTypeComponentFlags(this->colorSpaceInfo().colorType());
    if ((~legalReadComponents & dstComponents) & srcComponents) {
        callback(context, nullptr, 0);
        return;
    }

    std::unique_ptr<GrRenderTargetContext> tempRTC;
    int x = srcRect.fLeft;
    int y = srcRect.fTop;
    if (needsRescale) {
        tempRTC = this->rescale(info, srcRect, rescaleGamma, rescaleQuality);
        if (!tempRTC) {
            callback(context, nullptr, 0);
            return;
        }
        SkASSERT(SkColorSpace::Equals(tempRTC->colorSpaceInfo().colorSpace(), info.colorSpace()));
        SkASSERT(tempRTC->origin() == kTopLeft_GrSurfaceOrigin);
        x = y = 0;
    } else {
        sk_sp<GrColorSpaceXform> xform =
                GrColorSpaceXform::Make(this->colorSpaceInfo().colorSpace(),
                                        this->colorSpaceInfo().alphaType(),
                                        info.colorSpace(), info.alphaType());
        // Insert a draw to a temporary surface if we need to do a y-flip or color space conversion.
        if (this->origin() == kBottomLeft_GrSurfaceOrigin || xform) {
            // We flip or color convert by drawing and we don't currently support drawing to
            // kPremul.
            if (info.alphaType() == kUnpremul_SkAlphaType) {
                callback(context, nullptr, 0);
                return;
            }
            sk_sp<GrTextureProxy> texProxy = sk_ref_sp(fRenderTargetProxy->asTextureProxy());
            SkRect srcRectToDraw = SkRect::Make(srcRect);
            // If the src is not texturable first try to make a copy to a texture.
            if (!texProxy) {
                texProxy = GrSurfaceProxy::Copy(fContext, fRenderTargetProxy.get(),
                                                GrMipMapped::kNo, srcRect, SkBackingFit::kApprox,
                                                SkBudgeted::kNo);
                if (!texProxy) {
                    callback(context, nullptr, 0);
                    return;
                }
                srcRectToDraw = SkRect::MakeWH(srcRect.width(), srcRect.height());
            }
            tempRTC = direct->priv().makeDeferredRenderTargetContext(
                    SkBackingFit::kApprox, srcRect.width(), srcRect.height(),
                    this->colorSpaceInfo().colorType(), info.refColorSpace(), 1, GrMipMapped::kNo,
                    kTopLeft_GrSurfaceOrigin);
            if (!tempRTC) {
                callback(context, nullptr, 0);
                return;
            }
            tempRTC->drawTexture(GrNoClip(), std::move(texProxy), GrSamplerState::Filter::kNearest,
                                 SkBlendMode::kSrc, SK_PMColor4fWHITE, srcRectToDraw,
                                 SkRect::MakeWH(srcRect.width(), srcRect.height()), GrAA::kNo,
                                 GrQuadAAFlags::kNone, SkCanvas::kFast_SrcRectConstraint,
                                 SkMatrix::I(), std::move(xform));
            x = y = 0;
        }
    }
    auto rtc = tempRTC ? tempRTC.get() : this;
    return rtc->asyncReadPixels(SkIRect::MakeXYWH(x, y, info.width(), info.height()),
                                info.colorType(), callback, context);
}

void GrRenderTargetContext::asyncReadPixels(const SkIRect& rect, SkColorType colorType,
                                            ReadPixelsCallback callback,
                                            ReadPixelsContext context) {
    SkASSERT(rect.fLeft >= 0 && rect.fRight <= this->width());
    SkASSERT(rect.fTop >= 0 && rect.fBottom <= this->height());

    auto transferResult = this->transferPixels(SkColorTypeToGrColorType(colorType), rect);

    if (!transferResult.fTransferBuffer) {
        SkAutoPixmapStorage pm;
        auto ii = SkImageInfo::Make(rect.width(), rect.height(), colorType,
                                    this->colorSpaceInfo().alphaType(),
                                    this->colorSpaceInfo().refColorSpace());
        pm.alloc(ii);
        if (!this->readPixels(ii, pm.writable_addr(), pm.rowBytes(), {rect.fLeft, rect.fTop})) {
            callback(context, nullptr, 0);
        }
        callback(context, pm.addr(), pm.rowBytes());
        return;
    }

    struct FinishContext {
        ReadPixelsCallback* fClientCallback;
        ReadPixelsContext fClientContext;
        int fW, fH;
        SkColorType fColorType;
        PixelTransferResult fTransferResult;
    };
    // Assumption is that the caller would like to flush. We could take a parameter or require an
    // explicit flush from the caller. We'd have to have a way to defer attaching the finish
    // callback to GrGpu until after the next flush that flushes our op list, though.
    auto* finishContext = new FinishContext{callback, context, rect.width(),
                                            rect.height(), colorType, std::move(transferResult)};
    auto finishCallback = [](GrGpuFinishedContext c) {
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        const void* data = context->fTransferResult.fTransferBuffer->map();
        if (!data) {
            (*context->fClientCallback)(context->fClientContext, nullptr, 0);
            delete context;
            return;
        }
        std::unique_ptr<char[]> tmp;
        size_t rowBytes = context->fW * SkColorTypeBytesPerPixel(context->fColorType);
        if (context->fTransferResult.fPixelConverter) {
            tmp.reset(new char[rowBytes * context->fH]);
            context->fTransferResult.fPixelConverter(tmp.get(), data);
            data = tmp.get();
        }
        (*context->fClientCallback)(context->fClientContext, data, rowBytes);
        delete context;
    };
    GrFlushInfo flushInfo;
    flushInfo.fFinishedContext = finishContext;
    flushInfo.fFinishedProc = finishCallback;
    this->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
}

void GrRenderTargetContext::asyncRescaleAndReadPixelsYUV420(
        SkYUVColorSpace yuvColorSpace, sk_sp<SkColorSpace> dstColorSpace, const SkIRect& srcRect,
        int dstW, int dstH, RescaleGamma rescaleGamma, SkFilterQuality rescaleQuality,
        ReadPixelsCallbackYUV420 callback, ReadPixelsContext context) {
    SkASSERT(srcRect.fLeft >= 0 && srcRect.fRight <= this->width());
    SkASSERT(srcRect.fTop >= 0 && srcRect.fBottom <= this->height());
    SkASSERT((dstW % 2 == 0) && (dstH % 2 == 0));
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        callback(context, nullptr, nullptr);
        return;
    }
    if (fRenderTargetProxy->wrapsVkSecondaryCB()) {
        callback(context, nullptr, nullptr);
        return;
    }
    if (dstW & 0x1) {
        return;
    }
    int x = srcRect.fLeft;
    int y = srcRect.fTop;
    std::unique_ptr<GrRenderTargetContext> tempRTC;
    bool needsRescale = srcRect.width() != dstW || srcRect.height() != dstH;
    if (needsRescale) {
        // We assume the caller wants kPremul. There is no way to indicate a preference.
        auto info = SkImageInfo::Make(dstW, dstH, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                      dstColorSpace);
        // TODO: Incorporate the YUV conversion into last pass of rescaling.
        tempRTC = this->rescale(info, srcRect, rescaleGamma, rescaleQuality);
        if (!tempRTC) {
            callback(context, nullptr, nullptr);
            return;
        }
        SkASSERT(SkColorSpace::Equals(tempRTC->colorSpaceInfo().colorSpace(), info.colorSpace()));
        SkASSERT(tempRTC->origin() == kTopLeft_GrSurfaceOrigin);
        x = y = 0;
    } else {
        // We assume the caller wants kPremul. There is no way to indicate a preference.
        sk_sp<GrColorSpaceXform> xform = GrColorSpaceXform::Make(
                this->colorSpaceInfo().colorSpace(), this->colorSpaceInfo().alphaType(),
                dstColorSpace.get(), kPremul_SkAlphaType);
        if (xform) {
            sk_sp<GrTextureProxy> texProxy = this->asTextureProxyRef();
            // TODO: Do something if the input is not a texture already.
            if (!texProxy) {
                callback(context, nullptr, nullptr);
                return;
            }
            SkRect srcRectToDraw = SkRect::Make(srcRect);
            tempRTC = direct->priv().makeDeferredRenderTargetContext(
                    SkBackingFit::kApprox, dstW, dstH, this->colorSpaceInfo().colorType(),
                    dstColorSpace, 1, GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin);
            if (!tempRTC) {
                callback(context, nullptr, nullptr);
                return;
            }
            tempRTC->drawTexture(GrNoClip(), std::move(texProxy), GrSamplerState::Filter::kNearest,
                                 SkBlendMode::kSrc, SK_PMColor4fWHITE, srcRectToDraw,
                                 SkRect::MakeWH(srcRect.width(), srcRect.height()), GrAA::kNo,
                                 GrQuadAAFlags::kNone, SkCanvas::kFast_SrcRectConstraint,
                                 SkMatrix::I(), std::move(xform));
            x = y = 0;
        }
    }
    auto srcProxy = tempRTC ? tempRTC->asTextureProxyRef() : this->asTextureProxyRef();
    // TODO: Do something if the input is not a texture already.
    if (!srcProxy) {
        callback(context, nullptr, nullptr);
        return;
    }
    auto yRTC = direct->priv().makeDeferredRenderTargetContextWithFallback(
            SkBackingFit::kApprox, dstW, dstH, GrColorType::kAlpha_8, dstColorSpace, 1,
            GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin);
    auto uRTC = direct->priv().makeDeferredRenderTargetContextWithFallback(
            SkBackingFit::kApprox, dstW / 2, dstH / 2, GrColorType::kAlpha_8, dstColorSpace, 1,
            GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin);
    auto vRTC = direct->priv().makeDeferredRenderTargetContextWithFallback(
            SkBackingFit::kApprox, dstW / 2, dstH / 2, GrColorType::kAlpha_8, dstColorSpace, 1,
            GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin);
    if (!yRTC || !uRTC || !vRTC) {
        callback(context, nullptr, nullptr);
        return;
    }

    static constexpr float kRec601M[] {
             65.481f / 255, 128.553f / 255,  24.966f / 255,  16.f / 255,   // y
            -37.797f / 255, -74.203f / 255, 112.0f   / 255, 128.f / 255,  // u
            112.f    / 255, -93.786f / 255, -18.214f / 255, 128.f / 255,  // v
    };
    static constexpr float kRec709M[] {
             45.5594f / 255,  156.6288f / 255,  15.8118f / 255,  16.f / 255, // y
            -25.6642f / 255,  -86.3358f / 255, 112.f     / 255, 128.f / 255,  // u
            112.f     / 255, -101.7303f / 255, -10.2697f / 255, 128.f / 255,  // v
    };
    static constexpr float kJpegM[] {
             0.299f   ,  0.587f   ,  0.114f   ,   0.f / 255,  // y
            -0.168736f, -0.331264f,  0.5f     , 128.f / 255,  // u
             0.5f     , -0.418688f, -0.081312f, 128.f / 255,  // v
    };
    static constexpr float kIM[] {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
    };
    const float* baseM = kIM;
    switch (yuvColorSpace) {
        case kRec601_SkYUVColorSpace:
            baseM = kRec601M;
            break;
        case kRec709_SkYUVColorSpace:
            baseM = kRec709M;
            break;
        case kJPEG_SkYUVColorSpace:
            baseM = kJpegM;
            break;
        case kIdentity_SkYUVColorSpace:
            baseM = kIM;
            break;
    }
    // TODO: Use one transfer buffer for all three planes to reduce map/unmap cost?

    auto texMatrix = SkMatrix::MakeTrans(x, y);

    SkRect dstRectY = SkRect::MakeWH(dstW, dstH);
    SkRect dstRectUV = SkRect::MakeWH(dstW / 2, dstH / 2);

    // This matrix generates (r,g,b,a) = (0, 0, 0, y)
    float yM[20];
    std::fill_n(yM, 15, 0.f);
    yM[15] = baseM[0]; yM[16] = baseM[1]; yM[17] = baseM[2]; yM[18] = 0; yM[19] = baseM[3];
    GrPaint yPaint;
    yPaint.addColorTextureProcessor(srcProxy, texMatrix);
    auto yFP = GrColorMatrixFragmentProcessor::Make(yM, false, true, false);
    yPaint.addColorFragmentProcessor(std::move(yFP));
    yPaint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    yRTC->fillRectToRect(GrNoClip(), std::move(yPaint), GrAA::kNo, SkMatrix::I(),
                         dstRectY, dstRectY);
    auto yTransfer = yRTC->transferPixels(GrColorType::kAlpha_8,
                                          SkIRect::MakeWH(yRTC->width(), yRTC->height()));
    if (!yTransfer.fTransferBuffer) {
        callback(context, nullptr, nullptr);
        return;
    }

    texMatrix.preScale(2.f, 2.f);
    // This matrix generates (r,g,b,a) = (0, 0, 0, u)
    float uM[20];
    std::fill_n(uM, 15, 0.f);
    uM[15] = baseM[4]; uM[16] = baseM[5]; uM[17] = baseM[6]; uM[18] = 0; uM[19] = baseM[7];
    GrPaint uPaint;
    uPaint.addColorTextureProcessor(srcProxy, texMatrix, GrSamplerState::ClampBilerp());
    auto uFP = GrColorMatrixFragmentProcessor::Make(uM, false, true, false);
    uPaint.addColorFragmentProcessor(std::move(uFP));
    uPaint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    uRTC->fillRectToRect(GrNoClip(), std::move(uPaint), GrAA::kNo, SkMatrix::I(),
                         dstRectUV, dstRectUV);
    auto uTransfer = uRTC->transferPixels(GrColorType::kAlpha_8,
                                          SkIRect::MakeWH(uRTC->width(), uRTC->height()));
    if (!uTransfer.fTransferBuffer) {
        callback(context, nullptr, nullptr);
        return;
    }

    // This matrix generates (r,g,b,a) = (0, 0, 0, v)
    float vM[20];
    std::fill_n(vM, 15, 0.f);
    vM[15] = baseM[8]; vM[16] = baseM[9]; vM[17] = baseM[10]; vM[18] = 0; vM[19] = baseM[11];
    GrPaint vPaint;
    vPaint.addColorTextureProcessor(srcProxy, texMatrix, GrSamplerState::ClampBilerp());
    auto vFP = GrColorMatrixFragmentProcessor::Make(vM, false, true, false);
    vPaint.addColorFragmentProcessor(std::move(vFP));
    vPaint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    vRTC->fillRectToRect(GrNoClip(), std::move(vPaint), GrAA::kNo, SkMatrix::I(),
                         dstRectUV, dstRectUV);
    auto vTransfer = vRTC->transferPixels(GrColorType::kAlpha_8,
                                          SkIRect::MakeWH(vRTC->width(), vRTC->height()));
    if (!vTransfer.fTransferBuffer) {
        callback(context, nullptr, nullptr);
        return;
    }

    struct FinishContext {
        ReadPixelsCallbackYUV420* fClientCallback;
        ReadPixelsContext fClientContext;
        int fW, fH;
        PixelTransferResult fYTransfer;
        PixelTransferResult fUTransfer;
        PixelTransferResult fVTransfer;
    };
    // Assumption is that the caller would like to flush. We could take a parameter or require an
    // explicit flush from the caller. We'd have to have a way to defer attaching the finish
    // callback to GrGpu until after the next flush that flushes our op list, though.
    auto* finishContext = new FinishContext{callback,
                                            context,
                                            dstW,
                                            dstH,
                                            std::move(yTransfer),
                                            std::move(uTransfer),
                                            std::move(vTransfer)};
    auto finishCallback = [](GrGpuFinishedContext c) {
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        const void* y = context->fYTransfer.fTransferBuffer->map();
        const void* u = context->fUTransfer.fTransferBuffer->map();
        const void* v = context->fVTransfer.fTransferBuffer->map();
        if (!y || !u || !v) {
            if (y) {
                context->fYTransfer.fTransferBuffer->unmap();
            }
            if (u) {
                context->fUTransfer.fTransferBuffer->unmap();
            }
            if (v) {
                context->fVTransfer.fTransferBuffer->unmap();
            }
            (*context->fClientCallback)(context->fClientContext, nullptr, 0);
            delete context;
            return;
        }
        size_t w = SkToSizeT(context->fW);
        size_t h = SkToSizeT(context->fH);
        std::unique_ptr<uint8_t[]> yTemp;
        if (context->fYTransfer.fPixelConverter) {
            yTemp.reset(new uint8_t[w * h]);
            context->fYTransfer.fPixelConverter(yTemp.get(), y);
            y = yTemp.get();
        }
        std::unique_ptr<uint8_t[]> uTemp;
        if (context->fUTransfer.fPixelConverter) {
            uTemp.reset(new uint8_t[w / 2 * h / 2]);
            context->fUTransfer.fPixelConverter(uTemp.get(), u);
            u = uTemp.get();
        }
        std::unique_ptr<uint8_t[]> vTemp;
        if (context->fVTransfer.fPixelConverter) {
            vTemp.reset(new uint8_t[w / 2 * h / 2]);
            context->fVTransfer.fPixelConverter(vTemp.get(), v);
            v = vTemp.get();
        }
        const void* data[] = {y, u, v};
        size_t rowBytes[] = {w, w / 2, w / 2};
        (*context->fClientCallback)(context->fClientContext, data, rowBytes);
        context->fYTransfer.fTransferBuffer->unmap();
        context->fUTransfer.fTransferBuffer->unmap();
        context->fVTransfer.fTransferBuffer->unmap();
        delete context;
    };
    GrFlushInfo flushInfo;
    flushInfo.fFinishedContext = finishContext;
    flushInfo.fFinishedProc = finishCallback;
    this->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
}

GrSemaphoresSubmitted GrRenderTargetContext::flush(SkSurface::BackendSurfaceAccess access,
                                                   const GrFlushInfo& info) {
    ASSERT_SINGLE_OWNER
    if (fContext->priv().abandoned()) {
        return GrSemaphoresSubmitted::kNo;
    }
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "flush", fContext);

    return this->drawingManager()->flushSurface(fRenderTargetProxy.get(), access, info);
}

bool GrRenderTargetContext::waitOnSemaphores(int numSemaphores,
                                             const GrBackendSemaphore waitSemaphores[]) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "waitOnSemaphores", fContext);

    AutoCheckFlush acf(this->drawingManager());

    if (numSemaphores && !this->caps()->semaphoreSupport()) {
        return false;
    }

    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        return false;
    }

    auto resourceProvider = direct->priv().resourceProvider();

    for (int i = 0; i < numSemaphores; ++i) {
        sk_sp<GrSemaphore> sema = resourceProvider->wrapBackendSemaphore(
                waitSemaphores[i], GrResourceProvider::SemaphoreWrapType::kWillWait,
                kAdopt_GrWrapOwnership);
        std::unique_ptr<GrOp> waitOp(GrSemaphoreOp::MakeWait(fContext, std::move(sema),
                                                             fRenderTargetProxy.get()));
        this->getRTOpList()->addWaitOp(
                std::move(waitOp), GrTextureResolveManager(this->drawingManager()), *this->caps());
    }
    return true;
}

void GrRenderTargetContext::insertEventMarker(const SkString& str) {
    std::unique_ptr<GrOp> op(GrDebugMarkerOp::Make(fContext, fRenderTargetProxy.get(), str));
    this->addOp(std::move(op));
}

void GrRenderTargetContext::drawPath(const GrClip& clip,
                                     GrPaint&& paint,
                                     GrAA aa,
                                     const SkMatrix& viewMatrix,
                                     const SkPath& path,
                                     const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawPath", fContext);

    GrShape shape(path, style);

    this->drawShape(clip, std::move(paint), aa, viewMatrix, shape);
}

void GrRenderTargetContext::drawShape(const GrClip& clip,
                                      GrPaint&& paint,
                                      GrAA aa,
                                      const SkMatrix& viewMatrix,
                                      const GrShape& shape) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawShape", fContext);

    if (shape.isEmpty()) {
        if (shape.inverseFilled()) {
            this->drawPaint(clip, std::move(paint), viewMatrix);
        }
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    if (!shape.style().hasPathEffect()) {
        GrAAType aaType = this->chooseAAType(aa);
        SkRRect rrect;
        // We can ignore the starting point and direction since there is no path effect.
        bool inverted;
        if (shape.asRRect(&rrect, nullptr, nullptr, &inverted) && !inverted) {
            if (rrect.isRect()) {
                this->drawRect(clip, std::move(paint), aa, viewMatrix, rrect.rect(),
                               &shape.style());
                return;
            } else if (rrect.isOval()) {
                this->drawOval(clip, std::move(paint), aa, viewMatrix, rrect.rect(), shape.style());
                return;
            }
            this->drawRRect(clip, std::move(paint), aa, viewMatrix, rrect, shape.style());
            return;
        } else if (GrAAType::kCoverage == aaType && shape.style().isSimpleFill() &&
                   viewMatrix.rectStaysRect()) {
            // TODO: the rectStaysRect restriction could be lifted if we were willing to apply
            // the matrix to all the points individually rather than just to the rect
            SkRect rects[2];
            if (shape.asNestedRects(rects)) {
                // Concave AA paths are expensive - try to avoid them for special cases
                std::unique_ptr<GrDrawOp> op = GrStrokeRectOp::MakeNested(
                                fContext, std::move(paint), viewMatrix, rects);
                if (op) {
                    this->addDrawOp(clip, std::move(op));
                }
                // Returning here indicates that there is nothing to draw in this case.
                return;
            }
        }
    }

    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix, shape);
}

bool GrRenderTargetContextPriv::drawAndStencilPath(const GrHardClip& clip,
                                                   const GrUserStencilSettings* ss,
                                                   SkRegion::Op op,
                                                   bool invert,
                                                   GrAA aa,
                                                   const SkMatrix& viewMatrix,
                                                   const SkPath& path) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "drawAndStencilPath",
                                   fRenderTargetContext->fContext);

    if (path.isEmpty() && path.isInverseFillType()) {
        GrPaint paint;
        paint.setCoverageSetOpXPFactory(op, invert);
        this->stencilRect(clip, ss, std::move(paint), GrAA::kNo, SkMatrix::I(),
                          SkRect::MakeIWH(fRenderTargetContext->width(),
                                          fRenderTargetContext->height()));
        return true;
    }

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    GrAAType aaType = fRenderTargetContext->chooseAAType(aa);
    bool hasUserStencilSettings = !ss->isUnused();

    SkIRect clipConservativeBounds;
    clip.getConservativeBounds(fRenderTargetContext->width(), fRenderTargetContext->height(),
                               &clipConservativeBounds, nullptr);

    GrShape shape(path, GrStyle::SimpleFill());
    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = fRenderTargetContext->caps();
    canDrawArgs.fProxy = fRenderTargetContext->proxy();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fClipConservativeBounds = &clipConservativeBounds;
    canDrawArgs.fAAType = aaType;
    SkASSERT(!fRenderTargetContext->wrapsVkSecondaryCB());
    canDrawArgs.fTargetIsWrappedVkSecondaryCB = false;
    canDrawArgs.fHasUserStencilSettings = hasUserStencilSettings;

    // Don't allow the SW renderer
    GrPathRenderer* pr = fRenderTargetContext->drawingManager()->getPathRenderer(
            canDrawArgs, false, GrPathRendererChain::DrawType::kStencilAndColor);
    if (!pr) {
        return false;
    }

    GrPaint paint;
    paint.setCoverageSetOpXPFactory(op, invert);

    GrPathRenderer::DrawPathArgs args{fRenderTargetContext->drawingManager()->getContext(),
                                      std::move(paint),
                                      ss,
                                      fRenderTargetContext,
                                      &clip,
                                      &clipConservativeBounds,
                                      &viewMatrix,
                                      &shape,
                                      aaType,
                                      fRenderTargetContext->colorSpaceInfo().isLinearlyBlended()};
    pr->drawPath(args);
    return true;
}

SkBudgeted GrRenderTargetContextPriv::isBudgeted() const {
    ASSERT_SINGLE_OWNER_PRIV

    if (fRenderTargetContext->fContext->priv().abandoned()) {
        return SkBudgeted::kNo;
    }

    SkDEBUGCODE(fRenderTargetContext->validate();)

    return fRenderTargetContext->fRenderTargetProxy->isBudgeted();
}

void GrRenderTargetContext::drawShapeUsingPathRenderer(const GrClip& clip,
                                                       GrPaint&& paint,
                                                       GrAA aa,
                                                       const SkMatrix& viewMatrix,
                                                       const GrShape& originalShape) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "internalDrawPath", fContext);

    if (!viewMatrix.isFinite() || !originalShape.bounds().isFinite()) {
        return;
    }

    SkIRect clipConservativeBounds;
    clip.getConservativeBounds(this->width(), this->height(), &clipConservativeBounds, nullptr);

    GrShape tempShape;
    GrAAType aaType = this->chooseAAType(aa);

    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = this->caps();
    canDrawArgs.fProxy = this->proxy();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &originalShape;
    canDrawArgs.fClipConservativeBounds = &clipConservativeBounds;
    canDrawArgs.fTargetIsWrappedVkSecondaryCB = this->wrapsVkSecondaryCB();
    canDrawArgs.fHasUserStencilSettings = false;

    GrPathRenderer* pr;
    static constexpr GrPathRendererChain::DrawType kType = GrPathRendererChain::DrawType::kColor;
    if (originalShape.isEmpty() && !originalShape.inverseFilled()) {
        return;
    }

    canDrawArgs.fAAType = aaType;

    // Try a 1st time without applying any of the style to the geometry (and barring sw)
    pr = this->drawingManager()->getPathRenderer(canDrawArgs, false, kType);
    SkScalar styleScale =  GrStyle::MatrixToScaleFactor(viewMatrix);

    if (!pr && originalShape.style().pathEffect()) {
        // It didn't work above, so try again with the path effect applied.
        tempShape = originalShape.applyStyle(GrStyle::Apply::kPathEffectOnly, styleScale);
        if (tempShape.isEmpty()) {
            return;
        }
        canDrawArgs.fShape = &tempShape;
        pr = this->drawingManager()->getPathRenderer(canDrawArgs, false, kType);
    }
    if (!pr) {
        if (canDrawArgs.fShape->style().applies()) {
            tempShape = canDrawArgs.fShape->applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec,
                                                       styleScale);
            if (tempShape.isEmpty()) {
                return;
            }
            canDrawArgs.fShape = &tempShape;
            // This time, allow SW renderer
            pr = this->drawingManager()->getPathRenderer(canDrawArgs, true, kType);
        } else {
            pr = this->drawingManager()->getSoftwarePathRenderer();
        }
    }

    if (!pr) {
#ifdef SK_DEBUG
        SkDebugf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    GrPathRenderer::DrawPathArgs args{this->drawingManager()->getContext(),
                                      std::move(paint),
                                      &GrUserStencilSettings::kUnused,
                                      this,
                                      &clip,
                                      &clipConservativeBounds,
                                      &viewMatrix,
                                      canDrawArgs.fShape,
                                      aaType,
                                      this->colorSpaceInfo().isLinearlyBlended()};
    pr->drawPath(args);
}

static void op_bounds(SkRect* bounds, const GrOp* op) {
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

void GrRenderTargetContext::addOp(std::unique_ptr<GrOp> op) {
    this->getRTOpList()->addOp(
            std::move(op), GrTextureResolveManager(this->drawingManager()), *this->caps());
}

void GrRenderTargetContext::addDrawOp(const GrClip& clip, std::unique_ptr<GrDrawOp> op,
                                      const std::function<WillAddOpFn>& willAddFn) {
    ASSERT_SINGLE_OWNER
    if (fContext->priv().abandoned()) {
        fContext->priv().opMemoryPool()->release(std::move(op));
        return;
    }
    SkDEBUGCODE(this->validate();)
    SkDEBUGCODE(op->fAddDrawOpCalled = true;)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "addDrawOp", fContext);

    // Setup clip
    SkRect bounds;
    op_bounds(&bounds, op.get());
    GrAppliedClip appliedClip;
    GrDrawOp::FixedFunctionFlags fixedFunctionFlags = op->fixedFunctionFlags();
    bool usesHWAA = fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesHWAA;
    bool usesStencil = fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesStencil;

    if (usesStencil) {
        this->setNeedsStencil(usesHWAA);
    }

    if (!clip.apply(fContext, this, usesHWAA, usesStencil, &appliedClip, &bounds)) {
        fContext->priv().opMemoryPool()->release(std::move(op));
        return;
    }

    SkASSERT((!usesStencil && !appliedClip.hasStencilClip()) || (fNumStencilSamples > 0));

    GrClampType clampType = GrColorTypeClampType(this->colorSpaceInfo().colorType());
    // MIXED SAMPLES TODO: If we start using mixed samples for clips we will need to check the clip
    // here as well.
    bool hasMixedSampledCoverage = (usesHWAA && this->numSamples() <= 1);
#ifdef SK_DEBUG
    if (hasMixedSampledCoverage) {
        SkASSERT(usesStencil);
        SkASSERT(fRenderTargetProxy->canUseMixedSamples(*this->caps()));
    }
#endif
    GrProcessorSet::Analysis analysis = op->finalize(
            *this->caps(), &appliedClip, hasMixedSampledCoverage, clampType);

    GrXferProcessor::DstProxy dstProxy;
    if (analysis.requiresDstTexture()) {
        if (!this->setupDstProxy(this->asRenderTargetProxy(), clip, *op, &dstProxy)) {
            fContext->priv().opMemoryPool()->release(std::move(op));
            return;
        }
    }

    op->setClippedBounds(bounds);
    auto opList = this->getRTOpList();
    if (willAddFn) {
        willAddFn(op.get(), opList->uniqueID());
    }
    opList->addDrawOp(std::move(op), analysis, std::move(appliedClip), dstProxy,
                      GrTextureResolveManager(this->drawingManager()), *this->caps());
}

bool GrRenderTargetContext::setupDstProxy(GrRenderTargetProxy* rtProxy, const GrClip& clip,
                                          const GrOp& op, GrXferProcessor::DstProxy* dstProxy) {
    // If we are wrapping a vulkan secondary command buffer, we can't make a dst copy because we
    // don't actually have a VkImage to make a copy of. Additionally we don't have the power to
    // start and stop the render pass in order to make the copy.
    if (rtProxy->wrapsVkSecondaryCB()) {
        return false;
    }

    if (this->caps()->textureBarrierSupport()) {
        if (GrTextureProxy* texProxy = rtProxy->asTextureProxy()) {
            // The render target is a texture, so we can read from it directly in the shader. The XP
            // will be responsible to detect this situation and request a texture barrier.
            dstProxy->setProxy(sk_ref_sp(texProxy));
            dstProxy->setOffset(0, 0);
            return true;
        }
    }

    SkIRect copyRect = SkIRect::MakeWH(rtProxy->width(), rtProxy->height());

    SkIRect clippedRect;
    clip.getConservativeBounds(rtProxy->width(), rtProxy->height(), &clippedRect);
    SkRect opBounds = op.bounds();
    // If the op has aa bloating or is a infinitely thin geometry (hairline) outset the bounds by
    // 0.5 pixels.
    if (op.hasAABloat() || op.hasZeroArea()) {
        opBounds.outset(0.5f, 0.5f);
        // An antialiased/hairline draw can sometimes bleed outside of the clips bounds. For
        // performance we may ignore the clip when the draw is entirely inside the clip is float
        // space but will hit pixels just outside the clip when actually rasterizing.
        clippedRect.outset(1, 1);
        clippedRect.intersect(SkIRect::MakeWH(rtProxy->width(), rtProxy->height()));
    }
    SkIRect opIBounds;
    opBounds.roundOut(&opIBounds);
    if (!clippedRect.intersect(opIBounds)) {
#ifdef SK_DEBUG
        GrCapsDebugf(this->caps(), "setupDstTexture: Missed an early reject bailing on draw.");
#endif
        return false;
    }

    // MSAA consideration: When there is support for reading MSAA samples in the shader we could
    // have per-sample dst values by making the copy multisampled.
    GrCaps::DstCopyRestrictions restrictions = this->caps()->getDstCopyRestrictions(
            rtProxy, this->colorSpaceInfo().colorType());

    if (!restrictions.fMustCopyWholeSrc) {
        copyRect = clippedRect;
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
    sk_sp<GrTextureProxy> newProxy =
            GrSurfaceProxy::Copy(fContext, rtProxy, GrMipMapped::kNo, copyRect, fit,
                                 SkBudgeted::kYes, restrictions.fRectsMustMatch);
    SkASSERT(newProxy);

    dstProxy->setProxy(std::move(newProxy));
    dstProxy->setOffset(dstOffset);
    return true;
}

bool GrRenderTargetContext::blitTexture(GrTextureProxy* src, const SkIRect& srcRect,
                                        const SkIPoint& dstPoint) {
    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    if (!GrClipSrcRectAndDstPoint(this->asSurfaceProxy()->isize(), src->isize(), srcRect, dstPoint,
                                  &clippedSrcRect, &clippedDstPoint)) {
        return false;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    auto fp = GrSimpleTextureEffect::Make(sk_ref_sp(src->asTextureProxy()),
                                          SkMatrix::I());
    if (!fp) {
        return false;
    }
    paint.addColorFragmentProcessor(std::move(fp));

    this->fillRectToRect(
            GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
            SkRect::MakeXYWH(clippedDstPoint.fX, clippedDstPoint.fY, clippedSrcRect.width(),
                             clippedSrcRect.height()),
            SkRect::Make(clippedSrcRect));
    return true;
}


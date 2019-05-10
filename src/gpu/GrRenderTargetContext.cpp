/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRenderTargetContext.h"
#include "include/core/SkDrawable.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrRenderTarget.h"
#include "include/private/GrAuditTrail.h"
#include "include/private/GrColor.h"
#include "include/private/GrOpList.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/SkShadowFlags.h"
#include "include/utils/SkShadowUtils.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrQuad.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrShape.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/GrTextureDomain.h"
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
#include "src/gpu/ops/GrTransferFromOp.h"
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
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkSurfaceProps* surfaceProps,
                                             bool managedOpList)
        : GrSurfaceContext(context, rtp->config(), std::move(colorSpace))
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
        SkASSERT(fRenderTargetProxy->getLastOpList() == fOpList.get());
    }
}
#endif

GrRenderTargetContext::~GrRenderTargetContext() {
    ASSERT_SINGLE_OWNER
}

inline GrAAType GrRenderTargetContext::chooseAAType(GrAA aa) {
    auto fsaaType = this->fsaaType();
    if (GrAA::kNo == aa) {
        // On some devices we cannot disable MSAA if it is enabled so we make the AA type reflect
        // that.
        if (fsaaType == GrFSAAType::kUnifiedMSAA && !this->caps()->multisampleDisableSupport()) {
            return GrAAType::kMSAA;
        }
        return GrAAType::kNone;
    }
    switch (fsaaType) {
        case GrFSAAType::kNone:
        case GrFSAAType::kMixedSamples:
            return GrAAType::kCoverage;
        case GrFSAAType::kUnifiedMSAA:
            return GrAAType::kMSAA;
    }
    SK_ABORT("Unexpected fsaa type");
    return GrAAType::kNone;
}

static inline GrPathRenderer::AATypeFlags choose_path_aa_type_flags(
        GrAA aa, GrFSAAType fsaaType, const GrCaps& caps) {
    using AATypeFlags = GrPathRenderer::AATypeFlags;
    if (GrAA::kNo == aa) {
        // On some devices we cannot disable MSAA if it is enabled so we make the AA type flags
        // reflect that.
        if (fsaaType == GrFSAAType::kUnifiedMSAA && !caps.multisampleDisableSupport()) {
            return AATypeFlags::kMSAA;
        }
        return AATypeFlags::kNone;
    }
    switch (fsaaType) {
        case GrFSAAType::kNone:
            return AATypeFlags::kCoverage;
        case GrFSAAType::kMixedSamples:
            return AATypeFlags::kCoverage | AATypeFlags::kMixedSampledStencilThenCover;
        case GrFSAAType::kUnifiedMSAA:
            return AATypeFlags::kMSAA;
    }
    SK_ABORT("Invalid GrFSAAType.");
    return AATypeFlags::kNone;
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
        if (this->getRTOpList()->resetForFullscreenClear() &&
            !this->caps()->performColorClearsAsDraws()) {
            // The op list was emptied and native clears are allowed, so just use the load op
            this->getRTOpList()->setColorLoadOp(GrLoadOp::kClear, color);
            return;
        } else {
            // Will use an op for the clear, reset the load op to discard since the op will
            // blow away the color buffer contents
            this->getRTOpList()->setColorLoadOp(GrLoadOp::kDiscard);
        }

        // Must add an op to the list (either because we couldn't use a load op, or because the
        // clear load op isn't supported)
        if (this->caps()->performColorClearsAsDraws()) {
            SkRect rtRect = SkRect::MakeWH(this->width(), this->height());
            GrPaint paint;
            clear_to_grpaint(color, &paint);
            this->addDrawOp(GrFixedClip::Disabled(),
                            GrFillRectOp::Make(fContext, std::move(paint),
                                               GrAAType::kNone, SkMatrix::I(), rtRect));
        } else {
            this->getRTOpList()->addOp(GrClearOp::Make(fContext, SkIRect::MakeEmpty(), color,
                                                       /* fullscreen */ true), *this->caps());
        }
    } else {
        if (this->caps()->performPartialClearsAsDraws()) {
            // performPartialClearsAsDraws() also returns true if any clear has to be a draw.
            SkRect scissor = SkRect::Make(clip.scissorRect());
            GrPaint paint;
            clear_to_grpaint(color, &paint);

            this->addDrawOp(clip, GrFillRectOp::Make(fContext, std::move(paint), GrAAType::kNone,
                                                     SkMatrix::I(), scissor));
        } else {
            std::unique_ptr<GrOp> op(GrClearOp::Make(fContext, clip, color,
                                                     this->asSurfaceProxy()));
            // This version of the clear op factory can return null if the clip doesn't intersect
            // with the surface proxy's boundary
            if (!op) {
                return;
            }
            this->getRTOpList()->addOp(std::move(op), *this->caps());
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
            fRenderTargetContext->addDrawOp(GrFixedClip::Disabled(),
                                            GrFillRectOp::Make(fRenderTargetContext->fContext,
                                                               std::move(paint),
                                                               GrAAType::kNone,
                                                               SkMatrix::I(),
                                                               SkRect::Make(rtRect)));
        } else {
            // Must use the ClearOp factory that takes a boolean (false) instead of a surface
            // proxy. The surface proxy variant would intersect the clip rect with its logical
            // bounds, which is not desired in this special case.
            fRenderTargetContext->getRTOpList()->addOp(
                    GrClearOp::Make(fRenderTargetContext->fContext, rtRect, color,
                                    /* fullscreen */ false),
                    *fRenderTargetContext->caps());
        }
    } else {
        // Reset the oplist like in internalClear(), but do not rely on a load op for the clear
        fRenderTargetContext->getRTOpList()->resetForFullscreenClear();
        fRenderTargetContext->getRTOpList()->setColorLoadOp(GrLoadOp::kDiscard);

        if (fRenderTargetContext->caps()->performColorClearsAsDraws()) {
            // This draws a quad covering the worst case dimensions instead of just the logical
            // width and height like in internalClear().
            GrPaint paint;
            clear_to_grpaint(color, &paint);
            fRenderTargetContext->addDrawOp(GrFixedClip::Disabled(),
                                            GrFillRectOp::Make(fRenderTargetContext->fContext,
                                                               std::move(paint),
                                                               GrAAType::kNone,
                                                               SkMatrix::I(),
                                                               SkRect::Make(rtRect)));
        } else {
            // Nothing special about this path in absClear compared to internalClear()
            fRenderTargetContext->getRTOpList()->addOp(
                    GrClearOp::Make(fRenderTargetContext->fContext, SkIRect::MakeEmpty(), color,
                                    /* fullscreen */ true),
                    *fRenderTargetContext->caps());
        }
    }
}

void GrRenderTargetContext::drawPaint(const GrClip& clip,
                                      GrPaint&& paint,
                                      const SkMatrix& viewMatrix) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawPaint", fContext);

    // set rect to be big enough to fill the space, but not super-huge, so we
    // don't overflow fixed-point implementations

    SkRect r = fRenderTargetProxy->getBoundsRect();

    // Check if we can optimize a clipped drawPaint(). We only do the transformation when there are
    // no fragment processors because they may depend on having correct local coords and this path
    // draws in device space without a local matrix. It currently handles converting clipRRect()
    // to drawRRect() and solid colors to screen-filling drawRects() (which are then converted into
    // clears if possible in drawRect).
    if (!paint.numTotalFragmentProcessors()) {
        SkRRect rrect;
        GrAA aa = GrAA::kNo;
        if (clip.isRRect(r, &rrect, &aa)) {
            if (rrect.isRect()) {
                // Use drawFilledRect() with no clip and the reduced rectangle
                this->drawFilledRect(GrNoClip(), std::move(paint), aa, SkMatrix::I(), rrect.rect());
            } else {
                // Use drawRRect() with no clip
                this->drawRRect(GrNoClip(), std::move(paint), aa, SkMatrix::I(), rrect,
                                GrStyle::SimpleFill());
            }
        } else {
            // Use drawFilledRect() with no view matrix to draw a fullscreen quad, but preserve
            // the clip. Since the paint has no FPs we can drop the view matrix without worrying
            // about local coordinates. If the clip is simple, drawFilledRect() will turn this into
            // a clear or a scissored clear.
            this->drawFilledRect(clip, std::move(paint), aa, SkMatrix::I(), r);
        }
        return;
    }

    // Since the paint is not trivial, there's no way at this point drawRect() could have converted
    // this drawPaint() into an optimized clear. drawRect() would then use GrFillRectOp without
    // a local matrix, so we can simplify things and use the local matrix variant to draw a screen
    // filling rect with the inverse view matrix for local coords, which works for all matrix
    // conditions.
    SkMatrix localMatrix;
    if (!viewMatrix.invert(&localMatrix)) {
        return;
    }

    AutoCheckFlush acf(this->drawingManager());
    std::unique_ptr<GrDrawOp> op = GrFillRectOp::MakeWithLocalMatrix(
            fContext, std::move(paint), GrAAType::kNone, SkMatrix::I(), localMatrix, r);
    this->addDrawOp(clip, std::move(op));
}

static inline bool rect_contains_inclusive(const SkRect& rect, const SkPoint& point) {
    return point.fX >= rect.fLeft && point.fX <= rect.fRight &&
           point.fY >= rect.fTop && point.fY <= rect.fBottom;
}

// Attempts to crop a rect and optional local rect to the clip boundaries.
// Returns false if the draw can be skipped entirely.
static bool crop_filled_rect(int width, int height, const GrClip& clip,
                             const SkMatrix& viewMatrix, SkRect* rect,
                             SkRect* localRect = nullptr) {
    if (!viewMatrix.rectStaysRect()) {
        return true;
    }

    SkIRect clipDevBounds;
    SkRect clipBounds;

    clip.getConservativeBounds(width, height, &clipDevBounds);
    if (!SkMatrixPriv::InverseMapRect(viewMatrix, &clipBounds, SkRect::Make(clipDevBounds))) {
        return false;
    }

    if (localRect) {
        if (!rect->intersects(clipBounds)) {
            return false;
        }
        // localRect is force-sorted after clipping, so this is a sanity check to make sure callers
        // aren't intentionally using inverted local rectangles.
        SkASSERT(localRect->isSorted());
        const SkScalar dx = localRect->width() / rect->width();
        const SkScalar dy = localRect->height() / rect->height();
        if (clipBounds.fLeft > rect->fLeft) {
            localRect->fLeft += (clipBounds.fLeft - rect->fLeft) * dx;
            rect->fLeft = clipBounds.fLeft;
        }
        if (clipBounds.fTop > rect->fTop) {
            localRect->fTop += (clipBounds.fTop - rect->fTop) * dy;
            rect->fTop = clipBounds.fTop;
        }
        if (clipBounds.fRight < rect->fRight) {
            localRect->fRight -= (rect->fRight - clipBounds.fRight) * dx;
            rect->fRight = clipBounds.fRight;
        }
        if (clipBounds.fBottom < rect->fBottom) {
            localRect->fBottom -= (rect->fBottom - clipBounds.fBottom) * dy;
            rect->fBottom = clipBounds.fBottom;
        }
        // Ensure local coordinates remain sorted after clipping. If the original dstRect was very
        // large, numeric precision can invert the localRect
        localRect->sort();
        return true;
    }

    return rect->intersect(clipBounds);
}

bool GrRenderTargetContext::drawFilledRectAsClear(const GrClip& clip, GrPaint&& paint, GrAA aa,
                                                  const SkMatrix& viewMatrix, const SkRect& rect) {
    // Rules for a filled rect to become a clear [+scissor]:
    // 1. The paint is a constant blend color with no other FPs
    // 2. The view matrix maps rectangles to rectangles, or the transformed quad fully covers
    //    the render target (or clear region in #3).
    // 3. The clip is an intersection of rectangles, so the clear region will be the
    //    intersection of the clip and the provided rect.
    // 4. The clear region aligns with pixel bounds
    // 5. There are no user stencil settings (and since the clip was IOR, the clip won't need
    //    to use the stencil either).
    // If all conditions pass, the filled rect can either be a fullscreen clear (if it's big
    // enough), or the rectangle geometry will be used as the scissor clip on the clear.
    // If everything passes but rule #4, this submits a simplified fill rect op instead so that the
    // rounding differences between clip and draws don't fight each other.
    // NOTE: we route draws into clear() regardless of performColorClearsAsDraws() since the
    // clear call is allowed to reset the oplist even when it also happens to use a GrFillRectOp.

    SkPMColor4f clearColor;
    if (paint.numCoverageFragmentProcessors() > 0 || !paint.isConstantBlendedColor(&clearColor)) {
        return false;
    }

    const SkRect rtRect = fRenderTargetProxy->getBoundsRect();
    // Will be the intersection of render target, clip, and quad
    SkRect combinedRect = rtRect;

    SkRRect clipRRect;
    GrAA clipAA;
    if (!clip.quickContains(rtRect)) {
        // If the clip is an rrect with no rounding, then it can replace the full RT bounds as the
        // limiting region, although we will have to worry about AA. If the clip is anything
        // more complicated, just punt to the regular fill rect op.
        if (!clip.isRRect(rtRect, &clipRRect, &clipAA) || !clipRRect.isRect()) {
            return false;
        }
        combinedRect = clipRRect.rect();
    } else {
        // The clip is outside the render target, so the clip can be ignored
        clipAA = GrAA::kNo;
    }

    if (viewMatrix.rectStaysRect()) {
        // Skip the extra overhead of inverting the view matrix to see if rtRect is contained in the
        // drawn rectangle, and instead just intersect rtRect with the transformed rect. It will be
        // the new clear region.
        if (!combinedRect.intersect(viewMatrix.mapRect(rect))) {
            // No intersection means nothing should be drawn, so return true but don't add an op
            return true;
        }
    } else {
        // If the transformed rectangle does not contain the combined rt and clip, the draw is too
        // complex to be implemented as a clear
        SkMatrix invM;
        if (!viewMatrix.invert(&invM)) {
            return false;
        }
        // The clip region in the rect's local space, so the test becomes the local rect containing
        // the quad's points.
        GrQuad quad = GrQuad::MakeFromRect(rtRect, invM);
        if (!rect_contains_inclusive(rect, quad.point(0)) ||
            !rect_contains_inclusive(rect, quad.point(1)) ||
            !rect_contains_inclusive(rect, quad.point(2)) ||
            !rect_contains_inclusive(rect, quad.point(3))) {
            // No containment, so rtRect can't be filled by a solid color
            return false;
        }
        // combinedRect can be filled by a solid color but doesn't need to be modified since it's
        // inside the quad to be drawn.
    }

    // Almost every condition is met; now it requires that the combined rect align with pixel
    // boundaries in order for it to become a scissor-clear. Ignore the AA status in this case
    // since non-AA with partial-pixel coordinates can be rounded differently on the GPU,
    // leading to unexpected differences between a scissor test and a rasterized quad.
    // Also skip very small rectangles since the scissor+clear doesn't by us much then.
    if (combinedRect.contains(rtRect)) {
        // Full screen clear
        this->clear(nullptr, clearColor, CanClearFullscreen::kYes);
        return true;
    } else if (GrClip::IsPixelAligned(combinedRect) &&
               combinedRect.width() > 256 && combinedRect.height() > 256) {
        // Scissor + clear (round shouldn't do anything since we are pixel aligned)
        SkIRect scissorRect;
        combinedRect.round(&scissorRect);
        this->clear(&scissorRect, clearColor, CanClearFullscreen::kNo);
        return true;
    }

    // If we got here, we can't use a scissor + clear, but combinedRect represents the correct
    // geometry combination of quad + clip so we can perform a simplified fill rect op. We do this
    // mostly to avoid mismatches in rounding logic on the CPU vs. the GPU, which frequently appears
    // when drawing and clipping something to the same non-AA rect that never-the-less has
    // non-integer coordinates.

    // For AA, use non-AA only when both clip and draw are non-AA.
    if (clipAA == GrAA::kYes) {
        aa = GrAA::kYes;
    }
    GrAAType aaType = this->chooseAAType(aa);
    this->addDrawOp(GrFixedClip::Disabled(),
                    GrFillRectOp::Make(fContext, std::move(paint), aaType, SkMatrix::I(),
                                       combinedRect));
    return true;
}

void GrRenderTargetContext::drawFilledRect(const GrClip& clip,
                                           GrPaint&& paint,
                                           GrAA aa,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& rect,
                                           const GrUserStencilSettings* ss) {

    if (!ss) {
        if (this->drawFilledRectAsClear(clip, std::move(paint), aa, viewMatrix, rect)) {
            return;
        }
        // Fall through to fill rect op
        assert_alive(paint);
    }

    SkRect croppedRect = rect;
    if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix, &croppedRect)) {
        // The rectangle would not be drawn, so no need to add a draw op to the list
        return;
    }

    GrAAType aaType = this->chooseAAType(aa);
    this->addDrawOp(clip, GrFillRectOp::Make(fContext, std::move(paint), aaType, viewMatrix,
                                             croppedRect, ss));
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
        this->drawFilledRect(clip, std::move(paint), aa, viewMatrix, rect);
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
        paint.setColor4f({0.f, 0.f, 0.f, 0.f});
        paint.setPorterDuffXPFactory(SkBlendMode::kSrcOver);

        // Mark stencil usage here before addDrawOp() so that it doesn't try to re-call
        // internalStencilClear() just because the op has stencil settings.
        this->setNeedsStencil();
        this->addDrawOp(clip, GrFillRectOp::Make(fContext, std::move(paint),
                        GrAAType::kNone, SkMatrix::I(), rtRect, ss));
    } else {
        std::unique_ptr<GrOp> op(GrClearStencilClipOp::Make(fContext, clip, insideStencilMask,
                                                            fRenderTargetProxy.get()));
        if (!op) {
            return;
        }
        this->getRTOpList()->addOp(std::move(op), *this->caps());
    }
}

void GrRenderTargetContextPriv::stencilPath(const GrHardClip& clip,
                                            GrAA doStencilMSAA,
                                            const SkMatrix& viewMatrix,
                                            const GrPath* path) {
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

    fRenderTargetContext->setNeedsStencil();

    std::unique_ptr<GrOp> op = GrStencilPathOp::Make(fRenderTargetContext->fContext,
                                                     viewMatrix,
                                                     GrAA::kYes == doStencilMSAA,
                                                     path->getFillType(),
                                                     appliedClip.hasStencilClip(),
                                                     appliedClip.scissorState(),
                                                     path);
    if (!op) {
        return;
    }
    op->setClippedBounds(bounds);
    fRenderTargetContext->getRTOpList()->addOp(std::move(op), *fRenderTargetContext->caps());
}

void GrRenderTargetContextPriv::stencilRect(const GrHardClip& clip,
                                            const GrUserStencilSettings* ss,
                                            GrAA doStencilMSAA,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "stencilRect",
                                   fRenderTargetContext->fContext);

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());
    auto aaType = (GrAA::kYes == doStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;
    std::unique_ptr<GrDrawOp> op = GrFillRectOp::Make(
            fRenderTargetContext->fContext, std::move(paint), aaType, viewMatrix,  rect, ss);
    fRenderTargetContext->addDrawOp(clip, std::move(op));
}

bool GrRenderTargetContextPriv::drawAndStencilRect(const GrHardClip& clip,
                                                   const GrUserStencilSettings* ss,
                                                   SkRegion::Op op,
                                                   bool invert,
                                                   GrAA aa,
                                                   const SkMatrix& viewMatrix,
                                                   const SkRect& rect) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "drawAndStencilRect",
                                   fRenderTargetContext->fContext);

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    GrPaint paint;
    paint.setCoverageSetOpXPFactory(op, invert);

    // This will always succeed to draw a rectangle
    fRenderTargetContext->drawFilledRect(clip, std::move(paint), aa, viewMatrix, rect, ss);
    return true;
}

void GrRenderTargetContext::fillRectWithEdgeAA(const GrClip& clip, GrPaint&& paint, GrAA aa,
                                               GrQuadAAFlags edgeAA, const SkMatrix& viewMatrix,
                                               const SkRect& rect, const SkRect* localRect) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "fillRectWithEdgeAA", fContext);

    GrAAType aaType = this->chooseAAType(aa);
    std::unique_ptr<GrDrawOp> op;

    if (localRect) {
        // If local coordinates are provided, skip the optimization check to go through
        // drawFilledRect, and also calculate clipped local coordinates
        SkRect croppedRect = rect;
        SkRect croppedLocalRect = *localRect;
        if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix, &croppedRect,
                              &croppedLocalRect)) {
            return;
        }
        op = GrFillRectOp::MakePerEdgeWithLocalRect(fContext, std::move(paint), aaType, edgeAA,
                                                    viewMatrix, croppedRect, croppedLocalRect);
    } else {
        // If aaType turns into MSAA, make sure to keep quads with no AA edges as MSAA. Sending
        // those to drawFilledRect() would have it turn off MSAA in that case, which breaks seaming
        // with any partial AA edges that kept MSAA.
        if (aaType != GrAAType::kMSAA &&
            (edgeAA == GrQuadAAFlags::kNone || edgeAA == GrQuadAAFlags::kAll)) {
            // This is equivalent to a regular filled rect draw, so route through there to take
            // advantage of draw->clear optimizations
            this->drawFilledRect(clip, std::move(paint), GrAA(edgeAA == GrQuadAAFlags::kAll),
                                 viewMatrix, rect);
            return;
        }

        SkRect croppedRect = rect;
        if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix, &croppedRect)) {
            return;
        }
        op = GrFillRectOp::MakePerEdge(fContext, std::move(paint), aaType, edgeAA, viewMatrix,
                                       croppedRect);
    }

    AutoCheckFlush acf(this->drawingManager());
    this->addDrawOp(clip, std::move(op));
}

void GrRenderTargetContext::fillQuadWithEdgeAA(const GrClip& clip, GrPaint&& paint, GrAA aa,
                                               GrQuadAAFlags edgeAA, const SkMatrix& viewMatrix,
                                               const SkPoint quad[4], const SkPoint localQuad[4]) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "fillQuadWithEdgeAA", fContext);

    GrAAType aaType = this->chooseAAType(aa);

    AutoCheckFlush acf(this->drawingManager());
    // MakePerEdgeQuad automatically does the right thing if localQuad is null or not
    this->addDrawOp(clip, GrFillRectOp::MakePerEdgeQuad(fContext, std::move(paint), aaType, edgeAA,
                                                        viewMatrix, quad, localQuad));
}

// Creates a paint for GrFillRectOp that matches behavior of GrTextureOp
static void draw_texture_to_grpaint(sk_sp<GrTextureProxy> proxy, const SkRect* domain,
                                    GrSamplerState::Filter filter, SkBlendMode mode,
                                    const SkPMColor4f& color, sk_sp<GrColorSpaceXform> csXform,
                                    GrPaint* paint) {
    paint->setColor4f(color);
    paint->setXPFactory(SkBlendMode_AsXPFactory(mode));

    std::unique_ptr<GrFragmentProcessor> fp;
    if (domain) {
        SkRect correctedDomain = *domain;
        if (filter == GrSamplerState::Filter::kBilerp) {
            // Inset by 1/2 pixel, which GrTextureOp and GrTextureAdjuster handle automatically
            correctedDomain.inset(0.5f, 0.5f);
        }
        fp = GrTextureDomainEffect::Make(std::move(proxy), SkMatrix::I(), correctedDomain,
                                         GrTextureDomain::kClamp_Mode, filter);
    } else {
        fp = GrSimpleTextureEffect::Make(std::move(proxy), SkMatrix::I(), filter);
    }

    fp = GrColorSpaceXformEffect::Make(std::move(fp), csXform);
    paint->addColorFragmentProcessor(std::move(fp));
}

void GrRenderTargetContext::drawTexture(const GrClip& clip, sk_sp<GrTextureProxy> proxy,
                                        GrSamplerState::Filter filter, SkBlendMode mode,
                                        const SkPMColor4f& color, const SkRect& srcRect,
                                        const SkRect& dstRect, GrAA aa, GrQuadAAFlags aaFlags,
                                        SkCanvas::SrcRectConstraint constraint,
                                        const SkMatrix& viewMatrix,
                                        sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawTexture", fContext);
    if (constraint == SkCanvas::kStrict_SrcRectConstraint &&
        srcRect.contains(proxy->getWorstCaseBoundsRect())) {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }

    GrAAType aaType = this->chooseAAType(aa);
    SkRect clippedDstRect = dstRect;
    SkRect clippedSrcRect = srcRect;
    if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix, &clippedDstRect,
                          &clippedSrcRect)) {
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    std::unique_ptr<GrDrawOp> op;
    if (mode != SkBlendMode::kSrcOver) {
        // Emulation mode with GrPaint and GrFillRectOp
        if (filter != GrSamplerState::Filter::kNearest &&
            !GrTextureOp::GetFilterHasEffect(viewMatrix, clippedSrcRect, clippedDstRect)) {
            filter = GrSamplerState::Filter::kNearest;
        }

        GrPaint paint;
        draw_texture_to_grpaint(std::move(proxy),
                constraint == SkCanvas::kStrict_SrcRectConstraint ? &srcRect : nullptr,
                filter, mode, color, std::move(textureColorSpaceXform), &paint);
        op = GrFillRectOp::MakePerEdgeWithLocalRect(fContext, std::move(paint), aaType, aaFlags,
                                                    viewMatrix, clippedDstRect, clippedSrcRect);
    } else {
        // Can use a lighter weight op that can chain across proxies
        op = GrTextureOp::Make(fContext, std::move(proxy), filter, color, clippedSrcRect,
                               clippedDstRect, aaType, aaFlags, constraint, viewMatrix,
                               std::move(textureColorSpaceXform));
    }

    this->addDrawOp(clip, std::move(op));
}

void GrRenderTargetContext::drawTextureQuad(const GrClip& clip, sk_sp<GrTextureProxy> proxy,
                                            GrSamplerState::Filter filter, SkBlendMode mode,
                                            const SkPMColor4f& color, const SkPoint srcQuad[4],
                                            const SkPoint dstQuad[4], GrAA aa,
                                            GrQuadAAFlags aaFlags, const SkRect* domain,
                                            const SkMatrix& viewMatrix,
                                            sk_sp<GrColorSpaceXform> texXform) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawTextureQuad", fContext);
    if (domain && domain->contains(proxy->getWorstCaseBoundsRect())) {
        domain = nullptr;
    }

    GrAAType aaType = this->chooseAAType(aa);

    // Unlike drawTexture(), don't bother cropping or optimizing the filter type since we're
    // sampling an arbitrary quad of the texture.
    AutoCheckFlush acf(this->drawingManager());
    std::unique_ptr<GrDrawOp> op;
    if (mode != SkBlendMode::kSrcOver) {
        // Emulation mode, but don't bother converting to kNearest filter since it's an arbitrary
        // quad that is being drawn, which makes the tests too expensive here
        GrPaint paint;
        draw_texture_to_grpaint(
                std::move(proxy), domain, filter, mode, color, std::move(texXform), &paint);
        op = GrFillRectOp::MakePerEdgeQuad(fContext, std::move(paint), aaType, aaFlags, viewMatrix,
                                           dstQuad, srcQuad);
    } else {
        // Use lighter weight GrTextureOp
        op = GrTextureOp::MakeQuad(fContext, std::move(proxy), filter, color, srcQuad, dstQuad,
                                   aaType, aaFlags, domain, viewMatrix, std::move(texXform));
    }

    this->addDrawOp(clip, std::move(op));
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
        // Draw one at a time with GrFillRectOp and a GrPaint that emulates what GrTextureOp does
        SkMatrix ctm;
        for (int i = 0; i < cnt; ++i) {
            float alpha = set[i].fAlpha;
            ctm = viewMatrix;
            if (set[i].fPreViewMatrix) {
                ctm.preConcat(*set[i].fPreViewMatrix);
            }

            if (set[i].fDstClipQuad == nullptr) {
                // Stick with original rectangles, which allows the ops to know more about what's
                // being drawn.
                this->drawTexture(clip, set[i].fProxy, filter, mode, {alpha, alpha, alpha, alpha},
                                  set[i].fSrcRect, set[i].fDstRect, aa, set[i].fAAFlags,
                                  constraint, ctm, texXform);
            } else {
                // Generate interpolated texture coordinates to match the dst clip
                SkPoint srcQuad[4];
                GrMapRectPoints(set[i].fDstRect, set[i].fSrcRect, set[i].fDstClipQuad, srcQuad, 4);
                const SkRect* domain = constraint == SkCanvas::kStrict_SrcRectConstraint
                        ? &set[i].fSrcRect : nullptr;
                this->drawTextureQuad(clip, set[i].fProxy, filter, mode,
                                      {alpha, alpha, alpha, alpha}, srcQuad, set[i].fDstClipQuad,
                                      aa, set[i].fAAFlags, domain, ctm, texXform);
            }
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

void GrRenderTargetContext::fillRectWithLocalMatrix(const GrClip& clip,
                                                    GrPaint&& paint,
                                                    GrAA aa,
                                                    const SkMatrix& viewMatrix,
                                                    const SkRect& rectToDraw,
                                                    const SkMatrix& localMatrix) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "fillRectWithLocalMatrix", fContext);

    SkRect croppedRect = rectToDraw;
    if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix, &croppedRect)) {
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa);
    this->addDrawOp(clip, GrFillRectOp::MakeWithLocalMatrix(fContext, std::move(paint), aaType,
                                                            viewMatrix, localMatrix, croppedRect));
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
    if (style.isSimpleFill()) {
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
        SkScalarInvert(viewMatrix[SkMatrix::kMScaleX]) :
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
    if (style.isSimpleFill()) {
        // GrFillRRectOp has special geometry and a fragment-shader branch to conditionally evaluate
        // the arc equation. This same special geometry and fragment branch also turn out to be a
        // substantial optimization for drawing ovals (namely, by not evaluating the arc equation
        // inside the oval's inner diamond). Given these optimizations, it's a clear win to draw
        // ovals the exact same way we do round rects.
        //
        // However, we still don't draw true circles as round rects in coverage mode, because it can
        // cause perf regressions on some platforms as compared to the dedicated circle Op.
        if (GrAAType::kCoverage != aaType || oval.height() != oval.width()) {
            assert_alive(paint);
            op = GrFillRRectOp::Make(fContext, aaType, viewMatrix, SkRRect::MakeOval(oval),
                                     *this->caps(), std::move(paint));
        }
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
    this->getRTOpList()->addOp(std::move(op), *this->caps());
}

bool GrRenderTargetContext::asyncReadPixels(SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs,
                                            const SkIRect& srcRect, ReadPixelsCallback callback,
                                            ReadPixelsContext context) {
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        return false;
    }
    if (!this->caps()->transferBufferSupport()) {
        return false;
    }
    if (fRenderTargetProxy->wrapsVkSecondaryCB()) {
        return false;
    }
    // We currently don't know our own alpha type, we assume it's premul if we have an alpha channel
    // and opaque otherwise.
    if (!GrPixelConfigIsAlphaOnly(fRenderTargetProxy->config()) && at != kPremul_SkAlphaType) {
        return false;
    }
    // TODO(bsalomon): Enhance support for reading to different color types.
    auto dstCT = SkColorTypeToGrColorType(ct);
    auto readCT = this->caps()->supportedReadPixelsColorType(fRenderTargetProxy->config(), dstCT);
    if (readCT != dstCT) {
        return false;
    }
    if (!this->caps()->transferFromOffsetAlignment(readCT)) {
        return false;
    }

    // TODO(bsalomon): Support color space conversion.
    if (!SkColorSpace::Equals(cs.get(), this->colorSpaceInfo().colorSpace())) {
        return false;
    }

    // Insert a draw to a temporary surface if we need to do a y-flip (and in future for a color
    // space conversion.)
    if (this->origin() == kBottomLeft_GrSurfaceOrigin) {
        sk_sp<GrTextureProxy> texProxy = sk_ref_sp(fRenderTargetProxy->asTextureProxy());
        const auto& backendFormat = fRenderTargetProxy->backendFormat();
        SkRect srcRectToDraw = SkRect::Make(srcRect);
        // If the src is not texturable first try to make a copy to a texture.
        if (!texProxy) {
            GrSurfaceDesc desc;
            desc.fWidth = srcRect.width();
            desc.fHeight = srcRect.height();
            desc.fConfig = fRenderTargetProxy->config();
            auto sContext = direct->priv().makeDeferredSurfaceContext(
                    backendFormat, desc, this->origin(), GrMipMapped::kNo, SkBackingFit::kApprox,
                    SkBudgeted::kNo, this->colorSpaceInfo().refColorSpace());
            if (!sContext) {
                return false;
            }
            if (!sContext->copy(fRenderTargetProxy.get(), srcRect, {0, 0})) {
                return false;
            }
            texProxy = sk_ref_sp(sContext->asTextureProxy());
            SkASSERT(texProxy);
            srcRectToDraw = SkRect::MakeWH(srcRect.width(), srcRect.height());
        }
        auto rtc = direct->priv().makeDeferredRenderTargetContext(
                backendFormat, SkBackingFit::kApprox, srcRect.width(), srcRect.height(),
                fRenderTargetProxy->config(), cs, 1, GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin);
        if (!rtc) {
            return false;
        }
        rtc->drawTexture(GrNoClip(), std::move(texProxy), GrSamplerState::Filter::kNearest,
                         SkBlendMode::kSrc, SK_PMColor4fWHITE, srcRectToDraw,
                         SkRect::MakeWH(srcRect.width(), srcRect.height()), GrAA::kNo,
                         GrQuadAAFlags::kNone, SkCanvas::kFast_SrcRectConstraint, SkMatrix::I(),
                         /* colorSpaceXform = */ nullptr);
        return rtc->asyncReadPixels(ct, at, std::move(cs),
                                    SkIRect::MakeWH(srcRect.width(), srcRect.height()), callback,
                                    context);
    }
    size_t rowBytes = GrColorTypeBytesPerPixel(dstCT) * srcRect.width();
    size_t size = rowBytes * srcRect.height();
    auto buffer = direct->priv().resourceProvider()->createBuffer(
            size, GrGpuBufferType::kXferGpuToCpu, GrAccessPattern::kStream_GrAccessPattern);
    if (!buffer) {
        return false;
    }
    this->getRTOpList()->addOp(GrTransferFromOp::Make(fContext, srcRect, dstCT, buffer, 0),
                               *this->caps());
    struct FinishContext {
        ReadPixelsCallback* fClientCallback;
        ReadPixelsContext fClientContext;
        sk_sp<GrGpuBuffer> fBuffer;
        size_t fRowBytes;
    };
    // Assumption is that the caller would like to flush. We could take a parameter or require an
    // explicit flush from the caller. We'd have to have a way to defer attaching the finish
    // callback to GrGpu until after the next flush that flushes our op list, though.
    auto* finishContext = new FinishContext{callback, context, buffer, rowBytes};
    auto finishCallback = [](GrGpuFinishedContext c) {
        auto context = reinterpret_cast<const FinishContext*>(c);
        void* data = context->fBuffer->map();
        (*context->fClientCallback)(context->fClientContext, data, data ? context->fRowBytes : 0);
        if (data) {
            context->fBuffer->unmap();
        }
        delete context;
    };
    GrFlushInfo flushInfo;
    flushInfo.fFinishedContext = finishContext;
    flushInfo.fFinishedProc = finishCallback;
    this->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    return true;
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
        this->getRTOpList()->addWaitOp(std::move(waitOp), *this->caps());
    }
    return true;
}

void GrRenderTargetContext::insertEventMarker(const SkString& str) {
    std::unique_ptr<GrOp> op(GrDebugMarkerOp::Make(fContext, fRenderTargetProxy.get(), str));
    this->getRTOpList()->addOp(std::move(op), *this->caps());
}

const GrCaps* GrRenderTargetContext::caps() const {
    return fContext->priv().caps();
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
        this->drawAndStencilRect(clip, ss, op, invert, GrAA::kNo, SkMatrix::I(),
                                 SkRect::MakeIWH(fRenderTargetContext->width(),
                                                 fRenderTargetContext->height()));
        return true;
    }

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    auto aaTypeFlags = choose_path_aa_type_flags(
            aa, fRenderTargetContext->fsaaType(), *fRenderTargetContext->caps());
    bool hasUserStencilSettings = !ss->isUnused();

    SkIRect clipConservativeBounds;
    clip.getConservativeBounds(fRenderTargetContext->width(), fRenderTargetContext->height(),
                               &clipConservativeBounds, nullptr);

    GrShape shape(path, GrStyle::SimpleFill());
    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = fRenderTargetContext->caps();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fClipConservativeBounds = &clipConservativeBounds;
    canDrawArgs.fAATypeFlags = aaTypeFlags;
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
                                      aaTypeFlags,
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
    auto aaTypeFlags = choose_path_aa_type_flags(aa, this->fsaaType(), *this->caps());

    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = this->caps();
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

    canDrawArgs.fAATypeFlags = aaTypeFlags;

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
                                      aaTypeFlags,
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
    if (!clip.apply(fContext, this, fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesHWAA,
                    fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesStencil, &appliedClip,
                    &bounds)) {
        fContext->priv().opMemoryPool()->release(std::move(op));
        return;
    }

    if (fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesStencil ||
        appliedClip.hasStencilClip()) {
        if (this->caps()->performStencilClearsAsDraws()) {
            // Must use an op to perform the clear of the stencil buffer before this op, but only
            // have to clear the first time any draw needs it (this also ensures we don't loop
            // forever when the internal stencil clear adds a draw op that has stencil settings).
            if (!fRenderTargetProxy->needsStencil()) {
                // Send false so that the stencil buffer is fully cleared to 0
                this->internalStencilClear(GrFixedClip::Disabled(), /* inside mask */ false);
            }
        } else {
            // Just make sure the stencil buffer is cleared before the draw op, easy to do it as
            // a load at the start
            this->getRTOpList()->setStencilLoadOp(GrLoadOp::kClear);
        }

        this->setNeedsStencil();
    }

    GrClampType clampType = GrPixelConfigClampType(this->colorSpaceInfo().config());
    GrXferProcessor::DstProxy dstProxy;
    GrProcessorSet::Analysis analysis = op->finalize(
            *this->caps(), &appliedClip, this->fsaaType(), clampType);
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
    opList->addDrawOp(std::move(op), analysis, std::move(appliedClip), dstProxy, *this->caps());
}

bool GrRenderTargetContext::setupDstProxy(GrRenderTargetProxy* rtProxy, const GrClip& clip,
                                          const GrOp& op,
                                          GrXferProcessor::DstProxy* dstProxy) {
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
    GrSurfaceDesc desc;
    bool rectsMustMatch = false;
    bool disallowSubrect = false;
    GrSurfaceOrigin origin;
    if (!this->caps()->initDescForDstCopy(rtProxy, &desc, &origin, &rectsMustMatch,
                                          &disallowSubrect)) {
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        desc.fConfig = rtProxy->config();
        origin = rtProxy->origin();
    }

    if (!disallowSubrect) {
        copyRect = clippedRect;
    }

    SkIPoint dstPoint, dstOffset;
    SkBackingFit fit;
    if (rectsMustMatch) {
        desc.fWidth = rtProxy->width();
        desc.fHeight = rtProxy->height();
        dstPoint = {copyRect.fLeft, copyRect.fTop};
        dstOffset = {0, 0};
        fit = SkBackingFit::kExact;
    } else {
        desc.fWidth = copyRect.width();
        desc.fHeight = copyRect.height();
        dstPoint = {0, 0};
        dstOffset = {copyRect.fLeft, copyRect.fTop};
        fit = SkBackingFit::kApprox;
    }

    SkASSERT(rtProxy->backendFormat().textureType() == GrTextureType::k2D);
    const GrBackendFormat& format = rtProxy->backendFormat();
    sk_sp<GrSurfaceContext> sContext = fContext->priv().makeDeferredSurfaceContext(
            format, desc, origin, GrMipMapped::kNo, fit, SkBudgeted::kYes,
            sk_ref_sp(this->colorSpaceInfo().colorSpace()));
    if (!sContext) {
        SkDebugf("setupDstTexture: surfaceContext creation failed.\n");
        return false;
    }

    if (!sContext->copy(rtProxy, copyRect, dstPoint)) {
        SkDebugf("setupDstTexture: copy failed.\n");
        return false;
    }

    dstProxy->setProxy(sContext->asTextureProxyRef());
    dstProxy->setOffset(dstOffset);
    return true;
}

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetContext.h"
#include "../private/GrAuditTrail.h"
#include "../private/SkShadowFlags.h"
#include "GrAppliedClip.h"
#include "GrBackendSemaphore.h"
#include "GrBlurUtils.h"
#include "GrColor.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrFixedClip.h"
#include "GrGpuResourcePriv.h"
#include "GrOpList.h"
#include "GrPathRenderer.h"
#include "GrQuad.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "GrStencilAttachment.h"
#include "GrTracing.h"
#include "SkDrawShadowInfo.h"
#include "SkGr.h"
#include "SkLatticeIter.h"
#include "SkMatrixPriv.h"
#include "SkShadowUtils.h"
#include "SkSurfacePriv.h"
#include "effects/GrRRectEffect.h"
#include "ops/GrAtlasTextOp.h"
#include "ops/GrClearOp.h"
#include "ops/GrClearStencilClipOp.h"
#include "ops/GrDebugMarkerOp.h"
#include "ops/GrDrawAtlasOp.h"
#include "ops/GrDrawOp.h"
#include "ops/GrDrawVerticesOp.h"
#include "ops/GrLatticeOp.h"
#include "ops/GrOp.h"
#include "ops/GrOvalOpFactory.h"
#include "ops/GrRectOpFactory.h"
#include "ops/GrRegionOp.h"
#include "ops/GrSemaphoreOp.h"
#include "ops/GrShadowRRectOp.h"
#include "ops/GrStencilPathOp.h"
#include "ops/GrTextureOp.h"
#include "text/GrAtlasTextContext.h"
#include "text/GrStencilAndCoverTextContext.h"
#include "text/GrTextUtils.h"

class GrRenderTargetContext::TextTarget : public GrTextUtils::Target {
public:
    TextTarget(GrRenderTargetContext* renderTargetContext)
            : Target(renderTargetContext->width(), renderTargetContext->height(),
                     renderTargetContext->colorSpaceInfo())
            , fRenderTargetContext(renderTargetContext) {}

    void addDrawOp(const GrClip& clip, std::unique_ptr<GrAtlasTextOp> op) {
        fRenderTargetContext->addDrawOp(clip, std::move(op));
    }

    void drawPath(const GrClip& clip, const SkPath& path, const SkPaint& paint,
                  const SkMatrix& viewMatrix, const SkMatrix* pathMatrix,
                  const SkIRect& clipBounds) {
        GrBlurUtils::drawPathWithMaskFilter(fRenderTargetContext->fContext, fRenderTargetContext,
                                            clip, path, paint, viewMatrix, pathMatrix, clipBounds,
                                            false);
    }

    void makeGrPaint(GrMaskFormat maskFormat, const SkPaint& skPaint, const SkMatrix& viewMatrix,
                     GrPaint* grPaint) {
        GrContext* context = fRenderTargetContext->fContext;
        const GrColorSpaceInfo& colorSpaceInfo = fRenderTargetContext->colorSpaceInfo();
        if (kARGB_GrMaskFormat == maskFormat) {
            SkPaintToGrPaintWithPrimitiveColor(context, colorSpaceInfo, skPaint, grPaint);
        } else {
            SkPaintToGrPaint(context, colorSpaceInfo, skPaint, viewMatrix, grPaint);
        }
    }

private:
    GrRenderTargetContext* fRenderTargetContext;
};

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this->drawingManager()->getContext())
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)
#define ASSERT_SINGLE_OWNER_PRIV \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fRenderTargetContext->singleOwner());)
#define RETURN_IF_ABANDONED        if (this->drawingManager()->wasAbandoned()) { return; }
#define RETURN_IF_ABANDONED_PRIV   if (fRenderTargetContext->drawingManager()->wasAbandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED  if (this->drawingManager()->wasAbandoned()) { return false; }
#define RETURN_FALSE_IF_ABANDONED_PRIV  if (fRenderTargetContext->drawingManager()->wasAbandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED   if (this->drawingManager()->wasAbandoned()) { return nullptr; }

//////////////////////////////////////////////////////////////////////////////

GrAAType GrChooseAAType(GrAA aa, GrFSAAType fsaaType, GrAllowMixedSamples allowMixedSamples,
                        const GrCaps& caps) {
    if (GrAA::kNo == aa) {
        // On some devices we cannot disable MSAA if it is enabled so we make the AA type reflect
        // that.
        if (fsaaType == GrFSAAType::kUnifiedMSAA && !caps.multisampleDisableSupport()) {
            return GrAAType::kMSAA;
        }
        return GrAAType::kNone;
    }
    switch (fsaaType) {
        case GrFSAAType::kNone:
            return GrAAType::kCoverage;
        case GrFSAAType::kUnifiedMSAA:
            return GrAAType::kMSAA;
        case GrFSAAType::kMixedSamples:
            return GrAllowMixedSamples::kYes == allowMixedSamples ? GrAAType::kMixedSamples
                                                                  : GrAAType::kCoverage;
    }
    SK_ABORT("Unexpected fsaa type");
    return GrAAType::kNone;
}

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

bool GrRenderTargetContext::wasAbandoned() const {
    return this->drawingManager()->wasAbandoned();
}

// In MDB mode the reffing of the 'getLastOpList' call's result allows in-progress
// GrOpLists to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpList, a new one will be allocated
// when the renderTargetContext attempts to use it (via getOpList).
GrRenderTargetContext::GrRenderTargetContext(GrContext* context,
                                             GrDrawingManager* drawingMgr,
                                             sk_sp<GrRenderTargetProxy> rtp,
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkSurfaceProps* surfaceProps,
                                             GrAuditTrail* auditTrail,
                                             GrSingleOwner* singleOwner,
                                             bool managedOpList)
        : GrSurfaceContext(context, drawingMgr, rtp->config(), std::move(colorSpace), auditTrail,
                           singleOwner)
        , fRenderTargetProxy(std::move(rtp))
        , fOpList(sk_ref_sp(fRenderTargetProxy->getLastRenderTargetOpList()))
        , fSurfaceProps(SkSurfacePropsCopyOrDefault(surfaceProps))
        , fManagedOpList(managedOpList) {
#ifdef SK_DISABLE_EXPLICIT_GPU_RESOURCE_ALLOCATION
    // MDB TODO: to ensure all resources still get allocated in the correct order in the hybrid
    // world we need to get the correct opList here so that it, in turn, can grab and hold
    // its rendertarget.
    this->getRTOpList();
#endif
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
        fOpList = this->drawingManager()->newRTOpList(fRenderTargetProxy.get(), fManagedOpList);
    }

    return fOpList.get();
}

GrOpList* GrRenderTargetContext::getOpList() {
    return this->getRTOpList();
}

void GrRenderTargetContext::drawText(const GrClip& clip, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix, const char text[],
                                     size_t byteLength, SkScalar x, SkScalar y,
                                     const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawText", fContext);

    GrAtlasTextContext* atlasTextContext = this->drawingManager()->getAtlasTextContext();
    atlasTextContext->drawText(fContext, fTextTarget.get(), clip, skPaint, viewMatrix,
                               fSurfaceProps, text, byteLength, x, y, clipBounds);
}

void GrRenderTargetContext::drawPosText(const GrClip& clip, const SkPaint& paint,
                                        const SkMatrix& viewMatrix, const char text[],
                                        size_t byteLength, const SkScalar pos[],
                                        int scalarsPerPosition, const SkPoint& offset,
                                        const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawPosText", fContext);

    GrAtlasTextContext* atlasTextContext = this->drawingManager()->getAtlasTextContext();
    atlasTextContext->drawPosText(fContext, fTextTarget.get(), clip, paint, viewMatrix,
                                  fSurfaceProps, text, byteLength, pos, scalarsPerPosition, offset,
                                  clipBounds);
}

void GrRenderTargetContext::drawTextBlob(const GrClip& clip, const SkPaint& paint,
                                         const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                         SkScalar x, SkScalar y, SkDrawFilter* filter,
                                         const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawTextBlob", fContext);

    GrAtlasTextContext* atlasTextContext = this->drawingManager()->getAtlasTextContext();
    atlasTextContext->drawTextBlob(fContext, fTextTarget.get(), clip, paint, viewMatrix,
                                   fSurfaceProps, blob, x, y, filter, clipBounds);
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
                                  const GrColor color,
                                  CanClearFullscreen canClearFullscreen) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "clear", fContext);

    AutoCheckFlush acf(this->drawingManager());
    this->internalClear(rect ? GrFixedClip(*rect) : GrFixedClip::Disabled(), color,
                        canClearFullscreen);
}

void GrRenderTargetContextPriv::absClear(const SkIRect* clearRect, const GrColor color) {
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
    // An op-list that is initially cleared and has no other ops should receive an
    // extra draw.
    // This path doesn't handle coalescing of full screen clears b.c. it
    // has to clear the entire render target - not just the content area.
    // It could be done but will take more finagling.
    std::unique_ptr<GrOp> op(GrClearOp::Make(rtRect, color, !clearRect));
    if (!op) {
        return;
    }
    fRenderTargetContext->getRTOpList()->addOp(std::move(op), *fRenderTargetContext->caps());
}

void GrRenderTargetContextPriv::clear(const GrFixedClip& clip,
                                      const GrColor color,
                                      CanClearFullscreen canClearFullscreen) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "clear",
                                   fRenderTargetContext->fContext);

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());
    fRenderTargetContext->internalClear(clip, color, canClearFullscreen);
}

void GrRenderTargetContext::internalClear(const GrFixedClip& clip,
                                          const GrColor color,
                                          CanClearFullscreen canClearFullscreen) {
    bool isFull = false;
    if (!clip.hasWindowRectangles()) {
        isFull = !clip.scissorEnabled() ||
                 (CanClearFullscreen::kYes == canClearFullscreen &&
                  fContext->caps()->preferFullscreenClears()) ||
                 clip.scissorRect().contains(SkIRect::MakeWH(this->width(), this->height()));
    }

    if (isFull) {
        this->getRTOpList()->fullClear(*this->caps(), color);
    } else {
        std::unique_ptr<GrOp> op(GrClearOp::Make(clip, color, this->asSurfaceProxy()));
        if (!op) {
            return;
        }
        this->getRTOpList()->addOp(std::move(op), *this->caps());
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

    SkRRect rrect;
    GrAA aa;
    // Check if we can replace a clipRRect()/drawPaint() with a drawRRect(). We only do the
    // transformation for non-rect rrects. Rects caused a performance regression on an Android
    // test that needs investigation. We also skip cases where there are fragment processors
    // because they may depend on having correct local coords and this path draws in device space
    // without a local matrix.
    if (!paint.numTotalFragmentProcessors() && clip.isRRect(r, &rrect, &aa) && !rrect.isRect()) {
        this->drawRRect(GrNoClip(), std::move(paint), aa, SkMatrix::I(), rrect,
                        GrStyle::SimpleFill());
        return;
    }


    bool isPerspective = viewMatrix.hasPerspective();

    // We attempt to map r by the inverse matrix and draw that. mapRect will
    // map the four corners and bound them with a new rect. This will not
    // produce a correct result for some perspective matrices.
    if (!isPerspective) {
        if (!SkMatrixPriv::InverseMapRect(viewMatrix, &r, r)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }
        this->drawRect(clip, std::move(paint), GrAA::kNo, viewMatrix, r);
    } else {
        SkMatrix localMatrix;
        if (!viewMatrix.invert(&localMatrix)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }

        AutoCheckFlush acf(this->drawingManager());

        std::unique_ptr<GrDrawOp> op = GrRectOpFactory::MakeNonAAFillWithLocalMatrix(
                std::move(paint), SkMatrix::I(), localMatrix, r, GrAAType::kNone);
        this->addDrawOp(clip, std::move(op));
    }
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
        return true;
    }

    return rect->intersect(clipBounds);
}

bool GrRenderTargetContext::drawFilledRect(const GrClip& clip,
                                           GrPaint&& paint,
                                           GrAA aa,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& rect,
                                           const GrUserStencilSettings* ss) {
    SkRect croppedRect = rect;
    if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix, &croppedRect)) {
        return true;
    }

    GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);
    std::unique_ptr<GrDrawOp> op;
    if (GrAAType::kCoverage == aaType) {
        op = GrRectOpFactory::MakeAAFill(std::move(paint), viewMatrix, croppedRect, ss);
    } else {
        op = GrRectOpFactory::MakeNonAAFill(std::move(paint), viewMatrix, croppedRect, aaType, ss);
    }
    if (!op) {
        return false;
    }
    this->addDrawOp(clip, std::move(op));
    return true;
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
        // Check if this is a full RT draw and can be replaced with a clear. We don't bother
        // checking cases where the RT is fully inside a stroke.
        SkRect rtRect = fRenderTargetProxy->getBoundsRect();
        // Does the clip contain the entire RT?
        if (clip.quickContains(rtRect)) {
            SkMatrix invM;
            if (!viewMatrix.invert(&invM)) {
                return;
            }
            // Does the rect bound the RT?
            GrQuad quad;
            quad.setFromMappedRect(rtRect, invM);
            if (rect_contains_inclusive(rect, quad.point(0)) &&
                rect_contains_inclusive(rect, quad.point(1)) &&
                rect_contains_inclusive(rect, quad.point(2)) &&
                rect_contains_inclusive(rect, quad.point(3))) {
                // Will it blend?
                GrColor clearColor;
                if (paint.isConstantBlendedColor(&clearColor)) {
                    this->clear(nullptr, clearColor,
                                GrRenderTargetContext::CanClearFullscreen::kYes);
                    return;
                }
            }
        }

        if (this->drawFilledRect(clip, std::move(paint), aa, viewMatrix, rect, nullptr)) {
            return;
        }
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

        GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);
        if (GrAAType::kCoverage == aaType) {
            // The stroke path needs the rect to remain axis aligned (no rotation or skew).
            if (viewMatrix.rectStaysRect()) {
                op = GrRectOpFactory::MakeAAStroke(std::move(paint), viewMatrix, rect, stroke);
            }
        } else {
            op = GrRectOpFactory::MakeNonAAStroke(std::move(paint), viewMatrix, rect, stroke,
                                                  aaType);
        }

        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
    }
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix, GrShape(rect, *style));
}

int GrRenderTargetContextPriv::maxWindowRectangles() const {
    return fRenderTargetContext->fRenderTargetProxy->maxWindowRectangles(
                                                    *fRenderTargetContext->fContext->caps());
}

void GrRenderTargetContextPriv::clearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "clearStencilClip",
                                   fRenderTargetContext->fContext);

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    std::unique_ptr<GrOp> op(GrClearStencilClipOp::Make(
                                                 clip, insideStencilMask,
                                                 fRenderTargetContext->fRenderTargetProxy.get()));
    if (!op) {
        return;
    }
    fRenderTargetContext->getRTOpList()->addOp(std::move(op), *fRenderTargetContext->caps());
}

void GrRenderTargetContextPriv::stencilPath(const GrHardClip& clip,
                                            GrAAType aaType,
                                            const SkMatrix& viewMatrix,
                                            const GrPath* path) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "stencilPath",
                                   fRenderTargetContext->fContext);

    SkASSERT(aaType != GrAAType::kCoverage);

    bool useHWAA = GrAATypeIsHW(aaType);
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

    std::unique_ptr<GrOp> op = GrStencilPathOp::Make(viewMatrix,
                                                     useHWAA,
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
                                            GrAAType aaType,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "stencilRect",
                                   fRenderTargetContext->fContext);

    SkASSERT(GrAAType::kCoverage != aaType);
    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());
    std::unique_ptr<GrDrawOp> op =
            GrRectOpFactory::MakeNonAAFill(std::move(paint), viewMatrix, rect, aaType, ss);
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

    if (fRenderTargetContext->drawFilledRect(clip, std::move(paint), aa, viewMatrix, rect, ss)) {
        return true;
    }
    SkPath path;
    path.setIsVolatile(true);
    path.addRect(rect);
    return this->drawAndStencilPath(clip, ss, op, invert, aa, viewMatrix, path);
}

void GrRenderTargetContext::fillRectToRect(const GrClip& clip,
                                           GrPaint&& paint,
                                           GrAA aa,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& rectToDraw,
                                           const SkRect& localRect) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
            GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "fillRectToRect", fContext);

    SkRect croppedRect = rectToDraw;
    SkRect croppedLocalRect = localRect;
    if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix,
                          &croppedRect, &croppedLocalRect)) {
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);
    if (GrAAType::kCoverage != aaType) {
        std::unique_ptr<GrDrawOp> op = GrRectOpFactory::MakeNonAAFillWithLocalRect(
                std::move(paint), viewMatrix, croppedRect, croppedLocalRect, aaType);
        this->addDrawOp(clip, std::move(op));
        return;
    }

    std::unique_ptr<GrDrawOp> op = GrRectOpFactory::MakeAAFillWithLocalRect(
            std::move(paint), viewMatrix, croppedRect, croppedLocalRect);
    if (op) {
        this->addDrawOp(clip, std::move(op));
        return;
    }

    SkMatrix viewAndUnLocalMatrix;
    if (!viewAndUnLocalMatrix.setRectToRect(localRect, rectToDraw, SkMatrix::kFill_ScaleToFit)) {
        SkDebugf("fillRectToRect called with empty local matrix.\n");
        return;
    }
    viewAndUnLocalMatrix.postConcat(viewMatrix);

    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewAndUnLocalMatrix,
                                     GrShape(localRect));
}

static bool must_filter(const SkRect& src, const SkRect& dst, const SkMatrix& ctm) {
    // We don't currently look for 90 degree rotations, mirroring, or downscales that sample at
    // texel centers.
    if (!ctm.isTranslate()) {
        return true;
    }
    if (src.width() != dst.width() || src.height() != dst.height()) {
        return true;
    }
    // Check that the device space rectangle's fractional offset is the same as the src rectangle,
    // and that therefore integers in the src image fall on integers in device space.
    SkScalar x = ctm.getTranslateX(), y = ctm.getTranslateY();
    x += dst.fLeft; y += dst.fTop;
    x -= src.fLeft; y -= src.fTop;
    return !SkScalarIsInt(x) || !SkScalarIsInt(y);
}

void GrRenderTargetContext::drawTextureAffine(const GrClip& clip, sk_sp<GrTextureProxy> proxy,
                                              GrSamplerState::Filter filter, GrColor color,
                                              const SkRect& srcRect, const SkRect& dstRect, GrAA aa,
                                              const SkMatrix& viewMatrix,
                                              sk_sp<GrColorSpaceXform> colorSpaceXform) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawTextureAffine", fContext);
    SkASSERT(!viewMatrix.hasPerspective());
    if (filter != GrSamplerState::Filter::kNearest && !must_filter(srcRect, dstRect, viewMatrix)) {
        filter = GrSamplerState::Filter::kNearest;
    }
    SkRect clippedDstRect = dstRect;
    SkRect clippedSrcRect = srcRect;
    if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix, &clippedDstRect,
                          &clippedSrcRect)) {
        return;
    }
    GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);
    bool allowSRGB = SkToBool(this->colorSpaceInfo().colorSpace());
    this->addDrawOp(
            clip, GrTextureOp::Make(std::move(proxy), filter, color, clippedSrcRect, clippedDstRect,
                                    aaType, viewMatrix, std::move(colorSpaceXform), allowSRGB));
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

    GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);
    if (GrAAType::kCoverage != aaType) {
        std::unique_ptr<GrDrawOp> op = GrRectOpFactory::MakeNonAAFillWithLocalMatrix(
                std::move(paint), viewMatrix, localMatrix, croppedRect, aaType);
        this->addDrawOp(clip, std::move(op));
        return;
    }

    std::unique_ptr<GrDrawOp> op = GrRectOpFactory::MakeAAFillWithLocalMatrix(
            std::move(paint), viewMatrix, localMatrix, croppedRect);
    if (op) {
        this->addDrawOp(clip, std::move(op));
        return;
    }

    SkMatrix viewAndUnLocalMatrix;
    if (!localMatrix.invert(&viewAndUnLocalMatrix)) {
        SkDebugf("fillRectWithLocalMatrix called with degenerate local matrix.\n");
        return;
    }
    viewAndUnLocalMatrix.postConcat(viewMatrix);

    SkPath path;
    path.setIsVolatile(true);
    path.addRect(rectToDraw);
    path.transform(localMatrix);
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewAndUnLocalMatrix,
                                     GrShape(path));
}

void GrRenderTargetContext::drawVertices(const GrClip& clip,
                                         GrPaint&& paint,
                                         const SkMatrix& viewMatrix,
                                         sk_sp<SkVertices> vertices,
                                         GrPrimitiveType* overridePrimType) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawVertices", fContext);

    AutoCheckFlush acf(this->drawingManager());

    SkASSERT(vertices);
    GrAAType aaType = this->chooseAAType(GrAA::kNo, GrAllowMixedSamples::kNo);
    std::unique_ptr<GrDrawOp> op = GrDrawVerticesOp::Make(
            std::move(paint), std::move(vertices), viewMatrix, aaType,
            this->colorSpaceInfo().isGammaCorrect(),
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

    GrAAType aaType = this->chooseAAType(GrAA::kNo, GrAllowMixedSamples::kNo);
    std::unique_ptr<GrDrawOp> op = GrDrawAtlasOp::Make(std::move(paint), viewMatrix, aaType,
                                                       spriteCount, xform, texRect, colors);
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
    if (rrect.isEmpty()) {
       return;
    }

    GrNoClip noclip;
    const GrClip* clip = &origClip;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // The Android framework frequently clips rrects to themselves where the clip is non-aa and the
    // draw is aa. Since our lower level clip code works from op bounds, which are SkRects, it
    // doesn't detect that the clip can be ignored (modulo antialiasing). The following test
    // attempts to mitigate the stencil clip cost but will only help when the entire clip stack
    // can be ignored. We'd prefer to fix this in the framework by removing the clips calls.
    SkRRect devRRect;
    if (rrect.transform(viewMatrix, &devRRect) && clip->quickContains(devRRect)) {
        clip = &noclip;
    }
#endif
    SkASSERT(!style.pathEffect()); // this should've been devolved to a path in SkGpuDevice

    AutoCheckFlush acf(this->drawingManager());
    const SkStrokeRec stroke = style.strokeRec();

    GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);
    if (GrAAType::kCoverage == aaType) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        std::unique_ptr<GrDrawOp> op = GrOvalOpFactory::MakeRRectOp(std::move(paint),
                                                                    viewMatrix,
                                                                    rrect,
                                                                    stroke,
                                                                    shaderCaps);
        if (op) {
            this->addDrawOp(*clip, std::move(op));
            return;
        }
    }

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
                                           GrColor color4ub,
                                           const SkMatrix& viewMatrix,
                                           const SkPath& path,
                                           const SkDrawShadowRec& rec) {
    ASSERT_SINGLE_OWNER
    if (this->drawingManager()->wasAbandoned()) {
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
    bool isRRect = path.isRRect(&rrect) && rrect.isSimpleCircular() &&
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
    GrColor4f color = GrColor4f::FromGrColor(color4ub);
    bool transparent = SkToBool(rec.fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag);
    bool tonalColor = !SkToBool(rec.fFlags & SkShadowFlags::kDisableTonalColor_ShadowFlag);

    if (rec.fAmbientAlpha > 0) {
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
            SkScalar outsetRad = rrect.getSimpleRadii().fX + ambientPathOutset;
            ambientRRect = SkRRect::MakeRectXY(outsetRect, outsetRad, outsetRad);
        }

        GrColor ambientColor;
        if (tonalColor) {
            // with tonal color, the color only applies to the spot shadow
            ambientColor = GrColorPackRGBA(0, 0, 0, 255.999f*rec.fAmbientAlpha);
        } else {
            ambientColor = color.mulByScalar(rec.fAmbientAlpha).toGrColor();
        }
        if (transparent) {
            // set a large inset to force a fill
            devSpaceInsetWidth = ambientRRect.width();
        }
        // the fraction of the blur we want to apply is devSpaceInsetWidth/devSpaceAmbientBlur,
        // which is just 1/umbraRecipAlpha.
        SkScalar blurClamp = SkScalarInvert(umbraRecipAlpha);

        std::unique_ptr<GrDrawOp> op = GrShadowRRectOp::Make(ambientColor, viewMatrix,
                                                             ambientRRect,
                                                             devSpaceAmbientBlur,
                                                             devSpaceInsetWidth,
                                                             blurClamp);
        SkASSERT(op);
        this->addDrawOp(clip, std::move(op));
    }

    if (rec.fSpotAlpha > 0) {
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
                maxOffset = SkTMax(SkTMax(SkTAbs(spotShadowRRect.rect().fLeft -
                                                 rrect.rect().fLeft),
                                          SkTAbs(spotShadowRRect.rect().fTop -
                                                 rrect.rect().fTop)),
                                   SkTMax(SkTAbs(spotShadowRRect.rect().fRight -
                                                 rrect.rect().fRight),
                                          SkTAbs(spotShadowRRect.rect().fBottom -
                                                 rrect.rect().fBottom)));
            } else {
                SkScalar dr = spotRadius - rrect.getSimpleRadii().fX;
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
            insetWidth += maxOffset;
        }

        // Outset the shadow rrect to the border of the penumbra
        SkRect outsetRect = spotShadowRRect.rect().makeOutset(blurOutset, blurOutset);
        if (spotShadowRRect.isOval()) {
            spotShadowRRect = SkRRect::MakeOval(outsetRect);
        } else {
            SkScalar outsetRad = spotRadius + blurOutset;
            spotShadowRRect = SkRRect::MakeRectXY(outsetRect, outsetRad, outsetRad);
        }

        GrColor spotColor;
        if (tonalColor) {
            SkScalar colorScale;
            SkScalar tonalAlpha;
            SkShadowUtils::ComputeTonalColorParams(color.fRGBA[0], color.fRGBA[1],
                                                   color.fRGBA[2], color.fRGBA[3]*rec.fSpotAlpha,
                                                   &colorScale, &tonalAlpha);
            color.fRGBA[0] *= colorScale;
            color.fRGBA[1] *= colorScale;
            color.fRGBA[2] *= colorScale;
            color.fRGBA[3] = tonalAlpha;
            spotColor = color.toGrColor();
        } else {
            spotColor = color.mulByScalar(rec.fSpotAlpha).toGrColor();
        }

        std::unique_ptr<GrDrawOp> op = GrShadowRRectOp::Make(spotColor, viewMatrix,
                                                             spotShadowRRect,
                                                             2.0f * devSpaceSpotBlur,
                                                             insetWidth);
        SkASSERT(op);
        this->addDrawOp(clip, std::move(op));
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

    GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);

    if (GrAAType::kMSAA == aaType) {
        return false;
    }

    if (GrAAType::kCoverage == aaType && inner->isCircle() && outer->isCircle()) {
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
            auto op = GrOvalOpFactory::MakeOvalOp(std::move(paint), viewMatrix, circleBounds,
                                                  stroke, this->caps()->shaderCaps());
            if (op) {
                this->addDrawOp(clip, std::move(op));
                return true;
            }
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
        return this->drawPath(clip, std::move(paint), aa, viewMatrix, path, style);
    }

    GrAAType aaType = this->chooseAAType(GrAA::kNo, GrAllowMixedSamples::kNo);
    std::unique_ptr<GrDrawOp> op = GrRegionOp::Make(std::move(paint), viewMatrix, region, aaType,
                                                    ss);
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

    if (oval.isEmpty()) {
       return;
    }

    SkASSERT(!style.pathEffect()); // this should've been devolved to a path in SkGpuDevice

    AutoCheckFlush acf(this->drawingManager());
    const SkStrokeRec& stroke = style.strokeRec();

    GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);
    if (GrAAType::kCoverage == aaType) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        std::unique_ptr<GrDrawOp> op =
                GrOvalOpFactory::MakeOvalOp(std::move(paint), viewMatrix, oval, stroke, shaderCaps);
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
    }

    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix,
                                     GrShape(SkRRect::MakeOval(oval), style));
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

    GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);
    if (GrAAType::kCoverage == aaType) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        std::unique_ptr<GrDrawOp> op = GrOvalOpFactory::MakeArcOp(std::move(paint),
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
    }
    SkPath path;
    SkPathPriv::CreateDrawArcPath(&path, oval, startAngle, sweepAngle, useCenter,
                                  style.isSimpleFill());
    this->drawShapeUsingPathRenderer(clip, std::move(paint), aa, viewMatrix, GrShape(path, style));
}

void GrRenderTargetContext::drawImageLattice(const GrClip& clip,
                                             GrPaint&& paint,
                                             const SkMatrix& viewMatrix,
                                             int imageWidth,
                                             int imageHeight,
                                             std::unique_ptr<SkLatticeIter> iter,
                                             const SkRect& dst) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "drawImageLattice", fContext);

    AutoCheckFlush acf(this->drawingManager());

    std::unique_ptr<GrDrawOp> op = GrLatticeOp::MakeNonAA(std::move(paint), viewMatrix, imageWidth,
                                                          imageHeight, std::move(iter), dst);
    this->addDrawOp(clip, std::move(op));
}

GrSemaphoresSubmitted GrRenderTargetContext::prepareForExternalIO(
        int numSemaphores, GrBackendSemaphore backendSemaphores[]) {
    ASSERT_SINGLE_OWNER
    if (this->drawingManager()->wasAbandoned()) { return GrSemaphoresSubmitted::kNo; }
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "prepareForExternalIO", fContext);

    return this->drawingManager()->prepareSurfaceForExternalIO(fRenderTargetProxy.get(),
                                                               numSemaphores,
                                                               backendSemaphores);
}

bool GrRenderTargetContext::waitOnSemaphores(int numSemaphores,
                                             const GrBackendSemaphore* waitSemaphores) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "waitOnSemaphores", fContext);

    AutoCheckFlush acf(this->drawingManager());

    if (numSemaphores && !this->caps()->fenceSyncSupport()) {
        return false;
    }

    auto resourceProvider = fContext->contextPriv().resourceProvider();

    SkTArray<sk_sp<GrSemaphore>> semaphores(numSemaphores);
    for (int i = 0; i < numSemaphores; ++i) {
        sk_sp<GrSemaphore> sema = resourceProvider->wrapBackendSemaphore(
                waitSemaphores[i], GrResourceProvider::SemaphoreWrapType::kWillWait,
                kAdopt_GrWrapOwnership);
        std::unique_ptr<GrOp> waitOp(GrSemaphoreOp::MakeWait(sema, fRenderTargetProxy.get()));
        this->getRTOpList()->addOp(std::move(waitOp), *this->caps());
    }
    return true;
}

void GrRenderTargetContext::insertEventMarker(const SkString& str) {
    std::unique_ptr<GrOp> op(GrDebugMarkerOp::Make(fRenderTargetProxy.get(), str));
    this->getRTOpList()->addOp(std::move(op), *this->caps());
}


// Can 'path' be drawn as a pair of filled nested rectangles?
static bool fills_as_nested_rects(const SkMatrix& viewMatrix, const SkPath& path, SkRect rects[2]) {

    if (path.isInverseFillType()) {
        return false;
    }

    // TODO: this restriction could be lifted if we were willing to apply
    // the matrix to all the points individually rather than just to the rect
    if (!viewMatrix.rectStaysRect()) {
        return false;
    }

    SkPath::Direction dirs[2];
    if (!path.isNestedFillRects(rects, dirs)) {
        return false;
    }

    if (SkPath::kWinding_FillType == path.getFillType() && dirs[0] == dirs[1]) {
        // The two rects need to be wound opposite to each other
        return false;
    }

    // Right now, nested rects where the margin is not the same width
    // all around do not render correctly
    const SkScalar* outer = rects[0].asScalars();
    const SkScalar* inner = rects[1].asScalars();

    bool allEq = true;

    SkScalar margin = SkScalarAbs(outer[0] - inner[0]);
    bool allGoE1 = margin >= SK_Scalar1;

    for (int i = 1; i < 4; ++i) {
        SkScalar temp = SkScalarAbs(outer[i] - inner[i]);
        if (temp < SK_Scalar1) {
            allGoE1 = false;
        }
        if (!SkScalarNearlyEqual(margin, temp)) {
            allEq = false;
        }
    }

    return allEq || allGoE1;
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
            GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContextPriv", "drawPath", fContext);

    GrShape shape(path, style);
    if (shape.isEmpty()) {
        if (shape.inverseFilled()) {
            this->drawPaint(clip, std::move(paint), viewMatrix);
        }
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    if (!shape.style().hasPathEffect()) {
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
        }
    }

    GrAAType aaType = this->chooseAAType(aa, GrAllowMixedSamples::kNo);
    if (GrAAType::kCoverage == aaType) {
        // TODO: Make GrShape check for nested rects.
        SkRect rects[2];
        if (shape.style().isSimpleFill() && fills_as_nested_rects(viewMatrix, path, rects)) {
            // Concave AA paths are expensive - try to avoid them for special cases
            SkRect rects[2];

            if (fills_as_nested_rects(viewMatrix, path, rects)) {
                std::unique_ptr<GrDrawOp> op =
                        GrRectOpFactory::MakeAAFillNestedRects(std::move(paint), viewMatrix, rects);
                if (op) {
                    this->addDrawOp(clip, std::move(op));
                }
                // A null return indicates that there is nothing to draw in this case.
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
    GrAAType aaType = fRenderTargetContext->chooseAAType(aa, GrAllowMixedSamples::kNo);
    bool hasUserStencilSettings = !ss->isUnused();

    SkIRect clipConservativeBounds;
    clip.getConservativeBounds(fRenderTargetContext->width(), fRenderTargetContext->height(),
                               &clipConservativeBounds, nullptr);

    GrShape shape(path, GrStyle::SimpleFill());
    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = fRenderTargetContext->drawingManager()->getContext()->caps();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fClipConservativeBounds = &clipConservativeBounds;
    canDrawArgs.fAAType = aaType;
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
                                      fRenderTargetContext->colorSpaceInfo().isGammaCorrect()};
    pr->drawPath(args);
    return true;
}

SkBudgeted GrRenderTargetContextPriv::isBudgeted() const {
    ASSERT_SINGLE_OWNER_PRIV

    if (fRenderTargetContext->wasAbandoned()) {
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

    SkIRect clipConservativeBounds;
    clip.getConservativeBounds(this->width(), this->height(), &clipConservativeBounds, nullptr);

    GrShape tempShape;
    // NVPR cannot handle hairlines, so this would get picked up by a different stencil and
    // cover path renderer (i.e. default path renderer). The hairline renderer produces much
    // smoother hairlines than MSAA.
    GrAllowMixedSamples allowMixedSamples = originalShape.style().isSimpleHairline()
                                                    ? GrAllowMixedSamples::kNo
                                                    : GrAllowMixedSamples::kYes;
    GrAAType aaType = this->chooseAAType(aa, allowMixedSamples);
    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fCaps = this->drawingManager()->getContext()->caps();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &originalShape;
    canDrawArgs.fClipConservativeBounds = &clipConservativeBounds;
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
        }
        // This time, allow SW renderer
        pr = this->drawingManager()->getPathRenderer(canDrawArgs, true, kType);
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
                                      this->colorSpaceInfo().isGammaCorrect()};
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

uint32_t GrRenderTargetContext::addDrawOp(const GrClip& clip, std::unique_ptr<GrDrawOp> op) {
    ASSERT_SINGLE_OWNER
    if (this->drawingManager()->wasAbandoned()) {
        return SK_InvalidUniqueID;
    }
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrRenderTargetContext", "addDrawOp", fContext);

    // Setup clip
    SkRect bounds;
    op_bounds(&bounds, op.get());
    GrAppliedClip appliedClip;
    GrDrawOp::FixedFunctionFlags fixedFunctionFlags = op->fixedFunctionFlags();
    if (!clip.apply(fContext, this, fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesHWAA,
                    fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesStencil, &appliedClip,
                    &bounds)) {
        return SK_InvalidUniqueID;
    }

    if (fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesStencil ||
        appliedClip.hasStencilClip()) {
        this->getOpList()->setStencilLoadOp(GrLoadOp::kClear);

        this->setNeedsStencil();
    }

    GrPixelConfigIsClamped dstIsClamped =
            GrGetPixelConfigIsClamped(this->colorSpaceInfo().config());
    GrXferProcessor::DstProxy dstProxy;
    if (GrDrawOp::RequiresDstTexture::kYes == op->finalize(*this->caps(), &appliedClip,
                                                           dstIsClamped)) {
        if (!this->setupDstProxy(this->asRenderTargetProxy(), clip, op->bounds(), &dstProxy)) {
            return SK_InvalidUniqueID;
        }
    }

    op->setClippedBounds(bounds);
    return this->getRTOpList()->addOp(std::move(op), *this->caps(),
                                      std::move(appliedClip), dstProxy);
}

bool GrRenderTargetContext::setupDstProxy(GrRenderTargetProxy* rtProxy, const GrClip& clip,
                                          const SkRect& opBounds,
                                          GrXferProcessor::DstProxy* dstProxy) {
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
    SkIRect drawIBounds;
    opBounds.roundOut(&drawIBounds);
    // Cover up for any precision issues by outsetting the op bounds a pixel in each direction.
    drawIBounds.outset(1, 1);
    if (!clippedRect.intersect(drawIBounds)) {
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
    if (!this->caps()->initDescForDstCopy(rtProxy, &desc, &rectsMustMatch, &disallowSubrect)) {
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
        desc.fConfig = rtProxy->config();
    }

    if (!disallowSubrect) {
        copyRect = clippedRect;
    }

    SkIPoint dstPoint, dstOffset;
    SkBackingFit fit;
    if (rectsMustMatch) {
        SkASSERT(desc.fOrigin == rtProxy->origin());
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

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeDeferredSurfaceContext(
                                                                                desc,
                                                                                GrMipMapped::kNo,
                                                                                fit,
                                                                                SkBudgeted::kYes);
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

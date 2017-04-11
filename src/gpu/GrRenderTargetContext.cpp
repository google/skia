/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetContext.h"
#include "GrAppliedClip.h"
#include "GrColor.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrFixedClip.h"
#include "GrGpuResourcePriv.h"
#include "GrPathRenderer.h"
#include "GrPipelineBuilder.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetContextPriv.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceProvider.h"
#include "GrStencilAttachment.h"
#include "SkLatticeIter.h"
#include "SkMatrixPriv.h"
#include "SkSurfacePriv.h"
#include "effects/GrRRectEffect.h"
#include "instanced/InstancedRendering.h"
#include "ops/GrClearOp.h"
#include "ops/GrClearStencilClipOp.h"
#include "ops/GrDrawOp.h"
#include "ops/GrDrawAtlasOp.h"
#include "ops/GrDrawVerticesOp.h"
#include "ops/GrLatticeOp.h"
#include "ops/GrOp.h"
#include "ops/GrOvalOpFactory.h"
#include "ops/GrRectOpFactory.h"
#include "ops/GrRegionOp.h"
#include "ops/GrShadowRRectOp.h"
#include "ops/GrStencilPathOp.h"
#include "text/GrAtlasTextContext.h"
#include "text/GrStencilAndCoverTextContext.h"
#include "../private/GrAuditTrail.h"

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

using gr_instanced::InstancedRendering;

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
                                             GrSingleOwner* singleOwner)
    : GrSurfaceContext(context, drawingMgr, std::move(colorSpace), auditTrail, singleOwner)
    , fRenderTargetProxy(std::move(rtp))
    , fOpList(SkSafeRef(fRenderTargetProxy->getLastRenderTargetOpList()))
    , fInstancedPipelineInfo(fRenderTargetProxy.get())
    , fColorXformFromSRGB(nullptr)
    , fSurfaceProps(SkSurfacePropsCopyOrDefault(surfaceProps)) {
    if (fColorSpace) {
        // sRGB sources are very common (SkColor, etc...), so we cache that gamut transformation
        auto srgbColorSpace = SkColorSpace::MakeSRGB();
        fColorXformFromSRGB = GrColorSpaceXform::Make(srgbColorSpace.get(), fColorSpace.get());
    }
    SkDEBUGCODE(this->validate();)
}

#ifdef SK_DEBUG
void GrRenderTargetContext::validate() const {
    SkASSERT(fRenderTargetProxy);
    fRenderTargetProxy->validate(fContext);

    if (fOpList && !fOpList->isClosed()) {
        SkASSERT(fRenderTargetProxy->getLastOpList() == fOpList);
    }
}
#endif

GrRenderTargetContext::~GrRenderTargetContext() {
    ASSERT_SINGLE_OWNER
    SkSafeUnref(fOpList);
}

GrTextureProxy* GrRenderTargetContext::asTextureProxy() {
    return fRenderTargetProxy->asTextureProxy();
}

sk_sp<GrTextureProxy> GrRenderTargetContext::asTextureProxyRef() {
    return sk_ref_sp(fRenderTargetProxy->asTextureProxy());
}

GrRenderTargetOpList* GrRenderTargetContext::getOpList() {
    ASSERT_SINGLE_OWNER
    SkDEBUGCODE(this->validate();)

    if (!fOpList || fOpList->isClosed()) {
        fOpList = this->drawingManager()->newOpList(fRenderTargetProxy.get());
    }

    return fOpList;
}

// TODO: move this (and GrTextContext::copy) to GrSurfaceContext?
bool GrRenderTargetContext::onCopy(GrSurfaceProxy* srcProxy,
                                   const SkIRect& srcRect,
                                   const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::onCopy");

    return this->getOpList()->copySurface(fContext->resourceProvider(),
                                          fRenderTargetProxy.get(), srcProxy, srcRect, dstPoint);
}

void GrRenderTargetContext::drawText(const GrClip& clip, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix, const char text[],
                                     size_t byteLength, SkScalar x, SkScalar y,
                                     const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawText");

    GrAtlasTextContext* atlasTextContext = this->drawingManager()->getAtlasTextContext();
    atlasTextContext->drawText(fContext, this, clip, skPaint, viewMatrix, fSurfaceProps, text,
                               byteLength, x, y, clipBounds);
}

void GrRenderTargetContext::drawPosText(const GrClip& clip, const SkPaint& paint,
                                        const SkMatrix& viewMatrix, const char text[],
                                        size_t byteLength, const SkScalar pos[],
                                        int scalarsPerPosition, const SkPoint& offset,
                                        const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawPosText");

    GrAtlasTextContext* atlasTextContext = this->drawingManager()->getAtlasTextContext();
    atlasTextContext->drawPosText(fContext, this, clip, paint, viewMatrix, fSurfaceProps, text,
                                  byteLength, pos, scalarsPerPosition, offset, clipBounds);
}

void GrRenderTargetContext::drawTextBlob(const GrClip& clip, const SkPaint& paint,
                                         const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                         SkScalar x, SkScalar y, SkDrawFilter* filter,
                                         const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawTextBlob");

    GrAtlasTextContext* atlasTextContext = this->drawingManager()->getAtlasTextContext();
    atlasTextContext->drawTextBlob(fContext, this, clip, paint, viewMatrix, fSurfaceProps, blob, x,
                                   y, filter, clipBounds);
}

void GrRenderTargetContext::discard() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::discard");

    AutoCheckFlush acf(this->drawingManager());

    this->getOpList()->discard(this);
}

void GrRenderTargetContext::clear(const SkIRect* rect,
                                  const GrColor color,
                                  bool canIgnoreRect) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::clear");

    AutoCheckFlush acf(this->drawingManager());
    this->internalClear(rect ? GrFixedClip(*rect) : GrFixedClip::Disabled(), color, canIgnoreRect);
}

void GrRenderTargetContextPriv::absClear(const SkIRect* clearRect, const GrColor color) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContext::absClear");

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    SkIRect rtRect = SkIRect::MakeWH(fRenderTargetContext->fRenderTargetProxy->worstCaseWidth(
                                            *fRenderTargetContext->caps()),
                                     fRenderTargetContext->fRenderTargetProxy->worstCaseHeight(
                                            *fRenderTargetContext->caps()));

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
    if (fRenderTargetContext->fContext->caps()->useDrawInsteadOfClear()) {
        // This works around a driver bug with clear by drawing a rect instead.
        // The driver will ignore a clear if it is the only thing rendered to a
        // target before the target is read.
        GrPaint paint;
        paint.setColor4f(GrColor4f::FromGrColor(color));
        paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));

        // We don't call drawRect() here to avoid the cropping to the, possibly smaller,
        // RenderTargetProxy bounds
        fRenderTargetContext->drawNonAAFilledRect(GrNoClip(), std::move(paint), SkMatrix::I(),
                                                  SkRect::Make(rtRect), nullptr, nullptr, nullptr,
                                                  GrAAType::kNone);

    } else {
        // This path doesn't handle coalescing of full screen clears b.c. it
        // has to clear the entire render target - not just the content area.
        // It could be done but will take more finagling.
        std::unique_ptr<GrOp> op(GrClearOp::Make(rtRect, color, fRenderTargetContext, !clearRect));
        if (!op) {
            return;
        }
        fRenderTargetContext->getOpList()->addOp(std::move(op), fRenderTargetContext);
    }
}

void GrRenderTargetContextPriv::clear(const GrFixedClip& clip,
                                      const GrColor color,
                                      bool canIgnoreClip) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContextPriv::clear");

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());
    fRenderTargetContext->internalClear(clip, color, canIgnoreClip);
}

void GrRenderTargetContext::internalClear(const GrFixedClip& clip,
                                          const GrColor color,
                                          bool canIgnoreClip) {
    bool isFull = false;
    if (!clip.hasWindowRectangles()) {
        isFull = !clip.scissorEnabled() ||
                 (canIgnoreClip && fContext->caps()->fullClearIsFree()) ||
                 clip.scissorRect().contains(SkIRect::MakeWH(this->width(), this->height()));
    }

    if (fContext->caps()->useDrawInsteadOfClear()) {
        // This works around a driver bug with clear by drawing a rect instead.
        // The driver will ignore a clear if it is the only thing rendered to a
        // target before the target is read.
        SkIRect clearRect = SkIRect::MakeWH(this->width(), this->height());
        if (isFull) {
            this->discard();
        } else if (!clearRect.intersect(clip.scissorRect())) {
            return;
        }

        GrPaint paint;
        paint.setColor4f(GrColor4f::FromGrColor(color));
        paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));

        this->drawRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(), SkRect::Make(clearRect));
    } else if (isFull) {
        this->getOpList()->fullClear(this, color);
    } else {
        std::unique_ptr<GrOp> op(GrClearOp::Make(clip, color, this));
        if (!op) {
            return;
        }
        this->getOpList()->addOp(std::move(op), this);
    }
}

void GrRenderTargetContext::drawPaint(const GrClip& clip,
                                      GrPaint&& paint,
                                      const SkMatrix& viewMatrix) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawPaint");

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

        this->drawNonAAFilledRect(clip, std::move(paint), SkMatrix::I(), r, nullptr, &localMatrix,
                                  nullptr, GrAAType::kNone);
    }
}

static inline bool rect_contains_inclusive(const SkRect& rect, const SkPoint& point) {
    return point.fX >= rect.fLeft && point.fX <= rect.fRight &&
           point.fY >= rect.fTop && point.fY <= rect.fBottom;
}

static bool view_matrix_ok_for_aa_fill_rect(const SkMatrix& viewMatrix) {
    return viewMatrix.preservesRightAngles();
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

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport() &&
        (!ss || ss->isDisabled(false))) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        std::unique_ptr<GrDrawOp> op = ir->recordRect(croppedRect, viewMatrix, std::move(paint), aa,
                                                      fInstancedPipelineInfo);
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return true;
        }
    }
    GrAAType aaType = this->decideAAType(aa);
    if (GrAAType::kCoverage == aaType) {
        // The fill path can handle rotation but not skew.
        if (view_matrix_ok_for_aa_fill_rect(viewMatrix)) {
            SkRect devBoundRect;
            viewMatrix.mapRect(&devBoundRect, croppedRect);
            std::unique_ptr<GrLegacyMeshDrawOp> op =
                    GrRectOpFactory::MakeAAFill(paint, viewMatrix, rect, croppedRect, devBoundRect);
            if (op) {
                GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
                if (ss) {
                    pipelineBuilder.setUserStencil(ss);
                }
                this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
                return true;
            }
        }
    } else {
        this->drawNonAAFilledRect(clip, std::move(paint), viewMatrix, croppedRect, nullptr, nullptr,
                                  ss, aaType);
        return true;
    }

    return false;
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawRect");

    // Path effects should've been devolved to a path in SkGpuDevice
    SkASSERT(!style->pathEffect());

    AutoCheckFlush acf(this->drawingManager());

    const SkStrokeRec& stroke = style->strokeRec();
    if (stroke.getStyle() == SkStrokeRec::kFill_Style) {
        
        if (!fContext->caps()->useDrawInsteadOfClear()) {
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
                SkPoint srcSpaceRTQuad[4];
                invM.mapRectToQuad(srcSpaceRTQuad, rtRect);
                if (rect_contains_inclusive(rect, srcSpaceRTQuad[0]) &&
                    rect_contains_inclusive(rect, srcSpaceRTQuad[1]) &&
                    rect_contains_inclusive(rect, srcSpaceRTQuad[2]) &&
                    rect_contains_inclusive(rect, srcSpaceRTQuad[3])) {
                    // Will it blend?
                    GrColor clearColor;
                    if (paint.isConstantBlendedColor(&clearColor)) {
                        this->clear(nullptr, clearColor, true);
                        return;
                    }
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

        bool snapToPixelCenters = false;
        std::unique_ptr<GrLegacyMeshDrawOp> op;

        GrColor color = paint.getColor();
        GrAAType aaType = this->decideAAType(aa);
        if (GrAAType::kCoverage == aaType) {
            // The stroke path needs the rect to remain axis aligned (no rotation or skew).
            if (viewMatrix.rectStaysRect()) {
                op = GrRectOpFactory::MakeAAStroke(color, viewMatrix, rect, stroke);
            }
        } else {
            // Depending on sub-pixel coordinates and the particular GPU, we may lose a corner of
            // hairline rects. We jam all the vertices to pixel centers to avoid this, but not
            // when MSAA is enabled because it can cause ugly artifacts.
            snapToPixelCenters = stroke.getStyle() == SkStrokeRec::kHairline_Style &&
                                 !fRenderTargetProxy->isUnifiedMultisampled();
            op = GrRectOpFactory::MakeNonAAStroke(color, viewMatrix, rect, stroke,
                                                  snapToPixelCenters);
        }

        if (op) {
            GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
            pipelineBuilder.setSnapVerticesToPixelCenters(snapToPixelCenters);
            this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
            return;
        }
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRect(rect);
    this->internalDrawPath(clip, std::move(paint), aa, viewMatrix, path, *style);
}

int GrRenderTargetContextPriv::maxWindowRectangles() const {
    return fRenderTargetContext->fRenderTargetProxy->maxWindowRectangles(
                                                    *fRenderTargetContext->fContext->caps());
}

void GrRenderTargetContextPriv::clearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContextPriv::clearStencilClip");

    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    std::unique_ptr<GrOp> op(GrClearStencilClipOp::Make(clip, insideStencilMask,
                                                        fRenderTargetContext));
    if (!op) {
        return;
    }
    fRenderTargetContext->getOpList()->addOp(std::move(op), fRenderTargetContext);
}

void GrRenderTargetContextPriv::stencilPath(const GrClip& clip,
                                            GrAAType aaType,
                                            const SkMatrix& viewMatrix,
                                            const GrPath* path) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContext::stencilPath");

    SkASSERT(aaType != GrAAType::kCoverage);

    bool useHWAA = GrAATypeIsHW(aaType);
    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(path);
    SkASSERT(fRenderTargetContext->caps()->shaderCaps()->pathRenderingSupport());

    // FIXME: Use path bounds instead of this WAR once
    // https://bugs.chromium.org/p/skia/issues/detail?id=5640 is resolved.
    SkRect bounds = SkRect::MakeIWH(fRenderTargetContext->width(), fRenderTargetContext->height());

    // Setup clip
    GrAppliedClip appliedClip;
    if (!clip.apply(fRenderTargetContext->fContext, fRenderTargetContext, useHWAA, true,
                    &appliedClip, &bounds)) {
        return;
    }

    // Coverage AA does not make sense when rendering to the stencil buffer. The caller should never
    // attempt this in a situation that would require coverage AA.
    SkASSERT(!appliedClip.clipCoverageFragmentProcessor());

    GrRenderTarget* rt = fRenderTargetContext->accessRenderTarget();
    if (!rt) {
        return;
    }
    GrStencilAttachment* stencilAttachment =
            fRenderTargetContext->fContext->resourceProvider()->attachStencilAttachment(rt);
    if (!stencilAttachment) {
        SkDebugf("ERROR creating stencil attachment. Draw skipped.\n");
        return;
    }

    std::unique_ptr<GrOp> op = GrStencilPathOp::Make(viewMatrix,
                                                     useHWAA,
                                                     path->getFillType(),
                                                     appliedClip.hasStencilClip(),
                                                     stencilAttachment->bits(),
                                                     appliedClip.scissorState(),
                                                     fRenderTargetContext,
                                                     path);
    if (!op) {
        return;
    }
    op->setClippedBounds(bounds);
    fRenderTargetContext->getOpList()->addOp(std::move(op), fRenderTargetContext);
}

void GrRenderTargetContextPriv::stencilRect(const GrClip& clip,
                                            const GrUserStencilSettings* ss,
                                            GrAAType aaType,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContext::stencilRect");
    SkASSERT(GrAAType::kCoverage != aaType);
    AutoCheckFlush acf(fRenderTargetContext->drawingManager());

    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());

    fRenderTargetContext->drawNonAAFilledRect(clip, std::move(paint), viewMatrix, rect, nullptr,
                                              nullptr, ss, aaType);
}

bool GrRenderTargetContextPriv::drawAndStencilRect(const GrClip& clip,
                                                   const GrUserStencilSettings* ss,
                                                   SkRegion::Op op,
                                                   bool invert,
                                                   GrAA aa,
                                                   const SkMatrix& viewMatrix,
                                                   const SkRect& rect) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContext::drawAndStencilRect");

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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::fillRectToRect");

    SkRect croppedRect = rectToDraw;
    SkRect croppedLocalRect = localRect;
    if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix,
                          &croppedRect, &croppedLocalRect)) {
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        std::unique_ptr<GrDrawOp> op(ir->recordRect(croppedRect, viewMatrix, std::move(paint),
                                                    croppedLocalRect, aa, fInstancedPipelineInfo));
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
    }

    GrAAType aaType = this->decideAAType(aa);
    if (GrAAType::kCoverage != aaType) {
        this->drawNonAAFilledRect(clip, std::move(paint), viewMatrix, croppedRect,
                                  &croppedLocalRect, nullptr, nullptr, aaType);
        return;
    }

    if (view_matrix_ok_for_aa_fill_rect(viewMatrix)) {
        std::unique_ptr<GrLegacyMeshDrawOp> op = GrAAFillRectOp::MakeWithLocalRect(
                paint.getColor(), viewMatrix, croppedRect, croppedLocalRect);
        GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
        this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
        return;
    }

    SkMatrix viewAndUnLocalMatrix;
    if (!viewAndUnLocalMatrix.setRectToRect(localRect, rectToDraw, SkMatrix::kFill_ScaleToFit)) {
        SkDebugf("fillRectToRect called with empty local matrix.\n");
        return;
    }
    viewAndUnLocalMatrix.postConcat(viewMatrix);

    SkPath path;
    path.setIsVolatile(true);
    path.addRect(localRect);
    this->internalDrawPath(clip, std::move(paint), aa, viewAndUnLocalMatrix, path, GrStyle());
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::fillRectWithLocalMatrix");

    SkRect croppedRect = rectToDraw;
    if (!crop_filled_rect(this->width(), this->height(), clip, viewMatrix, &croppedRect)) {
        return;
    }

    AutoCheckFlush acf(this->drawingManager());

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        std::unique_ptr<GrDrawOp> op(ir->recordRect(croppedRect, viewMatrix, std::move(paint),
                                                    localMatrix, aa, fInstancedPipelineInfo));
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
    }

    GrAAType aaType = this->decideAAType(aa);
    if (GrAAType::kCoverage != aaType) {
        this->drawNonAAFilledRect(clip, std::move(paint), viewMatrix, croppedRect, nullptr,
                                  &localMatrix, nullptr, aaType);
        return;
    }

    if (view_matrix_ok_for_aa_fill_rect(viewMatrix)) {
        std::unique_ptr<GrLegacyMeshDrawOp> op =
                GrAAFillRectOp::Make(paint.getColor(), viewMatrix, localMatrix, croppedRect);
        GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
        this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
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
    this->internalDrawPath(clip, std::move(paint), aa, viewAndUnLocalMatrix, path, GrStyle());
}

void GrRenderTargetContext::drawVertices(const GrClip& clip,
                                         GrPaint&& paint,
                                         const SkMatrix& viewMatrix,
                                         GrPrimitiveType primitiveType,
                                         int vertexCount,
                                         const SkPoint positions[],
                                         const SkPoint texCoords[],
                                         const uint32_t colors[],
                                         const uint16_t indices[],
                                         int indexCount,
                                         ColorArrayType colorArrayType) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawVertices");

    AutoCheckFlush acf(this->drawingManager());

    // TODO clients should give us bounds
    SkRect bounds;
    if (!bounds.setBoundsCheck(positions, vertexCount)) {
        SkDebugf("drawVertices call empty bounds\n");
        return;
    }

    std::unique_ptr<GrLegacyMeshDrawOp> op = GrDrawVerticesOp::Make(
            paint.getColor(), primitiveType, viewMatrix, positions, vertexCount, indices,
            indexCount, colors, texCoords, bounds, colorArrayType);
    if (!op) {
        return;
    }
    GrPipelineBuilder pipelineBuilder(std::move(paint), GrAAType::kNone);
    this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
}

void GrRenderTargetContext::drawVertices(const GrClip& clip,
                                         GrPaint&& paint,
                                         const SkMatrix& viewMatrix,
                                         sk_sp<SkVertices> vertices) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawVertices");

    AutoCheckFlush acf(this->drawingManager());

    SkASSERT(vertices);
    std::unique_ptr<GrLegacyMeshDrawOp> op =
            GrDrawVerticesOp::Make(paint.getColor(), std::move(vertices), viewMatrix);
    if (!op) {
        return;
    }
    GrPipelineBuilder pipelineBuilder(std::move(paint), GrAAType::kNone);
    this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawAtlas");

    AutoCheckFlush acf(this->drawingManager());

    std::unique_ptr<GrLegacyMeshDrawOp> op =
            GrDrawAtlasOp::Make(paint.getColor(), viewMatrix, spriteCount, xform, texRect, colors);
    GrPipelineBuilder pipelineBuilder(std::move(paint), GrAAType::kNone);
    this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawRRect");
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

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport() &&
        stroke.isFillStyle()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        std::unique_ptr<GrDrawOp> op(
                ir->recordRRect(rrect, viewMatrix, std::move(paint), aa, fInstancedPipelineInfo));
        if (op) {
            this->addDrawOp(*clip, std::move(op));
            return;
        }
    }

    GrAAType aaType = this->decideAAType(aa);
    if (GrAAType::kCoverage == aaType) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        std::unique_ptr<GrLegacyMeshDrawOp> op =
                GrOvalOpFactory::MakeRRectOp(paint.getColor(),
                                             paint.usesDistanceVectorField(),
                                             viewMatrix,
                                             rrect,
                                             stroke,
                                             shaderCaps);
        if (op) {
            GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
            this->addLegacyMeshDrawOp(std::move(pipelineBuilder), *clip, std::move(op));
            return;
        }
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(rrect);
    this->internalDrawPath(*clip, std::move(paint), aa, viewMatrix, path, style);
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetContext::drawShadowRRect(const GrClip& clip,
                                            GrPaint&& paint,
                                            const SkMatrix& viewMatrix,
                                            const SkRRect& rrect,
                                            SkScalar blurRadius,
                                            const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawShadowRRect");
    if (rrect.isEmpty()) {
        return;
    }

    SkASSERT(!style.pathEffect()); // this should've been devolved to a path in SkGpuDevice

    AutoCheckFlush acf(this->drawingManager());
    const SkStrokeRec stroke = style.strokeRec();
    // TODO: add instancing support?

    const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
    std::unique_ptr<GrLegacyMeshDrawOp> op = GrShadowRRectOp::Make(
            paint.getColor(), viewMatrix, rrect, blurRadius, stroke, shaderCaps);
    if (op) {
        GrPipelineBuilder pipelineBuilder(std::move(paint), GrAAType::kNone);
        this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
        return;
    }
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

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        std::unique_ptr<GrDrawOp> op(ir->recordDRRect(
                origOuter, origInner, viewMatrix, std::move(paint), aa, fInstancedPipelineInfo));
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return true;
        }
    }

    GrAAType aaType = this->decideAAType(aa);

    GrPrimitiveEdgeType innerEdgeType, outerEdgeType;
    if (GrAAType::kCoverage == aaType) {
        innerEdgeType = kInverseFillAA_GrProcessorEdgeType;
        outerEdgeType = kFillAA_GrProcessorEdgeType;
    } else {
        innerEdgeType = kInverseFillBW_GrProcessorEdgeType;
        outerEdgeType = kFillBW_GrProcessorEdgeType;
    }

    SkTCopyOnFirstWrite<SkRRect> inner(origInner), outer(origOuter);
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

    // TODO these need to be a geometry processors
    sk_sp<GrFragmentProcessor> innerEffect(GrRRectEffect::Make(innerEdgeType, *inner));
    if (!innerEffect) {
        return false;
    }

    sk_sp<GrFragmentProcessor> outerEffect(GrRRectEffect::Make(outerEdgeType, *outer));
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawDRRect");

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

    this->internalDrawPath(clip, std::move(paint), aa, viewMatrix, path, GrStyle::SimpleFill());
}

///////////////////////////////////////////////////////////////////////////////

static inline bool is_int(float x) {
    return x == (float) sk_float_round2int(x);
}

void GrRenderTargetContext::drawRegion(const GrClip& clip,
                                       GrPaint&& paint,
                                       GrAA aa,
                                       const SkMatrix& viewMatrix,
                                       const SkRegion& region,
                                       const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawRegion");

    if (GrAA::kYes == aa) {
        // GrRegionOp performs no antialiasing but is much faster, so here we check the matrix
        // to see whether aa is really required.
        if (!SkToBool(viewMatrix.getType() & ~(SkMatrix::kTranslate_Mask)) &&
            is_int(viewMatrix.getTranslateX()) &&
            is_int(viewMatrix.getTranslateY())) {
            aa = GrAA::kNo;
        }
    }
    bool complexStyle = !style.isSimpleFill();
    if (complexStyle || GrAA::kYes == aa) {
        SkPath path;
        region.getBoundaryPath(&path);
        return this->drawPath(clip, std::move(paint), aa, viewMatrix, path, style);
    }

    std::unique_ptr<GrLegacyMeshDrawOp> op = GrRegionOp::Make(paint.getColor(), viewMatrix, region);
    GrPipelineBuilder pipelineBuilder(std::move(paint), GrAAType::kNone);
    this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawOval");

    if (oval.isEmpty()) {
       return;
    }

    SkASSERT(!style.pathEffect()); // this should've been devolved to a path in SkGpuDevice

    AutoCheckFlush acf(this->drawingManager());
    const SkStrokeRec& stroke = style.strokeRec();

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport() &&
        stroke.isFillStyle()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        std::unique_ptr<GrDrawOp> op(
                ir->recordOval(oval, viewMatrix, std::move(paint), aa, fInstancedPipelineInfo));
        if (op) {
            this->addDrawOp(clip, std::move(op));
            return;
        }
    }

    GrAAType aaType = this->decideAAType(aa);
    if (GrAAType::kCoverage == aaType) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        std::unique_ptr<GrLegacyMeshDrawOp> op =
                GrOvalOpFactory::MakeOvalOp(paint.getColor(), viewMatrix, oval, stroke, shaderCaps);
        if (op) {
            GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
            this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
            return;
        }
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addOval(oval);
    this->internalDrawPath(clip, std::move(paint), aa, viewMatrix, path, style);
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawArc");

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->decideAAType(aa);
    if (GrAAType::kCoverage == aaType) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        std::unique_ptr<GrLegacyMeshDrawOp> op = GrOvalOpFactory::MakeArcOp(paint.getColor(),
                                                                            viewMatrix,
                                                                            oval,
                                                                            startAngle,
                                                                            sweepAngle,
                                                                            useCenter,
                                                                            style,
                                                                            shaderCaps);
        if (op) {
            GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
            this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
            return;
        }
    }
    SkPath path;
    SkPathPriv::CreateDrawArcPath(&path, oval, startAngle, sweepAngle, useCenter,
                                  style.isSimpleFill());
    this->internalDrawPath(clip, std::move(paint), aa, viewMatrix, path, style);
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawImageLattice");

    AutoCheckFlush acf(this->drawingManager());

    std::unique_ptr<GrLegacyMeshDrawOp> op = GrLatticeOp::MakeNonAA(
            paint.getColor(), viewMatrix, imageWidth, imageHeight, std::move(iter), dst);

    GrPipelineBuilder pipelineBuilder(std::move(paint), GrAAType::kNone);
    this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
}

void GrRenderTargetContext::prepareForExternalIO() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::prepareForExternalIO");

    this->drawingManager()->prepareSurfaceForExternalIO(fRenderTargetProxy.get());
}

void GrRenderTargetContext::drawNonAAFilledRect(const GrClip& clip,
                                                GrPaint&& paint,
                                                const SkMatrix& viewMatrix,
                                                const SkRect& rect,
                                                const SkRect* localRect,
                                                const SkMatrix* localMatrix,
                                                const GrUserStencilSettings* ss,
                                                GrAAType hwOrNoneAAType) {
    SkASSERT(GrAAType::kCoverage != hwOrNoneAAType);
    SkASSERT(hwOrNoneAAType == GrAAType::kNone || this->isStencilBufferMultisampled());
    std::unique_ptr<GrLegacyMeshDrawOp> op = GrRectOpFactory::MakeNonAAFill(
            paint.getColor(), viewMatrix, rect, localRect, localMatrix);
    GrPipelineBuilder pipelineBuilder(std::move(paint), hwOrNoneAAType);
    if (ss) {
        pipelineBuilder.setUserStencil(ss);
    }
    this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawPath");

    if (path.isEmpty()) {
       if (path.isInverseFillType()) {
           this->drawPaint(clip, std::move(paint), viewMatrix);
       }
       return;
    }

    AutoCheckFlush acf(this->drawingManager());

    GrAAType aaType = this->decideAAType(aa);
    if (GrAAType::kCoverage == aaType && !style.pathEffect()) {
        if (style.isSimpleFill() && !path.isConvex()) {
            // Concave AA paths are expensive - try to avoid them for special cases
            SkRect rects[2];

            if (fills_as_nested_rects(viewMatrix, path, rects)) {
                std::unique_ptr<GrLegacyMeshDrawOp> op =
                        GrRectOpFactory::MakeAAFillNestedRects(paint.getColor(), viewMatrix, rects);
                if (op) {
                    GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
                    this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
                }
                return;
            }
        }
        SkRect ovalRect;
        bool isOval = path.isOval(&ovalRect);

        if (isOval && !path.isInverseFillType()) {
            const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
            std::unique_ptr<GrLegacyMeshDrawOp> op = GrOvalOpFactory::MakeOvalOp(
                    paint.getColor(), viewMatrix, ovalRect, style.strokeRec(), shaderCaps);
            if (op) {
                GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
                this->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip, std::move(op));
                return;
            }
        }
    }

    // Note that internalDrawPath may sw-rasterize the path into a scratch texture.
    // Scratch textures can be recycled after they are returned to the texture
    // cache. This presents a potential hazard for buffered drawing. However,
    // the writePixels that uploads to the scratch will perform a flush so we're
    // OK.
    this->internalDrawPath(clip, std::move(paint), aa, viewMatrix, path, style);
}

bool GrRenderTargetContextPriv::drawAndStencilPath(const GrClip& clip,
                                                   const GrUserStencilSettings* ss,
                                                   SkRegion::Op op,
                                                   bool invert,
                                                   GrAA aa,
                                                   const SkMatrix& viewMatrix,
                                                   const SkPath& path) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContextPriv::drawAndStencilPath");

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
    GrAAType aaType = fRenderTargetContext->decideAAType(aa);
    bool hasUserStencilSettings = !ss->isUnused();

    GrShape shape(path, GrStyle::SimpleFill());
    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fShaderCaps =
        fRenderTargetContext->drawingManager()->getContext()->caps()->shaderCaps();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
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

    GrPathRenderer::DrawPathArgs args{
            fRenderTargetContext->drawingManager()->getContext(),
            std::move(paint),
            ss,
            fRenderTargetContext,
            &clip,
            &viewMatrix,
            &shape,
            aaType,
            fRenderTargetContext->isGammaCorrect()};
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

void GrRenderTargetContext::internalDrawPath(const GrClip& clip,
                                             GrPaint&& paint,
                                             GrAA aa,
                                             const SkMatrix& viewMatrix,
                                             const SkPath& path,
                                             const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkASSERT(!path.isEmpty());
    GrShape shape;

    GrAAType aaType = this->decideAAType(aa, /*allowMixedSamples*/ true);
    if (style.isSimpleHairline() && aaType == GrAAType::kMixedSamples) {
        // NVPR cannot handle hairlines, so this will would get picked up by a different stencil and
        // cover path renderer (i.e. default path renderer). The hairline renderer produces much
        // smoother hairlines than MSAA.
        aaType = GrAAType::kCoverage;
    }
    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fShaderCaps = this->drawingManager()->getContext()->caps()->shaderCaps();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fHasUserStencilSettings = false;

    GrPathRenderer* pr;
    static constexpr GrPathRendererChain::DrawType kType = GrPathRendererChain::DrawType::kColor;
    do {
        shape = GrShape(path, style);
        if (shape.isEmpty()) {
            return;
        }

        canDrawArgs.fAAType = aaType;

        // Try a 1st time without applying any of the style to the geometry (and barring sw)
        pr = this->drawingManager()->getPathRenderer(canDrawArgs, false, kType);
        SkScalar styleScale =  GrStyle::MatrixToScaleFactor(viewMatrix);

        if (!pr && shape.style().pathEffect()) {
            // It didn't work above, so try again with the path effect applied.
            shape = shape.applyStyle(GrStyle::Apply::kPathEffectOnly, styleScale);
            if (shape.isEmpty()) {
                return;
            }
            pr = this->drawingManager()->getPathRenderer(canDrawArgs, false, kType);
        }
        if (!pr) {
            if (shape.style().applies()) {
                shape = shape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale);
                if (shape.isEmpty()) {
                    return;
                }
            }
            // This time, allow SW renderer
            pr = this->drawingManager()->getPathRenderer(canDrawArgs, true, kType);
        }
        if (!pr && GrAATypeIsHW(aaType)) {
            // There are exceptional cases where we may wind up falling back to coverage based AA
            // when the target is MSAA (e.g. through disabling path renderers via GrContextOptions).
            aaType = GrAAType::kCoverage;
        } else {
            break;
        }
    } while(true);

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
                                      &viewMatrix,
                                      &shape,
                                      aaType,
                                      this->isGammaCorrect()};
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::addDrawOp");

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

    // This forces instantiation of the render target.
    GrRenderTarget* rt = this->accessRenderTarget();
    if (!rt) {
        return SK_InvalidUniqueID;
    }

    if (fixedFunctionFlags & GrDrawOp::FixedFunctionFlags::kUsesStencil ||
        appliedClip.hasStencilClip()) {
        if (!fContext->resourceProvider()->attachStencilAttachment(rt)) {
            SkDebugf("ERROR creating stencil attachment. Draw skipped.\n");
            return SK_InvalidUniqueID;
        }
    }

    GrXferProcessor::DstTexture dstTexture;
    if (op->xpRequiresDstTexture(*this->caps(), &appliedClip)) {
        if (!this->setupDstTexture(fRenderTargetProxy.get(), clip, op->bounds(), &dstTexture)) {
            return SK_InvalidUniqueID;
        }
    }

    op->setClippedBounds(bounds);
    return this->getOpList()->addOp(std::move(op), this, std::move(appliedClip), dstTexture);
}

uint32_t GrRenderTargetContext::addLegacyMeshDrawOp(GrPipelineBuilder&& pipelineBuilder,
                                                    const GrClip& clip,
                                                    std::unique_ptr<GrLegacyMeshDrawOp> op) {
    ASSERT_SINGLE_OWNER
    if (this->drawingManager()->wasAbandoned()) {
        return SK_InvalidUniqueID;
    }
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::addLegacyMeshDrawOp");

    // Setup clip
    SkRect bounds;
    op_bounds(&bounds, op.get());
    GrAppliedClip appliedClip;
    if (!clip.apply(fContext, this, pipelineBuilder.isHWAntialias(),
                    pipelineBuilder.hasUserStencilSettings(), &appliedClip, &bounds)) {
        return SK_InvalidUniqueID;
    }

    // This forces instantiation of the render target. Pipeline creation is moving to flush time
    // by which point instantiation must have occurred anyway.
    GrRenderTarget* rt = this->accessRenderTarget();
    if (!rt) {
        return SK_InvalidUniqueID;
    }

    GrResourceProvider* resourceProvider = fContext->resourceProvider();
    bool usesStencil = pipelineBuilder.hasUserStencilSettings() || appliedClip.hasStencilClip();
    if (usesStencil) {
        if (!resourceProvider->attachStencilAttachment(this->accessRenderTarget())) {
            SkDebugf("ERROR creating stencil attachment. Draw skipped.\n");
            return SK_InvalidUniqueID;
        }
    }

    bool isMixedSamples = fRenderTargetProxy->isMixedSampled() &&
                          (pipelineBuilder.isHWAntialias() || usesStencil);

    GrColor overrideColor;
    GrProcessorSet::Analysis analysis = op->analyzeUpdateAndRecordProcessors(
            &pipelineBuilder, &appliedClip, isMixedSamples, *this->caps(), &overrideColor);

    GrPipeline::InitArgs args;
    pipelineBuilder.getPipelineInitArgs(&args);
    args.fAppliedClip = &appliedClip;
    args.fRenderTarget = rt;
    args.fCaps = this->caps();

    if (analysis.requiresDstTexture()) {
        if (!this->setupDstTexture(fRenderTargetProxy.get(), clip, bounds, &args.fDstTexture)) {
            return SK_InvalidUniqueID;
        }
    }
    op->initPipeline(args, analysis, overrideColor);
    // TODO: We need to add pipeline dependencies on textures, etc before recording this op.
    op->setClippedBounds(bounds);
    return this->getOpList()->addOp(std::move(op), this);
}

bool GrRenderTargetContext::setupDstTexture(GrRenderTargetProxy* rtProxy, const GrClip& clip,
                                            const SkRect& opBounds,
                                            GrXferProcessor::DstTexture* dstTexture) {
    if (this->caps()->textureBarrierSupport()) {
        if (GrTextureProxy* texProxy = rtProxy->asTextureProxy()) {
            // MDB TODO: remove this instantiation. Blocked on making DstTexture be proxy-based
            sk_sp<GrTexture> tex(sk_ref_sp(texProxy->instantiate(fContext->resourceProvider())));
            if (!tex) {
                SkDebugf("setupDstTexture: instantiation of src texture failed.\n");
                return false;  // We have bigger problems now
            }

            // The render target is a texture, so we can read from it directly in the shader. The XP
            // will be responsible to detect this situation and request a texture barrier.
            dstTexture->setTexture(std::move(tex));
            dstTexture->setOffset(0, 0);
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
        desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
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

    GrTextureProxy* copyProxy = sContext->asTextureProxy();
    // MDB TODO: remove this instantiation once DstTexture is proxy-backed
    sk_sp<GrTexture> copy(sk_ref_sp(copyProxy->instantiate(fContext->resourceProvider())));
    if (!copy) {
        SkDebugf("setupDstTexture: instantiation of copied texture failed.\n");
        return false;
    }

    dstTexture->setTexture(std::move(copy));
    dstTexture->setOffset(dstOffset);
    return true;
}

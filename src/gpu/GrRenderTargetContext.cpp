/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchTest.h"
#include "GrColor.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrDrawingManager.h"
#include "GrFixedClip.h"
#include "GrGpuResourcePriv.h"
#include "GrOvalRenderer.h"
#include "GrPathRenderer.h"
#include "GrPipelineBuilder.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceProvider.h"
#include "SkSurfacePriv.h"

#include "batches/GrOp.h"
#include "batches/GrClearBatch.h"
#include "batches/GrDrawAtlasBatch.h"
#include "batches/GrDrawVerticesBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "batches/GrNinePatch.h" // TODO Factory
#include "batches/GrRegionBatch.h"
#include "batches/GrShadowRRectBatch.h"

#include "effects/GrRRectEffect.h"

#include "instanced/InstancedRendering.h"

#include "text/GrAtlasTextContext.h"
#include "text/GrStencilAndCoverTextContext.h"

#include "../private/GrAuditTrail.h"

#include "SkGr.h"
#include "SkLatticeIter.h"
#include "SkMatrixPriv.h"

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == fDrawingManager->getContext())
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fSingleOwner);)
#define ASSERT_SINGLE_OWNER_PRIV \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fRenderTargetContext->fSingleOwner);)
#define RETURN_IF_ABANDONED        if (fDrawingManager->wasAbandoned()) { return; }
#define RETURN_IF_ABANDONED_PRIV   if (fRenderTargetContext->fDrawingManager->wasAbandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED  if (fDrawingManager->wasAbandoned()) { return false; }
#define RETURN_FALSE_IF_ABANDONED_PRIV  if (fRenderTargetContext->fDrawingManager->wasAbandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED   if (fDrawingManager->wasAbandoned()) { return nullptr; }

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
    return fDrawingManager->wasAbandoned();
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
    : GrSurfaceContext(context, auditTrail, singleOwner)
    , fDrawingManager(drawingMgr)
    , fRenderTargetProxy(std::move(rtp))
    , fOpList(SkSafeRef(fRenderTargetProxy->getLastRenderTargetOpList()))
    , fInstancedPipelineInfo(fRenderTargetProxy.get())
    , fColorSpace(std::move(colorSpace))
    , fColorXformFromSRGB(nullptr)
    , fSurfaceProps(SkSurfacePropsCopyOrDefault(surfaceProps))
{
    if (fColorSpace) {
        // sRGB sources are very common (SkColor, etc...), so we cache that gamut transformation
        auto srgbColorSpace = SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named);
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

GrRenderTarget* GrRenderTargetContext::instantiate() {
    return fRenderTargetProxy->instantiate(fContext->textureProvider());
}

GrTextureProxy* GrRenderTargetContext::asDeferredTexture() {
    return fRenderTargetProxy->asTextureProxy();
}

GrRenderTargetOpList* GrRenderTargetContext::getOpList() {
    ASSERT_SINGLE_OWNER
    SkDEBUGCODE(this->validate();)

    if (!fOpList || fOpList->isClosed()) {
        fOpList = fDrawingManager->newOpList(fRenderTargetProxy.get());
    }

    return fOpList;
}

bool GrRenderTargetContext::copySurface(GrSurface* src, const SkIRect& srcRect,
                                        const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::copySurface");

    // TODO: this needs to be fixed up since it ends the deferrable of the GrRenderTarget
    sk_sp<GrRenderTarget> rt(
                        sk_ref_sp(fRenderTargetProxy->instantiate(fContext->textureProvider())));
    if (!rt) {
        return false;
    }

    return this->getOpList()->copySurface(rt.get(), src, srcRect, dstPoint);
}

void GrRenderTargetContext::drawText(const GrClip& clip, const GrPaint& grPaint,
                                     const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix,
                                     const char text[], size_t byteLength,
                                     SkScalar x, SkScalar y, const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawText");

    GrAtlasTextContext* atlasTextContext = fDrawingManager->getAtlasTextContext();
    atlasTextContext->drawText(fContext, this, clip, grPaint, skPaint, viewMatrix, fSurfaceProps,
                               text, byteLength, x, y, clipBounds);
}

void GrRenderTargetContext::drawPosText(const GrClip& clip, const GrPaint& grPaint,
                                        const SkPaint& skPaint,
                                        const SkMatrix& viewMatrix,
                                        const char text[], size_t byteLength,
                                        const SkScalar pos[], int scalarsPerPosition,
                                        const SkPoint& offset, const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawPosText");

    GrAtlasTextContext* atlasTextContext = fDrawingManager->getAtlasTextContext();
    atlasTextContext->drawPosText(fContext, this, clip, grPaint, skPaint, viewMatrix,
                                  fSurfaceProps, text, byteLength, pos, scalarsPerPosition,
                                  offset, clipBounds);

}

void GrRenderTargetContext::drawTextBlob(const GrClip& clip, const SkPaint& skPaint,
                                         const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                         SkScalar x, SkScalar y,
                                         SkDrawFilter* filter, const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawTextBlob");

    GrAtlasTextContext* atlasTextContext = fDrawingManager->getAtlasTextContext();
    atlasTextContext->drawTextBlob(fContext, this, clip, skPaint, viewMatrix, fSurfaceProps, blob,
                                   x, y, filter, clipBounds);
}

void GrRenderTargetContext::discard() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::discard");

    AutoCheckFlush acf(fDrawingManager);

    // TODO: this needs to be fixed up since it ends the deferrable of the GrRenderTarget
    sk_sp<GrRenderTarget> rt(
                        sk_ref_sp(fRenderTargetProxy->instantiate(fContext->textureProvider())));
    if (!rt) {
        return;
    }

    this->getOpList()->discard(rt.get());
}

void GrRenderTargetContext::clear(const SkIRect* rect,
                                  const GrColor color,
                                  bool canIgnoreRect) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::clear");

    AutoCheckFlush acf(fDrawingManager);
    this->internalClear(rect ? GrFixedClip(*rect) : GrFixedClip::Disabled(), color, canIgnoreRect);
}

void GrRenderTargetContextPriv::clear(const GrFixedClip& clip,
                                      const GrColor color,
                                      bool canIgnoreClip) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContextPriv::clear");

    AutoCheckFlush acf(fRenderTargetContext->fDrawingManager);
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
        SkIRect clearRect = SkIRect::MakeWH(this->worstCaseWidth(), this->worstCaseHeight());
        if (isFull) {
            this->discard();
        } else if (!clearRect.intersect(clip.scissorRect())) {
            return;
        }

        GrPaint paint;
        paint.setColor4f(GrColor4f::FromGrColor(color));
        paint.setXPFactory(GrPorterDuffXPFactory::Make(SkBlendMode::kSrc));

        this->drawRect(clip, paint, SkMatrix::I(), SkRect::Make(clearRect));
    } else if (isFull) {
        if (this->accessRenderTarget()) {
            this->getOpList()->fullClear(this->accessRenderTarget(), color);
        }
    } else {
        if (!this->accessRenderTarget()) {
            return;
        }
        sk_sp<GrOp> batch(GrClearBatch::Make(clip, color, this->accessRenderTarget()));
        if (!batch) {
            return;
        }
        this->getOpList()->addBatch(std::move(batch));
    }
}

void GrRenderTargetContext::drawPaint(const GrClip& clip,
                                      const GrPaint& origPaint,
                                      const SkMatrix& viewMatrix) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawPaint");

    // set rect to be big enough to fill the space, but not super-huge, so we
    // don't overflow fixed-point implementations

    SkRect r = fRenderTargetProxy->getBoundsRect();
    SkTCopyOnFirstWrite<GrPaint> paint(origPaint);

    SkRRect rrect;
    bool aaRRect;
    // Check if we can replace a clipRRect()/drawPaint() with a drawRRect(). We only do the
    // transformation for non-rect rrects. Rects caused a performance regression on an Android
    // test that needs investigation. We also skip cases where there are fragment processors
    // because they may depend on having correct local coords and this path draws in device space
    // without a local matrix.
    if (!paint->numTotalFragmentProcessors() &&
        clip.isRRect(r, &rrect, &aaRRect) && !rrect.isRect()) {
        paint.writable()->setAntiAlias(aaRRect);
        this->drawRRect(GrNoClip(), *paint, SkMatrix::I(), rrect, GrStyle::SimpleFill());
        return;
    }

    // by definition this fills the entire clip, no need for AA
    if (paint->isAntiAlias()) {
        paint.writable()->setAntiAlias(false);
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
        this->drawRect(clip, *paint, viewMatrix, r);
    } else {
        SkMatrix localMatrix;
        if (!viewMatrix.invert(&localMatrix)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }

        AutoCheckFlush acf(fDrawingManager);

        this->drawNonAAFilledRect(clip, *paint, SkMatrix::I(), r, nullptr, &localMatrix, nullptr,
                                  false /* useHWAA */);
    }
}

static inline bool rect_contains_inclusive(const SkRect& rect, const SkPoint& point) {
    return point.fX >= rect.fLeft && point.fX <= rect.fRight &&
           point.fY >= rect.fTop && point.fY <= rect.fBottom;
}

static bool view_matrix_ok_for_aa_fill_rect(const SkMatrix& viewMatrix) {
    return viewMatrix.preservesRightAngles();
}

static bool should_apply_coverage_aa(const GrPaint& paint, GrRenderTargetProxy* rtp,
                                     bool* useHWAA = nullptr) {
    if (!paint.isAntiAlias()) {
        if (useHWAA) {
            *useHWAA = false;
        }
        return false;
    } else {
        if (useHWAA) {
            *useHWAA = rtp->isUnifiedMultisampled();
        }
        return !rtp->isUnifiedMultisampled();
    }
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
                                           const GrPaint& paint,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& rect,
                                           const GrUserStencilSettings* ss) {
    SkRect croppedRect = rect;
    if (!crop_filled_rect(this->worstCaseWidth(), this->worstCaseHeight(),
                          clip, viewMatrix, &croppedRect)) {
        return true;
    }

    sk_sp<GrDrawOp> batch;
    bool useHWAA;

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        batch.reset(ir->recordRect(croppedRect, viewMatrix, paint.getColor(),
                                   paint.isAntiAlias(), fInstancedPipelineInfo,
                                   &useHWAA));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);
            if (ss) {
                pipelineBuilder.setUserStencil(ss);
            }
            this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
            return true;
        }
    }

    if (should_apply_coverage_aa(paint, fRenderTargetProxy.get(), &useHWAA)) {
        // The fill path can handle rotation but not skew.
        if (view_matrix_ok_for_aa_fill_rect(viewMatrix)) {
            SkRect devBoundRect;
            viewMatrix.mapRect(&devBoundRect, croppedRect);

            batch.reset(GrRectBatchFactory::CreateAAFill(paint, viewMatrix, rect, croppedRect,
                                                         devBoundRect));
            if (batch) {
                GrPipelineBuilder pipelineBuilder(paint, useHWAA);
                if (ss) {
                    pipelineBuilder.setUserStencil(ss);
                }
                this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
                return true;
            }
        }
    } else {
        this->drawNonAAFilledRect(clip, paint, viewMatrix, croppedRect, nullptr, nullptr, ss,
                                  useHWAA);
        return true;
    }

    return false;
}

void GrRenderTargetContext::drawRect(const GrClip& clip,
                                     const GrPaint& paint,
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

    AutoCheckFlush acf(fDrawingManager);

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

        if (this->drawFilledRect(clip, paint, viewMatrix, rect, nullptr)) {
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
                    this->drawRect(clip, paint, viewMatrix,
                                   {rect.fLeft - r, rect.fTop - r,
                                    rect.fRight + r, rect.fBottom + r},
                                   &GrStyle::SimpleFill());
                    return;
                case SkPaint::kRound_Join:
                    // Raster draws nothing when both dimensions are empty.
                    if (rect.width() || rect.height()){
                        SkRRect rrect = SkRRect::MakeRectXY(rect.makeOutset(r, r), r, r);
                        this->drawRRect(clip, paint, viewMatrix, rrect, GrStyle::SimpleFill());
                        return;
                    }
                case SkPaint::kBevel_Join:
                    if (!rect.width()) {
                        this->drawRect(clip, paint, viewMatrix,
                                       {rect.fLeft - r, rect.fTop, rect.fRight + r, rect.fBottom},
                                       &GrStyle::SimpleFill());
                    } else {
                        this->drawRect(clip, paint, viewMatrix,
                                       {rect.fLeft, rect.fTop - r, rect.fRight, rect.fBottom + r},
                                       &GrStyle::SimpleFill());
                    }
                    return;
                }
        }

        bool useHWAA;
        bool snapToPixelCenters = false;
        sk_sp<GrDrawOp> batch;

        GrColor color = paint.getColor();
        if (should_apply_coverage_aa(paint, fRenderTargetProxy.get(), &useHWAA)) {
            // The stroke path needs the rect to remain axis aligned (no rotation or skew).
            if (viewMatrix.rectStaysRect()) {
                batch.reset(GrRectBatchFactory::CreateAAStroke(color, viewMatrix, rect, stroke));
            }
        } else {
            // Depending on sub-pixel coordinates and the particular GPU, we may lose a corner of
            // hairline rects. We jam all the vertices to pixel centers to avoid this, but not
            // when MSAA is enabled because it can cause ugly artifacts.
            snapToPixelCenters = stroke.getStyle() == SkStrokeRec::kHairline_Style &&
                                 !fRenderTargetProxy->isUnifiedMultisampled();
            batch.reset(GrRectBatchFactory::CreateNonAAStroke(color, viewMatrix, rect,
                                                              stroke, snapToPixelCenters));
        }

        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);

            if (snapToPixelCenters) {
                pipelineBuilder.setState(GrPipelineBuilder::kSnapVerticesToPixelCenters_Flag,
                                         snapToPixelCenters);
            }

            this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
            return;
        }
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRect(rect);
    this->internalDrawPath(clip, paint, viewMatrix, path, *style);
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

    AutoCheckFlush acf(fRenderTargetContext->fDrawingManager);
    if (!fRenderTargetContext->accessRenderTarget()) {
        return;
    }
    fRenderTargetContext->getOpList()->clearStencilClip(clip, insideStencilMask,
                                                        fRenderTargetContext->accessRenderTarget());
}

void GrRenderTargetContextPriv::stencilPath(const GrClip& clip,
                                            bool useHWAA,
                                            const SkMatrix& viewMatrix,
                                            const GrPath* path) {
    fRenderTargetContext->getOpList()->stencilPath(fRenderTargetContext, clip, useHWAA, viewMatrix,
                                                   path);
}

void GrRenderTargetContextPriv::stencilRect(const GrClip& clip,
                                            const GrUserStencilSettings* ss,
                                            bool useHWAA,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContext::stencilRect");

    AutoCheckFlush acf(fRenderTargetContext->fDrawingManager);

    GrPaint paint;
    paint.setAntiAlias(useHWAA);
    paint.setXPFactory(GrDisableColorXPFactory::Make());

    fRenderTargetContext->drawNonAAFilledRect(clip, paint, viewMatrix, rect, nullptr, nullptr, ss,
                                              useHWAA);
}

bool GrRenderTargetContextPriv::drawAndStencilRect(const GrClip& clip,
                                                   const GrUserStencilSettings* ss,
                                                   SkRegion::Op op,
                                                   bool invert,
                                                   bool doAA,
                                                   const SkMatrix& viewMatrix,
                                                   const SkRect& rect) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContext::drawAndStencilRect");

    AutoCheckFlush acf(fRenderTargetContext->fDrawingManager);

    GrPaint paint;
    paint.setAntiAlias(doAA);
    paint.setCoverageSetOpXPFactory(op, invert);

    if (fRenderTargetContext->drawFilledRect(clip, paint, viewMatrix, rect, ss)) {
        return true;
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRect(rect);
    return this->drawAndStencilPath(clip, ss, op, invert, doAA, viewMatrix, path);
}

void GrRenderTargetContext::fillRectToRect(const GrClip& clip,
                                           const GrPaint& paint,
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

    AutoCheckFlush acf(fDrawingManager);
    bool useHWAA;

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        sk_sp<GrDrawOp> batch(ir->recordRect(croppedRect, viewMatrix, paint.getColor(),
                                             croppedLocalRect, paint.isAntiAlias(),
                                             fInstancedPipelineInfo, &useHWAA));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);
            this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
            return;
        }
    }

    if (!should_apply_coverage_aa(paint, fRenderTargetProxy.get(), &useHWAA)) {
        this->drawNonAAFilledRect(clip, paint, viewMatrix, croppedRect, &croppedLocalRect,
                                  nullptr, nullptr, useHWAA);
        return;
    }

    if (view_matrix_ok_for_aa_fill_rect(viewMatrix)) {
        sk_sp<GrDrawOp> batch(GrAAFillRectBatch::CreateWithLocalRect(paint.getColor(),
                                                                     viewMatrix,
                                                                     croppedRect,
                                                                     croppedLocalRect));
        GrPipelineBuilder pipelineBuilder(paint, useHWAA);
        this->drawBatch(pipelineBuilder, clip, batch.get());
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
    this->internalDrawPath(clip, paint, viewAndUnLocalMatrix, path, GrStyle());
}

void GrRenderTargetContext::fillRectWithLocalMatrix(const GrClip& clip,
                                                    const GrPaint& paint,
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

    AutoCheckFlush acf(fDrawingManager);
    bool useHWAA;

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        sk_sp<GrDrawOp> batch(ir->recordRect(croppedRect, viewMatrix, paint.getColor(),
                                             localMatrix, paint.isAntiAlias(),
                                             fInstancedPipelineInfo, &useHWAA));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);
            this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
            return;
        }
    }

    if (!should_apply_coverage_aa(paint, fRenderTargetProxy.get(), &useHWAA)) {
        this->drawNonAAFilledRect(clip, paint, viewMatrix, croppedRect, nullptr,
                                  &localMatrix, nullptr, useHWAA);
        return;
    }

    if (view_matrix_ok_for_aa_fill_rect(viewMatrix)) {
        sk_sp<GrDrawOp> batch(GrAAFillRectBatch::Create(paint.getColor(), viewMatrix,
                                                        localMatrix, croppedRect));
        GrPipelineBuilder pipelineBuilder(paint, useHWAA);
        this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
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
    this->internalDrawPath(clip, paint, viewAndUnLocalMatrix, path, GrStyle());
}

void GrRenderTargetContext::drawVertices(const GrClip& clip,
                                         const GrPaint& paint,
                                         const SkMatrix& viewMatrix,
                                         GrPrimitiveType primitiveType,
                                         int vertexCount,
                                         const SkPoint positions[],
                                         const SkPoint texCoords[],
                                         const GrColor colors[],
                                         const uint16_t indices[],
                                         int indexCount) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawVertices");

    AutoCheckFlush acf(fDrawingManager);

    // TODO clients should give us bounds
    SkRect bounds;
    if (!bounds.setBoundsCheck(positions, vertexCount)) {
        SkDebugf("drawVertices call empty bounds\n");
        return;
    }

    viewMatrix.mapRect(&bounds);

    sk_sp<GrDrawOp> batch(new GrDrawVerticesBatch(paint.getColor(),
                                                  primitiveType, viewMatrix, positions,
                                                  vertexCount, indices, indexCount,
                                                  colors, texCoords, bounds));

    GrPipelineBuilder pipelineBuilder(paint, this->mustUseHWAA(paint));
    this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetContext::drawAtlas(const GrClip& clip,
                                      const GrPaint& paint,
                                      const SkMatrix& viewMatrix,
                                      int spriteCount,
                                      const SkRSXform xform[],
                                      const SkRect texRect[],
                                      const SkColor colors[]) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawAtlas");

    AutoCheckFlush acf(fDrawingManager);

    sk_sp<GrDrawOp> batch(new GrDrawAtlasBatch(paint.getColor(), viewMatrix, spriteCount,
                                               xform, texRect, colors));

    GrPipelineBuilder pipelineBuilder(paint, this->mustUseHWAA(paint));
    this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetContext::drawRRect(const GrClip& origClip,
                                      const GrPaint& paint,
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
    // draw is aa. Since our lower level clip code works from batch bounds, which are SkRects, it
    // doesn't detect that the clip can be ignored (modulo antialiasing). The following test
    // attempts to mitigate the stencil clip cost but will only help when the entire clip stack
    // can be ignored. We'd prefer to fix this in the framework by removing the clips calls.
    SkRRect devRRect;
    if (rrect.transform(viewMatrix, &devRRect) && clip->quickContains(devRRect)) {
        clip = &noclip;
    }
#endif
    SkASSERT(!style.pathEffect()); // this should've been devolved to a path in SkGpuDevice

    AutoCheckFlush acf(fDrawingManager);
    const SkStrokeRec stroke = style.strokeRec();
    bool useHWAA;

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport() &&
        stroke.isFillStyle()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        sk_sp<GrDrawOp> batch(ir->recordRRect(rrect, viewMatrix, paint.getColor(),
                                              paint.isAntiAlias(), fInstancedPipelineInfo,
                                              &useHWAA));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);
            this->getOpList()->drawBatch(pipelineBuilder, this, *clip, batch.get());
            return;
        }
    }

    if (should_apply_coverage_aa(paint, fRenderTargetProxy.get(), &useHWAA)) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        sk_sp<GrDrawOp> batch(GrOvalRenderer::CreateRRectBatch(paint.getColor(),
                                                               paint.usesDistanceVectorField(),
                                                               viewMatrix,
                                                               rrect,
                                                               stroke,
                                                               shaderCaps));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);
            this->getOpList()->drawBatch(pipelineBuilder, this, *clip, batch.get());
            return;
        }
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(rrect);
    this->internalDrawPath(*clip, paint, viewMatrix, path, style);
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetContext::drawShadowRRect(const GrClip& clip,
                                            const GrPaint& paint,
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

    AutoCheckFlush acf(fDrawingManager);
    const SkStrokeRec stroke = style.strokeRec();
    bool useHWAA;

    // TODO: add instancing support
    //if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport() &&
    //    stroke.isFillStyle()) {
    //    InstancedRendering* ir = this->getOpList()->instancedRendering();
    //    SkAutoTUnref<GrDrawOp> batch(ir->recordRRect(rrect, viewMatrix, paint.getColor(),
    //                                                 paint.isAntiAlias(), fInstancedPipelineInfo,
    //                                                 &useHWAA));
    //    if (batch) {
    //        GrPipelineBuilder pipelineBuilder(paint, useHWAA);
    //        this->getOpList()->drawBatch(pipelineBuilder, this, *clip, batch);
    //        return;
    //    }
    //}

    if (should_apply_coverage_aa(paint, fRenderTargetProxy.get(), &useHWAA)) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        sk_sp<GrDrawOp> batch(CreateShadowRRectBatch(paint.getColor(),
                                                     viewMatrix,
                                                     rrect,
                                                     blurRadius,
                                                     stroke,
                                                     shaderCaps));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);
            this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
            return;
        }
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(rrect);
    this->internalDrawPath(clip, paint, viewMatrix, path, style);
}

///////////////////////////////////////////////////////////////////////////////

bool GrRenderTargetContext::drawFilledDRRect(const GrClip& clip,
                                             const GrPaint& paintIn,
                                             const SkMatrix& viewMatrix,
                                             const SkRRect& origOuter,
                                             const SkRRect& origInner) {
    SkASSERT(!origInner.isEmpty());
    SkASSERT(!origOuter.isEmpty());

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport()) {
        bool useHWAA;
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        sk_sp<GrDrawOp> batch(ir->recordDRRect(origOuter, origInner, viewMatrix,
                                               paintIn.getColor(), paintIn.isAntiAlias(),
                                               fInstancedPipelineInfo, &useHWAA));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paintIn, useHWAA);
            this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
            return true;
        }
    }

    bool applyAA = paintIn.isAntiAlias() && !fRenderTargetProxy->isUnifiedMultisampled();

    GrPrimitiveEdgeType innerEdgeType = applyAA ? kInverseFillAA_GrProcessorEdgeType :
                                                  kInverseFillBW_GrProcessorEdgeType;
    GrPrimitiveEdgeType outerEdgeType = applyAA ? kFillAA_GrProcessorEdgeType :
                                                  kFillBW_GrProcessorEdgeType;

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

    GrPaint grPaint(paintIn);
    grPaint.setAntiAlias(false);

    // TODO these need to be a geometry processors
    sk_sp<GrFragmentProcessor> innerEffect(GrRRectEffect::Make(innerEdgeType, *inner));
    if (!innerEffect) {
        return false;
    }

    sk_sp<GrFragmentProcessor> outerEffect(GrRRectEffect::Make(outerEdgeType, *outer));
    if (!outerEffect) {
        return false;
    }

    grPaint.addCoverageFragmentProcessor(std::move(innerEffect));
    grPaint.addCoverageFragmentProcessor(std::move(outerEffect));

    SkRect bounds = outer->getBounds();
    if (applyAA) {
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);
    }

    this->fillRectWithLocalMatrix(clip, grPaint, SkMatrix::I(), bounds, inverseVM);
    return true;
}

void GrRenderTargetContext::drawDRRect(const GrClip& clip,
                                       const GrPaint& paint,
                                       const SkMatrix& viewMatrix,
                                       const SkRRect& outer,
                                       const SkRRect& inner) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawDRRect");

    SkASSERT(!outer.isEmpty());
    SkASSERT(!inner.isEmpty());

    AutoCheckFlush acf(fDrawingManager);

    if (this->drawFilledDRRect(clip, paint, viewMatrix, outer, inner)) {
        return;
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(inner);
    path.addRRect(outer);
    path.setFillType(SkPath::kEvenOdd_FillType);

    this->internalDrawPath(clip, paint, viewMatrix, path, GrStyle::SimpleFill());
}

///////////////////////////////////////////////////////////////////////////////

static inline bool is_int(float x) {
    return x == (float) sk_float_round2int(x);
}

void GrRenderTargetContext::drawRegion(const GrClip& clip,
                                       const GrPaint& paint,
                                       const SkMatrix& viewMatrix,
                                       const SkRegion& region,
                                       const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawRegion");

    bool needsAA = false;
    if (paint.isAntiAlias()) {
        // GrRegionBatch performs no antialiasing but is much faster, so here we check the matrix
        // to see whether aa is really required.
        needsAA = SkToBool(viewMatrix.getType() & ~(SkMatrix::kTranslate_Mask)) ||
                  !is_int(viewMatrix.getTranslateX()) ||
                  !is_int(viewMatrix.getTranslateY());
    }
    bool complexStyle = !style.isSimpleFill();
    if (complexStyle || needsAA) {
        SkPath path;
        region.getBoundaryPath(&path);
        return this->drawPath(clip, paint, viewMatrix, path, style);
    }

    sk_sp<GrDrawOp> batch(GrRegionBatch::Create(paint.getColor(), viewMatrix, region));
    GrPipelineBuilder pipelineBuilder(paint, false);
    this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
}

void GrRenderTargetContext::drawOval(const GrClip& clip,
                                     const GrPaint& paint,
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

    AutoCheckFlush acf(fDrawingManager);
    const SkStrokeRec& stroke = style.strokeRec();
    bool useHWAA;

    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport() &&
        stroke.isFillStyle()) {
        InstancedRendering* ir = this->getOpList()->instancedRendering();
        sk_sp<GrDrawOp> batch(ir->recordOval(oval, viewMatrix, paint.getColor(),
                                             paint.isAntiAlias(), fInstancedPipelineInfo,
                                             &useHWAA));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);
            this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
            return;
        }
    }

    if (should_apply_coverage_aa(paint, fRenderTargetProxy.get(), &useHWAA)) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        sk_sp<GrDrawOp> batch(GrOvalRenderer::CreateOvalBatch(paint.getColor(),
                                                              viewMatrix,
                                                              oval,
                                                              stroke,
                                                              shaderCaps));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);
            this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
            return;
        }
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addOval(oval);
    this->internalDrawPath(clip, paint, viewMatrix, path, style);
}

void GrRenderTargetContext::drawArc(const GrClip& clip,
                                    const GrPaint& paint,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& oval,
                                    SkScalar startAngle,
                                    SkScalar sweepAngle,
                                    bool useCenter,
                                    const GrStyle& style) {
    bool useHWAA;
    if (should_apply_coverage_aa(paint, fRenderTargetProxy.get(), &useHWAA)) {
        const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
        sk_sp<GrDrawOp> batch(GrOvalRenderer::CreateArcBatch(paint.getColor(),
                                                             viewMatrix,
                                                             oval,
                                                             startAngle,
                                                             sweepAngle,
                                                             useCenter,
                                                             style,
                                                             shaderCaps));
        if (batch) {
            GrPipelineBuilder pipelineBuilder(paint, useHWAA);
            this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
            return;
        }
    }
    SkPath path;
    SkPathPriv::CreateDrawArcPath(&path, oval, startAngle, sweepAngle, useCenter,
                                  style.isSimpleFill());
    this->internalDrawPath(clip, paint, viewMatrix, path, style);
    return;
}

void GrRenderTargetContext::drawImageLattice(const GrClip& clip,
                                             const GrPaint& paint,
                                             const SkMatrix& viewMatrix,
                                             int imageWidth,
                                             int imageHeight,
                                             std::unique_ptr<SkLatticeIter> iter,
                                             const SkRect& dst) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawImageLattice");

    AutoCheckFlush acf(fDrawingManager);

    sk_sp<GrDrawOp> batch(GrNinePatch::CreateNonAA(paint.getColor(), viewMatrix,
                                                   imageWidth, imageHeight,
                                                   std::move(iter), dst));

    GrPipelineBuilder pipelineBuilder(paint, this->mustUseHWAA(paint));
    this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
}

void GrRenderTargetContext::prepareForExternalIO() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::prepareForExternalIO");

    // Deferral of the VRAM resources must end in this instance anyway
    sk_sp<GrRenderTarget> rt(
                        sk_ref_sp(fRenderTargetProxy->instantiate(fContext->textureProvider())));
    if (!rt) {
        return;
    }

    ASSERT_OWNED_RESOURCE(rt);

    fDrawingManager->prepareSurfaceForExternalIO(rt.get());
}

void GrRenderTargetContext::drawNonAAFilledRect(const GrClip& clip,
                                                const GrPaint& paint,
                                                const SkMatrix& viewMatrix,
                                                const SkRect& rect,
                                                const SkRect* localRect,
                                                const SkMatrix* localMatrix,
                                                const GrUserStencilSettings* ss,
                                                bool useHWAA) {
    SkASSERT(!useHWAA || this->isStencilBufferMultisampled());
    sk_sp<GrDrawOp> batch(
            GrRectBatchFactory::CreateNonAAFill(paint.getColor(), viewMatrix, rect, localRect,
                                                localMatrix));
    GrPipelineBuilder pipelineBuilder(paint, useHWAA);
    if (ss) {
        pipelineBuilder.setUserStencil(ss);
    }
    this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
}

bool GrRenderTargetContext::readPixels(const SkImageInfo& dstInfo, void* dstBuffer,
                                       size_t dstRowBytes, int x, int y) {
    // TODO: teach fRenderTarget to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(dstInfo, *fContext->caps());
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }

    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == dstInfo.alphaType()) {
        flags = GrContext::kUnpremul_PixelOpsFlag;
    }

    // Deferral of the VRAM resources must end in this instance anyway
    sk_sp<GrRenderTarget> rt(
                        sk_ref_sp(fRenderTargetProxy->instantiate(fContext->textureProvider())));
    if (!rt) {
        return false;
    }

    return rt->readPixels(x, y, dstInfo.width(), dstInfo.height(),
                          config, dstBuffer, dstRowBytes, flags);
}

bool GrRenderTargetContext::writePixels(const SkImageInfo& srcInfo, const void* srcBuffer,
                                        size_t srcRowBytes, int x, int y) {
    // TODO: teach fRenderTarget to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(srcInfo, *fContext->caps());
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }
    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == srcInfo.alphaType()) {
        flags = GrContext::kUnpremul_PixelOpsFlag;
    }

    // Deferral of the VRAM resources must end in this instance anyway
    sk_sp<GrRenderTarget> rt(
                        sk_ref_sp(fRenderTargetProxy->instantiate(fContext->textureProvider())));
    if (!rt) {
        return false;
    }

    return rt->writePixels(x, y, srcInfo.width(), srcInfo.height(),
                           config, srcBuffer, srcRowBytes, flags);
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
                                     const GrPaint& paint,
                                     const SkMatrix& viewMatrix,
                                     const SkPath& path,
                                     const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawPath");

    if (path.isEmpty()) {
       if (path.isInverseFillType()) {
           this->drawPaint(clip, paint, viewMatrix);
       }
       return;
    }

    AutoCheckFlush acf(fDrawingManager);

    bool useHWAA;
    if (should_apply_coverage_aa(paint, fRenderTargetProxy.get(), &useHWAA) &&
                                                                            !style.pathEffect()) {
        if (style.isSimpleFill() && !path.isConvex()) {
            // Concave AA paths are expensive - try to avoid them for special cases
            SkRect rects[2];

            if (fills_as_nested_rects(viewMatrix, path, rects)) {
                sk_sp<GrDrawOp> batch(GrRectBatchFactory::CreateAAFillNestedRects(
                    paint.getColor(), viewMatrix, rects));
                if (batch) {
                    GrPipelineBuilder pipelineBuilder(paint, useHWAA);
                    this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
                }
                return;
            }
        }
        SkRect ovalRect;
        bool isOval = path.isOval(&ovalRect);

        if (isOval && !path.isInverseFillType()) {
            const GrShaderCaps* shaderCaps = fContext->caps()->shaderCaps();
            sk_sp<GrDrawOp> batch(GrOvalRenderer::CreateOvalBatch(paint.getColor(),
                                                                  viewMatrix,
                                                                  ovalRect,
                                                                  style.strokeRec(),
                                                                  shaderCaps));
            if (batch) {
                GrPipelineBuilder pipelineBuilder(paint, useHWAA);
                this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch.get());
                return;
            }
        }
    }

    // Note that internalDrawPath may sw-rasterize the path into a scratch texture.
    // Scratch textures can be recycled after they are returned to the texture
    // cache. This presents a potential hazard for buffered drawing. However,
    // the writePixels that uploads to the scratch will perform a flush so we're
    // OK.
    this->internalDrawPath(clip, paint, viewMatrix, path, style);
}

bool GrRenderTargetContextPriv::drawAndStencilPath(const GrClip& clip,
                                                   const GrUserStencilSettings* ss,
                                                   SkRegion::Op op,
                                                   bool invert,
                                                   bool doAA,
                                                   const SkMatrix& viewMatrix,
                                                   const SkPath& path) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkDEBUGCODE(fRenderTargetContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail, "GrRenderTargetContext::drawPath");

    if (path.isEmpty() && path.isInverseFillType()) {
        this->drawAndStencilRect(clip, ss, op, invert, false, SkMatrix::I(),
                                 SkRect::MakeIWH(fRenderTargetContext->width(),
                                                 fRenderTargetContext->height()));
        return true;
    }

    AutoCheckFlush acf(fRenderTargetContext->fDrawingManager);

    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    bool useCoverageAA = doAA && !fRenderTargetContext->isUnifiedMultisampled();
    bool hasUserStencilSettings = !ss->isUnused();
    bool isStencilBufferMSAA = fRenderTargetContext->isStencilBufferMultisampled();

    const GrPathRendererChain::DrawType type =
        useCoverageAA ? GrPathRendererChain::kColorAntiAlias_DrawType
                      : GrPathRendererChain::kColor_DrawType;

    GrShape shape(path, GrStyle::SimpleFill());
    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fShaderCaps =
        fRenderTargetContext->fDrawingManager->getContext()->caps()->shaderCaps();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fAntiAlias = useCoverageAA;
    canDrawArgs.fHasUserStencilSettings = hasUserStencilSettings;
    canDrawArgs.fIsStencilBufferMSAA = isStencilBufferMSAA;

    // Don't allow the SW renderer
    GrPathRenderer* pr = fRenderTargetContext->fDrawingManager->getPathRenderer(canDrawArgs, false,
                                                                                type);
    if (!pr) {
        return false;
    }

    GrPaint paint;
    paint.setCoverageSetOpXPFactory(op, invert);

    GrPathRenderer::DrawPathArgs args;
    args.fResourceProvider =
        fRenderTargetContext->fDrawingManager->getContext()->resourceProvider();
    args.fPaint = &paint;
    args.fUserStencilSettings = ss;
    args.fRenderTargetContext = fRenderTargetContext;
    args.fClip = &clip;
    args.fViewMatrix = &viewMatrix;
    args.fShape = &shape;
    args.fAntiAlias = useCoverageAA;
    args.fGammaCorrect = fRenderTargetContext->isGammaCorrect();
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
                                             const GrPaint& paint,
                                             const SkMatrix& viewMatrix,
                                             const SkPath& path,
                                             const GrStyle& style) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkASSERT(!path.isEmpty());

    bool useCoverageAA = should_apply_coverage_aa(paint, fRenderTargetProxy.get());
    constexpr bool kHasUserStencilSettings = false;
    bool isStencilBufferMSAA = this->isStencilBufferMultisampled();

    const GrPathRendererChain::DrawType type =
        useCoverageAA ? GrPathRendererChain::kColorAntiAlias_DrawType
                      : GrPathRendererChain::kColor_DrawType;

    GrShape shape(path, style);
    if (shape.isEmpty()) {
        return;
    }
    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fShaderCaps = fDrawingManager->getContext()->caps()->shaderCaps();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fShape = &shape;
    canDrawArgs.fAntiAlias = useCoverageAA;
    canDrawArgs.fHasUserStencilSettings = kHasUserStencilSettings;
    canDrawArgs.fIsStencilBufferMSAA = isStencilBufferMSAA;

    // Try a 1st time without applying any of the style to the geometry (and barring sw)
    GrPathRenderer* pr = fDrawingManager->getPathRenderer(canDrawArgs, false, type);
    SkScalar styleScale =  GrStyle::MatrixToScaleFactor(viewMatrix);

    if (!pr && shape.style().pathEffect()) {
        // It didn't work above, so try again with the path effect applied.
        shape = shape.applyStyle(GrStyle::Apply::kPathEffectOnly, styleScale);
        if (shape.isEmpty()) {
            return;
        }
        pr = fDrawingManager->getPathRenderer(canDrawArgs, false, type);
    }
    if (!pr) {
        if (shape.style().applies()) {
            shape = shape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale);
            if (shape.isEmpty()) {
                return;
            }
        }
        // This time, allow SW renderer
        pr = fDrawingManager->getPathRenderer(canDrawArgs, true, type);
    }

    if (!pr) {
#ifdef SK_DEBUG
        SkDebugf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    GrPathRenderer::DrawPathArgs args;
    args.fResourceProvider = fDrawingManager->getContext()->resourceProvider();
    args.fPaint = &paint;
    args.fUserStencilSettings = &GrUserStencilSettings::kUnused;
    args.fRenderTargetContext = this;
    args.fClip = &clip;
    args.fViewMatrix = &viewMatrix;
    args.fShape = canDrawArgs.fShape;
    args.fAntiAlias = useCoverageAA;
    args.fGammaCorrect = this->isGammaCorrect();
    pr->drawPath(args);
}

void GrRenderTargetContext::drawBatch(const GrPipelineBuilder& pipelineBuilder, const GrClip& clip,
                                      GrDrawOp* batch) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrRenderTargetContext::drawBatch");

    this->getOpList()->drawBatch(pipelineBuilder, this, clip, batch);
}

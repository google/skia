
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchTest.h"
#include "GrColor.h"
#include "GrDrawContext.h"
#include "GrDrawingManager.h"
#include "GrOvalRenderer.h"
#include "GrPathRenderer.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceProvider.h"
#include "SkSurfacePriv.h"

#include "batches/GrBatch.h"
#include "batches/GrDrawAtlasBatch.h"
#include "batches/GrDrawVerticesBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "batches/GrNinePatch.h" // TODO Factory

#include "text/GrAtlasTextContext.h"
#include "text/GrStencilAndCoverTextContext.h"

#include "../private/GrAuditTrail.h"

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == fDrawingManager->getContext())
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fSingleOwner);)
#define RETURN_IF_ABANDONED        if (fDrawingManager->abandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED  if (fDrawingManager->abandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED   if (fDrawingManager->abandoned()) { return nullptr; }

class AutoCheckFlush {
public:
    AutoCheckFlush(GrDrawingManager* drawingManager) : fDrawingManager(drawingManager) { 
        SkASSERT(fDrawingManager);
    }
    ~AutoCheckFlush() { fDrawingManager->getContext()->flushIfNecessary(); }

private:
    GrDrawingManager* fDrawingManager;
};

// In MDB mode the reffing of the 'getLastDrawTarget' call's result allows in-progress
// drawTargets to be picked up and added to by drawContexts lower in the call
// stack. When this occurs with a closed drawTarget, a new one will be allocated
// when the drawContext attempts to use it (via getDrawTarget).
GrDrawContext::GrDrawContext(GrDrawingManager* drawingMgr,
                             GrRenderTarget* rt,
                             const SkSurfaceProps* surfaceProps,
                             GrAuditTrail* auditTrail,
                             GrSingleOwner* singleOwner)
    : fDrawingManager(drawingMgr)
    , fRenderTarget(rt)
    , fDrawTarget(SkSafeRef(rt->getLastDrawTarget()))
    , fTextContext(nullptr)
    , fSurfaceProps(SkSurfacePropsCopyOrDefault(surfaceProps))
    , fAuditTrail(auditTrail)
#ifdef SK_DEBUG
    , fSingleOwner(singleOwner)
#endif
{
    SkDEBUGCODE(this->validate();)
}

#ifdef SK_DEBUG
void GrDrawContext::validate() const {
    SkASSERT(fRenderTarget);
    ASSERT_OWNED_RESOURCE(fRenderTarget);

    if (fDrawTarget && !fDrawTarget->isClosed()) {
        SkASSERT(fRenderTarget->getLastDrawTarget() == fDrawTarget);
    }
}
#endif

GrDrawContext::~GrDrawContext() {
    ASSERT_SINGLE_OWNER
    SkSafeUnref(fDrawTarget);
}

GrDrawTarget* GrDrawContext::getDrawTarget() {
    ASSERT_SINGLE_OWNER
    SkDEBUGCODE(this->validate();)

    if (!fDrawTarget || fDrawTarget->isClosed()) {
        fDrawTarget = fDrawingManager->newDrawTarget(fRenderTarget);
    }

    return fDrawTarget;
}

void GrDrawContext::copySurface(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::copySurface");

    this->getDrawTarget()->copySurface(fRenderTarget, src, srcRect, dstPoint);
}

void GrDrawContext::drawText(const GrClip& clip, const GrPaint& grPaint,
                             const SkPaint& skPaint,
                             const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             SkScalar x, SkScalar y, const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawText");

    if (!fTextContext) {
        fTextContext = fDrawingManager->textContext(fSurfaceProps, fRenderTarget);
    }

    fTextContext->drawText(this, clip, grPaint, skPaint, viewMatrix,
                           text, byteLength, x, y, clipBounds);
}

void GrDrawContext::drawPosText(const GrClip& clip, const GrPaint& grPaint,
                                const SkPaint& skPaint,
                                const SkMatrix& viewMatrix,
                                const char text[], size_t byteLength,
                                const SkScalar pos[], int scalarsPerPosition,
                                const SkPoint& offset, const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawPosText");

    if (!fTextContext) {
        fTextContext = fDrawingManager->textContext(fSurfaceProps, fRenderTarget);
    }

    fTextContext->drawPosText(this, clip, grPaint, skPaint, viewMatrix, text, byteLength,
                              pos, scalarsPerPosition, offset, clipBounds);

}

void GrDrawContext::drawTextBlob(const GrClip& clip, const SkPaint& skPaint,
                                 const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                 SkScalar x, SkScalar y,
                                 SkDrawFilter* filter, const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawTextBlob");

    if (!fTextContext) {
        fTextContext = fDrawingManager->textContext(fSurfaceProps, fRenderTarget);
    }

    fTextContext->drawTextBlob(this, clip, skPaint, viewMatrix, blob, x, y, filter, clipBounds);
}

void GrDrawContext::discard() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::discard");

    AutoCheckFlush acf(fDrawingManager);
    this->getDrawTarget()->discard(fRenderTarget);
}

void GrDrawContext::clear(const SkIRect* rect,
                          const GrColor color,
                          bool canIgnoreRect) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::clear");

    AutoCheckFlush acf(fDrawingManager);
    this->getDrawTarget()->clear(rect, color, canIgnoreRect, fRenderTarget);
}


void GrDrawContext::drawPaint(const GrClip& clip,
                              const GrPaint& origPaint,
                              const SkMatrix& viewMatrix) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawPaint");

    // set rect to be big enough to fill the space, but not super-huge, so we
    // don't overflow fixed-point implementations
    SkRect r;
    r.setLTRB(0, 0,
              SkIntToScalar(fRenderTarget->width()),
              SkIntToScalar(fRenderTarget->height()));
    SkTCopyOnFirstWrite<GrPaint> paint(origPaint);

    // by definition this fills the entire clip, no need for AA
    if (paint->isAntiAlias()) {
        paint.writable()->setAntiAlias(false);
    }

    bool isPerspective = viewMatrix.hasPerspective();

    // We attempt to map r by the inverse matrix and draw that. mapRect will
    // map the four corners and bound them with a new rect. This will not
    // produce a correct result for some perspective matrices.
    if (!isPerspective) {
        SkMatrix inverse;
        if (!viewMatrix.invert(&inverse)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }
        inverse.mapRect(&r);
        this->drawRect(clip, *paint, viewMatrix, r);
    } else {
        SkMatrix localMatrix;
        if (!viewMatrix.invert(&localMatrix)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }

        AutoCheckFlush acf(fDrawingManager);

        GrPipelineBuilder pipelineBuilder(*paint, fRenderTarget, clip);
        SkAutoTUnref<GrDrawBatch> batch(
                GrRectBatchFactory::CreateNonAAFill(paint->getColor(), SkMatrix::I(), r, nullptr,
                                                    &localMatrix));
        this->getDrawTarget()->drawBatch(pipelineBuilder, batch);
    }
}

static inline bool rect_contains_inclusive(const SkRect& rect, const SkPoint& point) {
    return point.fX >= rect.fLeft && point.fX <= rect.fRight &&
           point.fY >= rect.fTop && point.fY <= rect.fBottom;
}

static bool view_matrix_ok_for_aa_fill_rect(const SkMatrix& viewMatrix) {
    return viewMatrix.preservesRightAngles();
}

static bool should_apply_coverage_aa(const GrPaint& paint, GrRenderTarget* rt) {
    return paint.isAntiAlias() && !rt->isUnifiedMultisampled();
}

void GrDrawContext::drawRect(const GrClip& clip,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkRect& rect,
                             const GrStrokeInfo* strokeInfo) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawRect");

    // Dashing should've been devolved to a path in SkGpuDevice
    SkASSERT(!strokeInfo || !strokeInfo->isDashed());

    AutoCheckFlush acf(fDrawingManager);

    SkScalar width = nullptr == strokeInfo ? -1 : strokeInfo->getWidth();

    // Check if this is a full RT draw and can be replaced with a clear. We don't bother checking
    // cases where the RT is fully inside a stroke.
    if (width < 0) {
        SkRect rtRect;
        fRenderTarget->getBoundsRect(&rtRect);
        SkRect clipSpaceRTRect = rtRect;
        bool checkClip = GrClip::kWideOpen_ClipType != clip.clipType();
        if (checkClip) {
            clipSpaceRTRect.offset(SkIntToScalar(clip.origin().fX),
                                   SkIntToScalar(clip.origin().fY));
        }
        // Does the clip contain the entire RT?
        if (!checkClip || clip.quickContains(clipSpaceRTRect)) {
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
                    this->getDrawTarget()->clear(nullptr, clearColor, true, fRenderTarget);
                    return;
                }
            }
        }
    }

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);
    GrColor color = paint.getColor();

    SkAutoTUnref<GrDrawBatch> batch;
    if (should_apply_coverage_aa(paint, fRenderTarget)) {
        if (width >= 0) {
            // The stroke path needs the rect to remain axis aligned (no rotation or skew).
            if (viewMatrix.rectStaysRect()) {
                batch.reset(GrRectBatchFactory::CreateAAStroke(color, viewMatrix, rect,
                                                               *strokeInfo));
            }
        } else {
            // The fill path can handle rotation but not skew.
            if (view_matrix_ok_for_aa_fill_rect(viewMatrix)) {
                SkRect devBoundRect;
                viewMatrix.mapRect(&devBoundRect, rect);
                batch.reset(GrRectBatchFactory::CreateAAFill(color, viewMatrix, rect,
                                                             devBoundRect));
            }
        }
        if (!batch) {
            SkPath path;
            path.setIsVolatile(true);
            path.addRect(rect);
            this->internalDrawPath(&pipelineBuilder, viewMatrix, color, true, path, *strokeInfo);
            SkASSERT(paint.isAntiAlias());
            return;
        }
    } else if (width >= 0) {
        // Non-AA hairlines are snapped to pixel centers to make which pixels are hit deterministic
        bool snapToPixelCenters = (0 == width && !fRenderTarget->isUnifiedMultisampled());
        batch.reset(GrRectBatchFactory::CreateNonAAStroke(color, viewMatrix, rect, width,
                                                          snapToPixelCenters));

        // Depending on sub-pixel coordinates and the particular GPU, we may lose a corner of
        // hairline rects. We jam all the vertices to pixel centers to avoid this, but not when MSAA
        // is enabled because it can cause ugly artifacts.
        pipelineBuilder.setState(GrPipelineBuilder::kSnapVerticesToPixelCenters_Flag,
                                 snapToPixelCenters);
    } else {
        // filled BW rect
        batch.reset(GrRectBatchFactory::CreateNonAAFill(color, viewMatrix, rect, nullptr, nullptr));
    }
    this->getDrawTarget()->drawBatch(pipelineBuilder, batch);
}

void GrDrawContext::fillRectToRect(const GrClip& clip,
                                   const GrPaint& paint,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& rectToDraw,
                                   const SkRect& localRect) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::fillRectToRect");

    AutoCheckFlush acf(fDrawingManager);

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);
    SkAutoTUnref<GrDrawBatch> batch;
    if (should_apply_coverage_aa(paint, fRenderTarget) &&
        view_matrix_ok_for_aa_fill_rect(viewMatrix)) {
        batch.reset(GrAAFillRectBatch::CreateWithLocalRect(paint.getColor(), viewMatrix, rectToDraw,
                                                           localRect));
    } else {
        batch.reset(GrRectBatchFactory::CreateNonAAFill(paint.getColor(), viewMatrix, rectToDraw,
                                                        &localRect, nullptr));
    }

    if (batch) {
        this->drawBatch(&pipelineBuilder, batch);
    }
}

void GrDrawContext::fillRectWithLocalMatrix(const GrClip& clip,
                                            const GrPaint& paint,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rectToDraw,
                                            const SkMatrix& localMatrix) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::fillRectWithLocalMatrix");

    AutoCheckFlush acf(fDrawingManager);

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);

    SkAutoTUnref<GrDrawBatch> batch;
    if (should_apply_coverage_aa(paint, fRenderTarget) &&
        view_matrix_ok_for_aa_fill_rect(viewMatrix)) {
        batch.reset(GrAAFillRectBatch::Create(paint.getColor(), viewMatrix, localMatrix,
                                              rectToDraw));
    } else {
        batch.reset(GrRectBatchFactory::CreateNonAAFill(paint.getColor(), viewMatrix, rectToDraw,
                                                        nullptr, &localMatrix));
    }
    this->getDrawTarget()->drawBatch(pipelineBuilder, batch);
}

void GrDrawContext::drawVertices(const GrClip& clip,
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
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawVertices");

    AutoCheckFlush acf(fDrawingManager);

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);

    // TODO clients should give us bounds
    SkRect bounds;
    if (!bounds.setBoundsCheck(positions, vertexCount)) {
        SkDebugf("drawVertices call empty bounds\n");
        return;
    }

    viewMatrix.mapRect(&bounds);

    // If we don't have AA then we outset for a half pixel in each direction to account for
    // snapping. We also do this for the "hair" primitive types: lines and points since they have
    // a 1 pixel thickness in device space.
    if (!paint.isAntiAlias() || GrIsPrimTypeLines(primitiveType) ||
        kPoints_GrPrimitiveType == primitiveType) {
        bounds.outset(0.5f, 0.5f);
    }

    GrDrawVerticesBatch::Geometry geometry;
    geometry.fColor = paint.getColor();
    SkAutoTUnref<GrDrawBatch> batch(GrDrawVerticesBatch::Create(geometry, primitiveType, viewMatrix,
                                                                positions, vertexCount, indices,
                                                                indexCount, colors, texCoords,
                                                                bounds));

    this->getDrawTarget()->drawBatch(pipelineBuilder, batch);
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawAtlas(const GrClip& clip,
                              const GrPaint& paint,
                              const SkMatrix& viewMatrix,
                              int spriteCount,
                              const SkRSXform xform[],
                              const SkRect texRect[],
                              const SkColor colors[]) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawAtlas");

    AutoCheckFlush acf(fDrawingManager);
    
    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);
    
    GrDrawAtlasBatch::Geometry geometry;
    geometry.fColor = paint.getColor();
    SkAutoTUnref<GrDrawBatch> batch(GrDrawAtlasBatch::Create(geometry, viewMatrix, spriteCount,
                                                             xform, texRect, colors));
    
    this->getDrawTarget()->drawBatch(pipelineBuilder, batch);
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawRRect(const GrClip& clip,
                              const GrPaint& paint,
                              const SkMatrix& viewMatrix,
                              const SkRRect& rrect,
                              const GrStrokeInfo& strokeInfo) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawRRect");

    if (rrect.isEmpty()) {
       return;
    }

    SkASSERT(!strokeInfo.isDashed()); // this should've been devolved to a path in SkGpuDevice

    AutoCheckFlush acf(fDrawingManager);

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);
    GrColor color = paint.getColor();

    if (!GrOvalRenderer::DrawRRect(this->getDrawTarget(),
                                   pipelineBuilder,
                                   color,
                                   viewMatrix,
                                   paint.isAntiAlias(),
                                   rrect,
                                   strokeInfo)) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(rrect);
        this->internalDrawPath(&pipelineBuilder, viewMatrix, color,
                               paint.isAntiAlias(), path, strokeInfo);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawDRRect(const GrClip& clip,
                               const GrPaint& paint,
                               const SkMatrix& viewMatrix,
                               const SkRRect& outer,
                               const SkRRect& inner) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawDRRect");

    if (outer.isEmpty()) {
       return;
    }

    AutoCheckFlush acf(fDrawingManager);

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);
    GrColor color = paint.getColor();
    if (!GrOvalRenderer::DrawDRRect(this->getDrawTarget(),
                                    pipelineBuilder,
                                    color,
                                    viewMatrix,
                                    paint.isAntiAlias(),
                                    outer,
                                    inner)) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(inner);
        path.addRRect(outer);
        path.setFillType(SkPath::kEvenOdd_FillType);

        GrStrokeInfo fillRec(SkStrokeRec::kFill_InitStyle);
        this->internalDrawPath(&pipelineBuilder, viewMatrix, color,
                               paint.isAntiAlias(), path, fillRec);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawOval(const GrClip& clip,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkRect& oval,
                             const GrStrokeInfo& strokeInfo) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawOval");

    if (oval.isEmpty()) {
       return;
    }

    SkASSERT(!strokeInfo.isDashed()); // this should've been devolved to a path in SkGpuDevice

    AutoCheckFlush acf(fDrawingManager);

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);
    GrColor color = paint.getColor();

    if (!GrOvalRenderer::DrawOval(this->getDrawTarget(),
                                  pipelineBuilder,
                                  color,
                                  viewMatrix,
                                  paint.isAntiAlias(),
                                  oval,
                                  strokeInfo)) {
        SkPath path;
        path.setIsVolatile(true);
        path.addOval(oval);
        this->internalDrawPath(&pipelineBuilder, viewMatrix, color,
                               paint.isAntiAlias(), path, strokeInfo);
    }
}

void GrDrawContext::drawImageNine(const GrClip& clip,
                                  const GrPaint& paint,
                                  const SkMatrix& viewMatrix,
                                  int imageWidth,
                                  int imageHeight,
                                  const SkIRect& center,
                                  const SkRect& dst) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawImageNine");

    AutoCheckFlush acf(fDrawingManager);

    SkAutoTUnref<GrDrawBatch> batch(GrNinePatch::CreateNonAA(paint.getColor(), viewMatrix,
                                                             imageWidth, imageHeight,
                                                             center, dst));

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);
    this->getDrawTarget()->drawBatch(pipelineBuilder, batch);
}


// Can 'path' be drawn as a pair of filled nested rectangles?
static bool is_nested_rects(const SkMatrix& viewMatrix,
                            const SkPath& path,
                            const SkStrokeRec& stroke,
                            SkRect rects[2]) {
    SkASSERT(stroke.isFillStyle());

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

void GrDrawContext::drawBatch(const GrClip& clip,
                              const GrPaint& paint, GrDrawBatch* batch) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawBatch");

    AutoCheckFlush acf(fDrawingManager);

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);
    this->getDrawTarget()->drawBatch(pipelineBuilder, batch);
}

void GrDrawContext::drawPathBatch(const GrPipelineBuilder& pipelineBuilder,
                                  GrDrawPathBatchBase* batch) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawPathBatch");

    AutoCheckFlush acf(fDrawingManager);

    this->getDrawTarget()->drawPathBatch(pipelineBuilder, batch);
}

void GrDrawContext::drawPath(const GrClip& clip,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkPath& path,
                             const GrStrokeInfo& strokeInfo) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawPath");

    if (path.isEmpty()) {
       if (path.isInverseFillType()) {
           this->drawPaint(clip, paint, viewMatrix);
       }
       return;
    }

    GrColor color = paint.getColor();

    // Note that internalDrawPath may sw-rasterize the path into a scratch texture.
    // Scratch textures can be recycled after they are returned to the texture
    // cache. This presents a potential hazard for buffered drawing. However,
    // the writePixels that uploads to the scratch will perform a flush so we're
    // OK.
    AutoCheckFlush acf(fDrawingManager);

    GrPipelineBuilder pipelineBuilder(paint, fRenderTarget, clip);
    if (!strokeInfo.isDashed()) {
        bool useCoverageAA = should_apply_coverage_aa(paint, pipelineBuilder.getRenderTarget());

        if (useCoverageAA && strokeInfo.getWidth() < 0 && !path.isConvex()) {
            // Concave AA paths are expensive - try to avoid them for special cases
            SkRect rects[2];

            if (is_nested_rects(viewMatrix, path, strokeInfo, rects)) {
                SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateAAFillNestedRects(
                    color, viewMatrix, rects));
                this->getDrawTarget()->drawBatch(pipelineBuilder, batch);
                return;
            }
        }
        SkRect ovalRect;
        bool isOval = path.isOval(&ovalRect);

        if (isOval && !path.isInverseFillType()) {
            if (GrOvalRenderer::DrawOval(this->getDrawTarget(),
                                         pipelineBuilder,
                                         color,
                                         viewMatrix,
                                         paint.isAntiAlias(),
                                         ovalRect,
                                         strokeInfo)) {
                return;
            }
        }
    }
    this->internalDrawPath(&pipelineBuilder, viewMatrix, color,
                           paint.isAntiAlias(), path, strokeInfo);
}

void GrDrawContext::internalDrawPath(GrPipelineBuilder* pipelineBuilder,
                                     const SkMatrix& viewMatrix,
                                     GrColor color,
                                     bool useAA,
                                     const SkPath& path,
                                     const GrStrokeInfo& strokeInfo) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkASSERT(!path.isEmpty());

    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    bool useCoverageAA = useAA &&
        !pipelineBuilder->getRenderTarget()->isUnifiedMultisampled();
    bool isStencilDisabled = pipelineBuilder->getStencil().isDisabled();
    bool isStencilBufferMSAA = pipelineBuilder->getRenderTarget()->isStencilBufferMultisampled();

    const GrPathRendererChain::DrawType type =
        useCoverageAA ? GrPathRendererChain::kColorAntiAlias_DrawType
                      : GrPathRendererChain::kColor_DrawType;

    const SkPath* pathPtr = &path;
    SkTLazy<SkPath> tmpPath;
    const GrStrokeInfo* strokeInfoPtr = &strokeInfo;

    GrPathRenderer::CanDrawPathArgs canDrawArgs;
    canDrawArgs.fShaderCaps = fDrawingManager->getContext()->caps()->shaderCaps();
    canDrawArgs.fViewMatrix = &viewMatrix;
    canDrawArgs.fPath = pathPtr;
    canDrawArgs.fStroke = strokeInfoPtr;
    canDrawArgs.fAntiAlias = useCoverageAA;
    canDrawArgs.fIsStencilDisabled = isStencilDisabled;
    canDrawArgs.fIsStencilBufferMSAA = isStencilBufferMSAA;

    // Try a 1st time without stroking the path and without allowing the SW renderer
    GrPathRenderer* pr = fDrawingManager->getPathRenderer(canDrawArgs, false, type);

    GrStrokeInfo dashlessStrokeInfo(strokeInfo, false);
    if (nullptr == pr && strokeInfo.isDashed()) {
        // It didn't work above, so try again with dashed stroke converted to a dashless stroke.
        if (!strokeInfo.applyDashToPath(tmpPath.init(), &dashlessStrokeInfo, *pathPtr)) {
            return;
        }
        pathPtr = tmpPath.get();
        if (pathPtr->isEmpty()) {
            return;
        }
        strokeInfoPtr = &dashlessStrokeInfo;

        canDrawArgs.fPath = pathPtr;
        canDrawArgs.fStroke = strokeInfoPtr;

        pr = fDrawingManager->getPathRenderer(canDrawArgs, false, type);
    }

    if (nullptr == pr) {
        if (!GrPathRenderer::IsStrokeHairlineOrEquivalent(*strokeInfoPtr, viewMatrix, nullptr) &&
            !strokeInfoPtr->isFillStyle()) {
            // It didn't work above, so try again with stroke converted to a fill.
            if (!tmpPath.isValid()) {
                tmpPath.init();
            }
            dashlessStrokeInfo.setResScale(SkScalarAbs(viewMatrix.getMaxScale()));
            if (!dashlessStrokeInfo.applyToPath(tmpPath.get(), *pathPtr)) {
                return;
            }
            pathPtr = tmpPath.get();
            if (pathPtr->isEmpty()) {
                return;
            }
            dashlessStrokeInfo.setFillStyle();
            strokeInfoPtr = &dashlessStrokeInfo;
        }

        canDrawArgs.fPath = pathPtr;
        canDrawArgs.fStroke = strokeInfoPtr;

        // This time, allow SW renderer
        pr = fDrawingManager->getPathRenderer(canDrawArgs, true, type);
    }

    if (nullptr == pr) {
#ifdef SK_DEBUG
        SkDebugf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    GrPathRenderer::DrawPathArgs args;
    args.fTarget = this->getDrawTarget();
    args.fResourceProvider = fDrawingManager->getContext()->resourceProvider();
    args.fPipelineBuilder = pipelineBuilder;
    args.fColor = color;
    args.fViewMatrix = &viewMatrix;
    args.fPath = pathPtr;
    args.fStroke = strokeInfoPtr;
    args.fAntiAlias = useCoverageAA;
    pr->drawPath(args);
}

void GrDrawContext::drawBatch(GrPipelineBuilder* pipelineBuilder, GrDrawBatch* batch) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrDrawContext::drawBatch");

    this->getDrawTarget()->drawBatch(*pipelineBuilder, batch);
}

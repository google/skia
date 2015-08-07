
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAARectRenderer.h"
#include "GrAtlasTextContext.h"
#include "GrBatchTest.h"
#include "GrColor.h"
#include "GrDrawContext.h"
#include "GrOvalRenderer.h"
#include "GrPathRenderer.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAndCoverTextContext.h"

#include "batches/GrBatch.h"
#include "batches/GrDrawAtlasBatch.h"
#include "batches/GrDrawVerticesBatch.h"
#include "batches/GrStrokeRectBatch.h"

#include "SkGr.h"
#include "SkRSXform.h"

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == fContext)
#define RETURN_IF_ABANDONED        if (!fDrawTarget) { return; }
#define RETURN_FALSE_IF_ABANDONED  if (!fDrawTarget) { return false; }
#define RETURN_NULL_IF_ABANDONED   if (!fDrawTarget) { return NULL; }

class AutoCheckFlush {
public:
    AutoCheckFlush(GrContext* context) : fContext(context) { SkASSERT(context); }
    ~AutoCheckFlush() { fContext->flushIfNecessary(); }

private:
    GrContext* fContext;
};

GrDrawContext::GrDrawContext(GrContext* context,
                             GrDrawTarget* drawTarget,
                             const SkSurfaceProps& surfaceProps)
    : fContext(context)
    , fDrawTarget(SkRef(drawTarget))
    , fTextContext(NULL)
    , fSurfaceProps(surfaceProps) {
}

GrDrawContext::~GrDrawContext() {
    SkSafeUnref(fDrawTarget);
    SkDELETE(fTextContext);
}

void GrDrawContext::copySurface(GrRenderTarget* dst, GrSurface* src,
                                const SkIRect& srcRect, const SkIPoint& dstPoint) {
    if (!this->prepareToDraw(dst)) {
        return;
    }

    fDrawTarget->copySurface(dst, src, srcRect, dstPoint);
}

GrTextContext* GrDrawContext::createTextContext(GrRenderTarget* renderTarget,
                                                const SkSurfaceProps& surfaceProps) {
    if (fContext->caps()->shaderCaps()->pathRenderingSupport() &&
        renderTarget->isStencilBufferMultisampled() &&
        fSurfaceProps.isUseDistanceFieldFonts()) { // FIXME: Rename the dff flag to be more general.
        GrStencilAttachment* sb = renderTarget->renderTargetPriv().attachStencilAttachment();
        if (sb) {
            return GrStencilAndCoverTextContext::Create(fContext, this, surfaceProps);
        }
    } 

    return GrAtlasTextContext::Create(fContext, this, surfaceProps);
}

void GrDrawContext::drawText(GrRenderTarget* rt, const GrClip& clip, const GrPaint& grPaint,
                             const SkPaint& skPaint,
                             const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             SkScalar x, SkScalar y, const SkIRect& clipBounds) {
    if (!fTextContext) {
        fTextContext = this->createTextContext(rt, fSurfaceProps);
    }

    fTextContext->drawText(rt, clip, grPaint, skPaint, viewMatrix,
                           text, byteLength, x, y, clipBounds);

}
void GrDrawContext::drawPosText(GrRenderTarget* rt, const GrClip& clip, const GrPaint& grPaint,
                                const SkPaint& skPaint,
                                const SkMatrix& viewMatrix,
                                const char text[], size_t byteLength,
                                const SkScalar pos[], int scalarsPerPosition,
                                const SkPoint& offset, const SkIRect& clipBounds) {
    if (!fTextContext) {
        fTextContext = this->createTextContext(rt, fSurfaceProps);
    }

    fTextContext->drawPosText(rt, clip, grPaint, skPaint, viewMatrix, text, byteLength,
                               pos, scalarsPerPosition, offset, clipBounds);

}
void GrDrawContext::drawTextBlob(GrRenderTarget* rt, const GrClip& clip, const SkPaint& skPaint,
                                 const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                 SkScalar x, SkScalar y,
                                 SkDrawFilter* filter, const SkIRect& clipBounds) {
    if (!fTextContext) {
        fTextContext = this->createTextContext(rt, fSurfaceProps);
    }

    fTextContext->drawTextBlob(rt, clip, skPaint, viewMatrix, blob, x, y, filter, clipBounds);
}

void GrDrawContext::drawPaths(GrPipelineBuilder* pipelineBuilder,
                              const GrPathProcessor* pathProc,
                              const GrPathRange* pathRange,
                              const void* indices,
                              int /*GrDrawTarget::PathIndexType*/ indexType,
                              const float transformValues[],
                              int /*GrDrawTarget::PathTransformType*/ transformType,
                              int count,
                              int /*GrPathRendering::FillType*/ fill) {
    fDrawTarget->drawPaths(*pipelineBuilder, pathProc, pathRange,
                           indices, (GrDrawTarget::PathIndexType) indexType,
                           transformValues,
                           (GrDrawTarget::PathTransformType) transformType,
                           count, (GrPathRendering::FillType) fill);
}

void GrDrawContext::discard(GrRenderTarget* renderTarget) {
    RETURN_IF_ABANDONED
    SkASSERT(renderTarget);
    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(renderTarget)) {
        return;
    }
    fDrawTarget->discard(renderTarget);
}

void GrDrawContext::clear(GrRenderTarget* renderTarget,
                          const SkIRect* rect,
                          const GrColor color,
                          bool canIgnoreRect) {
    RETURN_IF_ABANDONED
    SkASSERT(renderTarget);

    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(renderTarget)) {
        return;
    }
    fDrawTarget->clear(rect, color, canIgnoreRect, renderTarget);
}


void GrDrawContext::drawPaint(GrRenderTarget* rt,
                              const GrClip& clip,
                              const GrPaint& origPaint,
                              const SkMatrix& viewMatrix) {
    RETURN_IF_ABANDONED
    // set rect to be big enough to fill the space, but not super-huge, so we
    // don't overflow fixed-point implementations
    SkRect r;
    r.setLTRB(0, 0,
              SkIntToScalar(rt->width()),
              SkIntToScalar(rt->height()));
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
        this->drawRect(rt, clip, *paint, viewMatrix, r);
    } else {
        SkMatrix localMatrix;
        if (!viewMatrix.invert(&localMatrix)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }

        AutoCheckFlush acf(fContext);
        if (!this->prepareToDraw(rt)) {
            return;
        }

        GrPipelineBuilder pipelineBuilder(*paint, rt, clip);
        fDrawTarget->drawBWRect(pipelineBuilder,
                                paint->getColor(),
                                SkMatrix::I(),
                                r,
                                NULL,
                                &localMatrix);
    }
}

static inline bool is_irect(const SkRect& r) {
  return SkScalarIsInt(r.fLeft)  && SkScalarIsInt(r.fTop) &&
         SkScalarIsInt(r.fRight) && SkScalarIsInt(r.fBottom);
}

static bool apply_aa_to_rect(GrDrawTarget* target,
                             GrPipelineBuilder* pipelineBuilder,
                             SkRect* devBoundRect,
                             const SkRect& rect,
                             SkScalar strokeWidth,
                             const SkMatrix& combinedMatrix,
                             GrColor color) {
    if (pipelineBuilder->getRenderTarget()->isUnifiedMultisampled() ||
        !combinedMatrix.preservesAxisAlignment()) {
        return false;
    }

    combinedMatrix.mapRect(devBoundRect, rect);
    if (!combinedMatrix.rectStaysRect()) {
        return true;
    }

    if (strokeWidth < 0) {
        return !is_irect(*devBoundRect);
    }

    return true;
}

static inline bool rect_contains_inclusive(const SkRect& rect, const SkPoint& point) {
    return point.fX >= rect.fLeft && point.fX <= rect.fRight &&
           point.fY >= rect.fTop && point.fY <= rect.fBottom;
}

void GrDrawContext::drawRect(GrRenderTarget* rt,
                             const GrClip& clip,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkRect& rect,
                             const GrStrokeInfo* strokeInfo) {
    RETURN_IF_ABANDONED
    if (strokeInfo && strokeInfo->isDashed()) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRect(rect);
        this->drawPath(rt, clip, paint, viewMatrix, path, *strokeInfo);
        return;
    }

    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(rt)) {
        return;
    }

    GrPipelineBuilder pipelineBuilder(paint, rt, clip);

    SkScalar width = NULL == strokeInfo ? -1 : strokeInfo->getWidth();

    // Check if this is a full RT draw and can be replaced with a clear. We don't bother checking
    // cases where the RT is fully inside a stroke.
    if (width < 0) {
        SkRect rtRect;
        pipelineBuilder.getRenderTarget()->getBoundsRect(&rtRect);
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
                    fDrawTarget->clear(NULL, clearColor, true, rt);
                    return;
                }
            }
        }
    }

    GrColor color = paint.getColor();
    SkRect devBoundRect;
    bool needAA = paint.isAntiAlias() &&
                  !pipelineBuilder.getRenderTarget()->isUnifiedMultisampled();
    bool doAA = needAA && apply_aa_to_rect(fDrawTarget, &pipelineBuilder, &devBoundRect, rect,
                                           width, viewMatrix, color);

    if (doAA) {
        if (width >= 0) {
            GrAARectRenderer::StrokeAARect(fDrawTarget,
                                           pipelineBuilder,
                                           color,
                                           viewMatrix,
                                           rect,
                                           devBoundRect,
                                           *strokeInfo);
        } else {
            // filled AA rect
            GrAARectRenderer::FillAARect(fDrawTarget,
                                         pipelineBuilder,
                                         color,
                                         viewMatrix,
                                         rect,
                                         devBoundRect);
        }
        return;
    }

    if (width >= 0) {
        GrStrokeRectBatch::Geometry geometry;
        geometry.fViewMatrix = viewMatrix;
        geometry.fColor = color;
        geometry.fRect = rect;
        geometry.fStrokeWidth = width;

        // Non-AA hairlines are snapped to pixel centers to make which pixels are hit deterministic
        bool snapToPixelCenters = (0 == width && !rt->isUnifiedMultisampled());
        SkAutoTUnref<GrBatch> batch(GrStrokeRectBatch::Create(geometry, snapToPixelCenters));

        // Depending on sub-pixel coordinates and the particular GPU, we may lose a corner of
        // hairline rects. We jam all the vertices to pixel centers to avoid this, but not when MSAA
        // is enabled because it can cause ugly artifacts.
        pipelineBuilder.setState(GrPipelineBuilder::kSnapVerticesToPixelCenters_Flag,
                                 snapToPixelCenters);
        fDrawTarget->drawBatch(pipelineBuilder, batch);
    } else {
        // filled BW rect
        fDrawTarget->drawSimpleRect(pipelineBuilder, color, viewMatrix, rect);
    }
}

void GrDrawContext::drawNonAARectToRect(GrRenderTarget* rt,
                                        const GrClip& clip,
                                        const GrPaint& paint,
                                        const SkMatrix& viewMatrix,
                                        const SkRect& rectToDraw,
                                        const SkRect& localRect,
                                        const SkMatrix* localMatrix) {
    RETURN_IF_ABANDONED
    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(rt)) {
        return;
    }

    GrPipelineBuilder pipelineBuilder(paint, rt, clip);
    fDrawTarget->drawBWRect(pipelineBuilder,
                            paint.getColor(),
                            viewMatrix,
                            rectToDraw,
                            &localRect,
                            localMatrix);
}

void GrDrawContext::drawVertices(GrRenderTarget* rt,
                                 const GrClip& clip,
                                 const GrPaint& paint,
                                 const SkMatrix& viewMatrix,
                                 GrPrimitiveType primitiveType,
                                 int vertexCount,
                                 const SkPoint positions[],
                                 const SkPoint texCoords[],
                                 const GrColor colors[],
                                 const uint16_t indices[],
                                 int indexCount) {
    RETURN_IF_ABANDONED
    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(rt)) {
        return;
    }

    GrPipelineBuilder pipelineBuilder(paint, rt, clip);

    // TODO clients should give us bounds
    SkRect bounds;
    if (!bounds.setBoundsCheck(positions, vertexCount)) {
        SkDebugf("drawVertices call empty bounds\n");
        return;
    }

    viewMatrix.mapRect(&bounds);

    // If we don't have AA then we outset for a half pixel in each direction to account for
    // snapping
    if (!paint.isAntiAlias()) {
        bounds.outset(0.5f, 0.5f);
    }

    GrDrawVerticesBatch::Geometry geometry;
    geometry.fColor = paint.getColor();
    SkAutoTUnref<GrBatch> batch(GrDrawVerticesBatch::Create(geometry, primitiveType, viewMatrix,
                                                            positions, vertexCount, indices,
                                                            indexCount, colors, texCoords,
                                                            bounds));

    fDrawTarget->drawBatch(pipelineBuilder, batch);
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawAtlas(GrRenderTarget* rt,
                              const GrClip& clip,
                              const GrPaint& paint,
                              const SkMatrix& viewMatrix,
                              int spriteCount,
                              const SkRSXform xform[],
                              const SkRect texRect[],
                              const SkColor colors[]) {
    RETURN_IF_ABANDONED
    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(rt)) {
        return;
    }
    
    GrPipelineBuilder pipelineBuilder(paint, rt, clip);
    
    // now build the renderable geometry
    const int vertCount = spriteCount * 4;
    SkAutoTMalloc<SkPoint> vertStorage(vertCount * 2);
    SkPoint* verts = vertStorage.get();
    SkPoint* texs = verts + vertCount;
    
    for (int i = 0; i < spriteCount; ++i) {
        xform[i].toQuad(texRect[i].width(), texRect[i].height(), verts);
        texRect[i].toQuad(texs);
        verts += 4;
        texs += 4;
    }
    
    // TODO clients should give us bounds
    SkRect bounds;
    if (!bounds.setBoundsCheck(vertStorage.get(), vertCount)) {
        SkDebugf("drawAtlas call empty bounds\n");
        return;
    }
    
    viewMatrix.mapRect(&bounds);
    
    // If we don't have AA then we outset for a half pixel in each direction to account for
    // snapping
    if (!paint.isAntiAlias()) {
        bounds.outset(0.5f, 0.5f);
    }
    
    SkAutoTMalloc<GrColor> colorStorage;
    GrColor* vertCols = NULL;
    if (colors) {
        colorStorage.reset(vertCount);
        vertCols = colorStorage.get();
        
        int paintAlpha = GrColorUnpackA(paint.getColor());

        // need to convert byte order and from non-PM to PM
        for (int i = 0; i < spriteCount; ++i) {
            SkColor color = colors[i];
            if (paintAlpha != 255) {
                color = SkColorSetA(color, SkMulDiv255Round(SkColorGetA(color), paintAlpha));
            }
            GrColor grColor = SkColor2GrColor(color);
            
            vertCols[0] = vertCols[1] = vertCols[2] = vertCols[3] = grColor;
            vertCols += 4;
        }
    }
    
    verts = vertStorage.get();
    texs = verts + vertCount;
    vertCols = colorStorage.get();
    
    GrDrawAtlasBatch::Geometry geometry;
    geometry.fColor = paint.getColor();
    SkAutoTUnref<GrBatch> batch(GrDrawAtlasBatch::Create(geometry, viewMatrix, verts, vertCount,
                                                         vertCols, texs, bounds));
    
    fDrawTarget->drawBatch(pipelineBuilder, batch);
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawRRect(GrRenderTarget*rt,
                              const GrClip& clip,
                              const GrPaint& paint,
                              const SkMatrix& viewMatrix,
                              const SkRRect& rrect,
                              const GrStrokeInfo& strokeInfo) {
    RETURN_IF_ABANDONED
    if (rrect.isEmpty()) {
       return;
    }

    if (strokeInfo.isDashed()) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(rrect);
        this->drawPath(rt, clip, paint, viewMatrix, path, strokeInfo);
        return;
    }

    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(rt)) {
        return;
    }

    GrPipelineBuilder pipelineBuilder(paint, rt, clip);
    GrColor color = paint.getColor();
    if (!GrOvalRenderer::DrawRRect(fDrawTarget,
                                   pipelineBuilder,
                                   color,
                                   viewMatrix,
                                   paint.isAntiAlias(),
                                   rrect,
                                   strokeInfo)) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(rrect);
        this->internalDrawPath(fDrawTarget, &pipelineBuilder, viewMatrix, color,
                               paint.isAntiAlias(), path, strokeInfo);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawDRRect(GrRenderTarget* rt,
                               const GrClip& clip,
                               const GrPaint& paint,
                               const SkMatrix& viewMatrix,
                               const SkRRect& outer,
                               const SkRRect& inner) {
    RETURN_IF_ABANDONED
    if (outer.isEmpty()) {
       return;
    }

    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(rt)) {
        return;
    }

    GrPipelineBuilder pipelineBuilder(paint, rt, clip);
    GrColor color = paint.getColor();
    if (!GrOvalRenderer::DrawDRRect(fDrawTarget,
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
        this->internalDrawPath(fDrawTarget, &pipelineBuilder, viewMatrix, color,
                               paint.isAntiAlias(), path, fillRec);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawOval(GrRenderTarget* rt,
                             const GrClip& clip,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkRect& oval,
                             const GrStrokeInfo& strokeInfo) {
    RETURN_IF_ABANDONED
    if (oval.isEmpty()) {
       return;
    }

    if (strokeInfo.isDashed()) {
        SkPath path;
        path.setIsVolatile(true);
        path.addOval(oval);
        this->drawPath(rt, clip, paint, viewMatrix, path, strokeInfo);
        return;
    }

    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(rt)) {
        return;
    }

    GrPipelineBuilder pipelineBuilder(paint, rt, clip);
    GrColor color = paint.getColor();
    if (!GrOvalRenderer::DrawOval(fDrawTarget,
                                  pipelineBuilder,
                                  color,
                                  viewMatrix,
                                  paint.isAntiAlias(),
                                  oval,
                                  strokeInfo)) {
        SkPath path;
        path.setIsVolatile(true);
        path.addOval(oval);
        this->internalDrawPath(fDrawTarget, &pipelineBuilder, viewMatrix, color,
                               paint.isAntiAlias(), path, strokeInfo);
    }
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
    if (!viewMatrix.preservesAxisAlignment()) {
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

void GrDrawContext::drawPath(GrRenderTarget* rt,
                             const GrClip& clip,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkPath& path,
                             const GrStrokeInfo& strokeInfo) {
    RETURN_IF_ABANDONED
    if (path.isEmpty()) {
       if (path.isInverseFillType()) {
           this->drawPaint(rt, clip, paint, viewMatrix);
       }
       return;
    }

    GrColor color = paint.getColor();

    // Note that internalDrawPath may sw-rasterize the path into a scratch texture.
    // Scratch textures can be recycled after they are returned to the texture
    // cache. This presents a potential hazard for buffered drawing. However,
    // the writePixels that uploads to the scratch will perform a flush so we're
    // OK.
    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(rt)) {
        return;
    }

    GrPipelineBuilder pipelineBuilder(paint, rt, clip);
    if (!strokeInfo.isDashed()) {
        bool useCoverageAA = paint.isAntiAlias() &&
                !pipelineBuilder.getRenderTarget()->isUnifiedMultisampled();

        if (useCoverageAA && strokeInfo.getWidth() < 0 && !path.isConvex()) {
            // Concave AA paths are expensive - try to avoid them for special cases
            SkRect rects[2];

            if (is_nested_rects(viewMatrix, path, strokeInfo, rects)) {
                GrAARectRenderer::FillAANestedRects(fDrawTarget, pipelineBuilder, color,
                                                    viewMatrix, rects);
                return;
            }
        }
        SkRect ovalRect;
        bool isOval = path.isOval(&ovalRect);

        if (isOval && !path.isInverseFillType()) {
            if (GrOvalRenderer::DrawOval(fDrawTarget,
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
    this->internalDrawPath(fDrawTarget, &pipelineBuilder, viewMatrix, color, paint.isAntiAlias(),
                           path, strokeInfo);
}

void GrDrawContext::internalDrawPath(GrDrawTarget* target,
                                     GrPipelineBuilder* pipelineBuilder,
                                     const SkMatrix& viewMatrix,
                                     GrColor color,
                                     bool useAA,
                                     const SkPath& path,
                                     const GrStrokeInfo& strokeInfo) {
    RETURN_IF_ABANDONED
    SkASSERT(!path.isEmpty());


    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    bool useCoverageAA = useAA &&
        !pipelineBuilder->getRenderTarget()->isUnifiedMultisampled();


    GrPathRendererChain::DrawType type =
        useCoverageAA ? GrPathRendererChain::kColorAntiAlias_DrawType :
                        GrPathRendererChain::kColor_DrawType;

    const SkPath* pathPtr = &path;
    SkTLazy<SkPath> tmpPath;
    const GrStrokeInfo* strokeInfoPtr = &strokeInfo;

    // Try a 1st time without stroking the path and without allowing the SW renderer
    GrPathRenderer* pr = fContext->getPathRenderer(target, pipelineBuilder, viewMatrix, *pathPtr,
                                                    *strokeInfoPtr, false, type);

    GrStrokeInfo dashlessStrokeInfo(strokeInfo, false);
    if (NULL == pr && strokeInfo.isDashed()) {
        // It didn't work above, so try again with dashed stroke converted to a dashless stroke.
        if (!strokeInfo.applyDashToPath(tmpPath.init(), &dashlessStrokeInfo, *pathPtr)) {
            return;
        }
        pathPtr = tmpPath.get();
        if (pathPtr->isEmpty()) {
            return;
        }
        strokeInfoPtr = &dashlessStrokeInfo;
        pr = fContext->getPathRenderer(target, pipelineBuilder, viewMatrix, *pathPtr, *strokeInfoPtr,
                                       false, type);
    }

    if (NULL == pr) {
        if (!GrPathRenderer::IsStrokeHairlineOrEquivalent(*strokeInfoPtr, viewMatrix, NULL) &&
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

        // This time, allow SW renderer
        pr = fContext->getPathRenderer(target, pipelineBuilder, viewMatrix, *pathPtr, *strokeInfoPtr,
                                       true, type);
    }

    if (NULL == pr) {
#ifdef SK_DEBUG
        SkDebugf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    GrPathRenderer::DrawPathArgs args;
    args.fTarget = target;
    args.fResourceProvider = fContext->resourceProvider();
    args.fPipelineBuilder = pipelineBuilder;
    args.fColor = color;
    args.fViewMatrix = &viewMatrix;
    args.fPath = pathPtr;
    args.fStroke = strokeInfoPtr;
    args.fAntiAlias = useCoverageAA;
    pr->drawPath(args);
}

bool GrDrawContext::prepareToDraw(GrRenderTarget* rt) {
    RETURN_FALSE_IF_ABANDONED

    ASSERT_OWNED_RESOURCE(rt);
    SkASSERT(rt);
    return true;
}

void GrDrawContext::drawBatch(GrPipelineBuilder* pipelineBuilder, GrBatch* batch) {
    fDrawTarget->drawBatch(*pipelineBuilder, batch);
}

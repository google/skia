/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawContext_DEFINED
#define GrDrawContext_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"
#include "SkSurfaceProps.h"

class GrBatch;
class GrClip;
class GrContext;
class GrDrawTarget;
class GrPaint;
class GrPathProcessor;
class GrPathRange;
class GrPipelineBuilder;
class GrRenderTarget;
class GrStrokeInfo;
class GrSurface;
class GrTextContext;
class SkDrawFilter;
struct SkIPoint;
struct SkIRect;
class SkMatrix;
class SkPaint;
class SkPath;
struct SkPoint;
struct SkRect;
class SkRRect;
struct SkRSXform;
class SkTextBlob;

/*
 * A helper object to orchestrate draws
 */
class SK_API GrDrawContext : public SkRefCnt {
public:
    ~GrDrawContext() override;

    void copySurface(GrRenderTarget* dst, GrSurface* src,
                     const SkIRect& srcRect, const SkIPoint& dstPoint);

    // TODO: it is odd that we need both the SkPaint in the following 3 methods.
    // We should extract the text parameters from SkPaint and pass them separately
    // akin to GrStrokeInfo (GrTextInfo?)
    void drawText(GrRenderTarget*, const GrClip&,  const GrPaint&, const SkPaint&,
                  const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                  SkScalar x, SkScalar y, const SkIRect& clipBounds);
    void drawPosText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                     const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                     const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& clipBounds);
    void drawTextBlob(GrRenderTarget*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkTextBlob*,
                      SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds);

    // drawPaths is thanks to GrStencilAndCoverTextContext
    // TODO: remove
    void drawPaths(GrPipelineBuilder* pipelineBuilder,
                   const GrPathProcessor* pathProc,
                   const GrPathRange* pathRange,
                   const void* indices,
                   int /*GrDrawTarget::PathIndexType*/ indexType,
                   const float transformValues[],
                   int /*GrDrawTarget::PathTransformType*/ transformType,
                   int count,
                   int /*GrPathRendering::FillType*/ fill);

    /**
     * Provides a perfomance hint that the render target's contents are allowed
     * to become undefined.
     */
    void discard(GrRenderTarget*);

    /**
     * Clear the entire or rect of the render target, ignoring any clips.
     * @param target The render target to clear.
     * @param rect  the rect to clear or the whole thing if rect is NULL.
     * @param color the color to clear to.
     * @param canIgnoreRect allows partial clears to be converted to whole
     *                      clears on platforms for which that is cheap
     */
    void clear(GrRenderTarget*, const SkIRect* rect, GrColor color, bool canIgnoreRect);

    /**
     *  Draw everywhere (respecting the clip) with the paint.
     */
    void drawPaint(GrRenderTarget*, const GrClip&, const GrPaint&, const SkMatrix& viewMatrix);

    /**
     *  Draw the rect using a paint.
     *  @param paint        describes how to color pixels.
     *  @param viewMatrix   transformation matrix
     *  @param strokeInfo   the stroke information (width, join, cap), and.
     *                      the dash information (intervals, count, phase).
     *                      If strokeInfo == NULL, then the rect is filled.
     *                      Otherwise, if stroke width == 0, then the stroke
     *                      is always a single pixel thick, else the rect is
     *                      mitered/beveled stroked based on stroke width.
     *  The rects coords are used to access the paint (through texture matrix)
     */
    void drawRect(GrRenderTarget*,
                  const GrClip&,
                  const GrPaint& paint,
                  const SkMatrix& viewMatrix,
                  const SkRect&,
                  const GrStrokeInfo* strokeInfo = NULL);

    /**
     * Maps a rectangle of shader coordinates to a rectangle and draws that rectangle
     *
     * @param paint         describes how to color pixels.
     * @param viewMatrix    transformation matrix which applies to rectToDraw
     * @param rectToDraw    the rectangle to draw
     * @param localRect     the rectangle of shader coordinates applied to rectToDraw
     * @param localMatrix   an optional matrix to transform the shader coordinates before applying
     *                      to rectToDraw
     */
    void drawNonAARectToRect(GrRenderTarget*,
                             const GrClip&,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkRect& rectToDraw,
                             const SkRect& localRect,
                             const SkMatrix* localMatrix = NULL);

    /**
     * Draws a non-AA rect with paint and a localMatrix
     */
    void drawNonAARectWithLocalMatrix(GrRenderTarget* rt,
                                      const GrClip& clip,
                                      const GrPaint& paint,
                                      const SkMatrix& viewMatrix,
                                      const SkRect& rect,
                                      const SkMatrix& localMatrix) {
        this->drawNonAARectToRect(rt, clip, paint, viewMatrix, rect, rect, &localMatrix);
    }

    /**
     *  Draw a roundrect using a paint.
     *
     *  @param paint        describes how to color pixels.
     *  @param viewMatrix   transformation matrix
     *  @param rrect        the roundrect to draw
     *  @param strokeInfo   the stroke information (width, join, cap) and
     *                      the dash information (intervals, count, phase).
     */
    void drawRRect(GrRenderTarget*,
                   const GrClip&,
                   const GrPaint&,
                   const SkMatrix& viewMatrix,
                   const SkRRect& rrect,
                   const GrStrokeInfo&);

    /**
     *  Shortcut for drawing an SkPath consisting of nested rrects using a paint.
     *  Does not support stroking. The result is undefined if outer does not contain
     *  inner.
     *
     *  @param paint        describes how to color pixels.
     *  @param viewMatrix   transformation matrix
     *  @param outer        the outer roundrect
     *  @param inner        the inner roundrect
     */
    void drawDRRect(GrRenderTarget*,
                    const GrClip&,
                    const GrPaint&,
                    const SkMatrix& viewMatrix,
                    const SkRRect& outer,
                    const SkRRect& inner);


    /**
     * Draws a path.
     *
     * @param paint         describes how to color pixels.
     * @param viewMatrix    transformation matrix
     * @param path          the path to draw
     * @param strokeInfo    the stroke information (width, join, cap) and
     *                      the dash information (intervals, count, phase).
     */
    void drawPath(GrRenderTarget*,
                  const GrClip&,
                  const GrPaint&,
                  const SkMatrix& viewMatrix,
                  const SkPath&,
                  const GrStrokeInfo&);

    /**
     * Draws vertices with a paint.
     *
     * @param   paint           describes how to color pixels.
     * @param   viewMatrix      transformation matrix
     * @param   primitiveType   primitives type to draw.
     * @param   vertexCount     number of vertices.
     * @param   positions       array of vertex positions, required.
     * @param   texCoords       optional array of texture coordinates used
     *                          to access the paint.
     * @param   colors          optional array of per-vertex colors, supercedes
     *                          the paint's color field.
     * @param   indices         optional array of indices. If NULL vertices
     *                          are drawn non-indexed.
     * @param   indexCount      if indices is non-null then this is the
     *                          number of indices.
     */
    void drawVertices(GrRenderTarget*,
                      const GrClip&,
                      const GrPaint& paint,
                      const SkMatrix& viewMatrix,
                      GrPrimitiveType primitiveType,
                      int vertexCount,
                      const SkPoint positions[],
                      const SkPoint texs[],
                      const GrColor colors[],
                      const uint16_t indices[],
                      int indexCount);

    /**
     * Draws textured sprites from an atlas with a paint.
     *
     * @param   paint           describes how to color pixels.
     * @param   viewMatrix      transformation matrix
     * @param   spriteCount     number of sprites.
     * @param   xform           array of compressed transformation data, required.
     * @param   texRect         array of texture rectangles used to access the paint.
     * @param   colors          optional array of per-sprite colors, supercedes
     *                          the paint's color field.
     */
    void drawAtlas(GrRenderTarget*,
                   const GrClip&,
                   const GrPaint& paint,
                   const SkMatrix& viewMatrix,
                   int spriteCount,
                   const SkRSXform xform[],
                   const SkRect texRect[],
                   const SkColor colors[]);
    
    /**
     * Draws an oval.
     *
     * @param paint         describes how to color pixels.
     * @param viewMatrix    transformation matrix
     * @param oval          the bounding rect of the oval.
     * @param strokeInfo    the stroke information (width, join, cap) and
     *                      the dash information (intervals, count, phase).
     */
    void drawOval(GrRenderTarget*,
                  const GrClip&,
                  const GrPaint& paint,
                  const SkMatrix& viewMatrix,
                  const SkRect& oval,
                  const GrStrokeInfo& strokeInfo);


private:
    friend class GrAtlasTextContext; // for access to drawBatch
    friend class GrContext; // for ctor

    GrDrawContext(GrContext*, GrDrawTarget*, const SkSurfaceProps&);

    GrTextContext* createTextContext(GrRenderTarget*, const SkSurfaceProps&);

    // Checks if the context has been abandoned and if the rendertarget is owned by this context
    bool prepareToDraw(GrRenderTarget* rt);

    void internalDrawPath(GrDrawTarget*,
                          GrPipelineBuilder*,
                          const SkMatrix& viewMatrix,
                          GrColor,
                          bool useAA,
                          const SkPath&,
                          const GrStrokeInfo&);

    // This entry point allows the GrTextContext-derived classes to add their batches to
    // the drawTarget.
    void drawBatch(GrPipelineBuilder* pipelineBuilder, GrBatch* batch);

    GrContext*          fContext;     // owning context -> no ref
    GrDrawTarget*       fDrawTarget;
    GrTextContext*      fTextContext; // lazily created

    SkSurfaceProps      fSurfaceProps;
};

#endif

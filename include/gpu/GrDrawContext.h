/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawContext_DEFINED
#define GrDrawContext_DEFINED

#include "GrColor.h"
#include "GrRenderTarget.h"
#include "SkRefCnt.h"
#include "SkSurfaceProps.h"

class GrClip;
class GrContext;
class GrDrawBatch;
class GrDrawingManager;
class GrDrawTarget;
class GrPaint;
class GrPathProcessor;
class GrPathRange;
class GrPathRangeDraw;
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

    void copySurface(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint);

    // TODO: it is odd that we need both the SkPaint in the following 3 methods.
    // We should extract the text parameters from SkPaint and pass them separately
    // akin to GrStrokeInfo (GrTextInfo?)
    void drawText(const GrClip&,  const GrPaint&, const SkPaint&,
                  const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                  SkScalar x, SkScalar y, const SkIRect& clipBounds);
    void drawPosText(const GrClip&, const GrPaint&, const SkPaint&,
                     const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                     const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& clipBounds);
    void drawTextBlob(const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkTextBlob*,
                      SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds);

    // drawPathsFromRange is thanks to GrStencilAndCoverTextContext
    // TODO: remove once path batches can be created external to GrDrawTarget.
    void drawPathsFromRange(const GrPipelineBuilder*,
                            const SkMatrix& viewMatrix,
                            const SkMatrix& localMatrix,
                            GrColor color,
                            GrPathRange* range,
                            GrPathRangeDraw* draw,
                            int /*GrPathRendering::FillType*/ fill,
                            const SkRect& bounds);

    /**
     * Provides a perfomance hint that the render target's contents are allowed
     * to become undefined.
     */
    void discard();

    /**
     * Clear the entire or rect of the render target, ignoring any clips.
     * @param rect  the rect to clear or the whole thing if rect is NULL.
     * @param color the color to clear to.
     * @param canIgnoreRect allows partial clears to be converted to whole
     *                      clears on platforms for which that is cheap
     */
    void clear(const SkIRect* rect, GrColor color, bool canIgnoreRect);

    /**
     *  Draw everywhere (respecting the clip) with the paint.
     */
    void drawPaint(const GrClip&, const GrPaint&, const SkMatrix& viewMatrix);

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
    void drawRect(const GrClip&,
                  const GrPaint& paint,
                  const SkMatrix& viewMatrix,
                  const SkRect&,
                  const GrStrokeInfo* strokeInfo = NULL);

    /**
     * Maps a rectangle of shader coordinates to a rectangle and fills that rectangle.
     *
     * @param paint         describes how to color pixels.
     * @param viewMatrix    transformation matrix which applies to rectToDraw
     * @param rectToDraw    the rectangle to draw
     * @param localRect     the rectangle of shader coordinates applied to rectToDraw
     */
    void fillRectToRect(const GrClip&,
                        const GrPaint& paint,
                        const SkMatrix& viewMatrix,
                        const SkRect& rectToDraw,
                        const SkRect& localRect);

    /**
     * Fills a rect with a paint and a localMatrix.
     */
    void fillRectWithLocalMatrix(const GrClip& clip,
                                 const GrPaint& paint,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkMatrix& localMatrix);

    /**
     *  Draw a roundrect using a paint.
     *
     *  @param paint        describes how to color pixels.
     *  @param viewMatrix   transformation matrix
     *  @param rrect        the roundrect to draw
     *  @param strokeInfo   the stroke information (width, join, cap) and
     *                      the dash information (intervals, count, phase).
     */
    void drawRRect(const GrClip&,
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
    void drawDRRect(const GrClip&,
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
    void drawPath(const GrClip&,
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
    void drawVertices(const GrClip&,
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
    void drawAtlas(const GrClip&,
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
    void drawOval(const GrClip&,
                  const GrPaint& paint,
                  const SkMatrix& viewMatrix,
                  const SkRect& oval,
                  const GrStrokeInfo& strokeInfo);

    /**
     *  Draw the image stretched differentially to fit into dst.
     *  center is a rect within the image, and logically divides the image
     *  into 9 sections (3x3). For example, if the middle pixel of a [5x5]
     *  image is the "center", then the center-rect should be [2, 2, 3, 3].
     *
     *  If the dst is >= the image size, then...
     *  - The 4 corners are not stretched at all.
     *  - The sides are stretched in only one axis.
     *  - The center is stretched in both axes.
     * Else, for each axis where dst < image,
     *  - The corners shrink proportionally
     *  - The sides (along the shrink axis) and center are not drawn
     */
    void drawImageNine(const GrClip&,
                       const GrPaint& paint,
                       const SkMatrix& viewMatrix,
                       int imageWidth,
                       int imageHeight,
                       const SkIRect& center,
                       const SkRect& dst);

    /**
     * Draws a batch
     *
     * @param paint    describes how to color pixels.
     * @param batch    the batch to draw
     */
    void drawBatch(const GrClip&, const GrPaint&, GrDrawBatch*);

    int width() const { return fRenderTarget->width(); }
    int height() const { return fRenderTarget->height(); }
    int numColorSamples() const { return fRenderTarget->numColorSamples(); }

private:
    friend class GrAtlasTextContext; // for access to drawBatch
    friend class GrDrawingManager; // for ctor

    SkDEBUGCODE(void validate() const;)

    GrDrawContext(GrDrawingManager*, GrRenderTarget*, const SkSurfaceProps* surfaceProps);

    void internalDrawPath(GrPipelineBuilder*,
                          const SkMatrix& viewMatrix,
                          GrColor,
                          bool useAA,
                          const SkPath&,
                          const GrStrokeInfo&);

    // This entry point allows the GrTextContext-derived classes to add their batches to
    // the drawTarget.
    void drawBatch(GrPipelineBuilder* pipelineBuilder, GrDrawBatch* batch);

    GrDrawTarget* getDrawTarget();

    GrDrawingManager* fDrawingManager;
    GrRenderTarget*   fRenderTarget;

    // In MDB-mode the drawTarget can be closed by some other drawContext that has picked
    // it up. For this reason, the drawTarget should only ever be accessed via 'getDrawTarget'.
    GrDrawTarget*     fDrawTarget;
    GrTextContext*    fTextContext; // lazily gotten from GrContext::DrawingManager

    SkSurfaceProps    fSurfaceProps;
};

#endif

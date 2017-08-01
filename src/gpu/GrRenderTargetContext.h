/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetContext_DEFINED
#define GrRenderTargetContext_DEFINED

#include "../private/GrInstancedPipelineInfo.h"
#include "../private/GrRenderTargetProxy.h"
#include "GrColor.h"
#include "GrContext.h"
#include "GrPaint.h"
#include "GrSurfaceContext.h"
#include "GrTypesPriv.h"
#include "GrXferProcessor.h"
#include "SkRefCnt.h"
#include "SkSurfaceProps.h"

class GrBackendSemaphore;
class GrCCPRAtlas;
class GrClip;
class GrCoverageCountingPathRenderer;
class GrDrawingManager;
class GrDrawOp;
class GrFixedClip;
class GrRenderTarget;
class GrRenderTargetContextPriv;
class GrRenderTargetOpList;
class GrStyle;
class GrTextureProxy;
struct GrUserStencilSettings;
class SkDrawFilter;
struct SkDrawShadowRec;
struct SkIPoint;
struct SkIRect;
class SkLatticeIter;
class SkMatrix;
class SkPaint;
class SkPath;
struct SkPoint;
struct SkRect;
class SkRegion;
class SkRRect;
struct SkRSXform;
class SkTextBlob;
class SkVertices;

/**
 * A helper object to orchestrate commands (draws, etc...) for GrSurfaces that are GrRenderTargets.
 */
class SK_API GrRenderTargetContext : public GrSurfaceContext {
public:
    ~GrRenderTargetContext() override;

    // We use SkPaint rather than GrPaint here for two reasons:
    //    * The SkPaint carries extra text settings. If these were extracted to a lighter object
    //      we could use GrPaint except that
    //    * SkPaint->GrPaint conversion depends upon whether the glyphs are color or grayscale and
    //      this can vary within a text run.
    virtual void drawText(const GrClip&, const SkPaint&, const SkMatrix& viewMatrix,
                          const char text[], size_t byteLength, SkScalar x, SkScalar y,
                          const SkIRect& clipBounds);
    virtual void drawPosText(const GrClip&, const SkPaint&, const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength, const SkScalar pos[],
                             int scalarsPerPosition, const SkPoint& offset,
                             const SkIRect& clipBounds);
    virtual void drawTextBlob(const GrClip&, const SkPaint&,
                              const SkMatrix& viewMatrix, const SkTextBlob*,
                              SkScalar x, SkScalar y,
                              SkDrawFilter*, const SkIRect& clipBounds);

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
    void drawPaint(const GrClip&, GrPaint&&, const SkMatrix& viewMatrix);

    /**
     * Draw the rect using a paint.
     * @param paint        describes how to color pixels.
     * @param GrAA         Controls whether rect is antialiased
     * @param viewMatrix   transformation matrix
     * @param style        The style to apply. Null means fill. Currently path effects are not
     *                     allowed.
     * The rects coords are used to access the paint (through texture matrix)
     */
    void drawRect(const GrClip&,
                  GrPaint&& paint,
                  GrAA,
                  const SkMatrix& viewMatrix,
                  const SkRect&,
                  const GrStyle* style = nullptr);

    /**
     * Maps a rectangle of shader coordinates to a rectangle and fills that rectangle.
     *
     * @param paint        describes how to color pixels.
     * @param GrAA         Controls whether rect is antialiased
     * @param viewMatrix   transformation matrix which applies to rectToDraw
     * @param rectToDraw   the rectangle to draw
     * @param localRect    the rectangle of shader coordinates applied to rectToDraw
     */
    void fillRectToRect(const GrClip&,
                        GrPaint&& paint,
                        GrAA,
                        const SkMatrix& viewMatrix,
                        const SkRect& rectToDraw,
                        const SkRect& localRect);

    /**
     * Fills a rect with a paint and a localMatrix.
     */
    void fillRectWithLocalMatrix(const GrClip& clip,
                                 GrPaint&& paint,
                                 GrAA,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkMatrix& localMatrix);

    /**
     * Draw a roundrect using a paint.
     *
     * @param paint       describes how to color pixels.
     * @param GrAA        Controls whether rrect is antialiased.
     * @param viewMatrix  transformation matrix
     * @param rrect       the roundrect to draw
     * @param style       style to apply to the rrect. Currently path effects are not allowed.
     */
    void drawRRect(const GrClip&,
                   GrPaint&&,
                   GrAA,
                   const SkMatrix& viewMatrix,
                   const SkRRect& rrect,
                   const GrStyle& style);

    /**
     * Use a fast method to render the ambient and spot shadows for a path.
     * Will return false if not possible for the given path.
     *
     * @param color        shadow color.
     * @param viewMatrix   transformation matrix
     * @param path         the path to shadow
     * @param rec          parameters for shadow rendering
     */
    bool drawFastShadow(const GrClip&,
                        GrColor color,
                        const SkMatrix& viewMatrix,
                        const SkPath& path,
                        const SkDrawShadowRec& rec);

    /**
     * Shortcut for filling a SkPath consisting of nested rrects using a paint. The result is
     * undefined if outer does not contain inner.
     *
     * @param paint        describes how to color pixels.
     * @param GrAA         Controls whether rrects edges are antialiased
     * @param viewMatrix   transformation matrix
     * @param outer        the outer roundrect
     * @param inner        the inner roundrect
     */
    void drawDRRect(const GrClip&,
                    GrPaint&&,
                    GrAA,
                    const SkMatrix& viewMatrix,
                    const SkRRect& outer,
                    const SkRRect& inner);

    /**
     * Draws a path.
     *
     * @param paint         describes how to color pixels.
     * @param GrAA          Controls whether the path is antialiased.
     * @param viewMatrix    transformation matrix
     * @param path          the path to draw
     * @param style         style to apply to the path.
     */
    void drawPath(const GrClip&,
                  GrPaint&&,
                  GrAA,
                  const SkMatrix& viewMatrix,
                  const SkPath&,
                  const GrStyle& style);

    /**
     * Draws vertices with a paint.
     *
     * @param   paint            describes how to color pixels.
     * @param   viewMatrix       transformation matrix
     * @param   vertices         specifies the mesh to draw.
     * @param   overridePrimType primitive type to draw. If NULL, derive prim type from vertices.
     */
    void drawVertices(const GrClip&,
                      GrPaint&& paint,
                      const SkMatrix& viewMatrix,
                      sk_sp<SkVertices> vertices,
                      GrPrimitiveType* overridePrimType = nullptr);

    /**
     * Draws textured sprites from an atlas with a paint. This currently does not support AA for the
     * sprite rectangle edges.
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
                   GrPaint&& paint,
                   const SkMatrix& viewMatrix,
                   int spriteCount,
                   const SkRSXform xform[],
                   const SkRect texRect[],
                   const SkColor colors[]);

    /**
     * Draws a region.
     *
     * @param paint         describes how to color pixels
     * @param viewMatrix    transformation matrix
     * @param aa            should the rects of the region be antialiased.
     * @param region        the region to be drawn
     * @param style         style to apply to the region
     */
    void drawRegion(const GrClip&,
                    GrPaint&& paint,
                    GrAA aa,
                    const SkMatrix& viewMatrix,
                    const SkRegion& region,
                    const GrStyle& style);

    /**
     * Draws an oval.
     *
     * @param paint         describes how to color pixels.
     * @param GrAA          Controls whether the oval is antialiased.
     * @param viewMatrix    transformation matrix
     * @param oval          the bounding rect of the oval.
     * @param style         style to apply to the oval. Currently path effects are not allowed.
     */
    void drawOval(const GrClip&,
                  GrPaint&& paint,
                  GrAA,
                  const SkMatrix& viewMatrix,
                  const SkRect& oval,
                  const GrStyle& style);
    /**
     * Draws a partial arc of an oval.
     *
     * @param paint         describes how to color pixels.
     * @param GrGrAA        Controls whether the arc is antialiased.
     * @param viewMatrix    transformation matrix.
     * @param oval          the bounding rect of the oval.
     * @param startAngle    starting angle in degrees.
     * @param sweepAngle    angle to sweep in degrees. Must be in (-360, 360)
     * @param useCenter     true means that the implied path begins at the oval center, connects as
     *                      a line to the point indicated by the start contains the arc indicated by
     *                      the sweep angle. If false the line beginning at the center point is
     *                      omitted.
     * @param style         style to apply to the oval.
     */
    void drawArc(const GrClip&,
                 GrPaint&& paint,
                 GrAA,
                 const SkMatrix& viewMatrix,
                 const SkRect& oval,
                 SkScalar startAngle,
                 SkScalar sweepAngle,
                 bool useCenter,
                 const GrStyle& style);

    /**
     * Draw the image as a set of rects, specified by |iter|.
     */
    void drawImageLattice(const GrClip&,
                          GrPaint&& paint,
                          const SkMatrix& viewMatrix,
                          int imageWidth,
                          int imageHeight,
                          std::unique_ptr<SkLatticeIter> iter,
                          const SkRect& dst);

    /**
     * After this returns any pending surface IO will be issued to the backend 3D API and
     * if the surface has MSAA it will be resolved.
     */
    bool prepareForExternalIO(int numSemaphores, GrBackendSemaphore* backendSemaphores);

    /**
     *  The next time this GrRenderTargetContext is flushed, the gpu will wait on the passed in
     *  semaphores before executing any commands.
     */
    bool waitOnSemaphores(int numSemaphores, const GrBackendSemaphore* waitSemaphores);

    GrFSAAType fsaaType() const { return fRenderTargetProxy->fsaaType(); }
    const GrCaps* caps() const { return fContext->caps(); }
    int width() const { return fRenderTargetProxy->width(); }
    int height() const { return fRenderTargetProxy->height(); }
    GrPixelConfig config() const { return fRenderTargetProxy->config(); }
    int numColorSamples() const { return fRenderTargetProxy->numColorSamples(); }
    int numStencilSamples() const { return fRenderTargetProxy->numStencilSamples(); }
    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }
    GrColorSpaceXform* getColorXformFromSRGB() const { return fColorXformFromSRGB.get(); }
    GrSurfaceOrigin origin() const { return fRenderTargetProxy->origin(); }

    bool wasAbandoned() const;

    GrRenderTarget* accessRenderTarget() {
        // TODO: usage of this entry point needs to be reduced and potentially eliminated
        // since it ends the deferral of the GrRenderTarget's allocation
        if (!fRenderTargetProxy->instantiate(fContext->resourceProvider())) {
            return nullptr;
        }
        return fRenderTargetProxy->priv().peekRenderTarget();
    }

    GrSurfaceProxy* asSurfaceProxy() override { return fRenderTargetProxy.get(); }
    const GrSurfaceProxy* asSurfaceProxy() const override { return fRenderTargetProxy.get(); }
    sk_sp<GrSurfaceProxy> asSurfaceProxyRef() override { return fRenderTargetProxy; }

    GrTextureProxy* asTextureProxy() override;
    sk_sp<GrTextureProxy> asTextureProxyRef() override;

    GrRenderTargetProxy* asRenderTargetProxy() override { return fRenderTargetProxy.get(); }
    sk_sp<GrRenderTargetProxy> asRenderTargetProxyRef() override { return fRenderTargetProxy; }

    GrRenderTargetContext* asRenderTargetContext() override { return this; }

    // Provides access to functions that aren't part of the public API.
    GrRenderTargetContextPriv priv();
    const GrRenderTargetContextPriv priv() const;

    bool isWrapped_ForTesting() const;

protected:
    GrRenderTargetContext(GrContext*, GrDrawingManager*, sk_sp<GrRenderTargetProxy>,
                          sk_sp<SkColorSpace>, const SkSurfaceProps*, GrAuditTrail*,
                          GrSingleOwner*, bool managedOpList = true);

    SkDEBUGCODE(void validate() const override;)

private:
    inline GrAAType chooseAAType(GrAA aa, GrAllowMixedSamples allowMixedSamples) {
        return GrChooseAAType(aa, this->fsaaType(), allowMixedSamples, *this->caps());
    }

    friend class GrAtlasTextBlob;               // for access to add[Mesh]DrawOp
    friend class GrStencilAndCoverTextContext;  // for access to add[Mesh]DrawOp

    friend class GrDrawingManager; // for ctor
    friend class GrRenderTargetContextPriv;
    friend class GrSWMaskHelper;  // for access to add[Mesh]DrawOp

    // All the path renderers currently make their own ops
    friend class GrSoftwarePathRenderer;             // for access to add[Mesh]DrawOp
    friend class GrAAConvexPathRenderer;             // for access to add[Mesh]DrawOp
    friend class GrDashLinePathRenderer;             // for access to add[Mesh]DrawOp
    friend class GrAAHairLinePathRenderer;           // for access to add[Mesh]DrawOp
    friend class GrAALinearizingConvexPathRenderer;  // for access to add[Mesh]DrawOp
    friend class GrSmallPathRenderer;                // for access to add[Mesh]DrawOp
    friend class GrDefaultPathRenderer;              // for access to add[Mesh]DrawOp
    friend class GrMSAAPathRenderer;                 // for access to add[Mesh]DrawOp
    friend class GrStencilAndCoverPathRenderer;      // for access to add[Mesh]DrawOp
    friend class GrTessellatingPathRenderer;         // for access to add[Mesh]DrawOp
    friend class GrCCPRAtlas;                        // for access to addDrawOp
    friend class GrCoverageCountingPathRenderer;     // for access to addDrawOp
    // for a unit test
    friend void test_draw_op(GrRenderTargetContext*,
                             sk_sp<GrFragmentProcessor>, sk_sp<GrTextureProxy>);

    void internalClear(const GrFixedClip&, const GrColor, bool canIgnoreClip);

    // Only consumes the GrPaint if successful.
    bool drawFilledDRRect(const GrClip& clip,
                          GrPaint&& paint,
                          GrAA,
                          const SkMatrix& viewMatrix,
                          const SkRRect& origOuter,
                          const SkRRect& origInner);

    // Only consumes the GrPaint if successful.
    bool drawFilledRect(const GrClip& clip,
                        GrPaint&& paint,
                        GrAA,
                        const SkMatrix& viewMatrix,
                        const SkRect& rect,
                        const GrUserStencilSettings* ss);

    void internalDrawPath(
            const GrClip&, GrPaint&&, GrAA, const SkMatrix&, const SkPath&, const GrStyle&);

    // These perform processing specific to Gr[Mesh]DrawOp-derived ops before recording them into
    // the op list. They return the id of the opList to which the op was added, or 0, if it was
    // dropped (e.g., due to clipping).
    uint32_t addDrawOp(const GrClip&, std::unique_ptr<GrDrawOp>);

    // Makes a copy of the proxy if it is necessary for the draw and places the texture that should
    // be used by GrXferProcessor to access the destination color in 'result'. If the return
    // value is false then a texture copy could not be made.
    bool SK_WARN_UNUSED_RESULT setupDstProxy(GrRenderTargetProxy*,
                                             const GrClip&,
                                             const SkRect& opBounds,
                                             GrXferProcessor::DstProxy* result);

    GrRenderTargetOpList* getRTOpList();
    GrOpList* getOpList() override;

    sk_sp<GrRenderTargetProxy>        fRenderTargetProxy;

    // In MDB-mode the GrOpList can be closed by some other renderTargetContext that has picked
    // it up. For this reason, the GrOpList should only ever be accessed via 'getOpList'.
    sk_sp<GrRenderTargetOpList>       fOpList;
    GrInstancedPipelineInfo           fInstancedPipelineInfo;

    sk_sp<GrColorSpaceXform>          fColorXformFromSRGB;
    SkSurfaceProps                    fSurfaceProps;
    bool                              fManagedOpList;

    typedef GrSurfaceContext INHERITED;
};

#endif

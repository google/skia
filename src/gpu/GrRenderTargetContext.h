/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetContext_DEFINED
#define GrRenderTargetContext_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/text/GrTextTarget.h"

class GrBackendSemaphore;
class GrClip;
class GrColorSpaceXform;
class GrCoverageCountingPathRenderer;
class GrDrawingManager;
class GrDrawOp;
class GrFixedClip;
class GrOp;
class GrRenderTarget;
class GrRenderTargetContextPriv;
class GrRenderTargetOpList;
class GrShape;
class GrStyle;
class GrTextureProxy;
struct GrUserStencilSettings;
struct SkDrawShadowRec;
class SkGlyphRunList;
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

    virtual void drawGlyphRunList(const GrClip&, const SkMatrix& viewMatrix, const SkGlyphRunList&);

    /**
     * Provides a perfomance hint that the render target's contents are allowed
     * to become undefined.
     */
    void discard();

    enum class CanClearFullscreen : bool {
        kNo = false,
        kYes = true
    };

    /**
     * Clear the entire or rect of the render target, ignoring any clips.
     * @param rect  the rect to clear or the whole thing if rect is NULL.
     * @param color the color to clear to.
     * @param CanClearFullscreen allows partial clears to be converted to fullscreen clears on
     *                           tiling platforms where that is an optimization.
     */
    void clear(const SkIRect* rect, const SkPMColor4f& color, CanClearFullscreen);

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
    void fillRectToRect(const GrClip& clip,
                        GrPaint&& paint,
                        GrAA aa,
                        const SkMatrix& viewMatrix,
                        const SkRect& rectToDraw,
                        const SkRect& localRect) {
        this->fillRectWithEdgeAA(clip, std::move(paint), aa,
                                 aa == GrAA::kYes ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone,
                                 viewMatrix, rectToDraw, &localRect);
    }

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
     * Creates an op that draws a fill rect with per-edge control over anti-aliasing.
     *
     * This is a specialized version of fillQuadWithEdgeAA, but is kept separate since knowing
     * the geometry is a rectangle affords more optimizations.
     */
    void fillRectWithEdgeAA(const GrClip& clip, GrPaint&& paint, GrAA aa, GrQuadAAFlags edgeAA,
                            const SkMatrix& viewMatrix, const SkRect& rect,
                            const SkRect* optionalLocalRect = nullptr);

    /**
     * Similar to fillRectWithEdgeAA but draws an arbitrary 2D convex quadrilateral transformed
     * by 'viewMatrix', with per-edge control over anti-aliasing. The quad should follow the
     * ordering used by SkRect::toQuad(), which determines how the edge AA is applied:
     *  - "top" = points [0] and [1]
     *  - "right" = points[1] and [2]
     *  - "bottom" = points[2] and [3]
     *  - "left" = points[3] and [0]
     *
     * The last argument, 'optionalLocalQuad', can be null if no separate local coordinates are
     * necessary.
     */
    void fillQuadWithEdgeAA(const GrClip& clip, GrPaint&& paint, GrAA aa, GrQuadAAFlags edgeAA,
                            const SkMatrix& viewMatrix, const SkPoint quad[4],
                            const SkPoint optionalLocalQuad[4]);

    /** Used with drawQuadSet */
    struct QuadSetEntry {
        SkRect fRect;
        SkPMColor4f fColor; // Overrides any color on the GrPaint
        SkMatrix fLocalMatrix;
        GrQuadAAFlags fAAFlags;
    };

    // TODO(michaelludwig) - remove if the bulk API is not useful for SkiaRenderer
    void drawQuadSet(const GrClip& clip, GrPaint&& paint, GrAA aa, const SkMatrix& viewMatrix,
                     const QuadSetEntry[], int cnt);

    /**
     * Creates an op that draws a subrectangle of a texture. The passed color is modulated by the
     * texture's color. 'srcRect' specifies the rectangle of the texture to draw. 'dstRect'
     * specifies the rectangle to draw in local coords which will be transformed by 'viewMatrix' to
     * device space.
     */
    void drawTexture(const GrClip& clip, sk_sp<GrTextureProxy>, GrSamplerState::Filter,
                     SkBlendMode mode, const SkPMColor4f&, const SkRect& srcRect,
                     const SkRect& dstRect, GrAA, GrQuadAAFlags, SkCanvas::SrcRectConstraint,
                     const SkMatrix& viewMatrix, sk_sp<GrColorSpaceXform> texXform);

    /**
     * Variant of drawTexture that instead draws the texture applied to 'dstQuad' transformed by
     * 'viewMatrix', using the 'srcQuad' texture coordinates clamped to the optional 'domain'. If
     * 'domain' is null, it's equivalent to using the fast src rect constraint. If 'domain' is
     * provided, the strict src rect constraint is applied using 'domain'.
     */
    void drawTextureQuad(const GrClip& clip, sk_sp<GrTextureProxy>, GrSamplerState::Filter,
                         SkBlendMode mode, const SkPMColor4f&, const SkPoint srcQuad[4],
                         const SkPoint dstQuad[4], GrAA, GrQuadAAFlags, const SkRect* domain,
                         const SkMatrix& viewMatrix, sk_sp<GrColorSpaceXform> texXform);

    /** Used with drawTextureSet */
    struct TextureSetEntry {
        sk_sp<GrTextureProxy> fProxy;
        SkRect fSrcRect;
        SkRect fDstRect;
        const SkPoint* fDstClipQuad; // Must be null, or point to an array of 4 points
        const SkMatrix* fPreViewMatrix; // If not null, entry's CTM is 'viewMatrix' * fPreViewMatrix
        float fAlpha;
        GrQuadAAFlags fAAFlags;
    };
    /**
     * Draws a set of textures with a shared filter, color, view matrix, color xform, and
     * texture color xform. The textures must all have the same GrTextureType and GrConfig.
     *
     * If any entries provide a non-null fDstClip array, it will be read from immediately based on
     * fDstClipCount, so the pointer can become invalid after this returns.
     */
    void drawTextureSet(const GrClip&, const TextureSetEntry[], int cnt, GrSamplerState::Filter,
                        SkBlendMode mode, GrAA aa, SkCanvas::SrcRectConstraint,
                        const SkMatrix& viewMatrix, sk_sp<GrColorSpaceXform> texXform);

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
     * @param viewMatrix   transformation matrix
     * @param path         the path to shadow
     * @param rec          parameters for shadow rendering
     */
    bool drawFastShadow(const GrClip&,
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
                  const GrStyle&);

    /**
     * Draws a shape.
     *
     * @param paint         describes how to color pixels.
     * @param GrAA          Controls whether the path is antialiased.
     * @param viewMatrix    transformation matrix
     * @param shape         the shape to draw
     */
    void drawShape(const GrClip&,
                   GrPaint&&,
                   GrAA,
                   const SkMatrix& viewMatrix,
                   const GrShape&);


    /**
     * Draws vertices with a paint.
     *
     * @param   paint            describes how to color pixels.
     * @param   viewMatrix       transformation matrix
     * @param   vertices         specifies the mesh to draw.
     * @param   bones            bone deformation matrices.
     * @param   boneCount        number of bone matrices.
     * @param   overridePrimType primitive type to draw. If NULL, derive prim type from vertices.
     */
    void drawVertices(const GrClip&,
                      GrPaint&& paint,
                      const SkMatrix& viewMatrix,
                      sk_sp<SkVertices> vertices,
                      const SkVertices::Bone bones[],
                      int boneCount,
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
                    const GrStyle& style,
                    const GrUserStencilSettings* ss = nullptr);

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
                          GrPaint&&,
                          const SkMatrix& viewMatrix,
                          sk_sp<GrTextureProxy>,
                          sk_sp<GrColorSpaceXform>,
                          GrSamplerState::Filter,
                          std::unique_ptr<SkLatticeIter>,
                          const SkRect& dst);

    /**
     * Draws the src texture with no matrix. The dstRect is the dstPoint with the width and height
     * of the srcRect. The srcRect and dstRect are clipped to the bounds of the src and dst surfaces
     * respectively.
     */
    bool blitTexture(GrTextureProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint);

    /**
     * Adds the necessary signal and wait semaphores and adds the passed in SkDrawable to the
     * command stream.
     */
    void drawDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>, const SkRect& bounds);

    using ReadPixelsCallback = SkSurface::ReadPixelsCallback;
    using ReadPixelsCallbackYUV420 = SkSurface::ReadPixelsCallbackYUV420;
    using ReadPixelsContext = SkSurface::ReadPixelsContext;
    using RescaleGamma = SkSurface::RescaleGamma;

    // GPU implementation for SkSurface::asyncRescaleAndReadPixels.
    void asyncRescaleAndReadPixels(const SkImageInfo& info, const SkIRect& srcRect,
                                   RescaleGamma rescaleGamma, SkFilterQuality rescaleQuality,
                                   ReadPixelsCallback callback, ReadPixelsContext context);
    // GPU implementation for SkSurface::asyncRescaleAndReadPixelsYUV420.
    void asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                         sk_sp<SkColorSpace> dstColorSpace, const SkIRect& srcRect,
                                         int dstW, int dstH, RescaleGamma rescaleGamma,
                                         SkFilterQuality rescaleQuality,
                                         ReadPixelsCallbackYUV420 callback,
                                         ReadPixelsContext context);

    /**
     * After this returns any pending surface IO will be issued to the backend 3D API and
     * if the surface has MSAA it will be resolved.
     */
    GrSemaphoresSubmitted flush(SkSurface::BackendSurfaceAccess access, const GrFlushInfo&);

    /**
     *  The next time this GrRenderTargetContext is flushed, the gpu will wait on the passed in
     *  semaphores before executing any commands.
     */
    bool waitOnSemaphores(int numSemaphores, const GrBackendSemaphore waitSemaphores[]);

    void insertEventMarker(const SkString&);

    GrFSAAType fsaaType() const { return fRenderTargetProxy->fsaaType(); }
    const GrCaps* caps() const;
    int width() const { return fRenderTargetProxy->width(); }
    int height() const { return fRenderTargetProxy->height(); }
    int numColorSamples() const { return fRenderTargetProxy->numColorSamples(); }
    int numStencilSamples() const { return fRenderTargetProxy->numStencilSamples(); }
    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }
    GrSurfaceOrigin origin() const { return fRenderTargetProxy->origin(); }
    bool wrapsVkSecondaryCB() const { return fRenderTargetProxy->wrapsVkSecondaryCB(); }
    GrMipMapped mipMapped() const;

    void setNeedsStencil() { fRenderTargetProxy->setNeedsStencil(); }

    // This entry point should only be called if the backing GPU object is known to be
    // instantiated.
    GrRenderTarget* accessRenderTarget() { return fRenderTargetProxy->peekRenderTarget(); }

    GrSurfaceProxy* asSurfaceProxy() override { return fRenderTargetProxy.get(); }
    const GrSurfaceProxy* asSurfaceProxy() const override { return fRenderTargetProxy.get(); }
    sk_sp<GrSurfaceProxy> asSurfaceProxyRef() override { return fRenderTargetProxy; }

    GrTextureProxy* asTextureProxy() override;
    const GrTextureProxy* asTextureProxy() const override;
    sk_sp<GrTextureProxy> asTextureProxyRef() override;

    GrRenderTargetProxy* asRenderTargetProxy() override { return fRenderTargetProxy.get(); }
    sk_sp<GrRenderTargetProxy> asRenderTargetProxyRef() override { return fRenderTargetProxy; }

    GrRenderTargetContext* asRenderTargetContext() override { return this; }

    // Provides access to functions that aren't part of the public API.
    GrRenderTargetContextPriv priv();
    const GrRenderTargetContextPriv priv() const;

    GrTextTarget* textTarget() { return fTextTarget.get(); }

    bool isWrapped_ForTesting() const;

protected:
    GrRenderTargetContext(GrRecordingContext*, sk_sp<GrRenderTargetProxy>,
                          sk_sp<SkColorSpace>, const SkSurfaceProps*,
                          bool managedOpList = true);

    SkDEBUGCODE(void validate() const override;)

private:
    class TextTarget;

    GrAAType chooseAAType(GrAA);

    friend class GrAtlasTextBlob;               // for access to add[Mesh]DrawOp
    friend class GrClipStackClip;               // for access to getOpList

    friend class GrDrawingManager; // for ctor
    friend class GrRenderTargetContextPriv;

    // All the path renderers currently make their own ops
    friend class GrSoftwarePathRenderer;             // for access to add[Mesh]DrawOp
    friend class GrAAConvexPathRenderer;             // for access to add[Mesh]DrawOp
    friend class GrDashLinePathRenderer;             // for access to add[Mesh]DrawOp
    friend class GrAAHairLinePathRenderer;           // for access to add[Mesh]DrawOp
    friend class GrAALinearizingConvexPathRenderer;  // for access to add[Mesh]DrawOp
    friend class GrSmallPathRenderer;                // for access to add[Mesh]DrawOp
    friend class GrDefaultPathRenderer;              // for access to add[Mesh]DrawOp
    friend class GrStencilAndCoverPathRenderer;      // for access to add[Mesh]DrawOp
    friend class GrTessellatingPathRenderer;         // for access to add[Mesh]DrawOp
    friend class GrCCPerFlushResources;              // for access to addDrawOp
    friend class GrCoverageCountingPathRenderer;     // for access to addDrawOp
    // for a unit test
    friend void test_draw_op(GrContext*,
                             GrRenderTargetContext*,
                             std::unique_ptr<GrFragmentProcessor>,
                             sk_sp<GrTextureProxy>);

    void internalClear(const GrFixedClip&, const SkPMColor4f&, CanClearFullscreen);
    void internalStencilClear(const GrFixedClip&, bool insideStencilMask);

    // Only consumes the GrPaint if successful.
    bool drawFilledDRRect(const GrClip& clip,
                          GrPaint&& paint,
                          GrAA,
                          const SkMatrix& viewMatrix,
                          const SkRRect& origOuter,
                          const SkRRect& origInner);

    void drawFilledRect(const GrClip& clip,
                        GrPaint&& paint,
                        GrAA,
                        const SkMatrix& viewMatrix,
                        const SkRect& rect,
                        const GrUserStencilSettings* ss = nullptr);

    // Only consumes the GrPaint if successful.
    bool drawFilledRectAsClear(const GrClip& clip,
                               GrPaint&& paint,
                               GrAA aa,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect);

    void drawShapeUsingPathRenderer(const GrClip&, GrPaint&&, GrAA, const SkMatrix&,
                                    const GrShape&);

    // Allows caller of addDrawOp to know which op list an op will be added to.
    using WillAddOpFn = void(GrOp*, uint32_t opListID);
    // These perform processing specific to GrDrawOp-derived ops before recording them into an
    // op list. Before adding the op to an op list the WillAddOpFn is called. Note that it
    // will not be called in the event that the op is discarded. Moreover, the op may merge into
    // another op after the function is called (either before addDrawOp returns or some time later).
    void addDrawOp(const GrClip&, std::unique_ptr<GrDrawOp>,
                   const std::function<WillAddOpFn>& = std::function<WillAddOpFn>());

    // Makes a copy of the proxy if it is necessary for the draw and places the texture that should
    // be used by GrXferProcessor to access the destination color in 'result'. If the return
    // value is false then a texture copy could not be made.
    bool SK_WARN_UNUSED_RESULT setupDstProxy(GrRenderTargetProxy*,
                                             const GrClip&,
                                             const GrOp& op,
                                             GrXferProcessor::DstProxy* result);

    // The rescaling step of asyncRescaleAndReadPixels[YUV420]().
    sk_sp<GrRenderTargetContext> rescale(const SkImageInfo& info, const SkIRect& srcRect,
                                         SkSurface::RescaleGamma rescaleGamma,
                                         SkFilterQuality rescaleQuality);
    // The async read step of asyncRescaleAndReadPixels()
    void asyncReadPixels(const SkIRect& rect, SkColorType colorType, ReadPixelsCallback callback,
                         ReadPixelsContext context);

    // Inserts a transfer, part of the implementation of asyncReadPixels and
    // asyncRescaleAndReadPixelsYUV420().
    struct PixelTransferResult {
        using ConversionSignature = void(void* dst, const void* mappedBuffer);
        // If null then the transfer could not be performed. Otherwise this buffer will contain
        // the pixel data when the transfer is complete.
        sk_sp<GrGpuBuffer> fTransferBuffer;
        // If this is null then the transfer buffer will contain the data in the requested
        // color type. Otherwise, when the transfer is done this must be called to convert
        // from the transfer buffer's color type to the requested color type.
        std::function<ConversionSignature> fPixelConverter;
    };
    // Inserts a transfer of rect to a buffer that this call will create.
    PixelTransferResult transferPixels(GrColorType colorType, const SkIRect& rect);

    GrRenderTargetOpList* getRTOpList();
    GrOpList* getOpList() override;

    std::unique_ptr<GrTextTarget> fTextTarget;
    sk_sp<GrRenderTargetProxy> fRenderTargetProxy;

    // In MDB-mode the GrOpList can be closed by some other renderTargetContext that has picked
    // it up. For this reason, the GrOpList should only ever be accessed via 'getOpList'.
    sk_sp<GrRenderTargetOpList> fOpList;

    SkSurfaceProps fSurfaceProps;
    bool fManagedOpList;

    typedef GrSurfaceContext INHERITED;
};

#endif

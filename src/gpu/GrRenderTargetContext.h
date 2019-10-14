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
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/geometry/GrQuad.h"
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
class GrRenderTargetContext : public GrSurfaceContext {
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

    void clear(const SkPMColor4f& color) {
        return this->clear(nullptr, color, CanClearFullscreen::kYes);
    }

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
        this->drawFilledQuad(clip, std::move(paint), aa,
                             aa == GrAA::kYes ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone,
                             GrQuad::MakeFromRect(rectToDraw, viewMatrix), GrQuad(localRect));
    }

    /**
     * Fills a rect with a paint and a localMatrix.
     */
    void fillRectWithLocalMatrix(const GrClip& clip,
                                 GrPaint&& paint,
                                 GrAA aa,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkMatrix& localMatrix) {
        this->drawFilledQuad(clip, std::move(paint), aa,
                             aa == GrAA::kYes ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone,
                             GrQuad::MakeFromRect(rect, viewMatrix),
                             GrQuad::MakeFromRect(rect, localMatrix));
    }

    /**
     * Creates an op that draws a fill rect with per-edge control over anti-aliasing.
     *
     * This is a specialized version of fillQuadWithEdgeAA, but is kept separate since knowing
     * the geometry is a rectangle affords more optimizations.
     */
    void fillRectWithEdgeAA(const GrClip& clip, GrPaint&& paint, GrAA aa, GrQuadAAFlags edgeAA,
                            const SkMatrix& viewMatrix, const SkRect& rect,
                            const SkRect* optionalLocalRect = nullptr) {
        const SkRect& localRect = optionalLocalRect ? *optionalLocalRect : rect;
        this->drawFilledQuad(clip, std::move(paint), aa, edgeAA,
                             GrQuad::MakeFromRect(rect, viewMatrix), GrQuad(localRect));
    }

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
                            const SkPoint optionalLocalQuad[4]) {
        const SkPoint* localQuad = optionalLocalQuad ? optionalLocalQuad : quad;
        this->drawFilledQuad(clip, std::move(paint), aa, edgeAA,
                             GrQuad::MakeFromSkQuad(quad, viewMatrix),
                             GrQuad::MakeFromSkQuad(localQuad, SkMatrix::I()));
    }

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
    void drawTexture(const GrClip& clip, sk_sp<GrTextureProxy> proxy, GrSamplerState::Filter filter,
                     SkBlendMode mode, const SkPMColor4f& color, const SkRect& srcRect,
                     const SkRect& dstRect, GrAA aa, GrQuadAAFlags edgeAA,
                     SkCanvas::SrcRectConstraint constraint, const SkMatrix& viewMatrix,
                     sk_sp<GrColorSpaceXform> texXform) {
        const SkRect* domain = constraint == SkCanvas::kStrict_SrcRectConstraint ?
                &srcRect : nullptr;
        this->drawTexturedQuad(clip, std::move(proxy), std::move(texXform), filter,
                               color, mode, aa, edgeAA, GrQuad::MakeFromRect(dstRect, viewMatrix),
                               GrQuad(srcRect), domain);
    }

    /**
     * Variant of drawTexture that instead draws the texture applied to 'dstQuad' transformed by
     * 'viewMatrix', using the 'srcQuad' texture coordinates clamped to the optional 'domain'. If
     * 'domain' is null, it's equivalent to using the fast src rect constraint. If 'domain' is
     * provided, the strict src rect constraint is applied using 'domain'.
     */
    void drawTextureQuad(const GrClip& clip, sk_sp<GrTextureProxy> proxy,
                         GrSamplerState::Filter filter, SkBlendMode mode, const SkPMColor4f& color,
                         const SkPoint srcQuad[4], const SkPoint dstQuad[4], GrAA aa,
                         GrQuadAAFlags edgeAA, const SkRect* domain, const SkMatrix& viewMatrix,
                         sk_sp<GrColorSpaceXform> texXform) {
        this->drawTexturedQuad(clip, std::move(proxy), std::move(texXform), filter, color, mode,
                               aa, edgeAA, GrQuad::MakeFromSkQuad(dstQuad, viewMatrix),
                               GrQuad::MakeFromSkQuad(srcQuad, SkMatrix::I()), domain);
    }

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
    using ReadPixelsContext = SkSurface::ReadPixelsContext;
    using RescaleGamma = SkSurface::RescaleGamma;

    // GPU implementation for SkSurface::asyncRescaleAndReadPixels.
    void asyncRescaleAndReadPixels(const SkImageInfo& info, const SkIRect& srcRect,
                                   RescaleGamma rescaleGamma, SkFilterQuality rescaleQuality,
                                   ReadPixelsCallback callback, ReadPixelsContext context);
    // GPU implementation for SkSurface::asyncRescaleAndReadPixelsYUV420.
    void asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                         sk_sp<SkColorSpace> dstColorSpace,
                                         const SkIRect& srcRect,
                                         const SkISize& dstSize,
                                         RescaleGamma rescaleGamma,
                                         SkFilterQuality rescaleQuality,
                                         ReadPixelsCallback callback,
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

    const GrRenderTargetProxy* proxy() const { return fRenderTargetProxy.get(); }
    int width() const { return fRenderTargetProxy->width(); }
    int height() const { return fRenderTargetProxy->height(); }
    int numSamples() const { return fRenderTargetProxy->numSamples(); }
    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }
    GrSurfaceOrigin origin() const { return fRenderTargetProxy->origin(); }
    bool wrapsVkSecondaryCB() const { return fRenderTargetProxy->wrapsVkSecondaryCB(); }
    GrMipMapped mipMapped() const;

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

#if GR_TEST_UTILS
    bool testingOnly_IsInstantiated() const { return fRenderTargetProxy->isInstantiated(); }
    void testingOnly_SetPreserveOpsOnFullClear() { fPreserveOpsOnFullClear_TestingOnly = true; }
    GrOpsTask* testingOnly_PeekLastOpsTask() { return fOpsTask.get(); }
#endif

protected:
    GrRenderTargetContext(GrRecordingContext*, sk_sp<GrRenderTargetProxy>, GrColorType,
                          sk_sp<SkColorSpace>, const SkSurfaceProps*, bool managedOpsTask = true);

    SkDEBUGCODE(void validate() const override;)

private:
    class TextTarget;
    enum class QuadOptimization;

    GrAAType chooseAAType(GrAA);

    friend class GrAtlasTextBlob;               // for access to add[Mesh]DrawOp
    friend class GrClipStackClip;               // for access to getOpsTask
    friend class GrOnFlushResourceProvider;     // for access to getOpsTask (http://skbug.com/9357)

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

    GrOpsTask::CanDiscardPreviousOps canDiscardPreviousOpsOnFullClear() const;
    void setNeedsStencil(bool useMixedSamplesIfNotMSAA);

    void internalClear(const GrFixedClip&, const SkPMColor4f&, CanClearFullscreen);
    void internalStencilClear(const GrFixedClip&, bool insideStencilMask);

    // Only consumes the GrPaint if successful.
    bool drawFilledDRRect(const GrClip& clip,
                          GrPaint&& paint,
                          GrAA,
                          const SkMatrix& viewMatrix,
                          const SkRRect& origOuter,
                          const SkRRect& origInner);

    // If the drawn quad's paint is a const blended color, provide it as a non-null pointer to
    // 'constColor', which enables the draw-as-clear optimization. Otherwise it is assumed the paint
    // requires some form of shading that invalidates using a clear op.
    //
    // The non-const pointers should be the original draw request on input, and will be updated as
    // appropriate depending on the returned optimization level.
    //
    // 'stencilSettings' are provided merely for decision making purposes; When non-null,
    // optimization strategies that submit special ops are avoided.
    QuadOptimization attemptQuadOptimization(const GrClip& clip,
                                             const SkPMColor4f* constColor,
                                             const GrUserStencilSettings* stencilSettings,
                                             GrAA* aa,
                                             GrQuadAAFlags* edgeFlags,
                                             GrQuad* deviceQuad,
                                             GrQuad* localQuad);

    // If stencil settings, 'ss', are non-null, AA controls MSAA or no AA. If they are null, then AA
    // can choose between coverage, MSAA as per chooseAAType(). This will always attempt to apply
    // quad optimizations, so all quad/rect public APIs should rely on this function for consistent
    // clipping behavior.
    void drawFilledQuad(const GrClip& clip,
                        GrPaint&& paint,
                        GrAA aa,
                        GrQuadAAFlags edgeFlags,
                        const GrQuad& deviceQuad,
                        const GrQuad& localQuad,
                        const GrUserStencilSettings* ss = nullptr);

    // Like drawFilledQuad but does not require using a GrPaint or FP for texturing
    void drawTexturedQuad(const GrClip& clip,
                          sk_sp<GrTextureProxy> proxy,
                          sk_sp<GrColorSpaceXform> textureXform,
                          GrSamplerState::Filter filter,
                          const SkPMColor4f& color,
                          SkBlendMode blendMode,
                          GrAA aa,
                          GrQuadAAFlags edgeFlags,
                          const GrQuad& deviceQuad,
                          const GrQuad& localQuad,
                          const SkRect* domain = nullptr);

    void drawShapeUsingPathRenderer(const GrClip&, GrPaint&&, GrAA, const SkMatrix&,
                                    const GrShape&);

    void addOp(std::unique_ptr<GrOp>);

    // Allows caller of addDrawOp to know which op list an op will be added to.
    using WillAddOpFn = void(GrOp*, uint32_t opsTaskID);
    // These perform processing specific to GrDrawOp-derived ops before recording them into an
    // op list. Before adding the op to an op list the WillAddOpFn is called. Note that it
    // will not be called in the event that the op is discarded. Moreover, the op may merge into
    // another op after the function is called (either before addDrawOp returns or some time later).
    void addDrawOp(const GrClip&, std::unique_ptr<GrDrawOp>,
                   const std::function<WillAddOpFn>& = std::function<WillAddOpFn>());

    // Makes a copy of the proxy if it is necessary for the draw and places the texture that should
    // be used by GrXferProcessor to access the destination color in 'result'. If the return
    // value is false then a texture copy could not be made.
    bool SK_WARN_UNUSED_RESULT setupDstProxy(const GrClip&, const GrOp& op,
                                             GrXferProcessor::DstProxy* result);

    class AsyncReadResult;

    // The async read step of asyncRescaleAndReadPixels()
    void asyncReadPixels(const SkIRect& rect, SkColorType colorType, ReadPixelsCallback callback,
                         ReadPixelsContext context);

    GrOpsTask* getOpsTask();

    std::unique_ptr<GrTextTarget> fTextTarget;
    sk_sp<GrRenderTargetProxy> fRenderTargetProxy;

    // In MDB-mode the GrOpsTask can be closed by some other renderTargetContext that has picked
    // it up. For this reason, the GrOpsTask should only ever be accessed via 'getOpsTask'.
    sk_sp<GrOpsTask> fOpsTask;

    SkSurfaceProps fSurfaceProps;
    bool fManagedOpsTask;

    int fNumStencilSamples = 0;
#if GR_TEST_UTILS
    bool fPreserveOpsOnFullClear_TestingOnly = false;
#endif

    typedef GrSurfaceContext INHERITED;
};

#endif

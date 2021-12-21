/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceDrawContext_v1_DEFINED
#define SurfaceDrawContext_v1_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/ops/OpsTask.h"
#include "src/gpu/v1/SurfaceFillContext_v1.h"

class GrBackendSemaphore;
class GrClip;
class GrColorSpaceXform;
class GrDrawOp;
class GrDstProxyView;
class GrHardClip;
class GrOp;
struct GrQuadSetEntry;
class GrRenderTarget;
class GrStyledShape;
class GrStyle;
class GrTextureProxy;
struct GrTextureSetEntry;
struct GrUserStencilSettings;
struct SkDrawShadowRec;
class SkGlyphRunList;
struct SkIPoint;
struct SkIRect;
class SkLatticeIter;
class SkMatrixProvider;
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

namespace skgpu::v1 {

/**
 * A helper object to orchestrate commands (draws, etc...) for GrSurfaces that are GrRenderTargets.
 */
class SurfaceDrawContext final : public SurfaceFillContext {
public:
    static std::unique_ptr<SurfaceDrawContext> Make(GrRecordingContext*,
                                                    GrColorType,
                                                    sk_sp<GrSurfaceProxy>,
                                                    sk_sp<SkColorSpace>,
                                                    GrSurfaceOrigin,
                                                    const SkSurfaceProps&,
                                                    bool flushTimeOpsTask = false);

    /* Uses the default texture format for the color type */
    static std::unique_ptr<SurfaceDrawContext> Make(GrRecordingContext*,
                                                    GrColorType,
                                                    sk_sp<SkColorSpace>,
                                                    SkBackingFit,
                                                    SkISize dimensions,
                                                    const SkSurfaceProps&,
                                                    int sampleCnt = 1,
                                                    GrMipmapped = GrMipmapped::kNo,
                                                    GrProtected = GrProtected::kNo,
                                                    GrSurfaceOrigin = kBottomLeft_GrSurfaceOrigin,
                                                    SkBudgeted = SkBudgeted::kYes);

    /**
     * Takes custom swizzles rather than determining swizzles from color type and format.
     * It will have color type kUnknown.
     */
    static std::unique_ptr<SurfaceDrawContext> Make(GrRecordingContext*,
                                                    sk_sp<SkColorSpace>,
                                                    SkBackingFit,
                                                    SkISize dimensions,
                                                    const GrBackendFormat&,
                                                    int sampleCnt,
                                                    GrMipmapped,
                                                    GrProtected,
                                                    GrSwizzle readSwizzle,
                                                    GrSwizzle writeSwizzle,
                                                    GrSurfaceOrigin,
                                                    SkBudgeted,
                                                    const SkSurfaceProps&);

    // Same as previous factory but will try to use fallback GrColorTypes if the one passed in
    // fails. The fallback GrColorType will have at least the number of channels and precision per
    // channel as the passed in GrColorType. It may also swizzle the changes (e.g., BGRA -> RGBA).
    // SRGB-ness will be preserved.
    static std::unique_ptr<SurfaceDrawContext> MakeWithFallback(
            GrRecordingContext*,
            GrColorType,
            sk_sp<SkColorSpace>,
            SkBackingFit,
            SkISize dimensions,
            const SkSurfaceProps&,
            int sampleCnt = 1,
            GrMipmapped = GrMipmapped::kNo,
            GrProtected = GrProtected::kNo,
            GrSurfaceOrigin = kBottomLeft_GrSurfaceOrigin,
            SkBudgeted = SkBudgeted::kYes);

    // Creates a SurfaceDrawContext that wraps the passed in GrBackendTexture.
    static std::unique_ptr<SurfaceDrawContext> MakeFromBackendTexture(
            GrRecordingContext*,
            GrColorType,
            sk_sp<SkColorSpace>,
            const GrBackendTexture&,
            int sampleCnt,
            GrSurfaceOrigin,
            const SkSurfaceProps&,
            sk_sp<GrRefCntedCallback> releaseHelper);

    SurfaceDrawContext(GrRecordingContext*,
                       GrSurfaceProxyView readView,
                       GrSurfaceProxyView writeView,
                       GrColorType,
                       sk_sp<SkColorSpace>,
                       const SkSurfaceProps&,
                       bool flushTimeOpsTask = false);

    ~SurfaceDrawContext() override;

    /**
     *  Draw everywhere (respecting the clip) with the paint.
     */
    void drawPaint(const GrClip*, GrPaint&&, const SkMatrix& viewMatrix);

    /**
     * Draw the rect using a paint.
     * @param paint        describes how to color pixels.
     * @param GrAA         Controls whether rect is antialiased
     * @param viewMatrix   transformation matrix
     * @param style        The style to apply. Null means fill. Currently path effects are not
     *                     allowed.
     * The rects coords are used to access the paint (through texture matrix)
     */
    void drawRect(const GrClip*,
                  GrPaint&& paint,
                  GrAA,
                  const SkMatrix& viewMatrix,
                  const SkRect&,
                  const GrStyle* style = nullptr);

    /**
     * Maps a rectangle of shader coordinates to a rectangle and fills that rectangle.
     *
     * @param GrPaint      describes how to color pixels.
     * @param GrAA         Controls whether rect is antialiased
     * @param SkMatrix     transformation matrix which applies to rectToDraw
     * @param rectToDraw   the rectangle to draw
     * @param localRect    the rectangle of shader coordinates applied to rectToDraw
     */
    void fillRectToRect(const GrClip*,
                        GrPaint&&,
                        GrAA,
                        const SkMatrix&,
                        const SkRect& rectToDraw,
                        const SkRect& localRect);

    /**
     * Fills a block of pixels with a paint and a localMatrix, respecting the clip.
     */
    void fillPixelsWithLocalMatrix(const GrClip* clip,
                                   GrPaint&& paint,
                                   const SkIRect& bounds,
                                   const SkMatrix& localMatrix) {
        SkRect rect = SkRect::Make(bounds);
        DrawQuad quad{GrQuad::MakeFromRect(rect, SkMatrix::I()),
                      GrQuad::MakeFromRect(rect, localMatrix), GrQuadAAFlags::kNone};
        this->drawFilledQuad(clip, std::move(paint), GrAA::kNo, &quad);
    }

    /**
     * Creates an op that draws a fill rect with per-edge control over anti-aliasing.
     *
     * This is a specialized version of fillQuadWithEdgeAA, but is kept separate since knowing
     * the geometry is a rectangle affords more optimizations.
     */
    void fillRectWithEdgeAA(const GrClip* clip, GrPaint&& paint, GrAA aa, GrQuadAAFlags edgeAA,
                            const SkMatrix& viewMatrix, const SkRect& rect,
                            const SkRect* optionalLocalRect = nullptr) {
        if (edgeAA == GrQuadAAFlags::kAll) {
            this->fillRectToRect(clip, std::move(paint), aa, viewMatrix, rect,
                                 (optionalLocalRect) ? *optionalLocalRect : rect);
            return;
        }
        const SkRect& localRect = optionalLocalRect ? *optionalLocalRect : rect;
        DrawQuad quad{GrQuad::MakeFromRect(rect, viewMatrix), GrQuad(localRect), edgeAA};
        this->drawFilledQuad(clip, std::move(paint), aa, &quad);
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
    void fillQuadWithEdgeAA(const GrClip* clip, GrPaint&& paint, GrAA aa, GrQuadAAFlags edgeAA,
                            const SkMatrix& viewMatrix, const SkPoint points[4],
                            const SkPoint optionalLocalPoints[4]) {
        const SkPoint* localPoints = optionalLocalPoints ? optionalLocalPoints : points;
        DrawQuad quad{GrQuad::MakeFromSkQuad(points, viewMatrix),
                      GrQuad::MakeFromSkQuad(localPoints, SkMatrix::I()), edgeAA};
        this->drawFilledQuad(clip, std::move(paint), aa, &quad);
    }

    // TODO(michaelludwig) - remove if the bulk API is not useful for SkiaRenderer
    void drawQuadSet(const GrClip* clip, GrPaint&& paint, GrAA aa, const SkMatrix& viewMatrix,
                     const GrQuadSetEntry[], int cnt);

    /**
     * Creates an op that draws a subrectangle of a texture. The passed color is modulated by the
     * texture's color. 'srcRect' specifies the rectangle of the texture to draw. 'dstRect'
     * specifies the rectangle to draw in local coords which will be transformed by 'viewMatrix' to
     * device space.
     */
    void drawTexture(const GrClip*,
                     GrSurfaceProxyView,
                     SkAlphaType,
                     GrSamplerState::Filter,
                     GrSamplerState::MipmapMode,
                     SkBlendMode,
                     const SkPMColor4f&,
                     const SkRect& srcRect,
                     const SkRect& dstRect,
                     GrAA,
                     GrQuadAAFlags,
                     SkCanvas::SrcRectConstraint,
                     const SkMatrix&,
                     sk_sp<GrColorSpaceXform>);

    /**
     * Variant of drawTexture that instead draws the texture applied to 'dstQuad' transformed by
     * 'viewMatrix', using the 'srcQuad' texture coordinates clamped to the optional 'subset'. If
     * 'subset' is null, it's equivalent to using the fast src rect constraint. If 'subset' is
     * provided, the strict src rect constraint is applied using 'subset'.
     */
    void drawTextureQuad(const GrClip* clip,
                         GrSurfaceProxyView view,
                         GrColorType srcColorType,
                         SkAlphaType srcAlphaType,
                         GrSamplerState::Filter filter,
                         GrSamplerState::MipmapMode mm,
                         SkBlendMode mode,
                         const SkPMColor4f& color,
                         const SkPoint srcQuad[4],
                         const SkPoint dstQuad[4],
                         GrAA aa,
                         GrQuadAAFlags edgeAA,
                         const SkRect* subset,
                         const SkMatrix& viewMatrix,
                         sk_sp<GrColorSpaceXform> texXform) {
        DrawQuad quad{GrQuad::MakeFromSkQuad(dstQuad, viewMatrix),
                      GrQuad::MakeFromSkQuad(srcQuad, SkMatrix::I()), edgeAA};
        this->drawTexturedQuad(clip, std::move(view), srcAlphaType, std::move(texXform), filter, mm,
                               color, mode, aa, &quad, subset);
    }

    /**
     * Draws a set of textures with a shared filter, color, view matrix, color xform, and
     * texture color xform. The textures must all have the same GrTextureType and GrConfig.
     *
     * If any entries provide a non-null fDstClip array, it will be read from immediately based on
     * fDstClipCount, so the pointer can become invalid after this returns.
     *
     * 'proxRunCnt' is the number of proxy changes encountered in the entry array. Technically this
     * can be inferred from the array within this function, but the information is already known
     * by SkGpuDevice, so no need to incur another iteration over the array.
     */
    void drawTextureSet(const GrClip*,
                        GrTextureSetEntry[],
                        int cnt,
                        int proxyRunCnt,
                        GrSamplerState::Filter,
                        GrSamplerState::MipmapMode,
                        SkBlendMode mode,
                        GrAA aa,
                        SkCanvas::SrcRectConstraint,
                        const SkMatrix& viewMatrix,
                        sk_sp<GrColorSpaceXform> texXform);

    /**
     * Draw a roundrect using a paint.
     *
     * @param paint       describes how to color pixels.
     * @param GrAA        Controls whether rrect is antialiased.
     * @param viewMatrix  transformation matrix
     * @param rrect       the roundrect to draw
     * @param style       style to apply to the rrect. Currently path effects are not allowed.
     */
    void drawRRect(const GrClip*,
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
    bool drawFastShadow(const GrClip*,
                        const SkMatrix& viewMatrix,
                        const SkPath& path,
                        const SkDrawShadowRec& rec);

    /**
     * Draws a path.
     *
     * @param paint         describes how to color pixels.
     * @param GrAA          Controls whether the path is antialiased.
     * @param viewMatrix    transformation matrix
     * @param path          the path to draw
     * @param style         style to apply to the path.
     */
    void drawPath(const GrClip*,
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
    void drawShape(const GrClip*,
                   GrPaint&&,
                   GrAA,
                   const SkMatrix& viewMatrix,
                   GrStyledShape&&);

    /**
     * Draws vertices with a paint.
     *
     * @param   paint            describes how to color pixels.
     * @param   viewMatrix       transformation matrix
     * @param   vertices         specifies the mesh to draw.
     * @param   overridePrimType primitive type to draw. If NULL, derive prim type from vertices.
     */
    void drawVertices(const GrClip*,
                      GrPaint&& paint,
                      const SkMatrixProvider& matrixProvider,
                      sk_sp<SkVertices> vertices,
                      GrPrimitiveType* overridePrimType = nullptr);

    /**
     * Draws vertices with a paint.
     *
     * @param   paint            describes how to color pixels.
     * @param   viewMatrix       transformation matrix
     * @param   vertices         specifies the mesh to draw.
     * @param   overridePrimType primitive type to draw. If NULL, derive prim type from vertices.
     * @param   effect           runtime effect that will handle custom vertex attributes.
     */
    void drawCustomMesh(const GrClip*,
                        GrPaint&& paint,
                        const SkMatrixProvider& matrixProvider,
                        SkCustomMesh);

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
    void drawAtlas(const GrClip*,
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
    void drawRegion(const GrClip*,
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
    void drawOval(const GrClip*,
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
    void drawArc(const GrClip*,
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
    void drawImageLattice(const GrClip*,
                          GrPaint&&,
                          const SkMatrix& viewMatrix,
                          GrSurfaceProxyView,
                          SkAlphaType alphaType,
                          sk_sp<GrColorSpaceXform>,
                          GrSamplerState::Filter,
                          std::unique_ptr<SkLatticeIter>,
                          const SkRect& dst);

    /**
     * Draw the text specified by the SkGlyphRunList.
     *
     * @param viewMatrix      transformationMatrix
     * @param glyphRunList    text, text positions, and paint.
     */
    void drawGlyphRunList(const GrClip*,
                          const SkMatrixProvider& viewMatrix,
                          const SkGlyphRunList& glyphRunList,
                          const SkPaint& paint);

    /**
     * Draw the text specified by the SkGlyphRunList.
     *
     * @param viewMatrix      transformationMatrix
     * @param glyphRunList    text, text positions, and paint.
     */
    void drawGlyphRunListNoCache(const GrClip*,
                                 const SkMatrixProvider& viewMatrix,
                                 const SkGlyphRunList& glyphRunList,
                                 const SkPaint& paint);

    /**
     * Convert the glyph-run list to a slug.
     */
    sk_sp<GrSlug> convertGlyphRunListToSlug(const SkMatrixProvider& viewMatrix,
                                            const SkGlyphRunList& glyphRunList,
                                            const SkPaint& paint);

    /**
     * Draw a slug.
     */
    void drawSlug(const GrClip* clip,
                  const SkMatrixProvider& viewMatrix,
                  GrSlug* slugPtr);

    /**
     * Adds the necessary signal and wait semaphores and adds the passed in SkDrawable to the
     * command stream.
     */
    void drawDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>, const SkRect& bounds);

    // called to note the last clip drawn to the stencil buffer.
    // TODO: remove after clipping overhaul.
    void setLastClip(uint32_t clipStackGenID,
                     const SkIRect& devClipBounds,
                     int numClipAnalyticElements);

    // called to determine if we have to render the clip into SB.
    // TODO: remove after clipping overhaul.
    bool mustRenderClip(uint32_t clipStackGenID,
                        const SkIRect& devClipBounds,
                        int numClipAnalyticElements);

    void clearStencilClip(const SkIRect& scissor, bool insideStencilMask) {
        this->internalStencilClear(&scissor, insideStencilMask);
    }

    // While this can take a general clip, since ClipStack relies on this function, it must take
    // care to only provide hard clips or we could get stuck in a loop. The general clip is needed
    // so that path renderers can use this function.
    void stencilRect(const GrClip* clip,
                     const GrUserStencilSettings* ss,
                     GrPaint&& paint,
                     GrAA doStencilMSAA,
                     const SkMatrix& viewMatrix,
                     const SkRect& rect,
                     const SkMatrix* localMatrix = nullptr) {
        // Since this provides stencil settings to drawFilledQuad, it performs a different AA type
        // resolution compared to regular rect draws, which is the main reason it remains separate.
        DrawQuad quad{GrQuad::MakeFromRect(rect, viewMatrix),
                      localMatrix ? GrQuad::MakeFromRect(rect, *localMatrix) : GrQuad(rect),
                      doStencilMSAA == GrAA::kYes ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone};
        this->drawFilledQuad(clip, std::move(paint), doStencilMSAA, &quad, ss);
    }

    // Fills the user stencil bits with a non-zero value at every sample inside the path. This will
    // likely be implemented with a Redbook algorithm, but it is not guaranteed. The samples being
    // rendered to must be zero initially.
    bool stencilPath(const GrHardClip*,
                     GrAA doStencilMSAA,
                     const SkMatrix& viewMatrix,
                     const SkPath&);

    /**
     * Draws a path, either AA or not, and touches the stencil buffer with the user stencil settings
     * for each color sample written.
     */
    bool drawAndStencilPath(const GrHardClip*,
                            const GrUserStencilSettings*,
                            SkRegion::Op op,
                            bool invert,
                            GrAA doStencilMSAA,
                            const SkMatrix& viewMatrix,
                            const SkPath&);

    SkBudgeted isBudgeted() const;

    int maxWindowRectangles() const;

    SkGlyphRunListPainter* glyphRunPainter() { return &fGlyphPainter; }

    /*
     * This unique ID will not change for a given SurfaceDrawContext. However, it is _NOT_
     * guaranteed to match the uniqueID of the underlying GrRenderTarget - beware!
     */
    GrSurfaceProxy::UniqueID uniqueID() const { return this->asSurfaceProxy()->uniqueID(); }

    // Allows caller of addDrawOp to know which op list an op will be added to.
    using WillAddOpFn = void(GrOp*, uint32_t opsTaskID);
    // These perform processing specific to GrDrawOp-derived ops before recording them into an
    // op list. Before adding the op to an op list the WillAddOpFn is called. Note that it
    // will not be called in the event that the op is discarded. Moreover, the op may merge into
    // another op after the function is called (either before addDrawOp returns or some time later).
    //
    // If the clip pointer is null, no clipping will be performed.
    void addDrawOp(const GrClip*,
                   GrOp::Owner,
                   const std::function<WillAddOpFn>& = std::function<WillAddOpFn>());
    void addDrawOp(GrOp::Owner op) { this->addDrawOp(nullptr, std::move(op)); }

    bool refsWrappedObjects() const { return this->asRenderTargetProxy()->refsWrappedObjects(); }

    /**
     *  The next time this SurfaceDrawContext is flushed, the gpu will wait on the passed in
     *  semaphores before executing any commands.
     */
    bool waitOnSemaphores(int numSemaphores, const GrBackendSemaphore waitSemaphores[],
                          bool deleteSemaphoresAfterWait);

    int numSamples() const { return this->asRenderTargetProxy()->numSamples(); }
    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }
    bool canUseDynamicMSAA() const { return fCanUseDynamicMSAA; }
    bool wrapsVkSecondaryCB() const { return this->asRenderTargetProxy()->wrapsVkSecondaryCB(); }

    bool alwaysAntialias() const {
        return fSurfaceProps.flags() & SkSurfaceProps::kDynamicMSAA_Flag;
    }

    GrAA chooseAA(const SkPaint& paint) {
        return GrAA(paint.isAntiAlias() || this->alwaysAntialias());
    }

    GrAAType chooseAAType(GrAA aa) {
        if (this->numSamples() > 1 || fCanUseDynamicMSAA) {
            // Always trigger DMSAA when it's available. The coverage ops that know how to handle
            // both single and multisample targets without popping will do so without calling
            // chooseAAType.
            return GrAAType::kMSAA;
        }
        return (aa == GrAA::kYes) ? GrAAType::kCoverage : GrAAType::kNone;
    }

    // This entry point should only be called if the backing GPU object is known to be
    // instantiated.
    GrRenderTarget* accessRenderTarget() { return this->asSurfaceProxy()->peekRenderTarget(); }

#if GR_TEST_UTILS
    void testingOnly_SetPreserveOpsOnFullClear() { fPreserveOpsOnFullClear_TestingOnly = true; }
#endif

private:
    enum class QuadOptimization;

    void willReplaceOpsTask(OpsTask* prevTask, OpsTask* nextTask) override;

    OpsTask::CanDiscardPreviousOps canDiscardPreviousOpsOnFullClear() const override;
    void setNeedsStencil();

    void internalStencilClear(const SkIRect* scissor, bool insideStencilMask);

    // 'stencilSettings' are provided merely for decision making purposes; When non-null,
    // optimization strategies that submit special ops are avoided.
    //
    // 'aa' and 'quad' should be the original draw request on input, and will be updated as
    // appropriate depending on the returned optimization level.
    //
    // If kSubmitted is returned, the provided paint was consumed. Otherwise it is left unchanged.
    QuadOptimization attemptQuadOptimization(const GrClip* clip,
                                             const GrUserStencilSettings* stencilSettings,
                                             GrAA* aa,
                                             DrawQuad* quad,
                                             GrPaint* paint);

    // If stencil settings, 'ss', are non-null, AA controls MSAA or no AA. If they are null, then AA
    // can choose between coverage, MSAA as per chooseAAType(). This will always attempt to apply
    // quad optimizations, so all quad/rect public APIs should rely on this function for consistent
    // clipping behavior. 'quad' will be modified in place to reflect final rendered geometry.
    void drawFilledQuad(const GrClip* clip,
                        GrPaint&& paint,
                        GrAA aa,
                        DrawQuad* quad,
                        const GrUserStencilSettings* ss = nullptr);

    // Like drawFilledQuad but does not require using a GrPaint or FP for texturing.
    // 'quad' may be modified in place to reflect final geometry.
    void drawTexturedQuad(const GrClip* clip,
                          GrSurfaceProxyView proxyView,
                          SkAlphaType alphaType,
                          sk_sp<GrColorSpaceXform> textureXform,
                          GrSamplerState::Filter filter,
                          GrSamplerState::MipmapMode,
                          const SkPMColor4f& color,
                          SkBlendMode blendMode,
                          GrAA aa,
                          DrawQuad* quad,
                          const SkRect* subset = nullptr);

    void drawStrokedLine(const GrClip*, GrPaint&&, GrAA, const SkMatrix&, const SkPoint[2],
                         const SkStrokeRec&);

    // Tries to detect if the given shape is a simple, and draws it without path rendering if
    // we know how.
    bool drawSimpleShape(const GrClip*, GrPaint*, GrAA, const SkMatrix&, const GrStyledShape&);

    // If 'attemptDrawSimple' is true, of if the original shape is marked as having been simplfied,
    // this will attempt to re-route through drawSimpleShape() to see if we can avoid path rendering
    // one more time.
    void drawShapeUsingPathRenderer(const GrClip*, GrPaint&&, GrAA, const SkMatrix&,
                                    GrStyledShape&&, bool attemptDrawSimple = false);

    // Makes a copy of the proxy if it is necessary for the draw and places the texture that should
    // be used by GrXferProcessor to access the destination color in 'result'. If the return
    // value is false then a texture copy could not be made.
    //
    // The op should have already had setClippedBounds called on it.
    bool SK_WARN_UNUSED_RESULT setupDstProxyView(const SkRect& opBounds,
                                                 bool opRequiresMSAA,
                                                 GrDstProxyView* result);

    OpsTask* replaceOpsTaskIfModifiesColor();

    SkGlyphRunListPainter* glyphPainter() { return &fGlyphPainter; }

    const SkSurfaceProps fSurfaceProps;
    const bool fCanUseDynamicMSAA;

    bool fNeedsStencil = false;

#if GR_TEST_UTILS
    bool fPreserveOpsOnFullClear_TestingOnly = false;
#endif
    SkGlyphRunListPainter fGlyphPainter;
};

} // namespace skgpu::v1

#endif // SurfaceDrawContext_v1_DEFINED

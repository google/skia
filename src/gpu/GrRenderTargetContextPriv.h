/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetContextPriv_DEFINED
#define GrRenderTargetContextPriv_DEFINED

#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetOpList.h"

class GrFixedClip;
class GrHardClip;
class GrPath;
class GrRenderTargetPriv;
struct GrUserStencilSettings;

/** Class that adds methods to GrRenderTargetContext that are only intended for use internal to
    Skia. This class is purely a privileged window into GrRenderTargetContext. It should never have
    additional data members or virtual methods. */
class GrRenderTargetContextPriv {
public:
    // called to note the last clip drawn to the stencil buffer.
    // TODO: remove after clipping overhaul.
    void setLastClip(uint32_t clipStackGenID, const SkIRect& devClipBounds,
                     int numClipAnalyticFPs) {
        GrRenderTargetOpList* opList = fRenderTargetContext->getRTOpList();
        opList->fLastClipStackGenID = clipStackGenID;
        opList->fLastDevClipBounds = devClipBounds;
        opList->fLastClipNumAnalyticFPs = numClipAnalyticFPs;
    }

    // called to determine if we have to render the clip into SB.
    // TODO: remove after clipping overhaul.
    bool mustRenderClip(uint32_t clipStackGenID, const SkIRect& devClipBounds,
                        int numClipAnalyticFPs) const {
        GrRenderTargetOpList* opList = fRenderTargetContext->getRTOpList();
        return opList->fLastClipStackGenID != clipStackGenID ||
               !opList->fLastDevClipBounds.contains(devClipBounds) ||
               opList->fLastClipNumAnalyticFPs != numClipAnalyticFPs;
    }

    using CanClearFullscreen = GrRenderTargetContext::CanClearFullscreen;

    void clear(const GrFixedClip&, const SkPMColor4f&, CanClearFullscreen);

    void clearStencilClip(const GrFixedClip&, bool insideStencilMask);

    /*
     * Some portions of the code, which use approximate-match rendertargets (i.e., ImageFilters),
     * rely on clears that lie outside of the content region to still have an effect.
     * For example, when sampling a decimated blurred image back up to full size, the GaussianBlur
     * code draws 1-pixel rects along the left and bottom edges to be able to use bilerp for
     * upsampling. The "absClear" entry point ignores the content bounds but does use the
     * worst case (instantiated) bounds.
     *
     * @param rect      if (!null) the rect to clear, otherwise it is a full screen clear
     * @param color     the color to clear to
     */
    void absClear(const SkIRect* rect, const SkPMColor4f& color);

    // void stencilRect(
            // const GrHardClip&, const GrUserStencilSettings* ss, GrAA doStencilMSAA,
            // const SkMatrix& viewMatrix, const SkRect& rect);

    /**
     * Fills a device-space axis-aligned rectangle 'rect', possibly with MSAA (will not use coverage
     * models). If provided, local coordinates for the four corners of 'rect' are taken from
     * 'localCoords', otherwise 'rect' is used.
     *
     * If used with stencil settings, the clip should be an instance of GrHardClip.
     */
    void fillDeviceRect(const GrClip&, GrPaint&&, GrAA doMSAA, const SkRect rect,
                        const GrPerspQuad* localCoords, const GrUserStencilSettings* stencil);
    // FIXME this may need to be stencilQuad() and parallel fillQuad() API but use different AA means,
    // and avoid the draw-as-clear optimization since stencil is involved. This is because some of the
    // GrFillRectOp::MakeWithLocalMatrix went between view+I or I+invView. And while that would kind
    // of match fillQuad() in the first case, we want to not do the optimization and be consistent with
    // AA means.  Then fillQuad() publicly could remove stencil settings?
    // And I guess we could still apply the optimization, but then we'd have a drawFilledQuad internally
    // that both stencilQuad() and fillQuad() routes to pretty quickly.
    // Then we only have to worry about the internal drawDevice, which would be no clip or stencil
    // and operates on SkIRect, used solely with clears (4x calls)


    void stencilPath(
            const GrHardClip&, GrAA doStencilMSAA, const SkMatrix& viewMatrix, const GrPath*);

    /**
     * Draws a rect, either AA or not, and touches the stencil buffer with the user stencil settings
     * for each color sample written.
     */
    // bool drawAndStencilRect(const GrHardClip&,
                            // const GrUserStencilSettings*,
                            // SkRegion::Op op,
                            // bool invert,
                            // GrAA doStencilMSAA,
                            // const SkMatrix& viewMatrix,
                            // const SkRect&);

    /**
     * Draws a path, either AA or not, and touches the stencil buffer with the user stencil settings
     * for each color sample written.
     */
    bool drawAndStencilPath(const GrHardClip&,
                            const GrUserStencilSettings*,
                            SkRegion::Op op,
                            bool invert,
                            GrAA doStencilMSAA,
                            const SkMatrix& viewMatrix,
                            const SkPath&);

    // void drawFilledRect(
    //         const GrClip& clip, GrPaint&& paint, GrAA aa, const SkMatrix& m, const SkRect& rect,
    //         const GrUserStencilSettings* ss = nullptr) {
    //     fRenderTargetContext->drawFilledRect(clip, std::move(paint), aa, m, rect, ss);
    // }

    SkBudgeted isBudgeted() const;

    int maxWindowRectangles() const;

    /*
     * This unique ID will not change for a given RenderTargetContext. However, it is _NOT_
     * guaranteed to match the uniqueID of the underlying GrRenderTarget - beware!
     */
    GrSurfaceProxy::UniqueID uniqueID() const {
        return fRenderTargetContext->fRenderTargetProxy->uniqueID();
    }

    uint32_t testingOnly_getOpListID();

    using WillAddOpFn = GrRenderTargetContext::WillAddOpFn;
    void testingOnly_addDrawOp(std::unique_ptr<GrDrawOp>);
    void testingOnly_addDrawOp(const GrClip&, std::unique_ptr<GrDrawOp>,
                               const std::function<WillAddOpFn>& = std::function<WillAddOpFn>());

    bool refsWrappedObjects() const {
        return fRenderTargetContext->fRenderTargetProxy->refsWrappedObjects();
    }

private:
    explicit GrRenderTargetContextPriv(GrRenderTargetContext* renderTargetContext)
        : fRenderTargetContext(renderTargetContext) {}
    GrRenderTargetContextPriv(const GrRenderTargetPriv&) {} // unimpl
    GrRenderTargetContextPriv& operator=(const GrRenderTargetPriv&); // unimpl

    // No taking addresses of this type.
    const GrRenderTargetContextPriv* operator&() const;
    GrRenderTargetContextPriv* operator&();

    GrRenderTargetContext* fRenderTargetContext;

    friend class GrRenderTargetContext; // to construct/copy this type.
};

inline GrRenderTargetContextPriv GrRenderTargetContext::priv() {
    return GrRenderTargetContextPriv(this);
}

inline const GrRenderTargetContextPriv GrRenderTargetContext::priv() const {
    return GrRenderTargetContextPriv(const_cast<GrRenderTargetContext*>(this));
}

#endif

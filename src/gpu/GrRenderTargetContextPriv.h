/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetContextPriv_DEFINED
#define GrRenderTargetContextPriv_DEFINED

#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrRenderTargetContext.h"

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
        GrOpsTask* opsTask = fRenderTargetContext->getOpsTask();
        opsTask->fLastClipStackGenID = clipStackGenID;
        opsTask->fLastDevClipBounds = devClipBounds;
        opsTask->fLastClipNumAnalyticFPs = numClipAnalyticFPs;
    }

    // called to determine if we have to render the clip into SB.
    // TODO: remove after clipping overhaul.
    bool mustRenderClip(uint32_t clipStackGenID, const SkIRect& devClipBounds,
                        int numClipAnalyticFPs) const {
        GrOpsTask* opsTask = fRenderTargetContext->getOpsTask();
        return opsTask->fLastClipStackGenID != clipStackGenID ||
               !opsTask->fLastDevClipBounds.contains(devClipBounds) ||
               opsTask->fLastClipNumAnalyticFPs != numClipAnalyticFPs;
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

    // While this can take a general clip, since GrReducedClip relies on this function, it must take
    // care to only provide hard clips or we could get stuck in a loop. The general clip is needed
    // so that path renderers can use this function.
    void stencilRect(
            const GrClip& clip, const GrUserStencilSettings* ss, GrPaint&& paint,
            GrAA doStencilMSAA, const SkMatrix& viewMatrix, const SkRect& rect,
            const SkMatrix* localMatrix = nullptr) {
        // Since this provides stencil settings to drawFilledQuad, it performs a different AA type
        // resolution compared to regular rect draws, which is the main reason it remains separate.
        GrQuad localQuad = localMatrix ? GrQuad::MakeFromRect(rect, *localMatrix) : GrQuad(rect);
        fRenderTargetContext->drawFilledQuad(
                clip, std::move(paint), doStencilMSAA, GrQuadAAFlags::kNone,
                GrQuad::MakeFromRect(rect, viewMatrix), localQuad, ss);
    }

    void stencilPath(
            const GrHardClip&, GrAA doStencilMSAA, const SkMatrix& viewMatrix, sk_sp<const GrPath>);

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

    SkBudgeted isBudgeted() const;

    int maxWindowRectangles() const;

    /*
     * This unique ID will not change for a given RenderTargetContext. However, it is _NOT_
     * guaranteed to match the uniqueID of the underlying GrRenderTarget - beware!
     */
    GrSurfaceProxy::UniqueID uniqueID() const {
        return fRenderTargetContext->fRenderTargetProxy->uniqueID();
    }

    uint32_t testingOnly_getOpsTaskID();

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

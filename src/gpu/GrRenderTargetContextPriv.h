/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetContextPriv_DEFINED
#define GrRenderTargetContextPriv_DEFINED

#include "GrRenderTargetContext.h"
#include "GrRenderTargetOpList.h"
#include "GrPathRendering.h"

class GrFixedClip;
class GrPath;
struct GrUserStencilSettings;

/** Class that adds methods to GrRenderTargetContext that are only intended for use internal to
    Skia. This class is purely a privileged window into GrRenderTargetContext. It should never have
    additional data members or virtual methods. */
class GrRenderTargetContextPriv {
public:
    gr_instanced::InstancedRendering* accessInstancedRendering() const {
        return fRenderTargetContext->getOpList()->instancedRendering();
    }

    // called to note the last clip drawn to the stencil buffer.
    // TODO: remove after clipping overhaul.
    void setLastClip(int32_t clipStackGenID,
                     const SkIRect& clipSpaceRect,
                     const SkIPoint clipOrigin) {
        GrRenderTargetOpList* opList = fRenderTargetContext->getOpList();
        opList->fLastClipStackGenID = clipStackGenID;
        opList->fLastClipStackRect = clipSpaceRect;
        opList->fLastClipOrigin = clipOrigin;
    }

    // called to determine if we have to render the clip into SB.
    // TODO: remove after clipping overhaul.
    bool mustRenderClip(int32_t clipStackGenID,
                        const SkIRect& clipSpaceRect,
                        const SkIPoint& clipOrigin) const {
        GrRenderTargetOpList* opList = fRenderTargetContext->getOpList();
        return opList->fLastClipStackGenID != clipStackGenID ||
               opList->fLastClipOrigin != clipOrigin ||
               !opList->fLastClipStackRect.contains(clipSpaceRect);
    }

    void clear(const GrFixedClip&, const GrColor, bool canIgnoreClip);

    void clearStencilClip(const GrFixedClip&, bool insideStencilMask);

    void stencilRect(const GrClip& clip,
                     const GrUserStencilSettings* ss,
                     bool useHWAA,
                     const SkMatrix& viewMatrix,
                     const SkRect& rect);

    void stencilPath(const GrClip&,
                     bool useHWAA,
                     const SkMatrix& viewMatrix,
                     const GrPath*);

    bool drawAndStencilRect(const GrClip&,
                            const GrUserStencilSettings*,
                            SkRegion::Op op,
                            bool invert,
                            bool doAA,
                            const SkMatrix& viewMatrix,
                            const SkRect&);

    bool drawAndStencilPath(const GrClip&,
                            const GrUserStencilSettings*,
                            SkRegion::Op op,
                            bool invert,
                            bool doAA,
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

    void testingOnly_drawBatch(const GrPaint&,
                               GrDrawBatch* batch,
                               const GrUserStencilSettings* = nullptr,
                               bool snapToCenters = false);

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

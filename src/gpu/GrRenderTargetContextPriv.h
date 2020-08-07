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

class GrHardClip;
class GrPath;
struct GrUserStencilSettings;

/** Class that adds methods to GrRenderTargetContext that are only intended for use internal to
    Skia. This class is purely a privileged window into GrRenderTargetContext. It should never have
    additional data members or virtual methods. */
class GrRenderTargetContextPriv {
public:
    GrRecordingContext* recordingContext() { return fRenderTargetContext->fContext; }
    // called to note the last clip drawn to the stencil buffer.
    // TODO: remove after clipping overhaul.
    void setLastClip(uint32_t clipStackGenID, const SkIRect& devClipBounds,
                     int numClipAnalyticElements) {
        GrOpsTask* opsTask = fRenderTargetContext->getOpsTask();
        opsTask->fLastClipStackGenID = clipStackGenID;
        opsTask->fLastDevClipBounds = devClipBounds;
        opsTask->fLastClipNumAnalyticElements = numClipAnalyticElements;
    }

    // called to determine if we have to render the clip into SB.
    // TODO: remove after clipping overhaul.
    bool mustRenderClip(uint32_t clipStackGenID, const SkIRect& devClipBounds,
                        int numClipAnalyticElements) const {
        GrOpsTask* opsTask = fRenderTargetContext->getOpsTask();
        return opsTask->fLastClipStackGenID != clipStackGenID ||
               !opsTask->fLastDevClipBounds.contains(devClipBounds) ||
               opsTask->fLastClipNumAnalyticElements != numClipAnalyticElements;
    }

    // Clear at minimum the pixels within 'scissor', but is allowed to clear the full render target
    // if that is the more performant option.
    void clearAtLeast(const SkIRect& scissor, const SkPMColor4f& color) {
        fRenderTargetContext->internalClear(&scissor, color, /* upgrade to full */ true);
    }

    void clearStencilClip(const SkIRect& scissor, bool insideStencilMask) {
        fRenderTargetContext->internalStencilClear(&scissor, insideStencilMask);
    }

    // While this can take a general clip, since GrReducedClip relies on this function, it must take
    // care to only provide hard clips or we could get stuck in a loop. The general clip is needed
    // so that path renderers can use this function.
    void stencilRect(
            const GrClip* clip, const GrUserStencilSettings* ss, GrPaint&& paint,
            GrAA doStencilMSAA, const SkMatrix& viewMatrix, const SkRect& rect,
            const SkMatrix* localMatrix = nullptr) {
        // Since this provides stencil settings to drawFilledQuad, it performs a different AA type
        // resolution compared to regular rect draws, which is the main reason it remains separate.
        DrawQuad quad{GrQuad::MakeFromRect(rect, viewMatrix),
                      localMatrix ? GrQuad::MakeFromRect(rect, *localMatrix) : GrQuad(rect),
                      GrQuadAAFlags::kNone};
        fRenderTargetContext->drawFilledQuad(clip, std::move(paint), doStencilMSAA, &quad, ss);
    }

    void stencilPath(
            const GrHardClip*, GrAA doStencilMSAA, const SkMatrix& viewMatrix, sk_sp<const GrPath>);

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

    /*
     * This unique ID will not change for a given RenderTargetContext. However, it is _NOT_
     * guaranteed to match the uniqueID of the underlying GrRenderTarget - beware!
     */
    GrSurfaceProxy::UniqueID uniqueID() const {
        return fRenderTargetContext->asSurfaceProxy()->uniqueID();
    }

    uint32_t testingOnly_getOpsTaskID();

    using WillAddOpFn = GrRenderTargetContext::WillAddOpFn;
    void testingOnly_addDrawOp(std::unique_ptr<GrDrawOp>);
    void testingOnly_addDrawOp(const GrClip*, std::unique_ptr<GrDrawOp>,
                               const std::function<WillAddOpFn>& = std::function<WillAddOpFn>());

    SkGlyphRunListPainter* testingOnly_glyphRunPainter() {
        return &fRenderTargetContext->fGlyphPainter;
    }

    bool refsWrappedObjects() const {
        return fRenderTargetContext->asRenderTargetProxy()->refsWrappedObjects();
    }

    void addDrawOp(const GrClip* clip, std::unique_ptr<GrDrawOp> op) {
        fRenderTargetContext->addDrawOp(clip, std::move(op));
    }

private:
    explicit GrRenderTargetContextPriv(GrRenderTargetContext* renderTargetContext)
        : fRenderTargetContext(renderTargetContext) {}
    GrRenderTargetContextPriv(const GrRenderTargetContextPriv&) = delete;
    GrRenderTargetContextPriv& operator=(const GrRenderTargetContextPriv&) = delete;

    // No taking addresses of this type.
    const GrRenderTargetContextPriv* operator&() const;
    GrRenderTargetContextPriv* operator&();

    GrRenderTargetContext* fRenderTargetContext;

    friend class GrRenderTargetContext; // to construct/copy this type.
};

inline GrRenderTargetContextPriv GrRenderTargetContext::priv() {
    return GrRenderTargetContextPriv(this);
}

inline const GrRenderTargetContextPriv GrRenderTargetContext::priv() const {  // NOLINT(readability-const-return-type)
    return GrRenderTargetContextPriv(const_cast<GrRenderTargetContext*>(this));
}

#endif

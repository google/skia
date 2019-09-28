/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrOpsRenderPass_DEFINED
#define GrOpsRenderPass_DEFINED

#include "include/core/SkDrawable.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/ops/GrDrawOp.h"

class GrOpFlushState;
class GrFixedClip;
class GrGpu;
class GrMesh;
class GrPipeline;
class GrPrimitiveProcessor;
class GrRenderTarget;
class GrSemaphore;
struct SkIRect;
struct SkRect;

/**
 * The GrOpsRenderPass is a series of commands (draws, clears, and discards), which all target the
 * same render target. It is possible that these commands execute immediately (GL), or get buffered
 * up for later execution (Vulkan). GrOps execute into a GrOpsRenderPass.
 */
class GrOpsRenderPass {
public:
    virtual ~GrOpsRenderPass() {}

    virtual void insertEventMarker(const char*) = 0;

    struct LoadAndStoreInfo {
        GrLoadOp    fLoadOp;
        GrStoreOp   fStoreOp;
        SkPMColor4f fClearColor;
    };

    // Load-time clears of the stencil buffer are always to 0 so we don't store
    // an 'fStencilClearValue'
    struct StencilLoadAndStoreInfo {
        GrLoadOp  fLoadOp;
        GrStoreOp fStoreOp;
    };

    virtual void begin() = 0;
    // Signals the end of recording to the GrOpsRenderPass and that it can now be submitted.
    virtual void end() = 0;

    // We pass in an array of meshCount GrMesh to the draw. The backend should loop over each
    // GrMesh object and emit a draw for it. Each draw will use the same GrPipeline and
    // GrPrimitiveProcessor. This may fail if the draw would exceed any resource limits (e.g.
    // number of vertex attributes is too large).
    bool draw(const GrPrimitiveProcessor&,
              const GrPipeline&,
              const GrPipeline::FixedDynamicState*,
              const GrPipeline::DynamicStateArrays*,
              const GrMesh[],
              int meshCount,
              const SkRect& bounds);

    // Performs an upload of vertex data in the middle of a set of a set of draws
    virtual void inlineUpload(GrOpFlushState*, GrDeferredTextureUploadFn&) = 0;

    /**
     * Clear the owned render target. Ignores the draw state and clip.
     */
    void clear(const GrFixedClip&, const SkPMColor4f&);

    void clearStencilClip(const GrFixedClip&, bool insideStencilMask);

    /**
     * Executes the SkDrawable object for the underlying backend.
     */
    virtual void executeDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>) {}

protected:
    GrOpsRenderPass() : fOrigin(kTopLeft_GrSurfaceOrigin), fRenderTarget(nullptr) {}

    GrOpsRenderPass(GrRenderTarget* rt, GrSurfaceOrigin origin)
            : fOrigin(origin)
            , fRenderTarget(rt) {
    }

    void set(GrRenderTarget* rt, GrSurfaceOrigin origin) {
        SkASSERT(!fRenderTarget);

        fRenderTarget = rt;
        fOrigin = origin;
    }

    GrSurfaceOrigin fOrigin;
    GrRenderTarget* fRenderTarget;

private:
    virtual GrGpu* gpu() = 0;

    // overridden by backend-specific derived class to perform the draw call.
    virtual void onDraw(const GrPrimitiveProcessor&,
                        const GrPipeline&,
                        const GrPipeline::FixedDynamicState*,
                        const GrPipeline::DynamicStateArrays*,
                        const GrMesh[],
                        int meshCount,
                        const SkRect& bounds) = 0;

    // overridden by backend-specific derived class to perform the clear.
    virtual void onClear(const GrFixedClip&, const SkPMColor4f&) = 0;

    virtual void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) = 0;

    typedef GrOpsRenderPass INHERITED;
};

#endif

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
class GrProgramInfo;
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

    // Updates the internal pipeline state for drawing with the provided GrProgramInfo.
    // Enters an internal "bad" state if the pipeline could not be set.
    void bindPipeline(const GrProgramInfo&, const SkRect& drawBounds);

    // The scissor rect is always dynamic state and therefore not stored on GrPipeline. If scissor
    // test is enabled on the current pipeline, then the client must call setScissorRect() before
    // drawing. The scissor rect may also be updated between draws without having to bind a new
    // pipeline.
    void setScissorRect(const SkIRect&);

    // Texture bindings are dynamic state and therefore not set during bindPipeline(). If the
    // current program uses textures, then the client must call bindTextures() before drawing.
    // The primitive processor textures may also be updated between draws by calling bindTextures()
    // again with a different array for primProcTextures. (On subsequent calls, if the backend is
    // capable of updating the primitive processor textures independently, then it will
    // automatically skip binding textures from GrPipeline.)
    void bindTextures(const GrPrimitiveProcessor&, const GrPipeline&,
                      const GrSurfaceProxy* const primProcTextures[]);

    // Draws the given array of meshes using the current pipeline state. The client must call
    // bindPipeline() before using this method.
    //
    // NOTE: This method will soon be deleted. While it continues to exist, it takes care of calling
    // setScissor() and bindTextures() on the client's behalf.
    void drawMeshes(const GrProgramInfo&, const GrMesh[], int meshCount);

    // Draws the given mesh using the current pipeline state. The client must call bindPipeline(),
    // followed setScissor() and/or bindTextures() if necessary, before using this method.
    void drawMesh(GrPrimitiveType, const GrMesh&);

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
    void executeDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>);

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

    // overridden by backend-specific derived class to perform the rendering command.
    virtual bool onBindPipeline(const GrProgramInfo&, const SkRect& drawBounds) = 0;
    virtual void onSetScissorRect(const SkIRect&) = 0;
    virtual bool onBindTextures(const GrPrimitiveProcessor&, const GrPipeline&,
                                const GrSurfaceProxy* const primProcTextures[] = nullptr) = 0;
    virtual void onDrawMesh(GrPrimitiveType, const GrMesh&) = 0;
    virtual void onClear(const GrFixedClip&, const SkPMColor4f&) = 0;
    virtual void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) = 0;
    virtual void onExecuteDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>) {}

    enum class DrawPipelineStatus {
        kOk = 0,
        kNotConfigured,
        kFailedToBind
    };

    DrawPipelineStatus fDrawPipelineStatus = DrawPipelineStatus::kNotConfigured;
    GrXferBarrierType fXferBarrierType;

#ifdef SK_DEBUG
    enum class DynamicStateStatus {
        kDisabled,
        kUninitialized,
        kConfigured
    };

    DynamicStateStatus fScissorStatus = DynamicStateStatus::kDisabled;
    DynamicStateStatus fTextureBindingStatus = DynamicStateStatus::kDisabled;
    bool fHasVertexAttributes = false;
    bool fHasInstanceAttributes = false;
#endif

    typedef GrOpsRenderPass INHERITED;
};

#endif

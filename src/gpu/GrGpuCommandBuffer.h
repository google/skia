/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGpuCommandBuffer_DEFINED
#define GrGpuCommandBuffer_DEFINED

#include "GrColor.h"
#include "GrPipeline.h"
#include "ops/GrDrawOp.h"

class GrOpFlushState;
class GrFixedClip;
class GrGpu;
class GrMesh;
class GrPipeline;
class GrPrimitiveProcessor;
class GrRenderTarget;
struct SkIRect;
struct SkRect;

class GrGpuRTCommandBuffer;

class GrGpuCommandBuffer {
public:
    virtual ~GrGpuCommandBuffer() {}

    // Copy src into current surface owned by either a GrGpuTextureCommandBuffer or
    // GrGpuRenderTargetCommandBuffer.
    virtual void copy(GrSurface* src, GrSurfaceOrigin srcOrigin,
                      const SkIRect& srcRect, const SkIPoint& dstPoint) = 0;

    virtual void insertEventMarker(const char*) = 0;

    virtual GrGpuRTCommandBuffer* asRTCommandBuffer() { return nullptr; }

    // Sends the command buffer off to the GPU object to execute the commands built up in the
    // buffer. The gpu object is allowed to defer execution of the commands until it is flushed.
    virtual void submit() = 0;

protected:
    GrGpuCommandBuffer(GrSurfaceOrigin origin) : fOrigin(origin) {}

    GrSurfaceOrigin fOrigin;
};

class GrGpuTextureCommandBuffer : public GrGpuCommandBuffer{
public:
    virtual ~GrGpuTextureCommandBuffer() {}

    virtual void submit() = 0;

protected:
    GrGpuTextureCommandBuffer(GrTexture* texture, GrSurfaceOrigin origin)
            : INHERITED(origin)
            , fTexture(texture) {}

    GrTexture* fTexture;

private:
    typedef GrGpuCommandBuffer INHERITED;
};

/**
 * The GrGpuRenderTargetCommandBuffer is a series of commands (draws, clears, and discards), which
 * all target the same render target. It is possible that these commands execute immediately (GL),
 * or get buffered up for later execution (Vulkan). GrOps will execute their draw commands into a
 * GrGpuCommandBuffer.
 */
class GrGpuRTCommandBuffer : public GrGpuCommandBuffer {
public:
    struct LoadAndStoreInfo {
        GrLoadOp  fLoadOp;
        GrStoreOp fStoreOp;
        GrColor   fClearColor;
    };

    // Load-time clears of the stencil buffer are always to 0 so we don't store
    // an 'fStencilClearValue'
    struct StencilLoadAndStoreInfo {
        GrLoadOp  fLoadOp;
        GrStoreOp fStoreOp;
    };

    virtual ~GrGpuRTCommandBuffer() {}

    GrGpuRTCommandBuffer* asRTCommandBuffer() { return this; }

    virtual void begin() = 0;
    // Signals the end of recording to the command buffer and that it can now be submitted.
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
    void clear(const GrFixedClip&, GrColor);

    void clearStencilClip(const GrFixedClip&, bool insideStencilMask);

    /**
     * Discards the contents render target.
     */
    // TODO: This should be removed in the future to favor using the load and store ops for discard
    virtual void discard() = 0;

protected:
    GrGpuRTCommandBuffer(GrRenderTarget* rt, GrSurfaceOrigin origin)
            : INHERITED(origin)
            , fRenderTarget(rt) {
    }

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
    virtual void onClear(const GrFixedClip&, GrColor) = 0;

    virtual void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) = 0;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif

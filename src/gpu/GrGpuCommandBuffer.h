/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGpuCommandBuffer_DEFINED
#define GrGpuCommandBuffer_DEFINED

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

class GrGpuRTCommandBuffer;

class GrGpuCommandBuffer {
public:
    virtual ~GrGpuCommandBuffer() {}

    // Copy src into current surface owned by either a GrGpuTextureCommandBuffer or
    // GrGpuRenderTargetCommandBuffer. The srcRect and dstPoint must be in dst coords and have
    // already been adjusted for any origin flips.
    virtual void copy(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) = 0;

    virtual void insertEventMarker(const char*) = 0;

    virtual GrGpuRTCommandBuffer* asRTCommandBuffer() { return nullptr; }
};

class GrGpuTextureCommandBuffer : public GrGpuCommandBuffer{
public:
    void set(GrTexture* texture, GrSurfaceOrigin origin) {
        SkASSERT(!fTexture);

        fTexture = texture;
    }

protected:
    GrGpuTextureCommandBuffer() : fTexture(nullptr) {}

    GrGpuTextureCommandBuffer(GrTexture* texture, GrSurfaceOrigin origin) : fTexture(texture) {}

    GrTexture*      fTexture;

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
    void clear(const GrFixedClip&, const SkPMColor4f&);

    void clearStencilClip(const GrFixedClip&, bool insideStencilMask);

    /**
     * Executes the SkDrawable object for the underlying backend.
     */
    virtual void executeDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>) {}

protected:
    GrGpuRTCommandBuffer() : fOrigin(kTopLeft_GrSurfaceOrigin), fRenderTarget(nullptr) {}

    GrGpuRTCommandBuffer(GrRenderTarget* rt, GrSurfaceOrigin origin)
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

    typedef GrGpuCommandBuffer INHERITED;
};

#endif

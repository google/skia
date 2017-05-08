/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGpuCommandBuffer_DEFINED
#define GrGpuCommandBuffer_DEFINED

#include "GrColor.h"
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

/**
 * The GrGpuCommandBuffer is a series of commands (draws, clears, and discards), which all target
 * the same render target. It is possible that these commands execute immediately (GL), or get
 * buffered up for later execution (Vulkan). GrOps will execute their draw commands into a
 * GrGpuCommandBuffer.
 *
 * Ideally we'd know the GrRenderTarget, or at least its properties when the GrGpuCommandBuffer, is
 * created. We also then wouldn't include it in the GrPipeline or as a parameter to the clear and
 * discard methods. The logical place for that will be in GrRenderTargetOpList post-MDB. For now
 * the render target is redundantly passed to each operation, though it will always be the same
 * render target for a given command buffer even pre-MDB.
 */
class GrGpuCommandBuffer {
public:
    enum class LoadOp {
        kLoad,
        kClear,
        kDiscard,
    };

    enum class StoreOp {
        kStore,
        kDiscard,
    };

    struct LoadAndStoreInfo {
        LoadOp  fLoadOp;
        StoreOp fStoreOp;
        GrColor fClearColor;
    };

    GrGpuCommandBuffer() {}
    virtual ~GrGpuCommandBuffer() {}

    // Signals the end of recording to the command buffer and that it can now be submitted.
    virtual void end() = 0;

    // Sends the command buffer off to the GPU object to execute the commands built up in the
    // buffer. The gpu object is allowed to defer execution of the commands until it is flushed.
    void submit();

    // We pass in an array of meshCount GrMesh to the draw. The backend should loop over each
    // GrMesh object and emit a draw for it. Each draw will use the same GrPipeline and
    // GrPrimitiveProcessor. This may fail if the draw would exceed any resource limits (e.g.
    // number of vertex attributes is too large).
    bool draw(const GrPipeline&,
              const GrPrimitiveProcessor&,
              const GrMesh*,
              int meshCount,
              const SkRect& bounds);

    // Performs an upload of vertex data in the middle of a set of a set of draws
    virtual void inlineUpload(GrOpFlushState* state, GrDrawOp::DeferredUploadFn& upload,
                              GrRenderTarget* rt) = 0;

    /**
     * Clear the passed in render target. Ignores the draw state and clip.
     */
    void clear(GrRenderTarget*, const GrFixedClip&, GrColor);

    void clearStencilClip(GrRenderTarget*, const GrFixedClip&, bool insideStencilMask);

    /**
     * Discards the contents render target. nullptr indicates that the current render target should
     * be discarded.
     */
    // TODO: This should be removed in the future to favor using the load and store ops for discard
    virtual void discard(GrRenderTarget*) = 0;

private:
    virtual GrGpu* gpu() = 0;
    virtual GrRenderTarget* renderTarget() = 0;

    virtual void onSubmit() = 0;

    // overridden by backend-specific derived class to perform the draw call.
    virtual void onDraw(const GrPipeline&,
                        const GrPrimitiveProcessor&,
                        const GrMesh*,
                        int meshCount,
                        const SkRect& bounds) = 0;

    // overridden by backend-specific derived class to perform the clear.
    virtual void onClear(GrRenderTarget*, const GrFixedClip&, GrColor) = 0;

    virtual void onClearStencilClip(GrRenderTarget*, const GrFixedClip&,
                                    bool insideStencilMask) = 0;

};

#endif

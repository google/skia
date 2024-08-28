/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrOpsRenderPass_DEFINED
#define GrOpsRenderPass_DEFINED

#include "include/core/SkDrawable.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrXferProcessor.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

class GrGeometryProcessor;
class GrGpu;
class GrOpFlushState;
class GrPipeline;
class GrProgramInfo;
class GrRenderTarget;
class GrScissorState;
class GrSurfaceProxy;
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
        GrLoadOp             fLoadOp;
        GrStoreOp            fStoreOp;
        std::array<float, 4> fClearColor;
    };

    // Load-time clears of the stencil buffer are always to 0 so we don't store
    // an 'fStencilClearValue'
    struct StencilLoadAndStoreInfo {
        GrLoadOp  fLoadOp;
        GrStoreOp fStoreOp;
    };

    void begin();
    // Signals the end of recording to the GrOpsRenderPass and that it can now be submitted.
    void end();

    // Updates the internal pipeline state for drawing with the provided GrProgramInfo. Enters an
    // internal "bad" state if the pipeline could not be set.
    void bindPipeline(const GrProgramInfo&, const SkRect& drawBounds);

    // The scissor rect is always dynamic state and therefore not stored on GrPipeline. If scissor
    // test is enabled on the current pipeline, then the client must call setScissorRect() before
    // drawing. The scissor rect may also be updated between draws without having to bind a new
    // pipeline.
    void setScissorRect(const SkIRect&);

    // Binds textures for the primitive processor and any FP on the GrPipeline. Texture bindings are
    // dynamic state and therefore not set during bindPipeline(). If the current program uses
    // textures, then the client must call bindTextures() before drawing. The primitive processor
    // textures may also be updated between draws by calling bindTextures() again with a different
    // array for primProcTextures. (On subsequent calls, if the backend is capable of updating the
    // primitive processor textures independently, then it will automatically skip re-binding
    // FP textures from GrPipeline.)
    //
    // If the current program does not use textures, this is a no-op.
    void bindTextures(const GrGeometryProcessor&,
                      const GrSurfaceProxy* const geomProcTextures[],
                      const GrPipeline&);

    void bindBuffers(sk_sp<const GrBuffer> indexBuffer, sk_sp<const GrBuffer> instanceBuffer,
                     sk_sp<const GrBuffer> vertexBuffer, GrPrimitiveRestart = GrPrimitiveRestart::kNo);

    // The next several draw*() methods issue draws using the current pipeline state. Before
    // drawing, the caller must configure the pipeline and dynamic state:
    //
    //   - Call bindPipeline()
    //   - If the scissor test is enabled, call setScissorRect()
    //   - If the current program uses textures, call bindTextures()
    //   - Call bindBuffers() (even if all buffers are null)
    void draw(int vertexCount, int baseVertex);
    void drawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue, uint16_t maxIndexValue,
                     int baseVertex);

    // Requires caps.drawInstancedSupport().
    void drawInstanced(int instanceCount, int baseInstance, int vertexCount, int baseVertex);

    // Requires caps.drawInstancedSupport().
    void drawIndexedInstanced(int indexCount, int baseIndex, int instanceCount, int baseInstance,
                              int baseVertex);

    // Executes multiple draws from an array of GrDrawIndirectCommand in the provided buffer.
    //
    // Requires caps.drawInstancedSupport().
    //
    // If caps.nativeDrawIndirectSupport() is unavailable, then 'drawIndirectBuffer' must be a
    // GrCpuBuffer in order to polyfill. Performance may suffer in this scenario.
    void drawIndirect(const GrBuffer* drawIndirectBuffer, size_t bufferOffset, int drawCount);

    // Executes multiple draws from an array of GrDrawIndexedIndirectCommand in the provided buffer.
    //
    // Requires caps.drawInstancedSupport().
    //
    // If caps.nativeDrawIndirectSupport() is unavailable, then 'drawIndirectBuffer' must be a
    // GrCpuBuffer in order to polyfill. Performance may suffer in this scenario.
    void drawIndexedIndirect(const GrBuffer* drawIndirectBuffer, size_t bufferOffset,
                             int drawCount);

    // This is a helper method for drawing a repeating pattern of vertices. The bound index buffer
    // is understood to contain 'maxPatternRepetitionsInIndexBuffer' repetitions of the pattern.
    // If more repetitions are required, then we loop.
    void drawIndexPattern(int patternIndexCount, int patternRepeatCount,
                          int maxPatternRepetitionsInIndexBuffer, int patternVertexCount,
                          int baseVertex);

    // Performs an upload of vertex data in the middle of a set of a set of draws
    virtual void inlineUpload(GrOpFlushState*, GrDeferredTextureUploadFn&) = 0;

    /**
     * Clear the owned render target. Clears the full target if 'scissor' is disabled, otherwise it
     * is restricted to 'scissor'. Must check caps.performPartialClearsAsDraws() before using an
     * enabled scissor test; must check caps.performColorClearsAsDraws() before using this at all.
     */
    void clear(const GrScissorState& scissor, std::array<float, 4> color);

    /**
     * Same as clear() but modifies the stencil; check caps.performStencilClearsAsDraws() and
     * caps.performPartialClearsAsDraws().
     */
    void clearStencilClip(const GrScissorState& scissor, bool insideStencilMask);

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

    // Backends may defer binding of certain buffers if their draw API requires a buffer, or if
    // their bind methods don't support base values.
    sk_sp<const GrBuffer> fActiveIndexBuffer;
    sk_sp<const GrBuffer> fActiveVertexBuffer;
    sk_sp<const GrBuffer> fActiveInstanceBuffer;

private:
    virtual GrGpu* gpu() = 0;

    void resetActiveBuffers() {
        fActiveIndexBuffer.reset();
        fActiveInstanceBuffer.reset();
        fActiveVertexBuffer.reset();
    }

    bool prepareToDraw();

    // overridden by backend-specific derived class to perform the rendering command.
    virtual void onBegin() {}
    virtual void onEnd() {}
    virtual bool onBindPipeline(const GrProgramInfo&, const SkRect& drawBounds) = 0;
    virtual void onSetScissorRect(const SkIRect&) = 0;
    virtual bool onBindTextures(const GrGeometryProcessor&,
                                const GrSurfaceProxy* const geomProcTextures[],
                                const GrPipeline&) = 0;
    virtual void onBindBuffers(sk_sp<const GrBuffer> indexBuffer, sk_sp<const GrBuffer> instanceBuffer,
                               sk_sp<const GrBuffer> vertexBuffer, GrPrimitiveRestart) = 0;
    virtual void onDraw(int vertexCount, int baseVertex) = 0;
    virtual void onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                               uint16_t maxIndexValue, int baseVertex) = 0;
    virtual void onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                                 int baseVertex) = 0;
    virtual void onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount,
                                        int baseInstance, int baseVertex) = 0;
    virtual void onDrawIndirect(const GrBuffer*, size_t offset, int drawCount) {
        SK_ABORT("Not implemented.");  // Only called if caps.nativeDrawIndirectSupport().
    }
    virtual void onDrawIndexedIndirect(const GrBuffer*, size_t offset, int drawCount) {
        SK_ABORT("Not implemented.");  // Only called if caps.nativeDrawIndirectSupport().
    }
    virtual void onClear(const GrScissorState&, std::array<float, 4> color) = 0;
    virtual void onClearStencilClip(const GrScissorState&, bool insideStencilMask) = 0;
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

    DynamicStateStatus fScissorStatus;
    DynamicStateStatus fTextureBindingStatus;
    bool fHasIndexBuffer;
    DynamicStateStatus fInstanceBufferStatus;
    DynamicStateStatus fVertexBufferStatus;
#endif

    using INHERITED = GrOpsRenderPass;
};

#endif

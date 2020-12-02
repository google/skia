/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLOpsRenderPass_DEFINED
#define GrGLOpsRenderPass_DEFINED

#include "src/gpu/GrOpsRenderPass.h"

#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLRenderTarget.h"

class GrGLGpu;
class GrGLRenderTarget;

class GrGLOpsRenderPass : public GrOpsRenderPass {
/**
 * We do not actually buffer up draws or do any work in the this class for GL. Instead commands
 * are immediately sent to the gpu to execute. Thus all the commands in this class are simply
 * pass through functions to corresponding calls in the GrGLGpu class.
 */
public:
    GrGLOpsRenderPass(GrGLGpu* gpu) : fGpu(gpu) {}

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override {
        state->doUpload(upload);
    }

    void set(GrRenderTarget*, const SkIRect& contentBounds, GrSurfaceOrigin,
             const LoadAndStoreInfo&, const StencilLoadAndStoreInfo&);

    void reset() {
        fRenderTarget = nullptr;
    }

private:
    GrGpu* gpu() override { return fGpu; }

    void bindInstanceBuffer(const GrBuffer*, int baseInstance);
    void bindVertexBuffer(const GrBuffer*, int baseVertex);

    const void* offsetForBaseIndex(int baseIndex) const {
        if (!fIndexPointer) {
            // nullptr != 0. Adding an offset to a nullptr is undefined.
            return (void*)(baseIndex * sizeof(uint16_t));
        }
        return fIndexPointer + baseIndex;
    }

    void onBegin() override;
    void onEnd() override;
    bool onBindPipeline(const GrProgramInfo& programInfo, const SkRect& drawBounds) override;
    void onSetScissorRect(const SkIRect& scissor) override;
    bool onBindTextures(const GrPrimitiveProcessor&, const GrSurfaceProxy* const primProcTextures[],
                        const GrPipeline& pipeline) override;
    void onBindBuffers(sk_sp<const GrBuffer> indexBuffer, sk_sp<const GrBuffer> instanceBuffer,
                       sk_sp<const GrBuffer> vertexBuffer, GrPrimitiveRestart) override;
    void onDraw(int vertexCount, int baseVertex) override;
    void onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                       uint16_t maxIndexValue, int baseVertex) override;
    void onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                         int baseVertex) override;
    void onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount, int baseInstance,
                                int baseVertex) override;
    void onDrawIndirect(const GrBuffer* drawIndirectBuffer, size_t offset, int drawCount) override;
    void multiDrawArraysANGLEOrWebGL(const GrBuffer* drawIndirectBuffer, size_t offset,
                                     int drawCount);
    void onDrawIndexedIndirect(const GrBuffer* drawIndirectBuffer, size_t offset,
                               int drawCount) override;
    void multiDrawElementsANGLEOrWebGL(const GrBuffer* drawIndirectBuffer, size_t offset,
                                       int drawCount);
    void onClear(const GrScissorState& scissor, std::array<float, 4> color) override;
    void onClearStencilClip(const GrScissorState& scissor, bool insideStencilMask) override;

    GrGLGpu* fGpu;
    SkIRect fContentBounds;
    LoadAndStoreInfo fColorLoadAndStoreInfo;
    StencilLoadAndStoreInfo fStencilLoadAndStoreInfo;

    // Per-pipeline state.
    GrPrimitiveType fPrimitiveType;
    GrGLAttribArrayState* fAttribArrayState = nullptr;

    // If using an index buffer, this gets set during onBindBuffers. It is either the CPU address of
    // the indices, or nullptr if they reside physically in GPU memory.
    const uint16_t* fIndexPointer;

    // This tracks whether or not we bound the respective buffers during the bindBuffers call.
    SkDEBUGCODE(bool fDidBindVertexBuffer = false;)
    SkDEBUGCODE(bool fDidBindInstanceBuffer = false;)

    using INHERITED = GrOpsRenderPass;
};

#endif

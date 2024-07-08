/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrGLOpsRenderPass_DEFINED
#define GrGLOpsRenderPass_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrNativeRect.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrOpsRenderPass.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"

#include <array>
#include <cstddef>
#include <cstdint>

class GrBuffer;
class GrGLAttribArrayState;
class GrGpu;
class GrPipeline;
class GrProgramInfo;
class GrRenderTarget;
class GrScissorState;
class GrSurfaceProxy;
enum GrSurfaceOrigin : int;
enum class GrPrimitiveRestart : bool;
enum class GrPrimitiveType : uint8_t;

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

    void set(GrRenderTarget*, bool useMSAASurface, const SkIRect& contentBounds, GrSurfaceOrigin,
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

    // Ideally we load and store DMSAA only within the content bounds of our render pass, but if
    // the caps don't allow for partial framebuffer blits, we resolve the full target.
    // We resolve the same bounds during load and store both because if we have to do a full size
    // resolve at the end, the full DMSAA attachment needs to have valid content.
    GrNativeRect dmsaaLoadStoreBounds() const;

    void onBegin() override;
    void onEnd() override;
    bool onBindPipeline(const GrProgramInfo& programInfo, const SkRect& drawBounds) override;
    void onSetScissorRect(const SkIRect& scissor) override;
    bool onBindTextures(const GrGeometryProcessor&,
                        const GrSurfaceProxy* const geomProcTextures[],
                        const GrPipeline&) override;
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

    GrGLGpu* const fGpu;

    bool fUseMultisampleFBO;
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

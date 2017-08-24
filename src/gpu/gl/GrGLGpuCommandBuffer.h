/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGLGpuCommandBuffer_DEFINED
#define GrGLGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"

#include "GrGLGpu.h"
#include "GrGLRenderTarget.h"
#include "GrOpFlushState.h"

class GrGLGpu;
class GrGLRenderTarget;

class GrGLGpuTextureCommandBuffer : public GrGpuTextureCommandBuffer {
public:
    GrGLGpuTextureCommandBuffer(GrGLGpu* gpu, GrTexture* texture, GrSurfaceOrigin origin)
        : INHERITED(texture, origin)
        , fGpu(gpu) {
    }

    ~GrGLGpuTextureCommandBuffer() override {}

    void submit() override {}

    void copy(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override {
        fGpu->copySurface(fTexture, src, srcRect, dstPoint);
    }

    void insertEventMarker(const char* msg) override {
        fGpu->insertEventMarker(msg);
    }

private:
    GrGLGpu*                    fGpu;

    typedef GrGpuTextureCommandBuffer INHERITED;
};

class GrGLGpuRTCommandBuffer : public GrGpuRTCommandBuffer {
/**
 * We do not actually buffer up draws or do any work in the this class for GL. Instead commands
 * are immediately sent to the gpu to execute. Thus all the commands in this class are simply
 * pass through functions to corresponding calls in the GrGLGpu class.
 */
public:
    GrGLGpuRTCommandBuffer(GrGLGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                           const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo)
            : INHERITED(rt, origin)
            , fGpu(gpu) {
        fClearSB = LoadOp::kClear == stencilInfo.fLoadOp;
    }

    ~GrGLGpuRTCommandBuffer() override {}

    void begin() override {
        if (fClearSB) {
            fGpu->clearStencil(fRenderTarget, 0x0);
        }
    }
    void end() override {}

    void discard() override { }

    void insertEventMarker(const char* msg) override {
        fGpu->insertEventMarker(msg);
    }

    void inlineUpload(GrOpFlushState* state, GrDrawOp::DeferredUploadFn& upload) override {
        state->doUpload(upload);
    }

    void copy(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override {
        fGpu->copySurface(fRenderTarget, src, srcRect, dstPoint);
    }

    void submit() override {}

private:
    GrGpu* gpu() override { return fGpu; }

    void onDraw(const GrPipeline& pipeline,
                const GrPrimitiveProcessor& primProc,
                const GrMesh mesh[],
                const GrPipeline::DynamicState dynamicStates[],
                int meshCount,
                const SkRect& bounds) override {
        SkASSERT(pipeline.renderTarget() == fRenderTarget);
        fGpu->draw(pipeline, primProc, mesh, dynamicStates, meshCount);
    }

    void onClear(const GrFixedClip& clip, GrColor color) override {
        fGpu->clear(clip, color, fRenderTarget, fOrigin);
    }

    void onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) override {
        fGpu->clearStencilClip(clip, insideStencilMask, fRenderTarget, fOrigin);
    }

    GrGLGpu*                    fGpu;
    bool                        fClearSB;

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif


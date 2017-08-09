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

class GrGLGpuCommandBuffer : public GrGpuCommandBuffer {
/**
 * We do not actually buffer up draws or do any work in the this class for GL. Instead commands
 * are immediately sent to the gpu to execute. Thus all the commands in this class are simply
 * pass through functions to corresponding calls in the GrGLGpu class.
 */
public:
    GrGLGpuCommandBuffer(GrGLGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                         const GrGpuCommandBuffer::LoadAndStoreInfo& colorInfo,
                         const GrGpuCommandBuffer::StencilLoadAndStoreInfo& stencilInfo)
            : INHERITED(rt, origin)
            , fGpu(gpu)
            , fColorLoadAndStoreInfo(colorInfo)
            , fStencilLoadAndStoreInfo(stencilInfo) {
    }

    ~GrGLGpuCommandBuffer() override {}

    void begin() override {
        if (GrLoadOp::kClear == fColorLoadAndStoreInfo.fLoadOp) {
            fGpu->clear(GrFixedClip::Disabled(), fColorLoadAndStoreInfo.fClearColor,
                        fRenderTarget, fOrigin);
        }
        if (GrLoadOp::kClear == fStencilLoadAndStoreInfo.fLoadOp) {
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

private:
    GrGpu* gpu() override { return fGpu; }

    void onSubmit() override {}

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

    GrGLGpu*                                    fGpu;
    GrGpuCommandBuffer::LoadAndStoreInfo        fColorLoadAndStoreInfo;
    GrGpuCommandBuffer::StencilLoadAndStoreInfo fStencilLoadAndStoreInfo;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif


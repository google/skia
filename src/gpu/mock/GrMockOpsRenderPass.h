/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockOpsRenderPass_DEFINED
#define GrMockOpsRenderPass_DEFINED

#include "src/gpu/GrOpsRenderPass.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/mock/GrMockGpu.h"

class GrMockOpsRenderPass : public GrOpsRenderPass {
public:
    GrMockOpsRenderPass(GrMockGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                        LoadAndStoreInfo colorInfo)
            : INHERITED(rt, origin)
            , fGpu(gpu)
            , fColorLoadOp(colorInfo.fLoadOp) {
    }

    GrGpu* gpu() override { return fGpu; }
    void inlineUpload(GrOpFlushState*, GrDeferredTextureUploadFn&) override {}
    void begin() override {
        if (GrLoadOp::kClear == fColorLoadOp) {
            this->markRenderTargetDirty();
        }
    }
    void end() override {}

    int numDraws() const { return fNumDraws; }

private:
    bool onBindPipeline(const GrProgramInfo&, const SkRect&) override {
        return true;
    }
    void onSetScissorRect(const SkIRect&) override {}
    bool onBindTextures(const GrPrimitiveProcessor&, const GrPipeline&,
                        const GrSurfaceProxy* const primProcTextures[]) override {
        return true;
    }
    void onDraw(const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) override {
        this->dummyDraw();
    }
    void onDrawIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                       GrPrimitiveRestart, uint16_t minIndexValue, uint16_t maxIndexValue,
                       const GrBuffer* vertexBuffer, int baseVertex) override {
        this->dummyDraw();
    }
    void onDrawInstanced(const GrBuffer* instanceBuffer, int instanceCount, int baseInstance,
                         const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) override {
        this->dummyDraw();
    }
    void onDrawIndexedInstanced(const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                                GrPrimitiveRestart, const GrBuffer* instanceBuffer,
                                int instanceCount, int baseInstance, const GrBuffer* vertexBuffer,
                                int baseVertex) override {
        this->dummyDraw();
    }
    void onClear(const GrFixedClip&, const SkPMColor4f&) override {
        this->markRenderTargetDirty();
    }
    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override {}
    void dummyDraw() {
        this->markRenderTargetDirty();
        ++fNumDraws;
    }
    void markRenderTargetDirty() {
        if (auto* tex = fRenderTarget->asTexture()) {
            tex->texturePriv().markMipMapsDirty();
        }
    }

    GrMockGpu* fGpu;
    GrLoadOp fColorLoadOp;
    int fNumDraws = 0;

    typedef GrOpsRenderPass INHERITED;
};

#endif

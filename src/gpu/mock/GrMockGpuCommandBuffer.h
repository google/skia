/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockGpuCommandBuffer_DEFINED
#define GrMockGpuCommandBuffer_DEFINED

#include "src/gpu/GrGpuCommandBuffer.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/mock/GrMockGpu.h"

class GrMockGpuTextureCommandBuffer : public GrGpuTextureCommandBuffer {
public:
    GrMockGpuTextureCommandBuffer(GrTexture* texture, GrSurfaceOrigin origin)
        : INHERITED(texture, origin) {
    }

    ~GrMockGpuTextureCommandBuffer() override {}

    void copy(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override {}
    void insertEventMarker(const char*) override {}

private:
    typedef GrGpuTextureCommandBuffer INHERITED;
};

class GrMockGpuRTCommandBuffer : public GrGpuRTCommandBuffer {
public:
    GrMockGpuRTCommandBuffer(GrMockGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                             LoadAndStoreInfo colorInfo)
            : INHERITED(rt, origin)
            , fGpu(gpu)
            , fColorLoadOp(colorInfo.fLoadOp) {
    }

    GrGpu* gpu() override { return fGpu; }
    void inlineUpload(GrOpFlushState*, GrDeferredTextureUploadFn&) override {}
    void insertEventMarker(const char*) override {}
    void begin() override {
        if (GrLoadOp::kClear == fColorLoadOp) {
            this->markRenderTargetDirty();
        }
    }
    void end() override {}
    void copy(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) override {
        this->markRenderTargetDirty();
    }

    int numDraws() const { return fNumDraws; }

private:
    void onDraw(const GrPrimitiveProcessor&, const GrPipeline&,
                const GrPipeline::FixedDynamicState*, const GrPipeline::DynamicStateArrays*,
                const GrMesh[], int meshCount, const SkRect& bounds) override {
        this->markRenderTargetDirty();
        ++fNumDraws;
    }
    void onClear(const GrFixedClip&, const SkPMColor4f&) override {
        this->markRenderTargetDirty();
    }
    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override {}

    void markRenderTargetDirty() {
        if (auto* tex = fRenderTarget->asTexture()) {
            tex->texturePriv().markMipMapsDirty();
        }
        fRenderTarget->flagAsNeedingResolve();
    }

    GrMockGpu* fGpu;
    GrLoadOp fColorLoadOp;
    int fNumDraws = 0;

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif

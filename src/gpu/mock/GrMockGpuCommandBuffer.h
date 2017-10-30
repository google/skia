/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockGpuCommandBuffer_DEFINED
#define GrMockGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"
#include "GrMockGpu.h"

class GrMockGpuTextureCommandBuffer : public GrGpuTextureCommandBuffer {
public:
    GrMockGpuTextureCommandBuffer(GrTexture* texture, GrSurfaceOrigin origin)
        : INHERITED(texture, origin) {
    }

    ~GrMockGpuTextureCommandBuffer() override {}

    void copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
              const SkIPoint& dstPoint) override {}
    void insertEventMarker(const char*) override {}

private:
    void submit() override {}

    typedef GrGpuTextureCommandBuffer INHERITED;
};

class GrMockGpuRTCommandBuffer : public GrGpuRTCommandBuffer {
public:
    GrMockGpuRTCommandBuffer(GrMockGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin)
            : INHERITED(rt, origin)
            , fGpu(gpu) {
    }

    GrGpu* gpu() override { return fGpu; }
    void inlineUpload(GrOpFlushState*, GrDeferredTextureUploadFn&) override {}
    void discard() override {}
    void insertEventMarker(const char*) override {}
    void begin() override {}
    void end() override {}
    void copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
              const SkIPoint& dstPoint) override {}

    int numDraws() const { return fNumDraws; }

    void submit() override { fGpu->submitCommandBuffer(this); }

private:
    void onDraw(const GrPipeline&, const GrPrimitiveProcessor&, const GrMesh[],
                const GrPipeline::DynamicState[], int meshCount, const SkRect& bounds) override {
        ++fNumDraws;
    }
    void onClear(const GrFixedClip&, GrColor) override {}
    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override {}

    GrMockGpu* fGpu;
    int fNumDraws = 0;

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif

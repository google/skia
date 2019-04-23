/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockGpuCommandBuffer_DEFINED
#define GrMockGpuCommandBuffer_DEFINED

#include "src/gpu/GrGpuCommandBuffer.h"
#include "src/gpu/mock/GrMockGpu.h"

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

private:
    void onDraw(const GrPrimitiveProcessor&, const GrPipeline&,
                const GrPipeline::FixedDynamicState*, const GrPipeline::DynamicStateArrays*,
                const GrMesh[], int meshCount, const SkRect& bounds) override {
        ++fNumDraws;
    }
    void onClear(const GrFixedClip&, const SkPMColor4f&) override {}
    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override {}

    GrMockGpu* fGpu;
    int fNumDraws = 0;

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif

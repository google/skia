/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkGpuCommandBuffer_DEFINED
#define GrVkGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"

#include "GrColor.h"
#include "GrTypes.h"
#include "GrVkPipelineState.h"

class GrNonInstancedMesh;
class GrVkGpu;
class GrVkImage;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkSecondaryCommandBuffer;

class GrVkGpuCommandBuffer : public GrGpuCommandBuffer {
public:
    GrVkGpuCommandBuffer(GrVkGpu* gpu,
                         GrVkRenderTarget*,
                         const LoadAndStoreInfo& colorInfo,
                         const LoadAndStoreInfo& stencilInfo);

    virtual ~GrVkGpuCommandBuffer();

    void end() override;

    void discard(GrRenderTarget* rt) override {}

private:
    GrGpu* gpu() override;

    void onSubmit(const SkIRect& bounds) override;

    // Bind vertex and index buffers
    void bindGeometry(const GrPrimitiveProcessor&, const GrNonInstancedMesh&);

    sk_sp<GrVkPipelineState> prepareDrawState(const GrPipeline&,
                                              const GrPrimitiveProcessor&,
                                              GrPrimitiveType,
                                              const GrVkRenderPass&);

    void onDraw(const GrPipeline& pipeline,
                const GrPrimitiveProcessor& primProc,
                const GrMesh* mesh,
                int meshCount) override;

    void onClear(GrRenderTarget* rt, const SkIRect& rect, GrColor color) override;

    void onClearStencilClip(GrRenderTarget*, const SkIRect& rect, bool insideClip) override;

    const GrVkRenderPass*       fRenderPass;
    GrVkSecondaryCommandBuffer* fCommandBuffer;
    GrVkGpu*                    fGpu;
    GrVkRenderTarget*           fRenderTarget;
    VkClearValue                fColorClearValue;

    SkTArray<GrVkImage*>        fSampledImages;

    bool                        fIsEmpty;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif

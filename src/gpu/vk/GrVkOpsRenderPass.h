/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkOpsRenderPass_DEFINED
#define GrVkOpsRenderPass_DEFINED

#include "src/gpu/GrOpsRenderPass.h"

#include "include/gpu/GrTypes.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrTRecorder.h"
#include "src/gpu/vk/GrVkPipelineState.h"

class GrVkGpu;
class GrVkImage;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkSecondaryCommandBuffer;

class GrVkOpsRenderPass : public GrOpsRenderPass, private GrMesh::SendToGpuImpl {
public:
    GrVkOpsRenderPass(GrVkGpu*);

    ~GrVkOpsRenderPass() override;

    void begin() override { }
    void end() override;

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;

    void onExecuteDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>) override;

    bool set(GrRenderTarget*, GrSurfaceOrigin, const SkIRect& bounds,
             const GrOpsRenderPass::LoadAndStoreInfo&,
             const GrOpsRenderPass::StencilLoadAndStoreInfo&,
             const SkTArray<GrSurfaceProxy*, true>& sampledProxies);
    void reset();

    void submit();

#ifdef SK_DEBUG
    bool isActive() const { return fIsActive; }
#endif

private:
    bool init(const GrOpsRenderPass::LoadAndStoreInfo&,
              const GrOpsRenderPass::StencilLoadAndStoreInfo&,
              const SkPMColor4f& clearColor);

    // Called instead of init when we are drawing to a render target that already wraps a secondary
    // command buffer.
    bool initWrapped();

    bool wrapsSecondaryCommandBuffer() const;

    GrGpu* gpu() override;

    GrVkCommandBuffer* currentCommandBuffer();

    // Bind vertex and index buffers
    void bindGeometry(const GrGpuBuffer* indexBuffer,
                      const GrGpuBuffer* vertexBuffer,
                      const GrGpuBuffer* instanceBuffer);

    bool onBindPipeline(const GrProgramInfo&, const SkRect& drawBounds) override;

    void onDrawMeshes(const GrProgramInfo&, const GrMesh[], int meshCount) override;

    // GrMesh::SendToGpuImpl methods. These issue the actual Vulkan draw commands.
    // Marked final as a hint to the compiler to not use virtual dispatch.
    void sendArrayMeshToGpu(GrPrimitiveType primitiveType, const GrMesh& mesh, int vertexCount,
                            int baseVertex) final {
        SkASSERT(!mesh.instanceBuffer());
        this->sendInstancedMeshToGpu(primitiveType, mesh, vertexCount, baseVertex, 1, 0);
    }
    void sendIndexedMeshToGpu(GrPrimitiveType primitiveType, const GrMesh& mesh, int indexCount,
                              int baseIndex, uint16_t minIndexValue, uint16_t maxIndexValue,
                              int baseVertex) final {
        SkASSERT(!mesh.instanceBuffer());
        this->sendIndexedInstancedMeshToGpu(primitiveType, mesh, indexCount, baseIndex, baseVertex,
                                            1, 0);
    }
    void sendInstancedMeshToGpu(GrPrimitiveType, const GrMesh&, int vertexCount, int baseVertex,
                                int instanceCount, int baseInstance) final;
    void sendIndexedInstancedMeshToGpu(GrPrimitiveType, const GrMesh&, int indexCount,
                                       int baseIndex, int baseVertex, int instanceCount,
                                       int baseInstance) final;

    void onClear(const GrFixedClip&, const SkPMColor4f& color) override;

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override;

    void addAdditionalRenderPass(bool mustUseSecondaryCommandBuffer);

    std::unique_ptr<GrVkSecondaryCommandBuffer> fCurrentSecondaryCommandBuffer;
    const GrVkRenderPass*                       fCurrentRenderPass;
    SkIRect                                     fCurrentPipelineBounds;
    GrVkPipelineState*                          fCurrentPipelineState = nullptr;
    bool                                        fCurrentCBIsEmpty = true;
    SkIRect                                     fBounds;
    GrVkGpu*                                    fGpu;

#ifdef SK_DEBUG
    // When we are actively recording into the GrVkOpsRenderPass we set this flag to true. This
    // then allows us to assert that we never submit a primary command buffer to the queue while in
    // a recording state. This is needed since when we submit to the queue we change command pools
    // and may trigger the old one to be reset, but a recording GrVkOpsRenderPass may still have
    // a outstanding secondary command buffer allocated from that pool that we'll try to access
    // after the pool as been reset.
    bool fIsActive = false;
#endif

    typedef GrOpsRenderPass INHERITED;
};

#endif

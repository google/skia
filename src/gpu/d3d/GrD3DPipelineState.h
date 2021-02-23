/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DPipelineState_DEFINED
#define GrD3DPipelineState_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrManagedResource.h"
#include "src/gpu/d3d/GrD3DPipelineStateDataManager.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

#include <vector>

class GrD3DDirectCommandList;
class GrD3DGpu;
class GrD3DRootSignature;
class GrProgramInfo;

class GrD3DPipelineState : public GrManagedResource {
public:
    using UniformInfoArray = GrD3DPipelineStateDataManager::UniformInfoArray;

    GrD3DPipelineState(gr_cp<ID3D12PipelineState> pipelineState,
                       sk_sp<GrD3DRootSignature> rootSignature,
                       const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
                       const UniformInfoArray& uniforms,
                       uint32_t uniformSize,
                       uint32_t numSamplers,
                       std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
                       std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
                       std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fpImpls,
                       size_t vertexStride,
                       size_t instanceStride);

#ifdef SK_TRACE_MANAGED_RESOURCES
    /** Output a human-readable dump of this resource's information
    */
    void dumpInfo() const override {
        SkDebugf("GrD3DPipelineState: %p (%d refs)\n", fPipelineState.get(), this->getRefCnt());
    }
#endif

    // This will be called right before this class is destroyed and there is no reason to explicitly
    // release the fPipelineState cause the gr_cp will handle that in the dtor.
    void freeGPUData() const override {}

    ID3D12PipelineState* pipelineState() const { return fPipelineState.get(); }
    const sk_sp<GrD3DRootSignature>& rootSignature() const { return fRootSignature; }

    void setAndBindConstants(GrD3DGpu*, const GrRenderTarget*, const GrProgramInfo&);

    void setAndBindTextures(GrD3DGpu*, const GrPrimitiveProcessor& primProc,
                            const GrSurfaceProxy* const primProcTextures[],
                            const GrPipeline& pipeline);

    void bindBuffers(GrD3DGpu*, sk_sp<const GrBuffer> indexBuffer,
                     sk_sp<const GrBuffer> instanceBuffer, sk_sp<const GrBuffer> vertexBuffer,
                     GrD3DDirectCommandList* commandList);

    // We can only cache non dirty uniform values until we submit a command list. After that, the
    // next frame will get a completely different uniform buffer and/or offset into the buffer. Thus
    // we need a way to mark them all as dirty during submit.
    void markUniformsDirty() { fDataManager.markDirty(); }

private:
    /**
     * We use the RT's size and origin to adjust from Skia device space to d3d normalized device
     * space and to make device space positions have the correct origin for processors that require
     * them.
     */
    struct RenderTargetState {
        SkISize         fRenderTargetSize;
        GrSurfaceOrigin fRenderTargetOrigin;

        RenderTargetState() { this->invalidate(); }
        void invalidate() {
            fRenderTargetSize.fWidth = -1;
            fRenderTargetSize.fHeight = -1;
            fRenderTargetOrigin = (GrSurfaceOrigin)-1;
        }

        /**
        * Gets a float4 that adjusts the position from Skia device coords to D3D's normalized device
        * coords. Assuming the transformed position, pos, is a homogeneous float3, the vec, v, is
        * applied as such:
        * pos.x = dot(v.xy, pos.xz)
        * pos.y = dot(v.zw, pos.yz)
        */
        void getRTAdjustmentVec(float* destVec) {
            destVec[0] = 2.f / fRenderTargetSize.fWidth;
            destVec[1] = -1.f;
            // D3D's NDC space is flipped from Vulkan and Metal
            if (kTopLeft_GrSurfaceOrigin == fRenderTargetOrigin) {
                destVec[2] = -2.f / fRenderTargetSize.fHeight;
                destVec[3] = 1.f;
            } else {
                destVec[2] = 2.f / fRenderTargetSize.fHeight;
                destVec[3] = -1.f;
            }
        }
    };

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin);

    gr_cp<ID3D12PipelineState> fPipelineState;
    sk_sp<GrD3DRootSignature> fRootSignature;

    // Tracks the current render target uniforms stored in the vertex buffer.
    RenderTargetState fRenderTargetState;
    GrGLSLBuiltinUniformHandles fBuiltinUniformHandles;

    // Processors in the GrD3DPipelineState
    std::unique_ptr<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fFPImpls;

    GrD3DPipelineStateDataManager fDataManager;

    unsigned int fNumSamplers;
    size_t fVertexStride;
    size_t fInstanceStride;
};

#endif

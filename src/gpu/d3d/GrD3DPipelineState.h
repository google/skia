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
class GrD3DPipeline;
class GrD3DRootSignature;
class GrProgramInfo;

class GrD3DPipelineState {
public:
    using UniformInfoArray = GrD3DPipelineStateDataManager::UniformInfoArray;

    GrD3DPipelineState(sk_sp<GrD3DPipeline> pipeline,
                       sk_sp<GrD3DRootSignature> rootSignature,
                       const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
                       const UniformInfoArray& uniforms,
                       uint32_t uniformSize,
                       uint32_t numSamplers,
                       std::unique_ptr<GrGeometryProcessor::ProgramImpl> gpImpl,
                       std::unique_ptr<GrXferProcessor::ProgramImpl> xpImpl,
                       std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fpImpls,
                       size_t vertexStride,
                       size_t instanceStride);

    const sk_sp<GrD3DPipeline>& pipeline() const { return fPipeline; }
    const sk_sp<GrD3DRootSignature>& rootSignature() const { return fRootSignature; }

    void setAndBindConstants(GrD3DGpu*, const GrRenderTarget*, const GrProgramInfo&);

    void setAndBindTextures(GrD3DGpu*,
                            const GrGeometryProcessor&,
                            const GrSurfaceProxy* const geomProcTextures[],
                            const GrPipeline&);

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
    };

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin);

    sk_sp<GrD3DPipeline> fPipeline;
    sk_sp<GrD3DRootSignature> fRootSignature;

    // Tracks the current render target uniforms stored in the vertex buffer.
    RenderTargetState fRenderTargetState;
    GrGLSLBuiltinUniformHandles fBuiltinUniformHandles;

    // Processors in the GrD3DPipelineState
    std::unique_ptr<GrGeometryProcessor::ProgramImpl>              fGPImpl;
    std::unique_ptr<GrXferProcessor::ProgramImpl>                  fXPImpl;
    std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fFPImpls;

    GrD3DPipelineStateDataManager fDataManager;

    unsigned int fNumSamplers;
    size_t fVertexStride;
    size_t fInstanceStride;
};

#endif

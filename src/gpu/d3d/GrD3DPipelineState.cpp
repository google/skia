/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DPipelineState.h"

#include "include/private/SkTemplates.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DRootSignature.h"
#include "src/gpu/d3d/GrD3DTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"

GrD3DPipelineState::GrD3DPipelineState(
        gr_cp<ID3D12PipelineState> pipelineState,
        sk_sp<GrD3DRootSignature> rootSignature,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms, uint32_t uniformSize,
        uint32_t numSamplers,
        std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fpImpls,
        size_t vertexStride,
        size_t instanceStride)
    : fPipelineState(std::move(pipelineState))
    , fRootSignature(std::move(rootSignature))
    , fBuiltinUniformHandles(builtinUniformHandles)
    , fGeometryProcessor(std::move(geometryProcessor))
    , fXferProcessor(std::move(xferProcessor))
    , fFPImpls(std::move(fpImpls))
    , fDataManager(uniforms, uniformSize)
    , fNumSamplers(numSamplers)
    , fVertexStride(vertexStride)
    , fInstanceStride(instanceStride) {}

void GrD3DPipelineState::setAndBindConstants(GrD3DGpu* gpu,
                                             const GrRenderTarget* renderTarget,
                                             const GrProgramInfo& programInfo) {
    this->setRenderTargetState(renderTarget, programInfo.origin());

    fGeometryProcessor->setData(fDataManager, programInfo.primProc());
    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        auto& fp = programInfo.pipeline().getFragmentProcessor(i);
        for (auto [fp, impl] : GrGLSLFragmentProcessor::ParallelRange(fp, *fFPImpls[i])) {
            impl.setData(fDataManager, fp);
        }
    }

    {
        SkIPoint offset;
        GrTexture* dstTexture = programInfo.pipeline().peekDstTexture(&offset);

        fXferProcessor->setData(fDataManager, programInfo.pipeline().getXferProcessor(),
                                dstTexture, offset);
    }

    D3D12_GPU_VIRTUAL_ADDRESS constantsAddress = fDataManager.uploadConstants(gpu);
    gpu->currentCommandList()->setGraphicsRootConstantBufferView(
        (unsigned int)(GrD3DRootSignature::ParamIndex::kConstantBufferView),
        constantsAddress);
}

void GrD3DPipelineState::setRenderTargetState(const GrRenderTarget* rt, GrSurfaceOrigin origin) {
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize dimensions = rt->dimensions();
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != dimensions) {
        fRenderTargetState.fRenderTargetSize = dimensions;
        fRenderTargetState.fRenderTargetOrigin = origin;

        float rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

void GrD3DPipelineState::setAndBindTextures(GrD3DGpu* gpu, const GrPrimitiveProcessor& primProc,
                                            const GrSurfaceProxy* const primProcTextures[],
                                            const GrPipeline& pipeline) {
    SkASSERT(primProcTextures || !primProc.numTextureSamplers());

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> shaderResourceViews(fNumSamplers);
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> samplers(fNumSamplers);
    unsigned int currTextureBinding = 0;

    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        SkASSERT(primProcTextures[i]->asTextureProxy());
        const auto& sampler = primProc.textureSampler(i);
        auto texture = static_cast<GrD3DTexture*>(primProcTextures[i]->peekTexture());
        shaderResourceViews[currTextureBinding] = texture->shaderResourceView();
        samplers[currTextureBinding++] =
                gpu->resourceProvider().findOrCreateCompatibleSampler(sampler.samplerState());
        gpu->currentCommandList()->addSampledTextureRef(texture);
    }

    pipeline.visitTextureEffects([&](const GrTextureEffect& te) {
        GrSamplerState samplerState = te.samplerState();
        auto* texture = static_cast<GrD3DTexture*>(te.texture());
        shaderResourceViews[currTextureBinding] = texture->shaderResourceView();
        samplers[currTextureBinding++] =
                gpu->resourceProvider().findOrCreateCompatibleSampler(samplerState);
        gpu->currentCommandList()->addSampledTextureRef(texture);
    });

    if (GrTexture* dstTexture = pipeline.peekDstTexture()) {
        auto texture = static_cast<GrD3DTexture*>(dstTexture);
        shaderResourceViews[currTextureBinding] = texture->shaderResourceView();
        samplers[currTextureBinding++] = gpu->resourceProvider().findOrCreateCompatibleSampler(
                                               GrSamplerState::Filter::kNearest);
        gpu->currentCommandList()->addSampledTextureRef(texture);
    }

    SkASSERT(fNumSamplers == currTextureBinding);

    // fill in descriptor tables and bind to root signature
    if (fNumSamplers > 0) {
        // set up and bind shader resource view table
        sk_sp<GrD3DDescriptorTable> srvTable =
                gpu->resourceProvider().findOrCreateShaderResourceTable(shaderResourceViews);
        gpu->currentCommandList()->setGraphicsRootDescriptorTable(
                static_cast<unsigned int>(GrD3DRootSignature::ParamIndex::kTextureDescriptorTable),
                srvTable->baseGpuDescriptor());

        // set up and bind sampler table
        sk_sp<GrD3DDescriptorTable> samplerTable =
                gpu->resourceProvider().findOrCreateSamplerTable(samplers);
        gpu->currentCommandList()->setGraphicsRootDescriptorTable(
                static_cast<unsigned int>(GrD3DRootSignature::ParamIndex::kSamplerDescriptorTable),
                samplerTable->baseGpuDescriptor());
    }
}

void GrD3DPipelineState::bindBuffers(GrD3DGpu* gpu, sk_sp<const GrBuffer> indexBuffer,
                                     sk_sp<const GrBuffer> instanceBuffer,
                                     sk_sp<const GrBuffer> vertexBuffer,
                                     GrD3DDirectCommandList* commandList) {
    // Here our vertex and instance inputs need to match the same 0-based bindings they were
    // assigned in the PipelineState. That is, vertex first (if any) followed by instance.
    if (vertexBuffer) {
        auto* d3dVertexBuffer = static_cast<const GrD3DBuffer*>(vertexBuffer.get());
        SkASSERT(!d3dVertexBuffer->isCpuBuffer());
        SkASSERT(!d3dVertexBuffer->isMapped());
        const_cast<GrD3DBuffer*>(d3dVertexBuffer)->setResourceState(
                gpu, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    }
    if (instanceBuffer) {
        auto* d3dInstanceBuffer = static_cast<const GrD3DBuffer*>(instanceBuffer.get());
        SkASSERT(!d3dInstanceBuffer->isCpuBuffer());
        SkASSERT(!d3dInstanceBuffer->isMapped());
        const_cast<GrD3DBuffer*>(d3dInstanceBuffer)->setResourceState(
                gpu, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    }
    commandList->setVertexBuffers(0, std::move(vertexBuffer), fVertexStride,
                                  std::move(instanceBuffer), fInstanceStride);

    if (auto* d3dIndexBuffer = static_cast<const GrD3DBuffer*>(indexBuffer.get())) {
        SkASSERT(!d3dIndexBuffer->isCpuBuffer());
        SkASSERT(!d3dIndexBuffer->isMapped());
        const_cast<GrD3DBuffer*>(d3dIndexBuffer)->setResourceState(
                gpu, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        commandList->setIndexBuffer(std::move(indexBuffer));
    }
}

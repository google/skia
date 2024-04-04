/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/d3d/GrD3DPipelineState.h"

#include "include/private/base/SkTemplates.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/d3d/GrD3DBuffer.h"
#include "src/gpu/ganesh/d3d/GrD3DGpu.h"
#include "src/gpu/ganesh/d3d/GrD3DPipeline.h"
#include "src/gpu/ganesh/d3d/GrD3DRootSignature.h"
#include "src/gpu/ganesh/d3d/GrD3DTexture.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/sksl/SkSLCompiler.h"

GrD3DPipelineState::GrD3DPipelineState(
        sk_sp<GrD3DPipeline> pipeline,
        sk_sp<GrD3DRootSignature> rootSignature,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms,
        uint32_t uniformSize,
        uint32_t numSamplers,
        std::unique_ptr<GrGeometryProcessor::ProgramImpl> gpImpl,
        std::unique_ptr<GrXferProcessor::ProgramImpl> xpImpl,
        std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fpImpls,
        size_t vertexStride,
        size_t instanceStride)
        : fPipeline(std::move(pipeline))
        , fRootSignature(std::move(rootSignature))
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fGPImpl(std::move(gpImpl))
        , fXPImpl(std::move(xpImpl))
        , fFPImpls(std::move(fpImpls))
        , fDataManager(uniforms, uniformSize)
        , fNumSamplers(numSamplers)
        , fVertexStride(vertexStride)
        , fInstanceStride(instanceStride) {}

void GrD3DPipelineState::setAndBindConstants(GrD3DGpu* gpu,
                                             const GrRenderTarget* renderTarget,
                                             const GrProgramInfo& programInfo) {
    this->setRenderTargetState(renderTarget, programInfo.origin());

    fGPImpl->setData(fDataManager, *gpu->caps()->shaderCaps(), programInfo.geomProc());

    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        const auto& fp = programInfo.pipeline().getFragmentProcessor(i);
        fp.visitWithImpls([&](const GrFragmentProcessor& fp,
                              GrFragmentProcessor::ProgramImpl& impl) {
            impl.setData(fDataManager, fp);
        }, *fFPImpls[i]);
    }

    programInfo.pipeline().setDstTextureUniforms(fDataManager, &fBuiltinUniformHandles);
    fXPImpl->setData(fDataManager, programInfo.pipeline().getXferProcessor());

    D3D12_GPU_VIRTUAL_ADDRESS constantsAddress = fDataManager.uploadConstants(gpu);
    gpu->currentCommandList()->setGraphicsRootConstantBufferView(
        (unsigned int)(GrD3DRootSignature::ParamIndex::kConstantBufferView),
        constantsAddress);
}

void GrD3DPipelineState::setRenderTargetState(const GrRenderTarget* rt, GrSurfaceOrigin origin) {
    // Set RT adjustment and RT flip
    SkISize dimensions = rt->dimensions();
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != dimensions) {
        fRenderTargetState.fRenderTargetSize = dimensions;
        fRenderTargetState.fRenderTargetOrigin = origin;

        // The client will mark a swap buffer as kTopLeft when making a SkSurface because
        // D3D's framebuffer space has (0, 0) at the top left. This agrees with Skia's device
        // coords. However, in NDC (-1, -1) is the bottom left. So we flip when origin is kTopLeft.
        bool flip = (origin == kTopLeft_GrSurfaceOrigin);
        std::array<float, 4> v = SkSL::Compiler::GetRTAdjustVector(dimensions, flip);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, v.data());
        if (fBuiltinUniformHandles.fRTFlipUni.isValid()) {
            // Note above that framebuffer space has origin top left. So we need !flip here.
            std::array<float, 2> d = SkSL::Compiler::GetRTFlipVector(rt->height(), !flip);
            fDataManager.set2fv(fBuiltinUniformHandles.fRTFlipUni, 1, d.data());
        }
    }
}

void GrD3DPipelineState::setAndBindTextures(GrD3DGpu* gpu,
                                            const GrGeometryProcessor& geomProc,
                                            const GrSurfaceProxy* const geomProcTextures[],
                                            const GrPipeline& pipeline) {
    SkASSERT(geomProcTextures || !geomProc.numTextureSamplers());

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> shaderResourceViews(fNumSamplers);
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> samplers(fNumSamplers);
    unsigned int currTextureBinding = 0;

    for (int i = 0; i < geomProc.numTextureSamplers(); ++i) {
        SkASSERT(geomProcTextures[i]->asTextureProxy());
        const auto& sampler = geomProc.textureSampler(i);
        auto texture = static_cast<GrD3DTexture*>(geomProcTextures[i]->peekTexture());
        shaderResourceViews[currTextureBinding] = texture->shaderResourceView();
        samplers[currTextureBinding++] =
                gpu->resourceProvider().findOrCreateCompatibleSampler(sampler.samplerState());
        gpu->currentCommandList()->addSampledTextureRef(texture);
    }

    if (GrTexture* dstTexture = pipeline.peekDstTexture()) {
        auto texture = static_cast<GrD3DTexture*>(dstTexture);
        shaderResourceViews[currTextureBinding] = texture->shaderResourceView();
        samplers[currTextureBinding++] = gpu->resourceProvider().findOrCreateCompatibleSampler(
                                               GrSamplerState::Filter::kNearest);
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

    SkASSERT(fNumSamplers == currTextureBinding);

    // fill in descriptor tables and bind to root signature
    if (fNumSamplers > 0) {
        // set up descriptor tables and bind heaps
        sk_sp<GrD3DDescriptorTable> srvTable =
                gpu->resourceProvider().findOrCreateShaderViewTable(shaderResourceViews);
        sk_sp<GrD3DDescriptorTable> samplerTable =
            gpu->resourceProvider().findOrCreateSamplerTable(samplers);
        gpu->currentCommandList()->setDescriptorHeaps(srvTable->heap(), samplerTable->heap());

        // bind shader resource view table
        gpu->currentCommandList()->setGraphicsRootDescriptorTable(
                (unsigned int)GrD3DRootSignature::ParamIndex::kShaderViewDescriptorTable,
                srvTable->baseGpuDescriptor());

        // bind sampler table
        gpu->currentCommandList()->setGraphicsRootDescriptorTable(
                (unsigned int)GrD3DRootSignature::ParamIndex::kSamplerDescriptorTable,
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

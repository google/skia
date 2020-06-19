/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DResourceProvider.h"

#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DCommandList.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DPipelineState.h"
#include "src/gpu/d3d/GrD3DPipelineStateBuilder.h"

GrD3DResourceProvider::GrD3DResourceProvider(GrD3DGpu* gpu)
        : fGpu(gpu)
        , fCpuDescriptorManager(gpu)
        , fDescriptorTableManager(gpu)
        , fPipelineStateCache(new PipelineStateCache(gpu)) {}

void GrD3DResourceProvider::destroyResources() {
    fSamplers.reset();

    fPipelineStateCache->release();
}

std::unique_ptr<GrD3DDirectCommandList> GrD3DResourceProvider::findOrCreateDirectCommandList() {
    if (fAvailableDirectCommandLists.count()) {
        std::unique_ptr<GrD3DDirectCommandList> list =
                std::move(fAvailableDirectCommandLists.back());
        fAvailableDirectCommandLists.pop_back();
        return list;
    }
    return GrD3DDirectCommandList::Make(fGpu->device());
}

void GrD3DResourceProvider::recycleDirectCommandList(
        std::unique_ptr<GrD3DDirectCommandList> commandList) {
    commandList->reset();
    fAvailableDirectCommandLists.push_back(std::move(commandList));
}

sk_sp<GrD3DRootSignature> GrD3DResourceProvider::findOrCreateRootSignature(int numTextureSamplers) {
    for (int i = 0; i < fRootSignatures.count(); ++i) {
        if (fRootSignatures[i]->isCompatible(numTextureSamplers)) {
            return fRootSignatures[i];
        }
    }

    auto rootSig = GrD3DRootSignature::Make(fGpu, numTextureSamplers);
    if (!rootSig) {
        return nullptr;
    }
    fRootSignatures.push_back(rootSig);
    return rootSig;
}


GrD3DDescriptorHeap::CPUHandle GrD3DResourceProvider::createRenderTargetView(
        ID3D12Resource* textureResource) {
    return fCpuDescriptorManager.createRenderTargetView(fGpu, textureResource);
}

void GrD3DResourceProvider::recycleRenderTargetView(
        const GrD3DDescriptorHeap::CPUHandle& rtvDescriptor) {
    fCpuDescriptorManager.recycleRenderTargetView(rtvDescriptor);
}

GrD3DDescriptorHeap::CPUHandle GrD3DResourceProvider::createDepthStencilView(
        ID3D12Resource* textureResource) {
    return fCpuDescriptorManager.createDepthStencilView(fGpu, textureResource);
}

void GrD3DResourceProvider::recycleDepthStencilView(
        const GrD3DDescriptorHeap::CPUHandle& dsvDescriptor) {
    fCpuDescriptorManager.recycleDepthStencilView(dsvDescriptor);
}

GrD3DDescriptorHeap::CPUHandle GrD3DResourceProvider::createConstantBufferView(
        ID3D12Resource* bufferResource, size_t offset, size_t size) {
    return fCpuDescriptorManager.createConstantBufferView(fGpu, bufferResource, offset, size);
}

GrD3DDescriptorHeap::CPUHandle GrD3DResourceProvider::createShaderResourceView(
        ID3D12Resource* resource) {
    return fCpuDescriptorManager.createShaderResourceView(fGpu, resource);
}

void GrD3DResourceProvider::recycleConstantOrShaderView(
        const GrD3DDescriptorHeap::CPUHandle& view) {
    fCpuDescriptorManager.recycleConstantOrShaderView(view);
}

static D3D12_TEXTURE_ADDRESS_MODE wrap_mode_to_d3d_address_mode(GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
    case GrSamplerState::WrapMode::kClamp:
        return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    case GrSamplerState::WrapMode::kRepeat:
        return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    case GrSamplerState::WrapMode::kMirrorRepeat:
        return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    case GrSamplerState::WrapMode::kClampToBorder:
        return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    }
    SK_ABORT("Unknown wrap mode.");
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DResourceProvider::findOrCreateCompatibleSampler(
        const GrSamplerState& params) {
    uint32_t key = GrSamplerState::GenerateKey(params);
    D3D12_CPU_DESCRIPTOR_HANDLE* samplerPtr = fSamplers.find(key);
    if (samplerPtr) {
        return *samplerPtr;
    }

    static D3D12_FILTER d3dFilterModes[] = {
        D3D12_FILTER_MIN_MAG_MIP_POINT,
        D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR
    };

    static_assert((int)GrSamplerState::Filter::kNearest == 0);
    static_assert((int)GrSamplerState::Filter::kBilerp == 1);
    static_assert((int)GrSamplerState::Filter::kMipMap == 2);

    D3D12_FILTER filter = d3dFilterModes[static_cast<int>(params.filter())];
    D3D12_TEXTURE_ADDRESS_MODE addressModeU = wrap_mode_to_d3d_address_mode(params.wrapModeX());
    D3D12_TEXTURE_ADDRESS_MODE addressModeV = wrap_mode_to_d3d_address_mode(params.wrapModeY());

    D3D12_CPU_DESCRIPTOR_HANDLE sampler =
            fCpuDescriptorManager.createSampler(
            fGpu, filter, addressModeU, addressModeV).fHandle;
    fSamplers.set(key, sampler);
    return sampler;
}

std::unique_ptr<GrD3DDescriptorTable> GrD3DResourceProvider::createShaderOrConstantResourceTable(
        unsigned int size) {
    return fDescriptorTableManager.createShaderOrConstantResourceTable(fGpu, size);
}

std::unique_ptr<GrD3DDescriptorTable> GrD3DResourceProvider::createSamplerTable(unsigned int size) {
    return fDescriptorTableManager.createSamplerTable(fGpu, size);
}

sk_sp<GrD3DPipelineState> GrD3DResourceProvider::findOrCreateCompatiblePipelineState(
        GrRenderTarget* rt, const GrProgramInfo& info) {
    return fPipelineStateCache->refPipelineState(rt, info);
}

D3D12_GPU_VIRTUAL_ADDRESS GrD3DResourceProvider::uploadConstantData(void* data, size_t size) {
    // constant size has to be aligned to 256
    constexpr int kConstantAlignment = 256;

    // Due to dependency on the resource cache we can't initialize this in the constructor, so
    // we do so it here.
    if (!fConstantBuffer) {
        fConstantBuffer = GrD3DConstantRingBuffer::Make(fGpu, 128 * 1024, kConstantAlignment);
        SkASSERT(fConstantBuffer);
    }

    // upload the data
    size_t paddedSize = GrAlignTo(size, kConstantAlignment);
    GrRingBuffer::Slice slice = fConstantBuffer->suballocate(paddedSize);
    char* destPtr = static_cast<char*>(slice.fBuffer->map()) + slice.fOffset;
    memcpy(destPtr, data, size);

    // create the associated constant buffer view descriptor
    GrD3DBuffer* d3dBuffer = static_cast<GrD3DBuffer*>(slice.fBuffer.get());
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = d3dBuffer->d3dResource()->GetGPUVirtualAddress();
    return gpuAddress + slice.fOffset;
}

void GrD3DResourceProvider::prepForSubmit() {
    fGpu->currentCommandList()->setCurrentConstantBuffer(fConstantBuffer);
    fDescriptorTableManager.prepForSubmit(fGpu);
}

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_PIPELINE_STATE_CACHE_STATS
// Display pipeline state cache usage
static const bool c_DisplayMtlPipelineCache{false};
#endif

struct GrD3DResourceProvider::PipelineStateCache::Entry {
    Entry(GrD3DGpu* gpu, sk_sp<GrD3DPipelineState> pipelineState)
            : fGpu(gpu), fPipelineState(std::move(pipelineState)) {}

    GrD3DGpu* fGpu;
    sk_sp<GrD3DPipelineState> fPipelineState;
};

GrD3DResourceProvider::PipelineStateCache::PipelineStateCache(GrD3DGpu* gpu)
        : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
        , fGpu(gpu)
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        , fTotalRequests(0)
        , fCacheMisses(0)
#endif
{
}

GrD3DResourceProvider::PipelineStateCache::~PipelineStateCache() {
    // dump stats
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    if (c_DisplayMtlPipelineCache) {
        SkDebugf("--- Pipeline State Cache ---\n");
        SkDebugf("Total requests: %d\n", fTotalRequests);
        SkDebugf("Cache misses: %d\n", fCacheMisses);
        SkDebugf("Cache miss %%: %f\n",
                 (fTotalRequests > 0) ? 100.f * fCacheMisses / fTotalRequests : 0.f);
        SkDebugf("---------------------\n");
    }
#endif
}

void GrD3DResourceProvider::PipelineStateCache::release() {
    fMap.reset();
}

sk_sp<GrD3DPipelineState> GrD3DResourceProvider::PipelineStateCache::refPipelineState(
        GrRenderTarget* renderTarget, const GrProgramInfo& programInfo) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif

    const GrCaps* caps = fGpu->caps();

    GrProgramDesc desc = caps->makeDesc(renderTarget, programInfo);
    if (!desc.isValid()) {
        GrCapsDebugf(fGpu->caps(), "Failed to build mtl program descriptor!\n");
        return nullptr;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        sk_sp<GrD3DPipelineState> pipelineState = GrD3DPipelineStateBuilder::MakePipelineState(
                fGpu, renderTarget, desc, programInfo);
        if (!pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(
                new Entry(fGpu, std::move(pipelineState))));
        return (*entry)->fPipelineState;
    }
    return (*entry)->fPipelineState;
}

void GrD3DResourceProvider::PipelineStateCache::markPipelineStateUniformsDirty() {
    fMap.foreach ([](const GrProgramDesc*, std::unique_ptr<Entry>* entry) {
        (*entry)->fPipelineState->markUniformsDirty();
    });
}


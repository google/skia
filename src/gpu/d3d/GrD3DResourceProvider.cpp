/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DResourceProvider.h"

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkOpts_spi.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DCommandList.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DPipelineState.h"
#include "src/gpu/d3d/GrD3DPipelineStateBuilder.h"

GrD3DResourceProvider::GrD3DResourceProvider(GrD3DGpu* gpu)
        : fGpu(gpu)
        , fCpuDescriptorManager(gpu)
        , fDescriptorTableManager(gpu)
        , fPipelineStateCache(new PipelineStateCache(gpu))
        , fShaderResourceDescriptorTableCache(gpu)
        , fSamplerDescriptorTableCache(gpu) {
}

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

sk_sp<GrD3DCommandSignature> GrD3DResourceProvider::findOrCreateCommandSignature(
        GrD3DCommandSignature::ForIndexed indexed, unsigned int slot) {
    for (int i = 0; i < fCommandSignatures.count(); ++i) {
        if (fCommandSignatures[i]->isCompatible(indexed, slot)) {
            return fCommandSignatures[i];
        }
    }

    auto commandSig = GrD3DCommandSignature::Make(fGpu, indexed, slot);
    if (!commandSig) {
        return nullptr;
    }
    fCommandSignatures.push_back(commandSig);
    return commandSig;
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

static D3D12_FILTER d3d_filter(GrSamplerState sampler) {
    switch (sampler.mipmapMode()) {
        // When the mode is kNone we disable filtering using maxLOD.
        case GrSamplerState::MipmapMode::kNone:
        case GrSamplerState::MipmapMode::kNearest:
            switch (sampler.filter()) {
                case GrSamplerState::Filter::kNearest: return D3D12_FILTER_MIN_MAG_MIP_POINT;
                case GrSamplerState::Filter::kLinear:  return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            }
            SkUNREACHABLE;
        case GrSamplerState::MipmapMode::kLinear:
            switch (sampler.filter()) {
                case GrSamplerState::Filter::kNearest: return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
                case GrSamplerState::Filter::kLinear:  return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            }
            SkUNREACHABLE;
    }
    SkUNREACHABLE;
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DResourceProvider::findOrCreateCompatibleSampler(
        const GrSamplerState& params) {
    uint32_t key = params.asIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE* samplerPtr = fSamplers.find(key);
    if (samplerPtr) {
        return *samplerPtr;
    }

    D3D12_FILTER filter = d3d_filter(params);
    // We disable MIP filtering using maxLOD. Otherwise, we want the max LOD to be unbounded.
    float maxLOD = params.mipmapped() == GrMipmapped::kYes ? std::numeric_limits<float>::max()
                                                           : 0.f;
    D3D12_TEXTURE_ADDRESS_MODE addressModeU = wrap_mode_to_d3d_address_mode(params.wrapModeX());
    D3D12_TEXTURE_ADDRESS_MODE addressModeV = wrap_mode_to_d3d_address_mode(params.wrapModeY());

    D3D12_CPU_DESCRIPTOR_HANDLE sampler =
            fCpuDescriptorManager.createSampler(
            fGpu, filter, maxLOD, addressModeU, addressModeV).fHandle;
    fSamplers.set(key, sampler);
    return sampler;
}

sk_sp<GrD3DDescriptorTable> GrD3DResourceProvider::findOrCreateShaderResourceTable(
    const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& shaderResourceViews) {

    auto createFunc = [this](GrD3DGpu* gpu, unsigned int numDesc) {
        return this->fDescriptorTableManager.createShaderOrConstantResourceTable(gpu, numDesc);
    };
    return fShaderResourceDescriptorTableCache.findOrCreateDescTable(shaderResourceViews,
                                                                     createFunc);
}

sk_sp<GrD3DDescriptorTable> GrD3DResourceProvider::findOrCreateSamplerTable(
        const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& samplers) {
    auto createFunc = [this](GrD3DGpu* gpu, unsigned int numDesc) {
        return this->fDescriptorTableManager.createSamplerTable(gpu, numDesc);
    };
    return fShaderResourceDescriptorTableCache.findOrCreateDescTable(samplers, createFunc);
}

sk_sp<GrD3DPipelineState> GrD3DResourceProvider::findOrCreateCompatiblePipelineState(
        GrRenderTarget* rt, const GrProgramInfo& info) {
    return fPipelineStateCache->refPipelineState(rt, info);
}

D3D12_GPU_VIRTUAL_ADDRESS GrD3DResourceProvider::uploadConstantData(void* data, size_t size) {
    // constant size has to be aligned to 256
    constexpr int kConstantAlignment = 256;

    // upload the data
    size_t paddedSize = GrAlignTo(size, kConstantAlignment);
    GrRingBuffer::Slice slice = fGpu->uniformsRingBuffer()->suballocate(paddedSize);
    char* destPtr = static_cast<char*>(slice.fBuffer->map()) + slice.fOffset;
    memcpy(destPtr, data, size);

    // create the associated constant buffer view descriptor
    GrD3DBuffer* d3dBuffer = static_cast<GrD3DBuffer*>(slice.fBuffer);
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = d3dBuffer->d3dResource()->GetGPUVirtualAddress();
    return gpuAddress + slice.fOffset;
}

void GrD3DResourceProvider::prepForSubmit() {
    fDescriptorTableManager.prepForSubmit(fGpu);
    // Any heap memory used for these will be returned when the command buffer finishes,
    // so we have to invalidate all entries.
    fShaderResourceDescriptorTableCache.release();
    fSamplerDescriptorTableCache.release();
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

////////////////////////////////////////////////////////////////////////////////////////////////

void GrD3DResourceProvider::DescriptorTableCache::release() {
    fMap.reset();
}

sk_sp<GrD3DDescriptorTable> GrD3DResourceProvider::DescriptorTableCache::findOrCreateDescTable(
        const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& cpuDescriptors,
        std::function<sk_sp<GrD3DDescriptorTable>(GrD3DGpu*, unsigned int numDesc)> createFunc) {
    sk_sp<GrD3DDescriptorTable>* entry = fMap.find(cpuDescriptors);
    if (entry) {
        return *entry;
    }

    unsigned int numDescriptors = cpuDescriptors.size();
    SkASSERT(numDescriptors <= kRangeSizesCount);
    sk_sp<GrD3DDescriptorTable> descTable = createFunc(fGpu, numDescriptors);
    fGpu->device()->CopyDescriptors(1, descTable->baseCpuDescriptorPtr(), &numDescriptors,
                                    numDescriptors, cpuDescriptors.data(), fRangeSizes,
                                    descTable->type());
    entry = fMap.insert(cpuDescriptors, std::move(descTable));
    return *entry;
}

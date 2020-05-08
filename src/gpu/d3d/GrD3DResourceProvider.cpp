/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DResourceProvider.h"

#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/d3d/GrD3DCommandList.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DPipelineState.h"
#include "src/gpu/d3d/GrD3DPipelineStateBuilder.h"

GrD3DResourceProvider::GrD3DResourceProvider(GrD3DGpu* gpu)
        : fGpu(gpu)
        , fAttachmentViewManager(gpu)
        , fPipelineStateCache(new PipelineStateCache(gpu)) {
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


D3D12_CPU_DESCRIPTOR_HANDLE GrD3DResourceProvider::createRenderTargetView(
        ID3D12Resource* textureResource) {
    return fAttachmentViewManager.createRenderTargetView(fGpu, textureResource);
}

void GrD3DResourceProvider::recycleRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE* rtvDescriptor) {
    fAttachmentViewManager.recycleRenderTargetView(rtvDescriptor);
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DResourceProvider::createDepthStencilView(
        ID3D12Resource* textureResource) {
    return fAttachmentViewManager.createDepthStencilView(fGpu, textureResource);
}

void GrD3DResourceProvider::recycleDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE* dsvDescriptor) {
    fAttachmentViewManager.recycleDepthStencilView(dsvDescriptor);
}

sk_sp<GrD3DPipelineState> GrD3DResourceProvider::findOrCreateCompatiblePipelineState(
        GrRenderTarget* rt, const GrProgramInfo& info) {
    return fPipelineStateCache->refPipelineState(rt, info);
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

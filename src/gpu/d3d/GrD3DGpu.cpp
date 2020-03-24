/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DGpu.h"

#include "src/gpu/d3d/GrD3DCaps.h"
#include "src/gpu/d3d/GrD3DOpsRenderPass.h"

sk_sp<GrGpu> GrD3DGpu::Make(const GrD3DBackendContext& backendContext,
                            const GrContextOptions& contextOptions, GrContext* context) {
    return sk_sp<GrGpu>(new GrD3DGpu(context, contextOptions, backendContext));
}

// This constant determines how many OutstandingCommandLists are allocated together as a block in
// the deque. As such it needs to balance allocating too much memory vs. incurring
// allocation/deallocation thrashing. It should roughly correspond to the max number of outstanding
// command lists we expect to see.
static const int kDefaultOutstandingAllocCnt = 8;

GrD3DGpu::GrD3DGpu(GrContext* context, const GrContextOptions& contextOptions,
                   const GrD3DBackendContext& backendContext)
        : INHERITED(context)
        , fDevice(backendContext.fDevice)

        , fQueue(backendContext.fQueue)
        , fResourceProvider(this)
        , fOutstandingCommandLists(sizeof(OutstandingCommandList), kDefaultOutstandingAllocCnt) {
    fCaps.reset(new GrD3DCaps(contextOptions,
                              backendContext.fAdapter.Get(),
                              backendContext.fDevice.Get()));

    fCurrentDirectCommandList = fResourceProvider.findOrCreateDirectCommandList();
    SkASSERT(fCurrentDirectCommandList);

    SkASSERT(fCurrentFenceValue == 0);
    SkDEBUGCODE(HRESULT hr = ) fDevice->CreateFence(fCurrentFenceValue, D3D12_FENCE_FLAG_NONE,
                                                    IID_PPV_ARGS(&fFence));
    SkASSERT(SUCCEEDED(hr));
}

GrD3DGpu::~GrD3DGpu() {
    this->destroyResources();
}

void GrD3DGpu::destroyResources() {
    if (fCurrentDirectCommandList) {
        fCurrentDirectCommandList->close();
        fCurrentDirectCommandList.reset();
    }

    // We need to make sure everything has finished on the queue.
    if (fFence->GetCompletedValue() < fCurrentFenceValue) {
        HANDLE fenceEvent;
        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        SkASSERT(fenceEvent);
        SkDEBUGCODE(HRESULT hr = ) fFence->SetEventOnCompletion(fCurrentFenceValue, fenceEvent);
        SkASSERT(SUCCEEDED(hr));
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
    }

    SkDEBUGCODE(uint64_t fenceValue = fFence->GetCompletedValue();)

    // We used a placement new for each object in fOutstandingCommandLists, so we're responsible
    // for calling the destructor on each of them as well.
    while (!fOutstandingCommandLists.empty()) {
        OutstandingCommandList* list = (OutstandingCommandList*)fOutstandingCommandLists.back();
        SkASSERT(list->fFenceValue <= fenceValue);
        // No reason to recycle the command lists since we are destroying all resources anyways.
        list->~OutstandingCommandList();
        fOutstandingCommandLists.pop_back();
    }
}

GrOpsRenderPass* GrD3DGpu::getOpsRenderPass(
    GrRenderTarget* rt, GrSurfaceOrigin origin, const SkIRect& bounds,
    const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
    const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
    const SkTArray<GrSurfaceProxy*, true>& sampledProxies) {
    if (!fCachedOpsRenderPass) {
        fCachedOpsRenderPass.reset(new GrD3DOpsRenderPass(this));
    }

    if (!fCachedOpsRenderPass->set(rt, origin, bounds, colorInfo, stencilInfo, sampledProxies)) {
        return nullptr;
    }
    return fCachedOpsRenderPass.get();
}

void GrD3DGpu::submitDirectCommandList() {
    SkASSERT(fCurrentDirectCommandList);

    fCurrentDirectCommandList->submit(fQueue.Get());

    new (fOutstandingCommandLists.push_back()) OutstandingCommandList(
            std::move(fCurrentDirectCommandList), ++fCurrentFenceValue);

    SkDEBUGCODE(HRESULT hr = ) fQueue->Signal(fFence.Get(), fCurrentFenceValue);
    SkASSERT(SUCCEEDED(hr));

    fCurrentDirectCommandList = fResourceProvider.findOrCreateDirectCommandList();

    // This should be done after we have a new command list in case the freeing of any resources
    // held by a finished command list causes us send a new command to the gpu (like changing the
    // resource state.
    this->checkForFinishedCommandLists();

    SkASSERT(fCurrentDirectCommandList);
}

void GrD3DGpu::checkForFinishedCommandLists() {
    uint64_t currentFenceValue = fFence->GetCompletedValue();

    // Iterate over all the outstanding command lists to see if any have finished. The commands
    // lists are in order from oldest to newest, so we start at the front to check if their fence
    // value is less than the last signaled value. If so we pop it off and move onto the next.
    // Repeat till we find a command list that has not finished yet (and all others afterwards are
    // also guaranteed to not have finished).
    SkDeque::F2BIter iter(fOutstandingCommandLists);
    const OutstandingCommandList* curList = (const OutstandingCommandList*)iter.next();
    while (curList && curList->fFenceValue <= currentFenceValue) {
        curList = (const OutstandingCommandList*)iter.next();
        OutstandingCommandList* front = (OutstandingCommandList*)fOutstandingCommandLists.front();
        fResourceProvider.recycleDirectCommandList(std::move(front->fCommandList));
        // Since we used placement new we are responsible for calling the destructor manually.
        front->~OutstandingCommandList();
        fOutstandingCommandLists.pop_front();
    }
}

void GrD3DGpu::submit(GrOpsRenderPass* renderPass) {
    // TODO: actually submit something here
    delete renderPass;
}

void GrD3DGpu::querySampleLocations(GrRenderTarget* rt, SkTArray<SkPoint>* sampleLocations) {
    // TODO
}

sk_sp<GrTexture> GrD3DGpu::onCreateTexture(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           GrRenderable renderable,
                                           int renderTargetSampleCnt,
                                           SkBudgeted budgeted,
                                           GrProtected isProtected,
                                           int mipLevelCount,
                                           uint32_t levelClearMask) {
    // TODO
    return nullptr;
}

sk_sp<GrTexture> GrD3DGpu::onCreateCompressedTexture(SkISize dimensions,
                                                     const GrBackendFormat& format,
                                                     SkBudgeted budgeted,
                                                     GrMipMapped mipMapped,
                                                     GrProtected isProtected,
                                                     const void* data, size_t dataSize) {
    // TODO
    return nullptr;
}

sk_sp<GrTexture> GrD3DGpu::onWrapBackendTexture(const GrBackendTexture& tex, GrColorType colorType,
                                                GrWrapOwnership ownership,
                                                GrWrapCacheable wrapType, GrIOType ioType) {
    // TODO
    return nullptr;
}

sk_sp<GrTexture> GrD3DGpu::onWrapCompressedBackendTexture(const GrBackendTexture& tex,
                                                          GrWrapOwnership ownership,
                                                          GrWrapCacheable wrapType) {
    return nullptr;
}

sk_sp<GrTexture> GrD3DGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                          int sampleCnt,
                                                          GrColorType colorType,
                                                          GrWrapOwnership ownership,
                                                          GrWrapCacheable cacheable) {
    // TODO
    return nullptr;
}

sk_sp<GrRenderTarget> GrD3DGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt,
                                                          GrColorType colorType) {
    // TODO
    return nullptr;
}

sk_sp<GrRenderTarget> GrD3DGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                    int sampleCnt,
                                                                    GrColorType colorType) {
    // TODO
    return nullptr;
}

sk_sp<GrGpuBuffer> GrD3DGpu::onCreateBuffer(size_t sizeInBytes, GrGpuBufferType type,
                                             GrAccessPattern accessPattern, const void*) {
    // TODO
    return nullptr;
}

GrStencilAttachment* GrD3DGpu::createStencilAttachmentForRenderTarget(
        const GrRenderTarget* rt, int width, int height, int numStencilSamples) {
    // TODO
    return nullptr;
}

GrBackendTexture GrD3DGpu::onCreateBackendTexture(SkISize dimensions,
                                                   const GrBackendFormat& format,
                                                   GrRenderable,
                                                   GrMipMapped mipMapped,
                                                   GrProtected,
                                                   const BackendTextureData*) {
    // TODO
    return GrBackendTexture();
}

GrBackendTexture GrD3DGpu::onCreateCompressedBackendTexture(SkISize dimensions,
                                                             const GrBackendFormat& format,
                                                             GrMipMapped mipMapped,
                                                             GrProtected,
                                                             const BackendTextureData*) {
    // TODO
    return GrBackendTexture();
}

void GrD3DGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    // TODO
}

bool GrD3DGpu::compile(const GrProgramDesc&, const GrProgramInfo&) {
    return false;
}

#if GR_TEST_UTILS
bool GrD3DGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    // TODO
    return false;
}

GrBackendRenderTarget GrD3DGpu::createTestingOnlyBackendRenderTarget(int w, int h,
                                                                      GrColorType colorType) {
    // TODO
    return GrBackendRenderTarget();
}

void GrD3DGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) {}
#endif

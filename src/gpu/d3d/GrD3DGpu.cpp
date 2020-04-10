/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DGpu.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/d3d/GrD3DBackendContext.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DCaps.h"
#include "src/gpu/d3d/GrD3DOpsRenderPass.h"
#include "src/gpu/d3d/GrD3DStencilAttachment.h"
#include "src/gpu/d3d/GrD3DTexture.h"
#include "src/gpu/d3d/GrD3DTextureRenderTarget.h"
#include "src/gpu/d3d/GrD3DUtil.h"

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
                              backendContext.fAdapter.get(),
                              backendContext.fDevice.get()));

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

    fCurrentDirectCommandList->submit(fQueue.get());

    new (fOutstandingCommandLists.push_back()) OutstandingCommandList(
            std::move(fCurrentDirectCommandList), ++fCurrentFenceValue);

    SkDEBUGCODE(HRESULT hr = ) fQueue->Signal(fFence.get(), fCurrentFenceValue);
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
    SkASSERT(fCachedOpsRenderPass.get() == renderPass);

    // TODO: actually submit something here
    fCachedOpsRenderPass.reset();
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
    DXGI_FORMAT dxgiFormat;
    SkAssertResult(format.asDxgiFormat(&dxgiFormat));
    SkASSERT(!GrDxgiFormatIsCompressed(dxgiFormat));

    D3D12_RESOURCE_FLAGS usageFlags = D3D12_RESOURCE_FLAG_NONE;

    if (renderable == GrRenderable::kYes) {
        usageFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }

    // This desc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this describes the resolved texture. Therefore we always have samples set
    // to 1.
    SkASSERT(mipLevelCount > 0);
    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    // TODO: will use 4MB alignment for MSAA textures and 64KB for everything else
    //       might want to manually set alignment to 4KB for smaller textures
    resourceDesc.Alignment = 0;
    resourceDesc.Width = dimensions.fWidth;
    resourceDesc.Height = dimensions.fHeight;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = mipLevelCount;
    resourceDesc.Format = dxgiFormat;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0; // quality levels are only supported for tiled resources
                                         // so ignore for now
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // use driver-selected swizzle
    resourceDesc.Flags = usageFlags;

    GrMipMapsStatus mipMapsStatus =
        mipLevelCount > 1 ? GrMipMapsStatus::kDirty : GrMipMapsStatus::kNotAllocated;

    sk_sp<GrD3DTexture> tex;
    if (renderable == GrRenderable::kYes) {
        tex = GrD3DTextureRenderTarget::MakeNewTextureRenderTarget(
                this, budgeted, dimensions, renderTargetSampleCnt, resourceDesc, isProtected,
                mipMapsStatus);
    } else {
        tex = GrD3DTexture::MakeNewTexture(this, budgeted, dimensions, resourceDesc, isProtected,
                                           mipMapsStatus);
    }

    if (!tex) {
        return nullptr;
    }

    if (levelClearMask) {
        // TODO
    }
    return std::move(tex);
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

static bool check_resource_info(const GrD3DTextureResourceInfo& info) {
    if (!info.fResource.get()) {
        return false;
    }
    return true;
}

static bool check_tex_resource_info(const GrD3DCaps& caps, const GrD3DTextureResourceInfo& info) {
    if (!caps.isFormatTexturable(info.fFormat)) {
        return false;
    }
    return true;
}

static bool check_rt_resource_info(const GrD3DCaps& caps, const GrD3DTextureResourceInfo& info,
                                int sampleCnt) {
    if (!caps.isFormatRenderable(info.fFormat, sampleCnt)) {
        return false;
    }
    return true;
}

sk_sp<GrTexture> GrD3DGpu::onWrapBackendTexture(const GrBackendTexture& tex,
                                                GrWrapOwnership,
                                                GrWrapCacheable wrapType,
                                                GrIOType ioType) {
    GrD3DTextureResourceInfo textureInfo;
    if (!tex.getD3DTextureResourceInfo(&textureInfo)) {
        return nullptr;
    }

    if (!check_resource_info(textureInfo)) {
        return nullptr;
    }

    if (!check_tex_resource_info(this->d3dCaps(), textureInfo)) {
        return nullptr;
    }

    // TODO: support protected context
    if (tex.isProtected()) {
        return nullptr;
    }

    sk_sp<GrD3DResourceState> state = tex.getGrD3DResourceState();
    SkASSERT(state);
    return GrD3DTexture::MakeWrappedTexture(this, tex.dimensions(), wrapType, ioType, textureInfo,
                                            std::move(state));
}

sk_sp<GrTexture> GrD3DGpu::onWrapCompressedBackendTexture(const GrBackendTexture& tex,
                                                          GrWrapOwnership,
                                                          GrWrapCacheable wrapType) {
    GrD3DTextureResourceInfo textureInfo;
    if (!tex.getD3DTextureResourceInfo(&textureInfo)) {
        return nullptr;
    }

    if (!check_resource_info(textureInfo)) {
        return nullptr;
    }

    if (!check_tex_resource_info(this->d3dCaps(), textureInfo)) {
        return nullptr;
    }

    // TODO: support protected context
    if (tex.isProtected()) {
        return nullptr;
    }

    sk_sp<GrD3DResourceState> state = tex.getGrD3DResourceState();
    SkASSERT(state);
    return GrD3DTexture::MakeWrappedTexture(this, tex.dimensions(), wrapType, kRead_GrIOType,
                                            textureInfo, std::move(state));
}

sk_sp<GrTexture> GrD3DGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                          int sampleCnt,
                                                          GrWrapOwnership ownership,
                                                          GrWrapCacheable cacheable) {
    GrD3DTextureResourceInfo textureInfo;
    if (!tex.getD3DTextureResourceInfo(&textureInfo)) {
        return nullptr;
    }

    if (!check_resource_info(textureInfo)) {
        return nullptr;
    }

    if (!check_tex_resource_info(this->d3dCaps(), textureInfo)) {
        return nullptr;
    }
    if (!check_rt_resource_info(this->d3dCaps(), textureInfo, sampleCnt)) {
        return nullptr;
    }

    // TODO: support protected context
    if (tex.isProtected()) {
        return nullptr;
    }

    sampleCnt = this->d3dCaps().getRenderTargetSampleCount(sampleCnt, textureInfo.fFormat);

    sk_sp<GrD3DResourceState> state = tex.getGrD3DResourceState();
    SkASSERT(state);

    return GrD3DTextureRenderTarget::MakeWrappedTextureRenderTarget(this, tex.dimensions(),
                                                                    sampleCnt, cacheable,
                                                                    textureInfo, std::move(state));
}

sk_sp<GrRenderTarget> GrD3DGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt) {
    // Currently the Direct3D backend does not support wrapping of msaa render targets directly. In
    // general this is not an issue since swapchain images in D3D are never multisampled. Thus if
    // you want a multisampled RT it is best to wrap the swapchain images and then let Skia handle
    // creating and owning the MSAA images.
    if (rt.sampleCnt() > 1) {
        return nullptr;
    }

    GrD3DTextureResourceInfo info;
    if (!rt.getD3DTextureResourceInfo(&info)) {
        return nullptr;
    }

    if (!check_resource_info(info)) {
        return nullptr;
    }

    if (!check_rt_resource_info(this->d3dCaps(), info, rt.sampleCnt())) {
        return nullptr;
    }

    // TODO: support protected context
    if (rt.isProtected()) {
        return nullptr;
    }

    sk_sp<GrD3DResourceState> state = rt.getGrD3DResourceState();

    sk_sp<GrD3DRenderTarget> tgt = GrD3DRenderTarget::MakeWrappedRenderTarget(
            this, rt.dimensions(), 1, info, std::move(state));

    // We don't allow the client to supply a premade stencil buffer. We always create one if needed.
    SkASSERT(!rt.stencilBits());
    if (tgt) {
        SkASSERT(tgt->canAttemptStencilAttachment());
    }

    return std::move(tgt);
}

sk_sp<GrRenderTarget> GrD3DGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                   int sampleCnt) {

    GrD3DTextureResourceInfo textureInfo;
    if (!tex.getD3DTextureResourceInfo(&textureInfo)) {
        return nullptr;
    }
    if (!check_resource_info(textureInfo)) {
        return nullptr;
    }

    if (!check_rt_resource_info(this->d3dCaps(), textureInfo, sampleCnt)) {
        return nullptr;
    }

    // TODO: support protected context
    if (tex.isProtected()) {
        return nullptr;
    }

    sampleCnt = this->d3dCaps().getRenderTargetSampleCount(sampleCnt, textureInfo.fFormat);
    if (!sampleCnt) {
        return nullptr;
    }

    sk_sp<GrD3DResourceState> state = tex.getGrD3DResourceState();
    SkASSERT(state);

    return GrD3DRenderTarget::MakeWrappedRenderTarget(this, tex.dimensions(), sampleCnt,
                                                      textureInfo, std::move(state));
}

sk_sp<GrGpuBuffer> GrD3DGpu::onCreateBuffer(size_t sizeInBytes, GrGpuBufferType type,
                                             GrAccessPattern accessPattern, const void* data) {
    sk_sp<GrD3DBuffer> buffer = GrD3DBuffer::Make(this, sizeInBytes, type, accessPattern);
    if (data && buffer) {
        buffer->updateData(data, sizeInBytes);
    }

    return std::move(buffer);
}

GrStencilAttachment* GrD3DGpu::createStencilAttachmentForRenderTarget(
        const GrRenderTarget* rt, int width, int height, int numStencilSamples) {
    SkASSERT(numStencilSamples == rt->numSamples() || this->caps()->mixedSamplesSupport());
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    const GrD3DCaps::StencilFormat& sFmt = this->d3dCaps().preferredStencilFormat();

    GrD3DStencilAttachment* stencil(GrD3DStencilAttachment::Make(this,
                                                                 width,
                                                                 height,
                                                                 numStencilSamples,
                                                                 sFmt));
    fStats.incStencilAttachmentCreates();
    return stencil;
}

bool GrD3DGpu::createTextureResourceForBackendSurface(DXGI_FORMAT dxgiFormat,
                                                      SkISize dimensions,
                                                      GrTexturable texturable,
                                                      GrRenderable renderable,
                                                      GrMipMapped mipMapped,
                                                      GrD3DTextureResourceInfo* info,
                                                      GrProtected isProtected,
                                                      const BackendTextureData* data) {
    SkASSERT(texturable == GrTexturable::kYes || renderable == GrRenderable::kYes);
    if (texturable == GrTexturable::kNo) {
        SkASSERT(!data && mipMapped == GrMipMapped::kNo);
    }

    if (this->protectedContext() != (isProtected == GrProtected::kYes)) {
        return false;
    }

    if (texturable == GrTexturable::kYes && !this->d3dCaps().isFormatTexturable(dxgiFormat)) {
        return false;
    }

    if (renderable == GrRenderable::kYes && !this->d3dCaps().isFormatRenderable(dxgiFormat, 1)) {
        return false;
    }

    int numMipLevels = 1;
    if (mipMapped == GrMipMapped::kYes) {
        numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    // create the texture
    D3D12_RESOURCE_FLAGS usageFlags = D3D12_RESOURCE_FLAG_NONE;
    if (renderable == GrRenderable::kYes) {
        usageFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0; // use default alignment
    resourceDesc.Width = dimensions.fWidth;
    resourceDesc.Height = dimensions.fHeight;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = numMipLevels;
    resourceDesc.Format = dxgiFormat;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0; // quality levels are only supported for tiled resources
                                         // so ignore for now
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // use driver-selected swizzle
    resourceDesc.Flags = usageFlags;

    if (!GrD3DTextureResource::InitTextureResourceInfo(this, resourceDesc, isProtected, info)) {
        SkDebugf("Failed to init texture resource info\n");
        return false;
    }

    if (!data) {
        return true;
    }

    // TODO: upload the data

    return true;
}

GrBackendTexture GrD3DGpu::onCreateBackendTexture(SkISize dimensions,
                                                  const GrBackendFormat& format,
                                                  GrRenderable renderable,
                                                  GrMipMapped mipMapped,
                                                  GrProtected isProtected,
                                                  const BackendTextureData* data) {
    this->handleDirtyContext();

    const GrD3DCaps& caps = this->d3dCaps();

    if (this->protectedContext() != (isProtected == GrProtected::kYes)) {
        return {};
    }

    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return {};
    }

    // TODO: move the texturability check up to GrGpu::createBackendTexture and just assert here
    if (!caps.isFormatTexturable(dxgiFormat)) {
        return {};
    }

    GrD3DTextureResourceInfo info;
    if (!this->createTextureResourceForBackendSurface(dxgiFormat, dimensions, GrTexturable::kYes,
                                                      renderable, mipMapped,
                                                      &info, isProtected, data)) {
        return {};
    }

    return GrBackendTexture(dimensions.width(), dimensions.height(), info);
}

GrBackendTexture GrD3DGpu::onCreateCompressedBackendTexture(SkISize dimensions,
                                                            const GrBackendFormat& format,
                                                            GrMipMapped mipMapped,
                                                            GrProtected isProtected,
                                                            const BackendTextureData* data) {
    this->handleDirtyContext();

    const GrD3DCaps& caps = this->d3dCaps();

    if (this->protectedContext() != (isProtected == GrProtected::kYes)) {
        return {};
    }

    DXGI_FORMAT dxgiFormat;
    if (!format.asDxgiFormat(&dxgiFormat)) {
        return {};
    }

    // TODO: move the texturability check up to GrGpu::createBackendTexture and just assert here
    if (!caps.isFormatTexturable(dxgiFormat)) {
        return {};
    }

    GrD3DTextureResourceInfo info;
    if (!this->createTextureResourceForBackendSurface(dxgiFormat, dimensions, GrTexturable::kYes,
                                                      GrRenderable::kNo, mipMapped,
                                                      &info, isProtected, data)) {
        return {};
    }

    return GrBackendTexture(dimensions.width(), dimensions.height(), info);
}

void GrD3DGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kDirect3D == tex.fBackend);
    // Nothing to do here, will get cleaned up when the GrBackendTexture object goes away
}

bool GrD3DGpu::compile(const GrProgramDesc&, const GrProgramInfo&) {
    return false;
}

#if GR_TEST_UTILS
bool GrD3DGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(GrBackendApi::kDirect3D == tex.backend());

    GrD3DTextureResourceInfo info;
    if (!tex.getD3DTextureResourceInfo(&info)) {
        return false;
    }
    ID3D12Resource* textureResource = info.fResource.get();
    if (!textureResource) {
        return false;
    }
    return !(textureResource->GetDesc().Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
}

GrBackendRenderTarget GrD3DGpu::createTestingOnlyBackendRenderTarget(int w, int h,
                                                                     GrColorType colorType) {
    this->handleDirtyContext();

    if (w > this->caps()->maxRenderTargetSize() || h > this->caps()->maxRenderTargetSize()) {
        return GrBackendRenderTarget();
    }

    DXGI_FORMAT dxgiFormat = this->d3dCaps().getFormatFromColorType(colorType);

    GrD3DTextureResourceInfo info;
    if (!this->createTextureResourceForBackendSurface(dxgiFormat, { w, h }, GrTexturable::kNo,
                                                      GrRenderable::kYes, GrMipMapped::kNo,
                                                      &info, GrProtected::kNo, nullptr)) {
        return {};
    }

    return GrBackendRenderTarget(w, h, 1, info);
}

void GrD3DGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    SkASSERT(GrBackendApi::kDirect3D == rt.backend());

    GrD3DTextureResourceInfo info;
    if (rt.getD3DTextureResourceInfo(&info)) {
        this->testingOnly_flushGpuAndSync();
        // Nothing else to do here, will get cleaned up when the GrBackendRenderTarget
        // is deleted.
    }
}

void GrD3DGpu::testingOnly_flushGpuAndSync() {
    // TODO
}
#endif

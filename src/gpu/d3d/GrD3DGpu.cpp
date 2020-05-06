/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DGpu.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/d3d/GrD3DBackendContext.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DCaps.h"
#include "src/gpu/d3d/GrD3DOpsRenderPass.h"
#include "src/gpu/d3d/GrD3DStencilAttachment.h"
#include "src/gpu/d3d/GrD3DTexture.h"
#include "src/gpu/d3d/GrD3DTextureRenderTarget.h"
#include "src/gpu/d3d/GrD3DUtil.h"
#include "src/sksl/SkSLCompiler.h"

#if GR_TEST_UTILS
#include <DXProgrammableCapture.h>
#endif

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
        , fOutstandingCommandLists(sizeof(OutstandingCommandList), kDefaultOutstandingAllocCnt)
        , fCompiler(new SkSL::Compiler()) {
    fCaps.reset(new GrD3DCaps(contextOptions,
                              backendContext.fAdapter.get(),
                              backendContext.fDevice.get()));

    fCurrentDirectCommandList = fResourceProvider.findOrCreateDirectCommandList();
    SkASSERT(fCurrentDirectCommandList);

    SkASSERT(fCurrentFenceValue == 0);
    SkDEBUGCODE(HRESULT hr = ) fDevice->CreateFence(fCurrentFenceValue, D3D12_FENCE_FLAG_NONE,
                                                    IID_PPV_ARGS(&fFence));
    SkASSERT(SUCCEEDED(hr));

#if GR_TEST_UTILS
    HRESULT getAnalysis = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&fGraphicsAnalysis));
    if (FAILED(getAnalysis)) {
        fGraphicsAnalysis = nullptr;
    }
#endif
}

GrD3DGpu::~GrD3DGpu() {
    this->destroyResources();
}

void GrD3DGpu::destroyResources() {
    if (fCurrentDirectCommandList) {
        fCurrentDirectCommandList->close();
        fCurrentDirectCommandList->reset();
    }

    // We need to make sure everything has finished on the queue.
    this->waitForQueueCompletion();

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

bool GrD3DGpu::submitDirectCommandList(SyncQueue sync) {
    SkASSERT(fCurrentDirectCommandList);

    GrD3DDirectCommandList::SubmitResult result = fCurrentDirectCommandList->submit(fQueue.get());
    if (result == GrD3DDirectCommandList::SubmitResult::kFailure) {
        return false;
    } else if (result == GrD3DDirectCommandList::SubmitResult::kNoWork) {
        if (sync == SyncQueue::kForce) {
            this->waitForQueueCompletion();
        }
        return true;
    }

    new (fOutstandingCommandLists.push_back()) OutstandingCommandList(
            std::move(fCurrentDirectCommandList), ++fCurrentFenceValue);

    SkDEBUGCODE(HRESULT hr = ) fQueue->Signal(fFence.get(), fCurrentFenceValue);
    SkASSERT(SUCCEEDED(hr));

    if (sync == SyncQueue::kForce) {
        this->waitForQueueCompletion();
    }

    fCurrentDirectCommandList = fResourceProvider.findOrCreateDirectCommandList();

    // This should be done after we have a new command list in case the freeing of any resources
    // held by a finished command list causes us send a new command to the gpu (like changing the
    // resource state.
    this->checkForFinishedCommandLists();

    SkASSERT(fCurrentDirectCommandList);
    return true;
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

void GrD3DGpu::waitForQueueCompletion() {
    if (fFence->GetCompletedValue() < fCurrentFenceValue) {
        HANDLE fenceEvent;
        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        SkASSERT(fenceEvent);
        SkDEBUGCODE(HRESULT hr = ) fFence->SetEventOnCompletion(fCurrentFenceValue, fenceEvent);
        SkASSERT(SUCCEEDED(hr));
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
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
    D3D12_RESOURCE_DESC resourceDesc = {};
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
    // quality levels are only supported for tiled resources so ignore for now
    resourceDesc.SampleDesc.Quality = GrD3DTextureResource::kDefaultQualityLevel;
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

static int get_surface_sample_cnt(GrSurface* surf) {
    if (const GrRenderTarget* rt = surf->asRenderTarget()) {
        return rt->numSamples();
    }
    return 0;
}

bool GrD3DGpu::onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                   const SkIPoint& dstPoint) {

    if (src->isProtected() && !dst->isProtected()) {
        SkDebugf("Can't copy from protected memory to non-protected");
        return false;
    }

    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);

    GrD3DTextureResource* dstTexResource;
    GrD3DTextureResource* srcTexResource;
    GrRenderTarget* dstRT = dst->asRenderTarget();
    if (dstRT) {
        GrD3DRenderTarget* d3dRT = static_cast<GrD3DRenderTarget*>(dstRT);
        dstTexResource = d3dRT->numSamples() > 1 ? d3dRT->msaaTextureResource() : d3dRT;
    } else {
        SkASSERT(dst->asTexture());
        dstTexResource = static_cast<GrD3DTexture*>(dst->asTexture());
    }
    GrRenderTarget* srcRT = src->asRenderTarget();
    if (srcRT) {
        GrD3DRenderTarget* d3dRT = static_cast<GrD3DRenderTarget*>(srcRT);
        srcTexResource = d3dRT->numSamples() > 1 ? d3dRT->msaaTextureResource() : d3dRT;
    } else {
        SkASSERT(src->asTexture());
        srcTexResource = static_cast<GrD3DTexture*>(src->asTexture());
    }

    DXGI_FORMAT dstFormat = dstTexResource->dxgiFormat();
    DXGI_FORMAT srcFormat = srcTexResource->dxgiFormat();

    if (this->d3dCaps().canCopyAsResolve(dstFormat, dstSampleCnt, srcFormat, srcSampleCnt)) {
        this->copySurfaceAsResolve(dst, src, srcRect, dstPoint);
        return true;
    }

    if (this->d3dCaps().canCopyTexture(dstFormat, dstSampleCnt, srcFormat, srcSampleCnt)) {
        this->copySurfaceAsCopyTexture(dst, src, dstTexResource, srcTexResource, srcRect, dstPoint);
        return true;
    }

    return false;
}

void GrD3DGpu::copySurfaceAsCopyTexture(GrSurface* dst, GrSurface* src,
                                        GrD3DTextureResource* dstResource,
                                        GrD3DTextureResource* srcResource,
                                        const SkIRect& srcRect, const SkIPoint& dstPoint) {
#ifdef SK_DEBUG
    int dstSampleCnt = get_surface_sample_cnt(dst);
    int srcSampleCnt = get_surface_sample_cnt(src);
    DXGI_FORMAT dstFormat = dstResource->dxgiFormat();
    DXGI_FORMAT srcFormat;
    SkAssertResult(dst->backendFormat().asDxgiFormat(&srcFormat));
    SkASSERT(this->d3dCaps().canCopyTexture(dstFormat, dstSampleCnt, srcFormat, srcSampleCnt));
#endif
    if (src->isProtected() && !dst->isProtected()) {
        SkDebugf("Can't copy from protected memory to non-protected");
        return;
    }

    dstResource->setResourceState(this, D3D12_RESOURCE_STATE_COPY_DEST);
    srcResource->setResourceState(this, D3D12_RESOURCE_STATE_COPY_SOURCE);

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = dstResource->d3dResource();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = srcResource->d3dResource();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = 0;

    D3D12_BOX srcBox = {};
    srcBox.left = srcRect.fLeft;
    srcBox.top = srcRect.fTop;
    srcBox.right = srcRect.fRight;
    srcBox.bottom = srcRect.fBottom;
    srcBox.front = 0;
    srcBox.back = 1;
    // TODO: use copyResource if copying full resource and sizes match
    fCurrentDirectCommandList->copyTextureRegion(dstResource->resource(),
                                                 &dstLocation,
                                                 dstPoint.fX, dstPoint.fY,
                                                 srcResource->resource(),
                                                 &srcLocation,
                                                 &srcBox);

    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    // The rect is already in device space so we pass in kTopLeft so no flip is done.
    this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
}

void GrD3DGpu::copySurfaceAsResolve(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                    const SkIPoint& dstPoint) {
    // TODO
}

bool GrD3DGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                            GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                            size_t rowBytes) {
    SkASSERT(surface);

    if (surfaceColorType != dstColorType) {
        return false;
    }

    // Set up src location and box
    GrD3DTextureResource* texResource = nullptr;
    GrD3DRenderTarget* rt = static_cast<GrD3DRenderTarget*>(surface->asRenderTarget());
    if (rt) {
        texResource = rt;
    } else {
        texResource = static_cast<GrD3DTexture*>(surface->asTexture());
    }

    if (!texResource) {
        return false;
    }

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = texResource->d3dResource();
    SkASSERT(srcLocation.pResource);
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = 0;

    D3D12_BOX srcBox = {};
    srcBox.left = left;
    srcBox.top = top;
    srcBox.right = left + width;
    srcBox.bottom = top + height;
    srcBox.front = 0;
    srcBox.back = 1;

    // Set up dst location and create transfer buffer
    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    UINT64 transferTotalBytes;
    const UINT64 baseOffset = 0;
    D3D12_RESOURCE_DESC desc = srcLocation.pResource->GetDesc();
    fDevice->GetCopyableFootprints(&desc, 0, 1, baseOffset, &dstLocation.PlacedFootprint,
                                   nullptr, nullptr, &transferTotalBytes);
    SkASSERT(transferTotalBytes);
    size_t bpp = GrColorTypeBytesPerPixel(dstColorType);
    if (this->d3dCaps().bytesPerPixel(texResource->dxgiFormat()) != bpp) {
        return false;
    }
    size_t tightRowBytes = bpp * width;

    // TODO: implement some way of reusing buffers instead of making a new one every time.
    sk_sp<GrGpuBuffer> transferBuffer = this->createBuffer(transferTotalBytes,
                                                           GrGpuBufferType::kXferGpuToCpu,
                                                           kDynamic_GrAccessPattern);
    GrD3DBuffer* d3dBuf = static_cast<GrD3DBuffer*>(transferBuffer.get());
    dstLocation.pResource = d3dBuf->d3dResource();

    // Need to change the resource state to COPY_SOURCE in order to download from it
    texResource->setResourceState(this, D3D12_RESOURCE_STATE_COPY_SOURCE);

    fCurrentDirectCommandList->copyTextureRegion(d3dBuf->resource(), &dstLocation, 0, 0,
                                                 texResource->resource(), &srcLocation, &srcBox);
    this->submitDirectCommandList(SyncQueue::kForce);

    const void* mappedMemory = transferBuffer->map();

    SkRectMemcpy(buffer, rowBytes, mappedMemory, dstLocation.PlacedFootprint.Footprint.RowPitch,
                 tightRowBytes, height);

    transferBuffer->unmap();

    return true;
}

bool GrD3DGpu::onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                             GrColorType surfaceColorType, GrColorType srcColorType,
                             const GrMipLevel texels[], int mipLevelCount,
                             bool prepForTexSampling) {
    GrD3DTexture* d3dTex = static_cast<GrD3DTexture*>(surface->asTexture());
    if (!d3dTex) {
        return false;
    }

    // Make sure we have at least the base level
    if (!mipLevelCount || !texels[0].fPixels) {
        return false;
    }

    SkASSERT(!GrDxgiFormatIsCompressed(d3dTex->dxgiFormat()));
    bool success = false;

    // Need to change the resource state to COPY_DEST in order to upload to it
    d3dTex->setResourceState(this, D3D12_RESOURCE_STATE_COPY_DEST);

    SkASSERT(mipLevelCount <= d3dTex->texturePriv().maxMipMapLevel() + 1);
    success = this->uploadToTexture(d3dTex, left, top, width, height, srcColorType, texels,
                                    mipLevelCount);

    if (prepForTexSampling) {
        d3dTex->setResourceState(this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    return success;
}

bool GrD3DGpu::uploadToTexture(GrD3DTexture* tex, int left, int top, int width, int height,
                               GrColorType colorType, const GrMipLevel* texels, int mipLevelCount) {
    SkASSERT(this->caps()->isFormatTexturable(tex->backendFormat()));
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(1 == mipLevelCount ||
             (0 == left && 0 == top && width == tex->width() && height == tex->height()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
    SkASSERT(1 == mipLevelCount || mipLevelCount == (tex->texturePriv().maxMipMapLevel() + 1));

    if (width == 0 || height == 0) {
        return false;
    }

    SkASSERT(this->d3dCaps().surfaceSupportsWritePixels(tex));
    SkASSERT(this->d3dCaps().areColorTypeAndFormatCompatible(colorType, tex->backendFormat()));

    ID3D12Resource* d3dResource = tex->d3dResource();
    SkASSERT(d3dResource);
    D3D12_RESOURCE_DESC desc = d3dResource->GetDesc();
    // Either upload only the first miplevel or all miplevels
    SkASSERT(1 == mipLevelCount || mipLevelCount == (int)desc.MipLevels);

    if (1 == mipLevelCount && !texels[0].fPixels) {
        return true;   // no data to upload
    }

    for (int i = 0; i < mipLevelCount; ++i) {
        // We do not allow any gaps in the mip data
        if (!texels[i].fPixels) {
            return false;
        }
    }

    SkAutoTMalloc<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> placedFootprints(mipLevelCount);
    UINT64 combinedBufferSize;
    // We reset the width and height in the description to match our subrectangle size
    // so we don't end up allocating more space than we need.
    desc.Width = width;
    desc.Height = height;
    fDevice->GetCopyableFootprints(&desc, 0, mipLevelCount, 0, placedFootprints.get(),
                                   nullptr, nullptr, &combinedBufferSize);
    size_t bpp = GrColorTypeBytesPerPixel(colorType);
    SkASSERT(combinedBufferSize);

    // TODO: do this until we have slices of buttery buffers
    sk_sp<GrGpuBuffer> transferBuffer = this->createBuffer(combinedBufferSize,
                                                           GrGpuBufferType::kXferCpuToGpu,
                                                           kDynamic_GrAccessPattern);
    if (!transferBuffer) {
        return false;
    }
    char* bufferData = (char*)transferBuffer->map();

    int currentWidth = width;
    int currentHeight = height;
    int layerHeight = tex->height();

    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (texels[currentMipLevel].fPixels) {
            SkASSERT(1 == mipLevelCount || currentHeight == layerHeight);

            const size_t trimRowBytes = currentWidth * bpp;
            const size_t srcRowBytes = texels[currentMipLevel].fRowBytes;

            char* dst = bufferData + placedFootprints[currentMipLevel].Offset;

            // copy data into the buffer, skipping any trailing bytes
            const char* src = (const char*)texels[currentMipLevel].fPixels;
            SkRectMemcpy(dst, placedFootprints[currentMipLevel].Footprint.RowPitch,
                         src, srcRowBytes, trimRowBytes, currentHeight);
        }
        currentWidth = std::max(1, currentWidth / 2);
        currentHeight = std::max(1, currentHeight / 2);
        layerHeight = currentHeight;
    }

    transferBuffer->unmap();

    GrD3DBuffer* d3dBuffer = static_cast<GrD3DBuffer*>(transferBuffer.get());
    fCurrentDirectCommandList->copyBufferToTexture(d3dBuffer, tex, mipLevelCount,
                                                   placedFootprints.get(), left, top);

    if (mipLevelCount < (int)desc.MipLevels) {
        tex->texturePriv().markMipMapsDirty();
    }

    return true;
}

void GrD3DGpu::clear(const GrFixedClip& clip, const SkPMColor4f& color, GrRenderTarget* rt) {
    GrD3DRenderTarget* d3dRT = static_cast<GrD3DRenderTarget*>(rt);

    d3dRT->setResourceState(this, D3D12_RESOURCE_STATE_RENDER_TARGET);

    fCurrentDirectCommandList->clearRenderTargetView(d3dRT, color, clip);
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
                                                      GrProtected isProtected) {
    SkASSERT(texturable == GrTexturable::kYes || renderable == GrRenderable::kYes);

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

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;  // use default alignment
    resourceDesc.Width = dimensions.fWidth;
    resourceDesc.Height = dimensions.fHeight;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = numMipLevels;
    resourceDesc.Format = dxgiFormat;
    resourceDesc.SampleDesc.Count = 1;
    // quality levels are only supported for tiled resources so ignore for now
    resourceDesc.SampleDesc.Quality = GrD3DTextureResource::kDefaultQualityLevel;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // use driver-selected swizzle
    resourceDesc.Flags = usageFlags;

    D3D12_CLEAR_VALUE* clearValuePtr = nullptr;
    D3D12_CLEAR_VALUE clearValue = {};
    if (renderable == GrRenderable::kYes) {
        clearValue.Format = dxgiFormat;
        // Assume transparent black
        clearValue.Color[0] = 0;
        clearValue.Color[1] = 0;
        clearValue.Color[2] = 0;
        clearValue.Color[3] = 0;
        clearValuePtr = &clearValue;
    }

    D3D12_RESOURCE_STATES initialState = (renderable == GrRenderable::kYes)
                                                 ? D3D12_RESOURCE_STATE_RENDER_TARGET
                                                 : D3D12_RESOURCE_STATE_COPY_DEST;
    if (!GrD3DTextureResource::InitTextureResourceInfo(this, resourceDesc, initialState,
                                                       isProtected, clearValuePtr, info)) {
        SkDebugf("Failed to init texture resource info\n");
        return false;
    }

    return true;
}

GrBackendTexture GrD3DGpu::onCreateBackendTexture(SkISize dimensions,
                                                  const GrBackendFormat& format,
                                                  GrRenderable renderable,
                                                  GrMipMapped mipMapped,
                                                  GrProtected isProtected) {
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
                                                      &info, isProtected)) {
        return {};
    }

    return GrBackendTexture(dimensions.width(), dimensions.height(), info);
}

bool GrD3DGpu::onUpdateBackendTexture(const GrBackendTexture& backendTexture,
                                      sk_sp<GrRefCntedCallback> finishedCallback,
                                      const BackendTextureData* data) {
    // TODO: handle finishedCallback and data upload
    return true;
}

GrBackendTexture GrD3DGpu::onCreateCompressedBackendTexture(
        SkISize dimensions, const GrBackendFormat& format, GrMipMapped mipMapped,
        GrProtected isProtected, sk_sp<GrRefCntedCallback> finishedCallback,
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
                                                      &info, isProtected)) {
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
        return {};
    }

    DXGI_FORMAT dxgiFormat = this->d3dCaps().getFormatFromColorType(colorType);

    GrD3DTextureResourceInfo info;
    if (!this->createTextureResourceForBackendSurface(dxgiFormat, { w, h }, GrTexturable::kNo,
                                                      GrRenderable::kYes, GrMipMapped::kNo,
                                                      &info, GrProtected::kNo)) {
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
    SkAssertResult(this->submitDirectCommandList(SyncQueue::kForce));
}

void GrD3DGpu::testingOnly_startCapture() {
    if (fGraphicsAnalysis) {
        fGraphicsAnalysis->BeginCapture();
    }
}

void GrD3DGpu::testingOnly_endCapture() {
    if (fGraphicsAnalysis) {
        fGraphicsAnalysis->EndCapture();
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////

void GrD3DGpu::addResourceBarriers(sk_sp<GrManagedResource> resource,
                                   int numBarriers,
                                   D3D12_RESOURCE_TRANSITION_BARRIER* barriers) const {
    SkASSERT(fCurrentDirectCommandList);
    SkASSERT(resource);

    fCurrentDirectCommandList->resourceBarrier(std::move(resource), numBarriers, barriers);
}

bool GrD3DGpu::onSubmitToGpu(bool syncCpu) {
    if (syncCpu) {
        return this->submitDirectCommandList(SyncQueue::kForce);
    } else {
        return this->submitDirectCommandList(SyncQueue::kSkip);
    }
}

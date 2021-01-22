/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrD3DTextureRenderTarget_DEFINED
#define GrD3DTextureRenderTarget_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/d3d/GrD3DRenderTarget.h"
#include "src/gpu/d3d/GrD3DTexture.h"

class GrD3DGpu;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

struct GrD3DTextureResourceInfo;

class GrD3DTextureRenderTarget: public GrD3DTexture, public GrD3DRenderTarget {
public:
    static sk_sp<GrD3DTextureRenderTarget> MakeNewTextureRenderTarget(GrD3DGpu*, SkBudgeted,
                                                                      SkISize dimensions,
                                                                      int sampleCnt,
                                                                      const D3D12_RESOURCE_DESC&,
                                                                      GrProtected isProtected,
                                                                      GrMipmapStatus);

    static sk_sp<GrD3DTextureRenderTarget> MakeWrappedTextureRenderTarget(
            GrD3DGpu*, SkISize dimensions, int sampleCnt, GrWrapCacheable,
            const GrD3DTextureResourceInfo&, sk_sp<GrD3DResourceState>);

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

protected:
    void onAbandon() override {
        // In order to correctly handle calling texture idle procs, GrD3DTexture must go first.
        GrD3DTexture::onAbandon();
        GrD3DRenderTarget::onAbandon();
    }

    void onRelease() override {
        // In order to correctly handle calling texture idle procs, GrD3DTexture must go first.
        GrD3DTexture::onRelease();
        GrD3DRenderTarget::onRelease();
    }

private:
    // MSAA, not-wrapped
    GrD3DTextureRenderTarget(GrD3DGpu* gpu,
                             SkBudgeted budgeted,
                             SkISize dimensions,
                             const GrD3DTextureResourceInfo& info,
                             sk_sp<GrD3DResourceState> state,
                             const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                             const GrD3DTextureResourceInfo& msaaInfo,
                             sk_sp<GrD3DResourceState> msaaState,
                             const GrD3DDescriptorHeap::CPUHandle& colorRenderTargetView,
                             const GrD3DDescriptorHeap::CPUHandle& resolveRenderTargetView,
                             GrMipmapStatus);

    // non-MSAA, not-wrapped
    GrD3DTextureRenderTarget(GrD3DGpu* gpu,
                             SkBudgeted budgeted,
                             SkISize dimensions,
                             const GrD3DTextureResourceInfo& info,
                             sk_sp<GrD3DResourceState> state,
                             const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                             const GrD3DDescriptorHeap::CPUHandle& renderTargetView,
                             GrMipmapStatus);

    // MSAA, wrapped
    GrD3DTextureRenderTarget(GrD3DGpu* gpu,
                             SkISize dimensions,
                             const GrD3DTextureResourceInfo& info,
                             sk_sp<GrD3DResourceState> state,
                             const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                             const GrD3DTextureResourceInfo& msaaInfo,
                             sk_sp<GrD3DResourceState> msaaState,
                             const GrD3DDescriptorHeap::CPUHandle& colorRenderTargetView,
                             const GrD3DDescriptorHeap::CPUHandle& resolveRenderTargetView,
                             GrMipmapStatus,
                             GrWrapCacheable);

    // non-MSAA, wrapped
    GrD3DTextureRenderTarget(GrD3DGpu* gpu,
                             SkISize dimensions,
                             const GrD3DTextureResourceInfo& info,
                             sk_sp<GrD3DResourceState> state,
                             const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                             const GrD3DDescriptorHeap::CPUHandle& renderTargetView,
                             GrMipmapStatus,
                             GrWrapCacheable);

    // GrGLRenderTarget accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override;

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrD3DImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {
        // Forward the release proc on to GrD3DImage
        this->setResourceRelease(std::move(releaseHelper));
    }
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif

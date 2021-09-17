/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DTexture_DEFINED
#define GrD3DTexture_DEFINED

#include "src/core/SkLRUCache.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/d3d/GrD3DDescriptorHeap.h"
#include "src/gpu/d3d/GrD3DTextureResource.h"

class GrD3DTexture : public GrTexture, public virtual GrD3DTextureResource {
public:
    static sk_sp<GrD3DTexture> MakeNewTexture(GrD3DGpu*,
                                              SkBudgeted,
                                              SkISize dimensions,
                                              const D3D12_RESOURCE_DESC&,
                                              GrProtected,
                                              GrMipmapStatus);

    static sk_sp<GrD3DTexture> MakeWrappedTexture(GrD3DGpu*,
                                                  SkISize dimensions,
                                                  GrWrapCacheable,
                                                  GrIOType,
                                                  const GrD3DTextureResourceInfo&,
                                                  sk_sp<GrD3DResourceState>);

    static sk_sp<GrD3DTexture> MakeAliasingTexture(GrD3DGpu*,
                                                   sk_sp<GrD3DTexture>,
                                                   const D3D12_RESOURCE_DESC& newDesc,
                                                   D3D12_RESOURCE_STATES);

    ~GrD3DTexture() override {}

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }
    D3D12_CPU_DESCRIPTOR_HANDLE shaderResourceView() { return fShaderResourceView.fHandle; }

    void textureParamsModified() override {}

protected:
    GrD3DTexture(GrD3DGpu*,
                 SkISize dimensions,
                 const GrD3DTextureResourceInfo&,
                 sk_sp<GrD3DResourceState>,
                 const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                 GrMipmapStatus);

    GrD3DGpu* getD3DGpu() const;

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
        return false;
    }

private:
    GrD3DTexture(GrD3DGpu*, SkBudgeted, SkISize dimensions, const GrD3DTextureResourceInfo&,
                 sk_sp<GrD3DResourceState>,
                 const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                 GrMipmapStatus);
    GrD3DTexture(GrD3DGpu*, SkISize dimensions, const GrD3DTextureResourceInfo&,
                 sk_sp<GrD3DResourceState>,
                 const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                 GrMipmapStatus, GrWrapCacheable, GrIOType);

    // In D3D we call the release proc after we are finished with the underlying
    // GrSurfaceResource::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {
        // Forward the release proc on to GrSurfaceResource
        this->setResourceRelease(std::move(releaseHelper));
    }

    struct SamplerHash {
        uint32_t operator()(GrSamplerState state) const { return state.asIndex(); }
    };

    GrD3DDescriptorHeap::CPUHandle fShaderResourceView;

    using INHERITED = GrTexture;
};

#endif

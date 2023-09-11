/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrD3DRenderTarget_DEFINED
#define GrD3DRenderTarget_DEFINED

#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/d3d/GrD3DTextureResource.h"

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/d3d/GrD3DDescriptorHeap.h"
#include "src/gpu/ganesh/d3d/GrD3DResourceProvider.h"

class GrD3DGpu;
class GrD3DRenderTarget;

struct GrD3DTextureResourceInfo;

class GrD3DRenderTarget: public GrRenderTarget, public virtual GrD3DTextureResource {
public:
    static sk_sp<GrD3DRenderTarget> MakeWrappedRenderTarget(GrD3DGpu*, SkISize, int sampleCnt,
                                                            const GrD3DTextureResourceInfo&,
                                                            sk_sp<GrD3DResourceState>);
    ~GrD3DRenderTarget() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    /**
     * If this render target is multisampled, this returns the MSAA texture for rendering. This
     * will be different than *this* when we have separate render/resolve images. If not
     * multisampled returns nullptr.
     */
    const GrD3DTextureResource* msaaTextureResource() const;
    GrD3DTextureResource* msaaTextureResource();

    bool canAttemptStencilAttachment(bool useMSAASurface) const override {
        SkASSERT(useMSAASurface == (this->numSamples() > 1));
        return true;
    }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    D3D12_CPU_DESCRIPTOR_HANDLE colorRenderTargetView() const {
        return fColorRenderTargetView.fHandle;
    }

    DXGI_FORMAT stencilDxgiFormat() const;

    // Key used for the program desc
    void genKey(skgpu::KeyBuilder* b) const;

protected:
    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      const GrD3DTextureResourceInfo& info,
                      sk_sp<GrD3DResourceState> state,
                      const GrD3DTextureResourceInfo& msaaInfo,
                      sk_sp<GrD3DResourceState> msaaState,
                      const GrD3DDescriptorHeap::CPUHandle& colorRenderTargetView,
                      const GrD3DDescriptorHeap::CPUHandle& resolveRenderTargetView,
                      std::string_view label);

    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      const GrD3DTextureResourceInfo& info,
                      sk_sp<GrD3DResourceState> state,
                      const GrD3DDescriptorHeap::CPUHandle& renderTargetView,
                      std::string_view label);

    void onAbandon() override;
    void onRelease() override;

    // This accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        int numColorSamples = this->numSamples();
        if (numColorSamples > 1) {
            // Add one to account for the resolved VkImage.
            numColorSamples += 1;
        }
        return GrSurface::ComputeSize(
                this->backendFormat(), this->dimensions(), numColorSamples, skgpu::Mipmapped::kNo);
    }

    void onSetLabel() override;

private:
    // Extra param to disambiguate from constructor used by subclasses.
    enum Wrapped { kWrapped };
    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      const GrD3DTextureResourceInfo& info,
                      sk_sp<GrD3DResourceState> state,
                      const GrD3DTextureResourceInfo& msaaInfo,
                      sk_sp<GrD3DResourceState> msaaState,
                      const GrD3DDescriptorHeap::CPUHandle& colorRenderTargetView,
                      const GrD3DDescriptorHeap::CPUHandle& resolveRenderTargetView,
                      Wrapped,
                      std::string_view label);

    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      const GrD3DTextureResourceInfo& info,
                      sk_sp<GrD3DResourceState> state,
                      const GrD3DDescriptorHeap::CPUHandle& renderTargetView,
                      Wrapped,
                      std::string_view label);

    GrD3DGpu* getD3DGpu() const;

    bool completeStencilAttachment(GrAttachment* stencil, bool useMSAASurface) override {
        SkASSERT(useMSAASurface == (this->numSamples() > 1));
        return true;
    }

    // In Direct3D we call the release proc after we are finished with the underlying
    // GrD3DTextureResource::Resource object (which occurs after the GPU finishes all work on it).
    void onSetRelease(sk_sp<RefCntedReleaseProc> releaseHelper) override {
        // Forward the release proc on to GrD3DTextureResource
        this->setResourceRelease(std::move(releaseHelper));
    }

    void releaseInternalObjects();

    std::unique_ptr<GrD3DTextureResource> fMSAATextureResource;

    GrD3DDescriptorHeap::CPUHandle fColorRenderTargetView;
    GrD3DDescriptorHeap::CPUHandle fResolveRenderTargetView;
};

#endif

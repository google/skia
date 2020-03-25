/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrD3DRenderTarget_DEFINED
#define GrD3DRenderTarget_DEFINED

#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/d3d/GrD3DSurfaceResource.h"

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/d3d/GrD3DResourceProvider.h"

class GrD3DCommandList;
class GrD3DGpu;
class GrD3DRenderTarget;

struct GrD3DTextureInfo;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrD3DRenderTarget: public GrRenderTarget, public virtual GrD3DSurfaceResource {
public:
    static sk_sp<GrD3DRenderTarget> MakeWrappedRenderTarget(GrD3DGpu*, SkISize, int sampleCnt,
                                                            const GrD3DTextureInfo&,
                                                            sk_sp<GrD3DResourceState>);
    ~GrD3DRenderTarget() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    const GrManagedResource* msaaTextureResource() const {
        if (fMSAATexture) {
            return fMSAATexture->resource();
        }
        return nullptr;
    }
    GrD3DSurfaceResource* msaaTexture() { return fMSAATexture.get(); }

    bool canAttemptStencilAttachment() const override {
        return false; // For now
    }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    void addResources(GrD3DCommandList& commandList);

protected:
    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      int sampleCnt,
                      const GrD3DTextureInfo& info,
                      sk_sp<GrD3DResourceState> layout,
                      const GrD3DTextureInfo& msaaInfo,
                      sk_sp<GrD3DResourceState> msaaLayout,
                      GrBackendObjectOwnership);

    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      const GrD3DTextureInfo& info,
                      sk_sp<GrD3DResourceState> layout,
                      GrBackendObjectOwnership);

    void onAbandon() override;
    void onRelease() override;

    // This accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        int numColorSamples = this->numSamples();
        if (numColorSamples > 1) {
            // Add one to account for the resolved VkImage.
            numColorSamples += 1;
        }
        const GrCaps& caps = *this->getGpu()->caps();
        return GrSurface::ComputeSize(caps, this->backendFormat(), this->dimensions(),
                                      numColorSamples, GrMipMapped::kNo);
    }

private:
    GrD3DRenderTarget(GrD3DGpu* gpu,
                     SkISize dimensions,
                     int sampleCnt,
                     const GrD3DTextureInfo& info,
                     sk_sp<GrD3DResourceState> state,
                     const GrD3DTextureInfo& msaaInfo,
                     sk_sp<GrD3DResourceState> msaaState);

    GrD3DRenderTarget(GrD3DGpu* gpu,
                     SkISize dimensions,
                     const GrD3DTextureInfo& info,
                     sk_sp<GrD3DResourceState> state);

    GrD3DGpu* getD3DGpu() const;

    bool completeStencilAttachment() override { /* TODO */ return false; }

    // In Vulkan we call the release proc after we are finished with the underlying
    // GrD3DImage::Resource object (which occurs after the GPU has finished all work on it).
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {
        // Forward the release proc on to GrD3DImage
        this->setResourceRelease(std::move(releaseHelper));
    }

    void releaseInternalObjects();

    std::unique_ptr<GrD3DSurfaceResource> fMSAATexture;
};

#endif

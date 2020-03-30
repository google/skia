/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrD3DRenderTarget_DEFINED
#define GrD3DRenderTarget_DEFINED

#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/d3d/GrD3DTextureResource.h"

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/d3d/GrD3DResourceProvider.h"

class GrD3DGpu;
class GrD3DRenderTarget;

struct GrD3DTextureResourceInfo;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrD3DRenderTarget: public GrRenderTarget, public virtual GrD3DTextureResource {
public:
    static sk_sp<GrD3DRenderTarget> MakeWrappedRenderTarget(GrD3DGpu*, SkISize, int sampleCnt,
                                                            const GrD3DTextureResourceInfo&,
                                                            sk_sp<GrD3DResourceState>);
    ~GrD3DRenderTarget() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    GrD3DTextureResource* msaaTextureResource() { return fMSAATextureResource.get(); }

    bool canAttemptStencilAttachment() const override {
        return false; // For now
    }

    GrBackendRenderTarget getBackendRenderTarget() const override;

protected:
    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      int sampleCnt,
                      const GrD3DTextureResourceInfo& info,
                      sk_sp<GrD3DResourceState> state,
                      const GrD3DTextureResourceInfo& msaaInfo,
                      sk_sp<GrD3DResourceState> msaaState);

    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      const GrD3DTextureResourceInfo& info,
                      sk_sp<GrD3DResourceState> state);

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
    // Extra param to disambiguate from constructor used by subclasses.
    enum Wrapped { kWrapped };
    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      int sampleCnt,
                      const GrD3DTextureResourceInfo& info,
                      sk_sp<GrD3DResourceState> state,
                      const GrD3DTextureResourceInfo& msaaInfo,
                      sk_sp<GrD3DResourceState> msaaState,
                      Wrapped);

    GrD3DRenderTarget(GrD3DGpu* gpu,
                      SkISize dimensions,
                      const GrD3DTextureResourceInfo& info,
                      sk_sp<GrD3DResourceState> state,
                      Wrapped);

    GrD3DGpu* getD3DGpu() const;

    bool completeStencilAttachment() override { /* TODO */ return false; }

    // In Direct3D we call the release proc after we are finished with the underlying
    // GrD3DTextureResource::Resource object (which occurs after the GPU finishes all work on it).
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {
        // Forward the release proc on to GrD3DTextureResource
        this->setResourceRelease(std::move(releaseHelper));
    }

    void releaseInternalObjects();

    std::unique_ptr<GrD3DTextureResource> fMSAATextureResource;
};

#endif

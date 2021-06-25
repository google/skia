/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DTextureResource_DEFINED
#define GrD3DTextureResource_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrManagedResource.h"
#include "src/gpu/d3d/GrD3DResourceState.h"

class GrD3DGpu;

class GrD3DTextureResource : SkNoncopyable {
private:
    class Resource;

public:
    GrD3DTextureResource(const GrD3DTextureResourceInfo& info, sk_sp<GrD3DResourceState> state)
            : fInfo(info)
            , fState(std::move(state))
            , fResource(new Resource(fInfo.fResource, info.fAlloc)) {
        // gr_cp will implicitly ref the ID3D12Resource for us, so we don't need to worry about
        // whether it's borrowed or not
    }
    virtual ~GrD3DTextureResource();

    ID3D12Resource* d3dResource() const {
        SkASSERT(fResource);
        return fInfo.fResource.get();
    }
    DXGI_FORMAT dxgiFormat() const { return fInfo.fFormat; }
    GrBackendFormat getBackendFormat() const {
        return GrBackendFormat::MakeDxgi(this->dxgiFormat());
    }
    sk_sp<Resource> resource() const {
        SkASSERT(fResource);
        return fResource;
    }
    uint32_t mipLevels() const { return fInfo.fLevelCount; }

    sk_sp<GrD3DResourceState> grD3DResourceState() const { return fState; }

    D3D12_RESOURCE_STATES currentState() const {
        return fState->getResourceState();
    }

    void setResourceState(const GrD3DGpu* gpu, D3D12_RESOURCE_STATES newResourceState,
                          unsigned int subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    // Changes the layout to present
    void prepareForPresent(GrD3DGpu* gpu);

    unsigned int sampleQualityPattern() const { return fInfo.fSampleQualityPattern; }

    // This simply updates our tracking of the resourceState and does not actually do any gpu work.
    // Externally, primarily used for implicit changes in resourceState due to certain GPU commands.
    void updateResourceState(D3D12_RESOURCE_STATES newState) {
        SkASSERT(fResource);
        fState->setResourceState(newState);
    }

    static bool InitTextureResourceInfo(GrD3DGpu* gpu, const D3D12_RESOURCE_DESC& desc,
                                        D3D12_RESOURCE_STATES initialState, GrProtected,
                                        D3D12_CLEAR_VALUE*, GrD3DTextureResourceInfo*);
    static std::pair<GrD3DTextureResourceInfo, sk_sp<GrD3DResourceState>> CreateMSAA(
            GrD3DGpu* gpu, SkISize dimensions, int sampleCnt, const GrD3DTextureResourceInfo& info,
            SkColor4f clearColor);

    void setResourceRelease(sk_sp<GrRefCntedCallback> releaseHelper);

protected:
    void releaseResource(GrD3DGpu* gpu);

    GrD3DTextureResourceInfo fInfo;
    sk_sp<GrD3DResourceState> fState;

private:
    class Resource : public GrTextureResource {
    public:
        explicit Resource()
            : fResource(nullptr)
            , fAlloc(nullptr) {
        }

        Resource(const gr_cp<ID3D12Resource>& textureResource,
                 const sk_sp<GrD3DAlloc>& alloc)
            : fResource(textureResource)
            , fAlloc(alloc) {
        }

        ~Resource() override {}

#ifdef SK_TRACE_MANAGED_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrD3DTextureResource: %p (%d refs)\n", fResource.get(), this->getRefCnt());
        }
#endif

    private:
        void freeGPUData() const override;

        mutable gr_cp<ID3D12Resource> fResource;
        mutable sk_sp<GrD3DAlloc> fAlloc;

        using INHERITED = GrTextureResource;
    };

    sk_sp<Resource> fResource;

    friend class GrD3DRenderTarget;
};

#endif

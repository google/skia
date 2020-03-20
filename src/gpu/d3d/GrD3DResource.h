/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DResource_DEFINED
#define GrD3DResource_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/d3d/GrD3D12.h"
#include "src/gpu/d3d/GrD3DResourceState.h"

class GrD3DGpu;

class GrD3DResource : SkNoncopyable {
public:
    GrD3DResource(ID3D12Resource* resource, const D3D12_RESOURCE_DESC& desc,
                  sk_sp<GrD3DResourceState> state,
                  GrBackendObjectOwnership ownership = GrBackendObjectOwnership::kOwned)
            : fDesc(desc)
            , fState(std::move(state))
            , fStateExplicitlySet(true)
            , fIsBorrowed(GrBackendObjectOwnership::kBorrowed == ownership)
            , fResource(resource) {
        if (fIsBorrowed) {
            fResource->AddRef();
        }
    }
    virtual ~GrD3DResource();

    const ID3D12Resource* resource() const {
        SkASSERT(fResource);
        return fResource;
    }
    DXGI_FORMAT dxgiFormat() const { return fDesc.Format; }
    GrBackendFormat getBackendFormat() const {
        return GrBackendFormat::MakeDxgi(this->dxgiFormat());
    }
    uint32_t mipLevels() const { return fDesc.MipLevels; }
    bool isBorrowed() const { return fIsBorrowed; }

    sk_sp<GrD3DResourceState> grD3DResourceState() const { return fState; }

    D3D12_RESOURCE_STATES currentState() const {
        return fState->getResourceState();
    }

    void setResourceState(const GrD3DGpu* gpu,
                          D3D12_RESOURCE_STATES newResourceState);

    // This simply updates our tracking of the resourceState and does not actually do any gpu work.
    // Externally, primarily used for implicit changes in resourceState due to certain GPU commands.
    void updateResourceState(D3D12_RESOURCE_STATES newState, bool explicitlySet) {
        SkASSERT(fResource);
        fState->setResourceState(newState);
    }

    static ID3D12Resource* CreateResource(GrD3DGpu* gpu, const D3D12_RESOURCE_DESC& desc,
                                          D3D12_RESOURCE_STATES resourceState);

protected:
    void releaseResource();
    bool hasResource() const { return SkToBool(fResource); }

    D3D12_RESOURCE_DESC fDesc;
    sk_sp<GrD3DResourceState> fState;
    bool fStateExplicitlySet;
    bool fIsBorrowed;

private:
    ID3D12Resource* fResource;
};

#endif

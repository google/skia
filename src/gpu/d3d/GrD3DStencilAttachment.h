/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrD3DStencilAttachment_DEFINED
#define GrD3DStencilAttachment_DEFINED

#include "src/gpu/GrStencilAttachment.h"

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/d3d/GrD3DDescriptorHeap.h"
#include "src/gpu/d3d/GrD3DTextureResource.h"

class GrD3DGpu;

class GrD3DStencilAttachment : public GrStencilAttachment, public GrD3DTextureResource {
public:
    struct Format {
        DXGI_FORMAT fInternalFormat;
        int  fStencilBits;
    };

    static GrD3DStencilAttachment* Make(GrD3DGpu* gpu, SkISize dimensions, int sampleCnt,
                                        const Format& format);

    ~GrD3DStencilAttachment() override {}

    GrBackendFormat backendFormat() const override { return GrBackendFormat::MakeDxgi(fFormat); }

    D3D12_CPU_DESCRIPTOR_HANDLE view() const {
        return fView.fHandle;
    }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    size_t onGpuMemorySize() const override;

    GrD3DStencilAttachment(GrD3DGpu* gpu,
                           SkISize dimensions,
                           const Format& format,
                           const D3D12_RESOURCE_DESC&,
                           const GrD3DTextureResourceInfo&,
                           sk_sp<GrD3DResourceState>,
                           const GrD3DDescriptorHeap::CPUHandle& view);

    GrD3DGpu* getD3DGpu() const;

    GrD3DDescriptorHeap::CPUHandle fView;
    DXGI_FORMAT fFormat;
};

#endif

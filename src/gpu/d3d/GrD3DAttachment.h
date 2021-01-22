/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DAttachment_DEFINED
#define GrD3DAttachment_DEFINED

#include "src/gpu/GrAttachment.h"

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/d3d/GrD3DDescriptorHeap.h"
#include "src/gpu/d3d/GrD3DTextureResource.h"

class GrD3DGpu;

class GrD3DAttachment : public GrAttachment, public GrD3DTextureResource {
public:
    static sk_sp<GrD3DAttachment> MakeStencil(GrD3DGpu* gpu,
                                              SkISize dimensions,
                                              int sampleCnt,
                                              DXGI_FORMAT format);

    ~GrD3DAttachment() override {}

    GrBackendFormat backendFormat() const override { return GrBackendFormat::MakeDxgi(fFormat); }

    D3D12_CPU_DESCRIPTOR_HANDLE view() const { return fView.fHandle; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    GrD3DAttachment(GrD3DGpu* gpu,
                    SkISize dimensions,
                    UsageFlags supportedUsages,
                    DXGI_FORMAT format,
                    const D3D12_RESOURCE_DESC&,
                    const GrD3DTextureResourceInfo&,
                    sk_sp<GrD3DResourceState>,
                    const GrD3DDescriptorHeap::CPUHandle& view);

    GrD3DGpu* getD3DGpu() const;

    GrD3DDescriptorHeap::CPUHandle fView;
    DXGI_FORMAT fFormat;
};

#endif

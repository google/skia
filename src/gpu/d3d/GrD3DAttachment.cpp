/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DAttachment.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DAttachment::GrD3DAttachment(GrD3DGpu* gpu,
                                 SkISize dimensions,
                                 UsageFlags supportedUsages,
                                 DXGI_FORMAT format,
                                 const D3D12_RESOURCE_DESC& desc,
                                 const GrD3DTextureResourceInfo& info,
                                 sk_sp<GrD3DResourceState> state,
                                 const GrD3DDescriptorHeap::CPUHandle& view)
        : GrAttachment(gpu, dimensions, supportedUsages, desc.SampleDesc.Count, GrMipmapped::kNo,
                       GrProtected::kNo)
        , GrD3DTextureResource(info, state)
        , fView(view)
        , fFormat(format) {
    this->registerWithCache(SkBudgeted::kYes);
}

sk_sp<GrD3DAttachment> GrD3DAttachment::MakeStencil(GrD3DGpu* gpu,
                                                    SkISize dimensions,
                                                    int sampleCnt,
                                                    DXGI_FORMAT format) {
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;  // default alignment
    resourceDesc.Width = dimensions.width();
    resourceDesc.Height = dimensions.height();
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = format;
    resourceDesc.SampleDesc.Count = sampleCnt;
    resourceDesc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // use driver-selected swizzle
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.DepthStencil.Depth = 0;
    clearValue.DepthStencil.Stencil = 0;

    GrD3DTextureResourceInfo info;
    if (!GrD3DTextureResource::InitTextureResourceInfo(gpu, resourceDesc,
                                                       D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                       GrProtected::kNo, &clearValue, &info)) {
        return nullptr;
    }

    GrD3DDescriptorHeap::CPUHandle view =
            gpu->resourceProvider().createDepthStencilView(info.fResource.get());

    sk_sp<GrD3DResourceState> state(new GrD3DResourceState(info.fResourceState));
    return sk_sp<GrD3DAttachment>(new GrD3DAttachment(gpu, dimensions,
                                                      UsageFlags::kStencilAttachment,
                                                      format, resourceDesc, info,
                                                      std::move(state), view));
}

void GrD3DAttachment::onRelease() {
    GrD3DGpu* gpu = this->getD3DGpu();
    this->releaseResource(gpu);

    GrAttachment::onRelease();
}

void GrD3DAttachment::onAbandon() {
    GrD3DGpu* gpu = this->getD3DGpu();
    this->releaseResource(gpu);

    GrAttachment::onAbandon();
}

GrD3DGpu* GrD3DAttachment::getD3DGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrD3DGpu*>(this->getGpu());
}

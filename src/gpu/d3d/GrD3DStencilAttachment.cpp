/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DStencilAttachment.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DStencilAttachment::GrD3DStencilAttachment(GrD3DGpu* gpu,
                                               const Format& format,
                                               const D3D12_RESOURCE_DESC& desc,
                                               const GrD3DTextureResourceInfo& info,
                                               sk_sp<GrD3DResourceState> state,
                                               const GrD3DDescriptorHeap::CPUHandle& view)
    : GrStencilAttachment(gpu, desc.Width, desc.Height, format.fStencilBits,
                          desc.SampleDesc.Count)
    , GrD3DTextureResource(info, state)
    , fView(view) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrD3DStencilAttachment* GrD3DStencilAttachment::Make(GrD3DGpu* gpu,
                                                     int width,
                                                     int height,
                                                     int sampleCnt,
                                                     const Format& format) {
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;  // default alignment
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = format.fInternalFormat;
    resourceDesc.SampleDesc.Count = sampleCnt;
    // quality levels are only supported for tiled resources so ignore for now
    resourceDesc.SampleDesc.Quality = GrD3DTextureResource::kDefaultQualityLevel;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // use driver-selected swizzle
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format.fInternalFormat;
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
    GrD3DStencilAttachment* stencil = new GrD3DStencilAttachment(gpu, format, resourceDesc,
                                                                 info, std::move(state), view);
    return stencil;
}

size_t GrD3DStencilAttachment::onGpuMemorySize() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= GrD3DCaps::GetStencilFormatTotalBitCount(this->dxgiFormat());
    size *= this->numSamples();
    return static_cast<size_t>(size / 8);
}

void GrD3DStencilAttachment::onRelease() {
    GrD3DGpu* gpu = this->getD3DGpu();
    this->releaseResource(gpu);

    GrStencilAttachment::onRelease();
}

void GrD3DStencilAttachment::onAbandon() {
    GrD3DGpu* gpu = this->getD3DGpu();
    this->releaseResource(gpu);

    GrStencilAttachment::onAbandon();
}

GrD3DGpu* GrD3DStencilAttachment::getD3DGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrD3DGpu*>(this->getGpu());
}

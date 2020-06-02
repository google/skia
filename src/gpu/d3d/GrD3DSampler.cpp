/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DSampler.h"

#include "src/gpu/GrSamplerState.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DResourceProvider.h"

static D3D12_TEXTURE_ADDRESS_MODE wrap_mode_to_d3d_address_mode(GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case GrSamplerState::WrapMode::kRepeat:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case GrSamplerState::WrapMode::kClampToBorder:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    }
    SK_ABORT("Unknown wrap mode.");
}

GrD3DSampler* GrD3DSampler::Create(GrD3DGpu* gpu, const GrSamplerState& samplerState) {
    static D3D12_FILTER d3dFilterModes[] = {
        D3D12_FILTER_MIN_MAG_MIP_POINT,
        D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR
    };

    static_assert((int)GrSamplerState::Filter::kNearest == 0);
    static_assert((int)GrSamplerState::Filter::kBilerp == 1);
    static_assert((int)GrSamplerState::Filter::kMipMap == 2);

    D3D12_FILTER filter = d3dFilterModes[static_cast<int>(samplerState.filter())];
    D3D12_TEXTURE_ADDRESS_MODE addressModeU =
            wrap_mode_to_d3d_address_mode(samplerState.wrapModeX());
    D3D12_TEXTURE_ADDRESS_MODE addressModeV =
            wrap_mode_to_d3d_address_mode(samplerState.wrapModeY());

    D3D12_CPU_DESCRIPTOR_HANDLE samplerDescriptor =
            gpu->resourceProvider().createSampler(filter, addressModeU, addressModeV);

    return new GrD3DSampler(samplerDescriptor, GenerateKey(samplerState));
}

GrD3DSampler::Key GrD3DSampler::GenerateKey(const GrSamplerState& samplerState) {
    return GrSamplerState::GenerateKey(samplerState);
}

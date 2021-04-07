/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DRootSignature.h"

#include "src/gpu/GrSPIRVUniformHandler.h"
#include "src/gpu/d3d/GrD3DGpu.h"

sk_sp<GrD3DRootSignature> GrD3DRootSignature::Make(GrD3DGpu* gpu, int numTextureSamplers,
                                                   int numUnorderedAccessViews) {
    // Just allocate enough space for 4 in case we need it.
    D3D12_ROOT_PARAMETER parameters[4];

    // The first will always be our uniforms
    parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    parameters[0].Descriptor.ShaderRegister = 0;
    parameters[0].Descriptor.RegisterSpace = GrSPIRVUniformHandler::kUniformDescriptorSet;
    parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    int parameterCount = 1;

    SkAutoTArray<D3D12_DESCRIPTOR_RANGE> samplerRanges(numTextureSamplers);
    SkAutoTArray<D3D12_DESCRIPTOR_RANGE> textureRanges(numTextureSamplers);
    if (numTextureSamplers) {
        // Now handle the textures and samplers. We need a range for each sampler because of the
        // interaction between how we set bindings and spirv-cross. Each binding value is used for
        // the register value in the HLSL shader. So setting a binding of i for a texture will give
        // it register t[i] in HLSL. We set the bindings of textures and samplers in pairs with the
        // sampler at i and the corresponding texture at i+1. Thus no textures or samplers will have
        // a contiguous range of HLSL registers so we must define a different range for each.
        for (int i = 0; i < numTextureSamplers; ++i) {
            samplerRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            samplerRanges[i].NumDescriptors = 1;
            samplerRanges[i].BaseShaderRegister = 2 * i;
            // Spirv-Cross uses the descriptor set as the space in HLSL
            samplerRanges[i].RegisterSpace = GrSPIRVUniformHandler::kSamplerTextureDescriptorSet;
            // In the descriptor table the descriptors will all be contiguous.
            samplerRanges[i].OffsetInDescriptorsFromTableStart =
                    D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

            textureRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            textureRanges[i].NumDescriptors = 1;
            textureRanges[i].BaseShaderRegister = 2 * i + 1;
            // Spirv-Cross uses the descriptor set as the space in HLSL
            textureRanges[i].RegisterSpace = GrSPIRVUniformHandler::kSamplerTextureDescriptorSet;
            // In the descriptor table the descriptors will all be contiguous.
            textureRanges[i].OffsetInDescriptorsFromTableStart =
                    D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        }
        parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[1].DescriptorTable.NumDescriptorRanges = numTextureSamplers;
        parameters[1].DescriptorTable.pDescriptorRanges = samplerRanges.get();
        parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[2].DescriptorTable.NumDescriptorRanges = numTextureSamplers;
        parameters[2].DescriptorTable.pDescriptorRanges = textureRanges.get();
        parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        parameterCount = 3;
    }

    D3D12_DESCRIPTOR_RANGE uavRange;
    if (numUnorderedAccessViews) {
        uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        uavRange.NumDescriptors = numUnorderedAccessViews;
        uavRange.BaseShaderRegister = 1;
        // Spirv-Cross uses the descriptor set as the space in HLSL
        // *** piggy-back on uniform register space for now
        uavRange.RegisterSpace = GrSPIRVUniformHandler::kUniformDescriptorSet;
        // In the descriptor table the descriptors will all be contiguous.
        uavRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        parameters[parameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[parameterCount].DescriptorTable.NumDescriptorRanges = 1;
        parameters[parameterCount].DescriptorTable.pDescriptorRanges = &uavRange;
        parameters[parameterCount].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        ++parameterCount;
    }

    D3D12_ROOT_SIGNATURE_DESC rootDesc{};
    rootDesc.NumParameters = parameterCount;
    rootDesc.pParameters = parameters;
    rootDesc.NumStaticSamplers = 0;
    rootDesc.pStaticSamplers = nullptr;
    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    gr_cp<ID3DBlob> rootSigBinary;
    gr_cp<ID3DBlob> error;
    // TODO: D3D Static Function
    HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
                                             &rootSigBinary, &error);

    if (!SUCCEEDED(hr)) {
        SkDebugf("Failed to serialize root signature. Error: %s\n",
                 reinterpret_cast<char*>(error->GetBufferPointer()));
        return nullptr;
    }

    gr_cp<ID3D12RootSignature> rootSig;

    hr = gpu->device()->CreateRootSignature(0, rootSigBinary->GetBufferPointer(),
                                            rootSigBinary->GetBufferSize(), IID_PPV_ARGS(&rootSig));
    if (!SUCCEEDED(hr)) {
        SkDebugf("Failed to create root signature.\n");
        return nullptr;
    }

    return sk_sp<GrD3DRootSignature>(new GrD3DRootSignature(std::move(rootSig),
                                                            numTextureSamplers,
                                                            numUnorderedAccessViews));
}

GrD3DRootSignature::GrD3DRootSignature(gr_cp<ID3D12RootSignature> rootSig, int numTextureSamplers,
                                       int numUnorderedAccessViews)
        : fRootSignature(std::move(rootSig))
        , fNumTextureSamplers(numTextureSamplers)
        , fNumUnorderedAccessViews(numUnorderedAccessViews) {
}

bool GrD3DRootSignature::isCompatible(int numTextureSamplers, int numUnorderedAccessViews) const {
    return (fNumTextureSamplers == numTextureSamplers &&
            fNumUnorderedAccessViews == numUnorderedAccessViews);
}

/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/d3d/GrD3DRootSignature.h"

#include "src/gpu/ganesh/GrSPIRVUniformHandler.h"
#include "src/gpu/ganesh/d3d/GrD3DGpu.h"

using namespace skia_private;

sk_sp<GrD3DRootSignature> GrD3DRootSignature::Make(GrD3DGpu* gpu, int numTextureSamplers,
                                                   int numUAVs) {
    // Just allocate enough space for 3 in case we need it.
    D3D12_ROOT_PARAMETER parameters[3];

    // The first will always be our uniforms
    parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    parameters[0].Descriptor.ShaderRegister = 0;
    parameters[0].Descriptor.RegisterSpace = GrSPIRVUniformHandler::kUniformDescriptorSet;
    parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    int parameterCount = 1;

    int numShaderViews = numTextureSamplers + numUAVs;
    AutoTArray<D3D12_DESCRIPTOR_RANGE> samplerRanges(numTextureSamplers);
    AutoTArray<D3D12_DESCRIPTOR_RANGE> shaderViewRanges(numShaderViews);
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

            shaderViewRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            shaderViewRanges[i].NumDescriptors = 1;
            shaderViewRanges[i].BaseShaderRegister = 2 * i + 1;
            // Spirv-Cross uses the descriptor set as the space in HLSL
            shaderViewRanges[i].RegisterSpace = GrSPIRVUniformHandler::kSamplerTextureDescriptorSet;
            // In the descriptor table the descriptors will all be contiguous.
            shaderViewRanges[i].OffsetInDescriptorsFromTableStart =
                    D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        }
    }
    if (numUAVs) {
        shaderViewRanges[numTextureSamplers].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        shaderViewRanges[numTextureSamplers].NumDescriptors = numUAVs;
        // The assigned register range for the texture SRVs and samplers is from 0 to
        // 2*(numTextureSamplers-1) + 1, so we start with the next register, 2*numTextureSamplers
        shaderViewRanges[numTextureSamplers].BaseShaderRegister = 2 * numTextureSamplers;
        // We share texture descriptor set
        shaderViewRanges[numTextureSamplers].RegisterSpace =
                GrSPIRVUniformHandler::kSamplerTextureDescriptorSet;
        // In the descriptor table the descriptors will all be contiguous.
        shaderViewRanges[numTextureSamplers].OffsetInDescriptorsFromTableStart =
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    }

    if (numShaderViews) {
        unsigned numDescriptorRanges = numUAVs ? numTextureSamplers + 1 : numTextureSamplers;
        parameters[parameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[parameterCount].DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
        parameters[parameterCount].DescriptorTable.pDescriptorRanges = shaderViewRanges.get();
        parameters[parameterCount].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        parameterCount++;
    }

    if (numTextureSamplers) {
        parameters[parameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[parameterCount].DescriptorTable.NumDescriptorRanges = numTextureSamplers;
        parameters[parameterCount].DescriptorTable.pDescriptorRanges = samplerRanges.get();
        parameters[parameterCount].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        parameterCount++;
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
                                                            numUAVs));
}

GrD3DRootSignature::GrD3DRootSignature(gr_cp<ID3D12RootSignature> rootSig, int numTextureSamplers,
                                       int numUAVs)
        : fRootSignature(std::move(rootSig))
        , fNumTextureSamplers(numTextureSamplers)
        , fNumUAVs(numUAVs) {
}

bool GrD3DRootSignature::isCompatible(int numTextureSamplers, int numUAVs) const {
    return fNumTextureSamplers == numTextureSamplers && fNumUAVs == numUAVs;
}

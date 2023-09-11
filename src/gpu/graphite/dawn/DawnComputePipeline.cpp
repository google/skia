/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnComputePipeline.h"

#include "src/gpu/PipelineUtils.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/dawn/DawnAsyncWait.h"
#include "src/gpu/graphite/dawn/DawnErrorChecker.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"

namespace skgpu::graphite {
namespace {

struct ShaderInfo {
    wgpu::ShaderModule fModule;
    std::string fEntryPoint;

    bool isValid() const { return static_cast<bool>(fModule); }
};

static ShaderInfo compile_shader_module(const DawnSharedContext* sharedContext,
                                        const ComputePipelineDesc& pipelineDesc) {
    SkASSERT(sharedContext);

    ShaderInfo info;

    const Caps* caps = sharedContext->caps();
    const ComputeStep* step = pipelineDesc.computeStep();
    ShaderErrorHandler* errorHandler = caps->shaderErrorHandler();

    if (step->supportsNativeShader()) {
        auto nativeShader = step->nativeShaderSource(ComputeStep::NativeShaderFormat::kWGSL);
        if (!DawnCompileWGSLShaderModule(sharedContext,
                                         std::string(nativeShader.fSource),
                                         &info.fModule,
                                         errorHandler)) {
            return {};
        }
        info.fEntryPoint = std::move(nativeShader.fEntryPoint);
    } else {
        // TODO(skia:40044196) Compile to WGSL when SkSL supports it
        std::string spirv;
        SkSL::Program::Interface interface;
        SkSL::ProgramSettings settings;

        SkSL::Compiler compiler(caps->shaderCaps());
        std::string sksl = BuildComputeSkSL(caps, step);
        if (SkSLToSPIRV(&compiler,
                        sksl,
                        SkSL::ProgramKind::kCompute,
                        settings,
                        &spirv,
                        &interface,
                        errorHandler)) {
            if (!DawnCompileSPIRVShaderModule(sharedContext, spirv, &info.fModule, errorHandler)) {
                return {};
            }
            info.fEntryPoint = "main";
        }
    }

    return info;
}

}  // namespace

sk_sp<DawnComputePipeline> DawnComputePipeline::Make(const DawnSharedContext* sharedContext,
                                                     const ComputePipelineDesc& pipelineDesc) {
    auto [shaderModule, entryPointName] = compile_shader_module(sharedContext, pipelineDesc);
    if (!shaderModule) {
        return nullptr;
    }

    const ComputeStep* step = pipelineDesc.computeStep();

    // ComputeStep resources are listed in the order that they must be declared in the shader. This
    // order is then used for the index assignment using an "indexed by order" policy that has
    // backend-specific semantics. The semantics on Dawn is to assign the index number in increasing
    // order.
    //
    // All resources get assigned to a single bind group at index 0.
    SkASSERT(!sharedContext->caps()->resourceBindingRequirements().fDistinctIndexRanges);
    std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries;
    auto resources = step->resources();

    // Sampled textures count as 2 resources (1 texture and 1 sampler). All other types count as 1.
    size_t resourceCount = 0;
    for (const ComputeStep::ResourceDesc& r : resources) {
        resourceCount++;
        if (r.fType == ComputeStep::ResourceType::kSampledTexture) {
            resourceCount++;
        }
    }

    bindGroupLayoutEntries.reserve(resourceCount);
    int declarationIndex = 0;
    for (const ComputeStep::ResourceDesc& r : resources) {
        bindGroupLayoutEntries.emplace_back();
        uint32_t bindingIndex = bindGroupLayoutEntries.size() - 1;

        wgpu::BindGroupLayoutEntry& entry = bindGroupLayoutEntries.back();
        entry.binding = bindingIndex;
        entry.visibility = wgpu::ShaderStage::Compute;
        switch (r.fType) {
            case ComputeStep::ResourceType::kUniformBuffer:
                entry.buffer.type = wgpu::BufferBindingType::Uniform;
                break;
            case ComputeStep::ResourceType::kStorageBuffer:
                entry.buffer.type = wgpu::BufferBindingType::Storage;
                break;
            case ComputeStep::ResourceType::kReadOnlyTexture:
                entry.texture.sampleType = wgpu::TextureSampleType::Float;
                entry.texture.viewDimension = wgpu::TextureViewDimension::e2D;
                break;
            case ComputeStep::ResourceType::kWriteOnlyStorageTexture: {
                entry.storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                entry.storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;

                auto [_, colorType] = step->calculateTextureParameters(declarationIndex, r);
                auto textureInfo = sharedContext->caps()->getDefaultStorageTextureInfo(colorType);
                entry.storageTexture.format = textureInfo.dawnTextureSpec().fFormat;
                break;
            }
            case ComputeStep::ResourceType::kSampledTexture: {
                entry.sampler.type = wgpu::SamplerBindingType::Filtering;

                // Add an additional entry for the texture.
                bindGroupLayoutEntries.emplace_back();
                wgpu::BindGroupLayoutEntry& texEntry = bindGroupLayoutEntries.back();
                texEntry.binding = bindingIndex + 1;
                texEntry.visibility = wgpu::ShaderStage::Compute;
                texEntry.texture.sampleType = wgpu::TextureSampleType::Float;
                texEntry.texture.viewDimension = wgpu::TextureViewDimension::e2D;
                break;
            }
        }
        declarationIndex++;
    }

    const wgpu::Device& device = sharedContext->device();

    // All resources of a ComputeStep currently get assigned to a single bind group at index 0.
    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entryCount = bindGroupLayoutEntries.size();
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&bindGroupLayoutDesc);
    if (!bindGroupLayout) {
        return nullptr;
    }

    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.label = step->name();
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
    wgpu::PipelineLayout layout = device.CreatePipelineLayout(&pipelineLayoutDesc);
    if (!layout) {
        return nullptr;
    }

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.label = step->name();
    descriptor.compute.module = std::move(shaderModule);
    descriptor.compute.entryPoint = entryPointName.c_str();
    descriptor.layout = std::move(layout);

    DawnErrorChecker errorChecker(device);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);
    SkASSERT(pipeline);
    if (errorChecker.popErrorScopes() != DawnErrorType::kNoError) {
        return nullptr;
    }

    return sk_sp<DawnComputePipeline>(new DawnComputePipeline(sharedContext, std::move(pipeline)));
}

DawnComputePipeline::DawnComputePipeline(const SharedContext* sharedContext,
                                         wgpu::ComputePipeline pso)
        : ComputePipeline(sharedContext), fPipeline(std::move(pso)) {}

void DawnComputePipeline::freeGpuData() { fPipeline = nullptr; }

}  // namespace skgpu::graphite

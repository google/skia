/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnSharedContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {
namespace {

wgpu::ShaderModule CreateNoopFragment(const wgpu::Device& device) {
#if defined(__EMSCRIPTEN__)
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
#else
    wgpu::ShaderSourceWGSL wgslDesc;
#endif
    wgslDesc.code =
            "@fragment\n"
            "fn main() {}\n";
    wgpu::ShaderModuleDescriptor smDesc;
    smDesc.nextInChain = &wgslDesc;
    smDesc.label = "no-op";
    auto fsModule = device.CreateShaderModule(&smDesc);
    return fsModule;
}

}

sk_sp<SharedContext> DawnSharedContext::Make(const DawnBackendContext& backendContext,
                                             const ContextOptions& options) {
    if (!backendContext.fDevice || !backendContext.fQueue) {
        return {};
    }

    auto noopFragment = CreateNoopFragment(backendContext.fDevice);
    if (!noopFragment) {
        return {};
    }

    auto caps = std::make_unique<const DawnCaps>(backendContext, options);

    return sk_sp<SharedContext>(new DawnSharedContext(backendContext,
                                                      std::move(caps),
                                                      std::move(noopFragment),
                                                      options.fExecutor,
                                                      options.fUserDefinedKnownRuntimeEffects));
}

DawnSharedContext::DawnSharedContext(const DawnBackendContext& backendContext,
                                     std::unique_ptr<const DawnCaps> caps,
                                     wgpu::ShaderModule noopFragment,
                                     SkExecutor* executor,
                                     SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects)
        : SharedContext(std::move(caps),
                        BackendApi::kDawn,
                        executor,
                        userDefinedKnownRuntimeEffects)
        , fInstance(backendContext.fInstance)
        , fDevice(backendContext.fDevice)
        , fQueue(backendContext.fQueue)
        , fTick(backendContext.fTick)
        , fNoopFragment(std::move(noopFragment)) {
    fThreadSafeResourceProvider = std::make_unique<DawnThreadSafeResourceProvider>(
        this->makeResourceProvider(&fSingleOwner,
                                   SK_InvalidGenID,
                                   kThreadedSafeResourceBudget));

    this->createUniformBuffersBindGroupLayout();
    this->createSingleTextureSamplerBindGroupLayout();
}

DawnSharedContext::~DawnSharedContext() {
    fThreadSafeResourceProvider.reset();

    // need to clear out resources before any allocator is removed
    this->globalCache()->deleteResources();
}

DawnThreadSafeResourceProvider* DawnSharedContext::threadSafeResourceProvider() const {
    return static_cast<DawnThreadSafeResourceProvider*>(fThreadSafeResourceProvider.get());
}

std::unique_ptr<ResourceProvider> DawnSharedContext::makeResourceProvider(
        SingleOwner* singleOwner,
        uint32_t recorderID,
        size_t resourceBudget) {
    return std::unique_ptr<ResourceProvider>(new DawnResourceProvider(this,
                                                                      singleOwner,
                                                                      recorderID,
                                                                      resourceBudget));
}

void DawnSharedContext::deviceTick(Context* context) {
#if !defined(__EMSCRIPTEN__)
    this->device().Tick();
#endif
    context->checkAsyncWorkCompletion();
}

void DawnSharedContext::createUniformBuffersBindGroupLayout() {
    const Caps* caps = this->caps();

    std::array<wgpu::BindGroupLayoutEntry, 4> entries;
    entries[0].binding = DawnGraphicsPipeline::kIntrinsicUniformBufferIndex;
    entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[0].buffer.hasDynamicOffset = true;
    entries[0].buffer.minBindingSize = 0;

    entries[1].binding = DawnGraphicsPipeline::kRenderStepUniformBufferIndex;
    entries[1].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[1].buffer.type = caps->storageBufferSupport()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[1].buffer.hasDynamicOffset = true;
    entries[1].buffer.minBindingSize = 0;

    entries[2].binding = DawnGraphicsPipeline::kPaintUniformBufferIndex;
    entries[2].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[2].buffer.type = caps->storageBufferSupport()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[2].buffer.hasDynamicOffset = true;
    entries[2].buffer.minBindingSize = 0;

    // Gradient buffer will only be used when storage buffers are preferred, else large
    // gradients use a texture fallback, set binding type as a uniform when not in use to
    // satisfy any binding type restrictions for non-supported ssbo devices.
    entries[3].binding = DawnGraphicsPipeline::kGradientBufferIndex;
    entries[3].visibility = wgpu::ShaderStage::Fragment;
    entries[3].buffer.type = caps->storageBufferSupport()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[3].buffer.hasDynamicOffset = true;
    entries[3].buffer.minBindingSize = 0;

    wgpu::BindGroupLayoutDescriptor groupLayoutDesc;
    if (caps->setBackendLabels()) {
        groupLayoutDesc.label = "Uniform buffers bind group layout";
    }

    groupLayoutDesc.entryCount = entries.size();
    groupLayoutDesc.entries = entries.data();
    fUniformBuffersBindGroupLayout = this->device().CreateBindGroupLayout(&groupLayoutDesc);
}

void DawnSharedContext::createSingleTextureSamplerBindGroupLayout() {
    const Caps* caps = this->caps();

    std::array<wgpu::BindGroupLayoutEntry, 2> entries;

    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Fragment;
    entries[0].sampler.type = wgpu::SamplerBindingType::Filtering;

    entries[1].binding = 1;
    entries[1].visibility = wgpu::ShaderStage::Fragment;
    entries[1].texture.sampleType = wgpu::TextureSampleType::Float;
    entries[1].texture.viewDimension = wgpu::TextureViewDimension::e2D;
    entries[1].texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor groupLayoutDesc;
    if (caps->setBackendLabels()) {
        groupLayoutDesc.label = "Single texture + sampler bind group layout";
    }

    groupLayoutDesc.entryCount = entries.size();
    groupLayoutDesc.entries = entries.data();
    fSingleTextureSamplerBindGroupLayout = this->device().CreateBindGroupLayout(&groupLayoutDesc);
}

sk_sp<GraphicsPipeline> DawnSharedContext::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t compilationID) {
    return DawnGraphicsPipeline::Make(this,
                                      runtimeDict,
                                      pipelineKey,
                                      pipelineDesc,
                                      renderPassDesc,
                                      pipelineCreationFlags,
                                      compilationID);
}

} // namespace skgpu::graphite

/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkAlign.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/dawn/DawnBuffer.h"
#include "src/gpu/graphite/dawn/DawnComputePipeline.h"
#include "src/gpu/graphite/dawn/DawnErrorChecker.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnSampler.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"
#include "src/sksl/SkSLCompiler.h"

namespace skgpu::graphite {

namespace {

constexpr int kBufferBindingSizeAlignment = 16;
constexpr int kMaxNumberOfCachedBufferBindGroups = 32;
constexpr int kMaxNumberOfCachedTextureBindGroups = 4096;

wgpu::ShaderModule create_shader_module(const wgpu::Device& device, const char* source) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.code = source;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device.CreateShaderModule(&descriptor);
}

wgpu::RenderPipeline create_blit_render_pipeline(const DawnSharedContext* sharedContext,
                                                 const char* label,
                                                 wgpu::ShaderModule vsModule,
                                                 wgpu::ShaderModule fsModule,
                                                 wgpu::TextureFormat renderPassColorFormat,
                                                 wgpu::TextureFormat renderPassDepthStencilFormat,
                                                 int numSamples) {
    wgpu::RenderPipelineDescriptor descriptor;
    descriptor.label = label;
    descriptor.layout = nullptr;

    wgpu::ColorTargetState colorTarget;
    colorTarget.format = renderPassColorFormat;
    colorTarget.blend = nullptr;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::DepthStencilState depthStencil;
    if (renderPassDepthStencilFormat != wgpu::TextureFormat::Undefined) {
        depthStencil.format = renderPassDepthStencilFormat;
        depthStencil.depthWriteEnabled = false;
        depthStencil.depthCompare = wgpu::CompareFunction::Always;

        descriptor.depthStencil = &depthStencil;
    }

    wgpu::FragmentState fragment;
    fragment.module = std::move(fsModule);
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    descriptor.fragment = &fragment;

    descriptor.vertex.module = std::move(vsModule);
    descriptor.vertex.entryPoint = "main";
    descriptor.vertex.constantCount = 0;
    descriptor.vertex.constants = nullptr;
    descriptor.vertex.bufferCount = 0;
    descriptor.vertex.buffers = nullptr;

    descriptor.primitive.frontFace = wgpu::FrontFace::CCW;
    descriptor.primitive.cullMode = wgpu::CullMode::None;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
    descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

    descriptor.multisample.count = numSamples;
    descriptor.multisample.mask = 0xFFFFFFFF;
    descriptor.multisample.alphaToCoverageEnabled = false;

    std::optional<DawnErrorChecker> errorChecker;
    if (sharedContext->dawnCaps()->allowScopedErrorChecks()) {
        errorChecker.emplace(sharedContext);
    }
    auto pipeline = sharedContext->device().CreateRenderPipeline(&descriptor);
    if (errorChecker.has_value() && errorChecker->popErrorScopes() != DawnErrorType::kNoError) {
        return nullptr;
    }

    return pipeline;
}

UniqueKey make_ubo_bind_group_key(
        const std::array<std::pair<const DawnBuffer*, uint32_t>, 3>& boundBuffersAndSizes) {
    static const UniqueKey::Domain kBufferBindGroupDomain = UniqueKey::GenerateDomain();

    UniqueKey uniqueKey;
    {
        // Each entry in the bind group needs 2 uint32_t in the key:
        //  - buffer's unique ID: 32 bits.
        //  - buffer's binding size: 32 bits.
        // We need total of 3 entries in the uniform buffer bind group.
        // Unused entries will be assigned zero values.
        UniqueKey::Builder builder(
                &uniqueKey, kBufferBindGroupDomain, 6, "GraphicsPipelineBufferBindGroup");

        for (uint32_t i = 0; i < boundBuffersAndSizes.size(); ++i) {
            const DawnBuffer* boundBuffer = boundBuffersAndSizes[i].first;
            const uint32_t bindingSize = boundBuffersAndSizes[i].second;
            if (boundBuffer) {
                builder[2 * i] = boundBuffer->uniqueID().asUInt();
                builder[2 * i + 1] = bindingSize;
            } else {
                builder[2 * i] = 0;
                builder[2 * i + 1] = 0;
            }
        }

        builder.finish();
    }

    return uniqueKey;
}

UniqueKey make_texture_bind_group_key(const DawnSampler* sampler, const DawnTexture* texture) {
    static const UniqueKey::Domain kTextureBindGroupDomain = UniqueKey::GenerateDomain();

    UniqueKey uniqueKey;
    {
        UniqueKey::Builder builder(&uniqueKey,
                                   kTextureBindGroupDomain,
                                   2,
                                   "GraphicsPipelineSingleTextureSamplerBindGroup");

        builder[0] = sampler->uniqueID().asUInt();
        builder[1] = texture->uniqueID().asUInt();

        builder.finish();
    }

    return uniqueKey;
}
}  // namespace

DawnResourceProvider::DawnResourceProvider(SharedContext* sharedContext,
                                           SingleOwner* singleOwner,
                                           uint32_t recorderID,
                                           size_t resourceBudget)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget)
        , fUniformBufferBindGroupCache(kMaxNumberOfCachedBufferBindGroups)
        , fSingleTextureSamplerBindGroups(kMaxNumberOfCachedTextureBindGroups) {}

DawnResourceProvider::~DawnResourceProvider() = default;

wgpu::RenderPipeline DawnResourceProvider::findOrCreateBlitWithDrawPipeline(
        const RenderPassDesc& renderPassDesc) {
    uint64_t renderPassKey =
            this->dawnSharedContext()->dawnCaps()->getRenderPassDescKey(renderPassDesc);
    wgpu::RenderPipeline pipeline = fBlitWithDrawPipelines[renderPassKey];
    if (!pipeline) {
        static constexpr char kVertexShaderText[] = R"(
            var<private> fullscreenTriPositions : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2(-1.0, -1.0), vec2(-1.0, 3.0), vec2(3.0, -1.0));

            @vertex
            fn main(@builtin(vertex_index) vertexIndex : u32) -> @builtin(position) vec4<f32> {
                return vec4(fullscreenTriPositions[vertexIndex], 1.0, 1.0);
            }
        )";

        static constexpr char kFragmentShaderText[] = R"(
            @group(0) @binding(0) var colorMap: texture_2d<f32>;

            @fragment
            fn main(@builtin(position) fragPosition : vec4<f32>) -> @location(0) vec4<f32> {
                var coords : vec2<i32> = vec2<i32>(i32(fragPosition.x), i32(fragPosition.y));
                return textureLoad(colorMap, coords, 0);
            }
        )";

        auto vsModule = create_shader_module(dawnSharedContext()->device(), kVertexShaderText);
        auto fsModule = create_shader_module(dawnSharedContext()->device(), kFragmentShaderText);

        pipeline = create_blit_render_pipeline(
                dawnSharedContext(),
                /*label=*/"BlitWithDraw",
                std::move(vsModule),
                std::move(fsModule),
                /*renderPassColorFormat=*/
                renderPassDesc.fColorAttachment.fTextureInfo.dawnTextureSpec().getViewFormat(),
                /*renderPassDepthStencilFormat=*/
                renderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid()
                        ? renderPassDesc.fDepthStencilAttachment.fTextureInfo.dawnTextureSpec()
                                  .getViewFormat()
                        : wgpu::TextureFormat::Undefined,
                /*numSamples=*/renderPassDesc.fColorAttachment.fTextureInfo.numSamples());

        if (pipeline) {
            fBlitWithDrawPipelines.set(renderPassKey, pipeline);
        }
    }

    return pipeline;
}

sk_sp<Texture> DawnResourceProvider::onCreateWrappedTexture(const BackendTexture& texture) {
    // Convert to smart pointers. wgpu::Texture* constructor will increment the ref count.
    wgpu::Texture dawnTexture         = texture.getDawnTexturePtr();
    wgpu::TextureView dawnTextureView = texture.getDawnTextureViewPtr();
    SkASSERT(!dawnTexture || !dawnTextureView);

    if (!dawnTexture && !dawnTextureView) {
        return {};
    }

    if (dawnTexture) {
        return DawnTexture::MakeWrapped(this->dawnSharedContext(),
                                        texture.dimensions(),
                                        texture.info(),
                                        std::move(dawnTexture));
    } else {
        return DawnTexture::MakeWrapped(this->dawnSharedContext(),
                                        texture.dimensions(),
                                        texture.info(),
                                        std::move(dawnTextureView));
    }
}

sk_sp<DawnTexture> DawnResourceProvider::findOrCreateDiscardableMSAALoadTexture(
        SkISize dimensions, const TextureInfo& msaaInfo) {
    SkASSERT(msaaInfo.isValid());

    // Derive the load texture's info from MSAA texture's info.
    DawnTextureInfo dawnMsaaLoadTextureInfo;
    msaaInfo.getDawnTextureInfo(&dawnMsaaLoadTextureInfo);
    dawnMsaaLoadTextureInfo.fSampleCount = 1;
    dawnMsaaLoadTextureInfo.fUsage |= wgpu::TextureUsage::TextureBinding;

#if !defined(__EMSCRIPTEN__)
    // MSAA texture can be transient attachment (memoryless) but the load texture cannot be.
    // This is because the load texture will need to have its content retained between two passes
    // loading:
    // - first pass: the resolve texture is blitted to the load texture.
    // - 2nd pass: the actual render pass is started and the load texture is blitted to the MSAA
    // texture.
    dawnMsaaLoadTextureInfo.fUsage &= (~wgpu::TextureUsage::TransientAttachment);
#endif

    auto texture = this->findOrCreateDiscardableMSAAAttachment(dimensions, dawnMsaaLoadTextureInfo);

    return sk_sp<DawnTexture>(static_cast<DawnTexture*>(texture.release()));
}

sk_sp<GraphicsPipeline> DawnResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    return DawnGraphicsPipeline::Make(this->dawnSharedContext(),
                                      this,
                                      runtimeDict,
                                      pipelineDesc,
                                      renderPassDesc);
}

sk_sp<ComputePipeline> DawnResourceProvider::createComputePipeline(
        const ComputePipelineDesc& desc) {
    return DawnComputePipeline::Make(this->dawnSharedContext(), desc);
}

sk_sp<Texture> DawnResourceProvider::createTexture(SkISize dimensions,
                                                   const TextureInfo& info,
                                                   skgpu::Budgeted budgeted) {
    return DawnTexture::Make(this->dawnSharedContext(),
                             dimensions,
                             info,
                             budgeted);
}

sk_sp<Buffer> DawnResourceProvider::createBuffer(size_t size,
                                                 BufferType type,
                                                 AccessPattern accessPattern) {
    return DawnBuffer::Make(this->dawnSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> DawnResourceProvider::createSampler(const SamplerDesc& samplerDesc) {
    return DawnSampler::Make(this->dawnSharedContext(),
                             samplerDesc.samplingOptions(),
                             samplerDesc.tileModeX(),
                             samplerDesc.tileModeY());
}

BackendTexture DawnResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                            const TextureInfo& info) {
    wgpu::Texture texture = DawnTexture::MakeDawnTexture(this->dawnSharedContext(),
                                                         dimensions,
                                                         info);
    if (!texture) {
        return {};
    }

    return BackendTexture(texture.MoveToCHandle());
}

void DawnResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
    SkASSERT(texture.isValid());
    SkASSERT(texture.backend() == BackendApi::kDawn);

    // Automatically release the pointers in wgpu::TextureView & wgpu::Texture's dtor.
    // Acquire() won't increment the ref count.
    wgpu::TextureView::Acquire(texture.getDawnTextureViewPtr());
    // We need to explicitly call Destroy() here since since that is the recommended way to delete
    // a Dawn texture predictably versus just dropping a ref and relying on garbage collection.
    //
    // Additionally this helps to work around an issue where Skia may have cached a BindGroup that
    // references the underlying texture. Skia currently doesn't destroy BindGroups when its use of
    // the texture goes away, thus a ref to the texture remains on the BindGroup and memory is never
    // cleared up unless we call Destroy() here.
    wgpu::Texture::Acquire(texture.getDawnTexturePtr()).Destroy();
}

DawnSharedContext* DawnResourceProvider::dawnSharedContext() const {
    return static_cast<DawnSharedContext*>(fSharedContext);
}

sk_sp<DawnBuffer> DawnResourceProvider::findOrCreateDawnBuffer(size_t size,
                                                               BufferType type,
                                                               AccessPattern accessPattern,
                                                               std::string_view label) {
    sk_sp<Buffer> buffer = this->findOrCreateBuffer(size, type, accessPattern, std::move(label));
    DawnBuffer* ptr = static_cast<DawnBuffer*>(buffer.release());
    return sk_sp<DawnBuffer>(ptr);
}

const wgpu::BindGroupLayout& DawnResourceProvider::getOrCreateUniformBuffersBindGroupLayout() {
    if (fUniformBuffersBindGroupLayout) {
        return fUniformBuffersBindGroupLayout;
    }

    std::array<wgpu::BindGroupLayoutEntry, 3> entries;
    entries[0].binding = DawnGraphicsPipeline::kIntrinsicUniformBufferIndex;
    entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[0].buffer.hasDynamicOffset = true;
    entries[0].buffer.minBindingSize = 0;

    entries[1].binding = DawnGraphicsPipeline::kRenderStepUniformBufferIndex;
    entries[1].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[1].buffer.type = fSharedContext->caps()->storageBufferPreferred()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[1].buffer.hasDynamicOffset = true;
    entries[1].buffer.minBindingSize = 0;

    entries[2].binding = DawnGraphicsPipeline::kPaintUniformBufferIndex;
    entries[2].visibility = wgpu::ShaderStage::Fragment;
    entries[2].buffer.type = fSharedContext->caps()->storageBufferPreferred()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[2].buffer.hasDynamicOffset = true;
    entries[2].buffer.minBindingSize = 0;

    wgpu::BindGroupLayoutDescriptor groupLayoutDesc;
    if (fSharedContext->caps()->setBackendLabels()) {
        groupLayoutDesc.label = "Uniform buffers bind group layout";
    }

    groupLayoutDesc.entryCount = entries.size();
    groupLayoutDesc.entries = entries.data();
    fUniformBuffersBindGroupLayout =
            this->dawnSharedContext()->device().CreateBindGroupLayout(&groupLayoutDesc);

    return fUniformBuffersBindGroupLayout;
}

const wgpu::BindGroupLayout&
DawnResourceProvider::getOrCreateSingleTextureSamplerBindGroupLayout() {
    if (fSingleTextureSamplerBindGroupLayout) {
        return fSingleTextureSamplerBindGroupLayout;
    }

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
    if (fSharedContext->caps()->setBackendLabels()) {
        groupLayoutDesc.label = "Single texture + sampler bind group layout";
    }

    groupLayoutDesc.entryCount = entries.size();
    groupLayoutDesc.entries = entries.data();
    fSingleTextureSamplerBindGroupLayout =
            this->dawnSharedContext()->device().CreateBindGroupLayout(&groupLayoutDesc);

    return fSingleTextureSamplerBindGroupLayout;
}

const wgpu::Buffer& DawnResourceProvider::getOrCreateNullBuffer() {
    if (!fNullBuffer) {
        wgpu::BufferDescriptor desc;
        if (fSharedContext->caps()->setBackendLabels()) {
            desc.label = "UnusedBufferSlot";
        }
        desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform |
                     wgpu::BufferUsage::Storage;
        desc.size = kBufferBindingSizeAlignment;
        desc.mappedAtCreation = false;

        fNullBuffer = this->dawnSharedContext()->device().CreateBuffer(&desc);
        SkASSERT(fNullBuffer);
    }

    return fNullBuffer;
}

const wgpu::BindGroup& DawnResourceProvider::findOrCreateUniformBuffersBindGroup(
        const std::array<std::pair<const DawnBuffer*, uint32_t>, 3>& boundBuffersAndSizes) {
    auto key = make_ubo_bind_group_key(boundBuffersAndSizes);
    auto* existingBindGroup = fUniformBufferBindGroupCache.find(key);
    if (existingBindGroup) {
        // cache hit.
        return *existingBindGroup;
    }

    // Translate to wgpu::BindGroupDescriptor
    std::array<wgpu::BindGroupEntry, 3> entries;

    constexpr uint32_t kBindingIndices[] = {
        DawnGraphicsPipeline::kIntrinsicUniformBufferIndex,
        DawnGraphicsPipeline::kRenderStepUniformBufferIndex,
        DawnGraphicsPipeline::kPaintUniformBufferIndex,
    };

    for (uint32_t i = 0; i < boundBuffersAndSizes.size(); ++i) {
        const DawnBuffer* boundBuffer = boundBuffersAndSizes[i].first;
        const uint32_t bindingSize = boundBuffersAndSizes[i].second;

        entries[i].binding = kBindingIndices[i];
        entries[i].offset = 0;
        if (boundBuffer) {
            entries[i].buffer = boundBuffer->dawnBuffer();
            entries[i].size = SkAlignTo(bindingSize, kBufferBindingSizeAlignment);
        } else {
            entries[i].buffer = this->getOrCreateNullBuffer();
            entries[i].size = wgpu::kWholeSize;
        }
    }

    wgpu::BindGroupDescriptor desc;
    desc.layout = this->getOrCreateUniformBuffersBindGroupLayout();
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    const auto& device = this->dawnSharedContext()->device();
    auto bindGroup = device.CreateBindGroup(&desc);

    return *fUniformBufferBindGroupCache.insert(key, bindGroup);
}

const wgpu::BindGroup& DawnResourceProvider::findOrCreateSingleTextureSamplerBindGroup(
        const DawnSampler* sampler, const DawnTexture* texture) {
    auto key = make_texture_bind_group_key(sampler, texture);
    auto* existingBindGroup = fSingleTextureSamplerBindGroups.find(key);
    if (existingBindGroup) {
        // cache hit.
        return *existingBindGroup;
    }

    std::array<wgpu::BindGroupEntry, 2> entries;

    entries[0].binding = 0;
    entries[0].sampler = sampler->dawnSampler();
    entries[1].binding = 1;
    entries[1].textureView = texture->sampleTextureView();

    wgpu::BindGroupDescriptor desc;
    desc.layout = getOrCreateSingleTextureSamplerBindGroupLayout();
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    const auto& device = this->dawnSharedContext()->device();
    auto bindGroup = device.CreateBindGroup(&desc);

    return *fSingleTextureSamplerBindGroups.insert(key, bindGroup);
}

} // namespace skgpu::graphite

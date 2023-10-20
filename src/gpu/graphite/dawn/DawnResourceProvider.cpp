/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "src/gpu/graphite/ComputePipeline.h"
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
wgpu::ShaderModule create_shader_module(const wgpu::Device& device, const char* source) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.code = source;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device.CreateShaderModule(&descriptor);
}

wgpu::RenderPipeline create_blit_render_pipeline(const wgpu::Device& device,
                                                 const char* label,
                                                 wgpu::ShaderModule vsModule,
                                                 wgpu::ShaderModule fsModule,
                                                 wgpu::TextureFormat renderPassColorFormat,
                                                 wgpu::TextureFormat renderPassDepthStencilFormat,
                                                 int numSamples) {
    wgpu::RenderPipelineDescriptor descriptor;
#if defined(SK_DEBUG)
    descriptor.label = label;
#endif
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

    DawnErrorChecker errorChecker(device);
    auto pipeline = device.CreateRenderPipeline(&descriptor);
    if (errorChecker.popErrorScopes() != DawnErrorType::kNoError) {
        return nullptr;
    }

    return pipeline;
}
}  // namespace

DawnResourceProvider::DawnResourceProvider(SharedContext* sharedContext,
                                           SingleOwner* singleOwner,
                                           uint32_t recorderID,
                                           size_t resourceBudget)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget) {}

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
                dawnSharedContext()->device(),
                /*label=*/"BlitWithDraw",
                std::move(vsModule),
                std::move(fsModule),
                /*renderPassColorFormat=*/
                renderPassDesc.fColorAttachment.fTextureInfo.dawnTextureSpec().fFormat,
                /*renderPassDepthStencilFormat=*/
                renderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid()
                        ? renderPassDesc.fDepthStencilAttachment.fTextureInfo.dawnTextureSpec()
                                  .fFormat
                        : wgpu::TextureFormat::Undefined,
                /*numSamples=*/renderPassDesc.fColorAttachment.fTextureInfo.numSamples());

        if (pipeline) {
            fBlitWithDrawPipelines.set(renderPassKey, pipeline);
        }
    }

    return pipeline;
}

sk_sp<Texture> DawnResourceProvider::createWrappedTexture(const BackendTexture& texture) {
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

    // MSAA texture can be transient attachment (memoryless) but the load texture cannot be.
    // This is because the load texture will need to have its content retained between two passes
    // loading:
    // - first pass: the resolve texture is blitted to the load texture.
    // - 2nd pass: the actual render pass is started and the load texture is blitted to the MSAA
    // texture.
    dawnMsaaLoadTextureInfo.fUsage &= (~wgpu::TextureUsage::TransientAttachment);

    auto texture = this->findOrCreateDiscardableMSAAAttachment(dimensions, dawnMsaaLoadTextureInfo);

    return sk_sp<DawnTexture>(static_cast<DawnTexture*>(texture.release()));
}

sk_sp<GraphicsPipeline> DawnResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    SkSL::Compiler skslCompiler(fSharedContext->caps()->shaderCaps());
    return DawnGraphicsPipeline::Make(this->dawnSharedContext(),
                                      &skslCompiler,
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
    return DawnTexture::Make(this->dawnSharedContext(), dimensions, info, budgeted);
}

sk_sp<Buffer> DawnResourceProvider::createBuffer(size_t size,
                                                 BufferType type,
                                                 AccessPattern accessPattern) {
    return DawnBuffer::Make(this->dawnSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> DawnResourceProvider::createSampler(const SkSamplingOptions& options,
                                                   SkTileMode xTileMode,
                                                   SkTileMode yTileMode) {
    return DawnSampler::Make(this->dawnSharedContext(), options, xTileMode, yTileMode);
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
    wgpu::Texture::Acquire(texture.getDawnTexturePtr());
}

const DawnSharedContext* DawnResourceProvider::dawnSharedContext() const {
    return static_cast<const DawnSharedContext*>(fSharedContext);
}

} // namespace skgpu::graphite

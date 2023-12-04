/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnCommandBuffer.h"

#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/dawn/DawnBuffer.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnComputePipeline.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"
#include "src/gpu/graphite/dawn/DawnQueueManager.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSampler.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"

namespace skgpu::graphite {

namespace {

using IntrinsicConstant = float[4];

constexpr int kBufferBindingOffsetAlignment = 256;

constexpr int kIntrinsicConstantAlignedSize =
        SkAlignTo(sizeof(IntrinsicConstant), kBufferBindingOffsetAlignment);

#if defined(__EMSCRIPTEN__)
// When running against WebGPU in WASM we don't have the wgpu::CommandBuffer::WriteBuffer method. We
// allocate a fixed size buffer to hold the intrinsics constants. If we overflow we allocate another
// buffer.
constexpr int kNumSlotsForIntrinsicConstantBuffer = 8;
#else
// Dawn has an in-band WriteBuffer command, so we can just keep overwriting the same slot between
// render passes. Zero indicates this behavior.
constexpr int kNumSlotsForIntrinsicConstantBuffer = 0;
#endif

}  // namespace

std::unique_ptr<DawnCommandBuffer> DawnCommandBuffer::Make(const DawnSharedContext* sharedContext,
                                                           DawnResourceProvider* resourceProvider) {
    std::unique_ptr<DawnCommandBuffer> cmdBuffer(
            new DawnCommandBuffer(sharedContext, resourceProvider));
    if (!cmdBuffer->setNewCommandBufferResources()) {
        return {};
    }
    return cmdBuffer;
}

DawnCommandBuffer::DawnCommandBuffer(const DawnSharedContext* sharedContext,
                                     DawnResourceProvider* resourceProvider)
        : fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider) {}

DawnCommandBuffer::~DawnCommandBuffer() {}

wgpu::CommandBuffer DawnCommandBuffer::finishEncoding() {
    SkASSERT(fCommandEncoder);
    wgpu::CommandBuffer cmdBuffer = fCommandEncoder.Finish();

    fCommandEncoder = nullptr;

    return cmdBuffer;
}

void DawnCommandBuffer::onResetCommandBuffer() {
    fIntrinsicConstantBuffer = nullptr;

    fActiveGraphicsPipeline = nullptr;
    fActiveRenderPassEncoder = nullptr;
    fActiveComputePassEncoder = nullptr;
    fCommandEncoder = nullptr;

    for (auto& bufferSlot : fBoundUniformBuffers) {
        bufferSlot = nullptr;
    }
    fBoundUniformBuffersDirty = true;
}

bool DawnCommandBuffer::setNewCommandBufferResources() {
    SkASSERT(!fCommandEncoder);
    fCommandEncoder = fSharedContext->device().CreateCommandEncoder();
    SkASSERT(fCommandEncoder);
    return true;
}

bool DawnCommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                        const Texture* colorTexture,
                                        const Texture* resolveTexture,
                                        const Texture* depthStencilTexture,
                                        SkRect viewport,
                                        const DrawPassList& drawPasses) {
    // Update viewport's constant buffer before starting a render pass.
    this->preprocessViewport(viewport);

    if (!this->beginRenderPass(renderPassDesc, colorTexture, resolveTexture, depthStencilTexture)) {
        return false;
    }

    this->setViewport(viewport);

    for (const auto& drawPass : drawPasses) {
        this->addDrawPass(drawPass.get());
    }

    this->endRenderPass();
    return true;
}

bool DawnCommandBuffer::onAddComputePass(const DispatchGroupList& groups) {
    this->beginComputePass();
    for (const auto& group : groups) {
        group->addResourceRefs(this);
        for (const auto& dispatch : group->dispatches()) {
            this->bindComputePipeline(group->getPipeline(dispatch.fPipelineIndex));
            this->bindDispatchResources(*group, dispatch);
            this->dispatchWorkgroups(dispatch.fParams.fGlobalDispatchSize);
        }
    }
    this->endComputePass();
    return true;
}

bool DawnCommandBuffer::beginRenderPass(const RenderPassDesc& renderPassDesc,
                                        const Texture* colorTexture,
                                        const Texture* resolveTexture,
                                        const Texture* depthStencilTexture) {
    SkASSERT(!fActiveRenderPassEncoder);
    SkASSERT(!fActiveComputePassEncoder);

    constexpr static wgpu::LoadOp wgpuLoadActionMap[]{
            wgpu::LoadOp::Load,
            wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear  // Don't care
    };
    static_assert((int)LoadOp::kLoad == 0);
    static_assert((int)LoadOp::kClear == 1);
    static_assert((int)LoadOp::kDiscard == 2);
    static_assert(std::size(wgpuLoadActionMap) == kLoadOpCount);

    constexpr static wgpu::StoreOp wgpuStoreActionMap[]{wgpu::StoreOp::Store,
                                                        wgpu::StoreOp::Discard};
    static_assert((int)StoreOp::kStore == 0);
    static_assert((int)StoreOp::kDiscard == 1);
    static_assert(std::size(wgpuStoreActionMap) == kStoreOpCount);

    wgpu::RenderPassDescriptor wgpuRenderPass = {};
    wgpu::RenderPassColorAttachment wgpuColorAttachment;
    wgpu::RenderPassDepthStencilAttachment wgpuDepthStencilAttachment;

    // Set up color attachment.
#ifndef __EMSCRIPTEN__
    wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled mssaRenderToSingleSampledDesc;
#endif

    auto& colorInfo = renderPassDesc.fColorAttachment;
    bool loadMSAAFromResolveExplicitly = false;
    if (colorTexture) {
        wgpuRenderPass.colorAttachments = &wgpuColorAttachment;
        wgpuRenderPass.colorAttachmentCount = 1;

        // TODO: check Texture matches RenderPassDesc
        const auto* dawnColorTexture = static_cast<const DawnTexture*>(colorTexture);
        SkASSERT(dawnColorTexture->renderTextureView());
        wgpuColorAttachment.view = dawnColorTexture->renderTextureView();

        const std::array<float, 4>& clearColor = renderPassDesc.fClearColor;
        wgpuColorAttachment.clearValue = {
                clearColor[0], clearColor[1], clearColor[2], clearColor[3]};
        wgpuColorAttachment.loadOp = wgpuLoadActionMap[static_cast<int>(colorInfo.fLoadOp)];
        wgpuColorAttachment.storeOp = wgpuStoreActionMap[static_cast<int>(colorInfo.fStoreOp)];

        // Set up resolve attachment
        if (resolveTexture) {
            SkASSERT(renderPassDesc.fColorResolveAttachment.fStoreOp == StoreOp::kStore);
            // TODO: check Texture matches RenderPassDesc
            const auto* dawnResolveTexture = static_cast<const DawnTexture*>(resolveTexture);
            SkASSERT(dawnResolveTexture->renderTextureView());
            wgpuColorAttachment.resolveTarget = dawnResolveTexture->renderTextureView();

            // Inclusion of a resolve texture implies the client wants to finish the
            // renderpass with a resolve.
            SkASSERT(wgpuColorAttachment.storeOp == wgpu::StoreOp::Discard);

            // But it also means we have to load the resolve texture into the MSAA color attachment
            loadMSAAFromResolveExplicitly =
                    renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
            // TODO: If the color resolve texture is read-only we can use a private (vs. memoryless)
            // msaa attachment that's coupled to the framebuffer and the StoreAndMultisampleResolve
            // action instead of loading as a draw.
        } else {
            [[maybe_unused]] bool isMSAAToSingleSampled = renderPassDesc.fSampleCount > 1 &&
                                                          colorTexture->numSamples() == 1;
#if defined(__EMSCRIPTEN__)
            SkASSERT(!isMSAAToSingleSampled);
#else
            if (isMSAAToSingleSampled) {
                // If render pass is multi sampled but the color attachment is single sampled, we
                // need to activate multisampled render to single sampled feature for this render
                // pass.
                SkASSERT(fSharedContext->device().HasFeature(
                        wgpu::FeatureName::MSAARenderToSingleSampled));

                wgpuColorAttachment.nextInChain = &mssaRenderToSingleSampledDesc;
                mssaRenderToSingleSampledDesc.implicitSampleCount = renderPassDesc.fSampleCount;
            }
#endif
        }
    }

    // Set up stencil/depth attachment
    auto& depthStencilInfo = renderPassDesc.fDepthStencilAttachment;
    if (depthStencilTexture) {
        const auto* dawnDepthStencilTexture = static_cast<const DawnTexture*>(depthStencilTexture);
        auto format = dawnDepthStencilTexture->textureInfo().dawnTextureSpec().fFormat;
        SkASSERT(DawnFormatIsDepthOrStencil(format));

        // TODO: check Texture matches RenderPassDesc
        SkASSERT(dawnDepthStencilTexture->renderTextureView());
        wgpuDepthStencilAttachment.view = dawnDepthStencilTexture->renderTextureView();

        if (DawnFormatIsDepth(format)) {
            wgpuDepthStencilAttachment.depthClearValue = renderPassDesc.fClearDepth;
            wgpuDepthStencilAttachment.depthLoadOp =
                    wgpuLoadActionMap[static_cast<int>(depthStencilInfo.fLoadOp)];
            wgpuDepthStencilAttachment.depthStoreOp =
                    wgpuStoreActionMap[static_cast<int>(depthStencilInfo.fStoreOp)];
        }

        if (DawnFormatIsStencil(format)) {
            wgpuDepthStencilAttachment.stencilClearValue = renderPassDesc.fClearStencil;
            wgpuDepthStencilAttachment.stencilLoadOp =
                    wgpuLoadActionMap[static_cast<int>(depthStencilInfo.fLoadOp)];
            wgpuDepthStencilAttachment.stencilStoreOp =
                    wgpuStoreActionMap[static_cast<int>(depthStencilInfo.fStoreOp)];
        }

        wgpuRenderPass.depthStencilAttachment = &wgpuDepthStencilAttachment;
    } else {
        SkASSERT(!depthStencilInfo.fTextureInfo.isValid());
    }

    if (loadMSAAFromResolveExplicitly) {
        // Manually load the contents of the resolve texture into the MSAA attachment as a draw,
        // so the actual load op for the MSAA attachment had better have been discard.

        if (!this->loadMSAAFromResolveAndBeginRenderPassEncoder(
                    renderPassDesc,
                    wgpuRenderPass,
                    static_cast<const DawnTexture*>(colorTexture))) {
            return false;
        }
    }
    else {
        fActiveRenderPassEncoder = fCommandEncoder.BeginRenderPass(&wgpuRenderPass);
    }

    return true;
}

bool DawnCommandBuffer::loadMSAAFromResolveAndBeginRenderPassEncoder(
        const RenderPassDesc& frontendRenderPassDesc,
        const wgpu::RenderPassDescriptor& wgpuRenderPassDesc,
        const DawnTexture* msaaTexture) {
    SkASSERT(!fActiveRenderPassEncoder);

    // Copy from resolve texture to an intermediate texture. Using blit with draw
    // pipeline because the resolveTexture might be created from a swapchain, and it
    // is possible that only its texture view is available. So onCopyTextureToTexture()
    // which operates on wgpu::Texture instead of wgpu::TextureView cannot be used in that case.
    auto msaaLoadTexture = fResourceProvider->findOrCreateDiscardableMSAALoadTexture(
            msaaTexture->dimensions(), msaaTexture->textureInfo());
    if (!msaaLoadTexture) {
        SKGPU_LOG_E("DawnCommandBuffer::loadMSAAFromResolveAndBeginRenderPassEncoder: "
                    "Can't create MSAA Load Texture.");
        return false;
    }

    this->trackCommandBufferResource(msaaLoadTexture);

    // Creating intermediate render pass (copy from resolve texture -> MSAA load texture)
    RenderPassDesc intermediateRenderPassDesc = {};
    intermediateRenderPassDesc.fColorAttachment.fLoadOp = LoadOp::kDiscard;
    intermediateRenderPassDesc.fColorAttachment.fStoreOp = StoreOp::kStore;
    intermediateRenderPassDesc.fColorAttachment.fTextureInfo =
            frontendRenderPassDesc.fColorResolveAttachment.fTextureInfo;

    wgpu::RenderPassColorAttachment wgpuIntermediateColorAttachment;
     // Dawn doesn't support actual DontCare so use LoadOp::Clear.
    wgpuIntermediateColorAttachment.loadOp = wgpu::LoadOp::Clear;
    wgpuIntermediateColorAttachment.clearValue = {1, 1, 1, 1};
    wgpuIntermediateColorAttachment.storeOp = wgpu::StoreOp::Store;
    wgpuIntermediateColorAttachment.view = msaaLoadTexture->renderTextureView();

    wgpu::RenderPassDescriptor wgpuIntermediateRenderPassDesc;
    wgpuIntermediateRenderPassDesc.colorAttachmentCount = 1;
    wgpuIntermediateRenderPassDesc.colorAttachments = &wgpuIntermediateColorAttachment;

    auto renderPassEncoder = fCommandEncoder.BeginRenderPass(&wgpuIntermediateRenderPassDesc);

    bool blitSucceeded = this->doBlitWithDraw(
            renderPassEncoder,
            intermediateRenderPassDesc,
            /*sourceTextureView=*/wgpuRenderPassDesc.colorAttachments[0].resolveTarget,
            msaaTexture->dimensions().width(),
            msaaTexture->dimensions().height());

    renderPassEncoder.End();

    if (!blitSucceeded) {
        return false;
    }

    // Start actual render pass (blit from MSAA load texture -> MSAA texture)
    renderPassEncoder = fCommandEncoder.BeginRenderPass(&wgpuRenderPassDesc);

    if (!this->doBlitWithDraw(renderPassEncoder,
                              frontendRenderPassDesc,
                              /*sourceTextureView=*/msaaLoadTexture->renderTextureView(),
                              msaaTexture->dimensions().width(),
                              msaaTexture->dimensions().height())) {
        renderPassEncoder.End();
        return false;
    }

    fActiveRenderPassEncoder = renderPassEncoder;

    return true;
}

bool DawnCommandBuffer::doBlitWithDraw(const wgpu::RenderPassEncoder& renderEncoder,
                                       const RenderPassDesc& frontendRenderPassDesc,
                                       const wgpu::TextureView& sourceTextureView,
                                       int width,
                                       int height) {
    auto loadPipeline = fResourceProvider->findOrCreateBlitWithDrawPipeline(frontendRenderPassDesc);
    if (!loadPipeline) {
        SKGPU_LOG_E("Unable to create pipeline to blit with draw");
        return false;
    }

    SkASSERT(renderEncoder);

    renderEncoder.SetPipeline(loadPipeline);

    // The load msaa pipeline takes no uniforms, no vertex/instance attributes and only uses
    // one texture that does not require a sampler.

    // TODO: b/260368758
    // cache single texture's bind group creation.
    wgpu::BindGroupEntry entry;
    entry.binding = 0;
    entry.textureView = sourceTextureView;

    wgpu::BindGroupDescriptor desc;
    desc.layout = loadPipeline.GetBindGroupLayout(0);
    desc.entryCount = 1;
    desc.entries = &entry;

    auto bindGroup = fSharedContext->device().CreateBindGroup(&desc);

    renderEncoder.SetBindGroup(0, bindGroup);

    renderEncoder.SetScissorRect(0, 0, width, height);
    renderEncoder.SetViewport(0, 0, width, height, 0, 1);

    // Fullscreen triangle
    renderEncoder.Draw(3);

    return true;
}

void DawnCommandBuffer::endRenderPass() {
    SkASSERT(fActiveRenderPassEncoder);
    fActiveRenderPassEncoder.End();
    fActiveRenderPassEncoder = nullptr;
}

void DawnCommandBuffer::addDrawPass(const DrawPass* drawPass) {
    drawPass->addResourceRefs(this);
    for (auto [type, cmdPtr] : drawPass->commands()) {
        switch (type) {
            case DrawPassCommands::Type::kBindGraphicsPipeline: {
                auto bgp = static_cast<DrawPassCommands::BindGraphicsPipeline*>(cmdPtr);
                this->bindGraphicsPipeline(drawPass->getPipeline(bgp->fPipelineIndex));
                break;
            }
            case DrawPassCommands::Type::kSetBlendConstants: {
                auto sbc = static_cast<DrawPassCommands::SetBlendConstants*>(cmdPtr);
                this->setBlendConstants(sbc->fBlendConstants);
                break;
            }
            case DrawPassCommands::Type::kBindUniformBuffer: {
                auto bub = static_cast<DrawPassCommands::BindUniformBuffer*>(cmdPtr);
                this->bindUniformBuffer(bub->fInfo, bub->fSlot);
                break;
            }
            case DrawPassCommands::Type::kBindDrawBuffers: {
                auto bdb = static_cast<DrawPassCommands::BindDrawBuffers*>(cmdPtr);
                this->bindDrawBuffers(
                        bdb->fVertices, bdb->fInstances, bdb->fIndices, bdb->fIndirect);
                break;
            }
            case DrawPassCommands::Type::kBindTexturesAndSamplers: {
                auto bts = static_cast<DrawPassCommands::BindTexturesAndSamplers*>(cmdPtr);
                bindTextureAndSamplers(*drawPass, *bts);
                break;
            }
            case DrawPassCommands::Type::kSetScissor: {
                auto ss = static_cast<DrawPassCommands::SetScissor*>(cmdPtr);
                const SkIRect& rect = ss->fScissor;
                this->setScissor(rect.fLeft, rect.fTop, rect.width(), rect.height());
                break;
            }
            case DrawPassCommands::Type::kDraw: {
                auto draw = static_cast<DrawPassCommands::Draw*>(cmdPtr);
                this->draw(draw->fType, draw->fBaseVertex, draw->fVertexCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexed: {
                auto draw = static_cast<DrawPassCommands::DrawIndexed*>(cmdPtr);
                this->drawIndexed(
                        draw->fType, draw->fBaseIndex, draw->fIndexCount, draw->fBaseVertex);
                break;
            }
            case DrawPassCommands::Type::kDrawInstanced: {
                auto draw = static_cast<DrawPassCommands::DrawInstanced*>(cmdPtr);
                this->drawInstanced(draw->fType,
                                    draw->fBaseVertex,
                                    draw->fVertexCount,
                                    draw->fBaseInstance,
                                    draw->fInstanceCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedInstanced: {
                auto draw = static_cast<DrawPassCommands::DrawIndexedInstanced*>(cmdPtr);
                this->drawIndexedInstanced(draw->fType,
                                           draw->fBaseIndex,
                                           draw->fIndexCount,
                                           draw->fBaseVertex,
                                           draw->fBaseInstance,
                                           draw->fInstanceCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndirect: {
                auto draw = static_cast<DrawPassCommands::DrawIndirect*>(cmdPtr);
                this->drawIndirect(draw->fType);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedIndirect: {
                auto draw = static_cast<DrawPassCommands::DrawIndexedIndirect*>(cmdPtr);
                this->drawIndexedIndirect(draw->fType);
                break;
            }
        }
    }
}

void DawnCommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
    SkASSERT(fActiveRenderPassEncoder);

    fActiveGraphicsPipeline = static_cast<const DawnGraphicsPipeline*>(graphicsPipeline);
    fActiveRenderPassEncoder.SetPipeline(fActiveGraphicsPipeline->dawnRenderPipeline());
    fBoundUniformBuffersDirty = true;
}

void DawnCommandBuffer::bindUniformBuffer(const BindUniformBufferInfo& info, UniformSlot slot) {
    SkASSERT(fActiveRenderPassEncoder);

    auto dawnBuffer = static_cast<const DawnBuffer*>(info.fBuffer);

    unsigned int bufferIndex = 0;
    switch (slot) {
        case UniformSlot::kRenderStep:
            bufferIndex = DawnGraphicsPipeline::kRenderStepUniformBufferIndex;
            break;
        case UniformSlot::kPaint:
            bufferIndex = DawnGraphicsPipeline::kPaintUniformBufferIndex;
            break;
        default:
            SkASSERT(false);
    }

    fBoundUniformBuffers[bufferIndex] = dawnBuffer;
    fBoundUniformBufferOffsets[bufferIndex] = static_cast<uint32_t>(info.fOffset);
    fBoundUniformBufferSizes[bufferIndex] = info.fBindingSize;

    fBoundUniformBuffersDirty = true;
}

void DawnCommandBuffer::bindDrawBuffers(const BindBufferInfo& vertices,
                                        const BindBufferInfo& instances,
                                        const BindBufferInfo& indices,
                                        const BindBufferInfo& indirect) {
    SkASSERT(fActiveRenderPassEncoder);

    if (vertices.fBuffer) {
        auto dawnBuffer = static_cast<const DawnBuffer*>(vertices.fBuffer)->dawnBuffer();
        fActiveRenderPassEncoder.SetVertexBuffer(
                DawnGraphicsPipeline::kVertexBufferIndex, dawnBuffer, vertices.fOffset);
    }
    if (instances.fBuffer) {
        auto dawnBuffer = static_cast<const DawnBuffer*>(instances.fBuffer)->dawnBuffer();
        fActiveRenderPassEncoder.SetVertexBuffer(
                DawnGraphicsPipeline::kInstanceBufferIndex, dawnBuffer, instances.fOffset);
    }
    if (indices.fBuffer) {
        auto dawnBuffer = static_cast<const DawnBuffer*>(indices.fBuffer)->dawnBuffer();
        fActiveRenderPassEncoder.SetIndexBuffer(
                dawnBuffer, wgpu::IndexFormat::Uint16, indices.fOffset);
    }
    if (indirect.fBuffer) {
        fCurrentIndirectBuffer = static_cast<const DawnBuffer*>(indirect.fBuffer)->dawnBuffer();
        fCurrentIndirectBufferOffset = indirect.fOffset;
    } else {
        fCurrentIndirectBuffer = nullptr;
        fCurrentIndirectBufferOffset = 0;
    }
}

void DawnCommandBuffer::bindTextureAndSamplers(
        const DrawPass& drawPass, const DrawPassCommands::BindTexturesAndSamplers& command) {
    SkASSERT(fActiveRenderPassEncoder);
    SkASSERT(fActiveGraphicsPipeline);

    wgpu::BindGroup bindGroup;
    if (command.fNumTexSamplers == 1) {
        // Optimize for single texture.
        SkASSERT(fActiveGraphicsPipeline->numTexturesAndSamplers() == 2);

        const auto* texture =
                static_cast<const DawnTexture*>(drawPass.getTexture(command.fTextureIndices[0]));
        const auto* sampler =
                static_cast<const DawnSampler*>(drawPass.getSampler(command.fSamplerIndices[0]));

        bindGroup = fResourceProvider->findOrCreateSingleTextureSamplerBindGroup(sampler, texture);
    } else {
        std::vector<wgpu::BindGroupEntry> entries(2 * command.fNumTexSamplers);

        for (int i = 0; i < command.fNumTexSamplers; ++i) {
            const auto* texture = static_cast<const DawnTexture*>(
                    drawPass.getTexture(command.fTextureIndices[i]));
            const auto* sampler = static_cast<const DawnSampler*>(
                    drawPass.getSampler(command.fSamplerIndices[i]));
            auto& wgpuTextureView = texture->sampleTextureView();
            auto& wgpuSampler = sampler->dawnSampler();

            // Assuming shader generator assigns binding slot to sampler then texture,
            // then the next sampler and texture, and so on, we need to use
            // 2 * i as base binding index of the sampler and texture.
            // TODO: https://b.corp.google.com/issues/259457090:
            // Better configurable way of assigning samplers and textures' bindings.
            entries[2 * i].binding = 2 * i;
            entries[2 * i].sampler = wgpuSampler;

            entries[2 * i + 1].binding = 2 * i + 1;
            entries[2 * i + 1].textureView = wgpuTextureView;
        }

        wgpu::BindGroupDescriptor desc;
        const auto& groupLayouts = fActiveGraphicsPipeline->dawnGroupLayouts();
        desc.layout = groupLayouts[DawnGraphicsPipeline::kTextureBindGroupIndex];
        desc.entryCount = entries.size();
        desc.entries = entries.data();

        bindGroup = fSharedContext->device().CreateBindGroup(&desc);
    }

    fActiveRenderPassEncoder.SetBindGroup(DawnGraphicsPipeline::kTextureBindGroupIndex, bindGroup);
}

void DawnCommandBuffer::syncUniformBuffers() {
    if (fBoundUniformBuffersDirty) {
        fBoundUniformBuffersDirty = false;

        std::array<uint32_t, 3> dynamicOffsets;
        std::array<std::pair<const DawnBuffer*, uint32_t>, 3> boundBuffersAndSizes;
        boundBuffersAndSizes[0].first = fIntrinsicConstantBuffer.get();
        boundBuffersAndSizes[0].second = sizeof(IntrinsicConstant);

        int activeIntrinsicBufferSlot = fIntrinsicConstantBufferSlotsUsed - 1;
        dynamicOffsets[0] = activeIntrinsicBufferSlot * kIntrinsicConstantAlignedSize;

        if (fActiveGraphicsPipeline->hasStepUniforms() &&
            fBoundUniformBuffers[DawnGraphicsPipeline::kRenderStepUniformBufferIndex]) {
            boundBuffersAndSizes[1].first =
                    fBoundUniformBuffers[DawnGraphicsPipeline::kRenderStepUniformBufferIndex];
            boundBuffersAndSizes[1].second =
                    fBoundUniformBufferSizes[DawnGraphicsPipeline::kRenderStepUniformBufferIndex];
            dynamicOffsets[1] =
                    fBoundUniformBufferOffsets[DawnGraphicsPipeline::kRenderStepUniformBufferIndex];
        } else {
            // Unused buffer entry
            boundBuffersAndSizes[1].first = nullptr;
            dynamicOffsets[1] = 0;
        }

        if (fActiveGraphicsPipeline->hasPaintUniforms() &&
            fBoundUniformBuffers[DawnGraphicsPipeline::kPaintUniformBufferIndex]) {
            boundBuffersAndSizes[2].first =
                    fBoundUniformBuffers[DawnGraphicsPipeline::kPaintUniformBufferIndex];
            boundBuffersAndSizes[2].second =
                    fBoundUniformBufferSizes[DawnGraphicsPipeline::kPaintUniformBufferIndex];
            dynamicOffsets[2] =
                    fBoundUniformBufferOffsets[DawnGraphicsPipeline::kPaintUniformBufferIndex];
        } else {
            // Unused buffer entry
            boundBuffersAndSizes[2].first = nullptr;
            dynamicOffsets[2] = 0;
        }

        auto bindGroup =
                fResourceProvider->findOrCreateUniformBuffersBindGroup(boundBuffersAndSizes);

        fActiveRenderPassEncoder.SetBindGroup(DawnGraphicsPipeline::kUniformBufferBindGroupIndex,
                                              bindGroup,
                                              dynamicOffsets.size(),
                                              dynamicOffsets.data());
    }
}

void DawnCommandBuffer::setScissor(unsigned int left,
                                   unsigned int top,
                                   unsigned int width,
                                   unsigned int height) {
    SkASSERT(fActiveRenderPassEncoder);
    SkIRect scissor = SkIRect::MakeXYWH(
            left + fReplayTranslation.x(), top + fReplayTranslation.y(), width, height);
    if (!scissor.intersect(SkIRect::MakeSize(fRenderPassSize))) {
        scissor.setEmpty();
    }
    fActiveRenderPassEncoder.SetScissorRect(
            scissor.x(), scissor.y(), scissor.width(), scissor.height());
}

void DawnCommandBuffer::preprocessViewport(const SkRect& viewport) {
    // Dawn's framebuffer space has (0, 0) at the top left. This agrees with Skia's device coords.
    // However, in NDC (-1, -1) is the bottom left. So we flip the origin here (assuming all
    // surfaces we have are TopLeft origin).
    const float x = viewport.x() - fReplayTranslation.x();
    const float y = viewport.y() - fReplayTranslation.y();
    const float invTwoW = 2.f / viewport.width();
    const float invTwoH = 2.f / viewport.height();
    const IntrinsicConstant rtAdjust = {invTwoW, -invTwoH, -1.f - x * invTwoW, 1.f + y * invTwoH};

    bool needNewBuffer = !fIntrinsicConstantBuffer;
    if (!needNewBuffer && kNumSlotsForIntrinsicConstantBuffer > 0) {
        needNewBuffer = (fIntrinsicConstantBufferSlotsUsed == kNumSlotsForIntrinsicConstantBuffer);
    }

    if (needNewBuffer) {
        size_t bufferSize;

        if constexpr (kNumSlotsForIntrinsicConstantBuffer > 1) {
            bufferSize = kIntrinsicConstantAlignedSize * kNumSlotsForIntrinsicConstantBuffer;
        } else {
            bufferSize = kIntrinsicConstantAlignedSize;
        }

        fIntrinsicConstantBuffer = fResourceProvider->findOrCreateDawnBuffer(
                bufferSize, BufferType::kUniform, AccessPattern::kGpuOnly);

        fIntrinsicConstantBufferSlotsUsed = 0;
        SkASSERT(fIntrinsicConstantBuffer);

        this->trackResource(fIntrinsicConstantBuffer);
    }

    // TODO: https://b.corp.google.com/issues/259267703
    // Make updating intrinsic constants faster. Metal has setVertexBytes method
    // to quickly sending intrinsic constants to vertex shader without any buffer. But Dawn doesn't
    // have similar capability. So we have to use WriteBuffer(), and this method is not allowed to
    // be called when there is an active render pass.
    SkASSERT(!fActiveRenderPassEncoder);
    SkASSERT(!fActiveComputePassEncoder);

    if constexpr (kNumSlotsForIntrinsicConstantBuffer > 0) {
        uint64_t offset = fIntrinsicConstantBufferSlotsUsed * kIntrinsicConstantAlignedSize;
        fSharedContext->queue().WriteBuffer(fIntrinsicConstantBuffer->dawnBuffer(),
                                            offset,
                                            &rtAdjust,
                                            sizeof(rtAdjust));
        fIntrinsicConstantBufferSlotsUsed++;
    } else {
#if !defined(__EMSCRIPTEN__)
        fCommandEncoder.WriteBuffer(fIntrinsicConstantBuffer->dawnBuffer(),
                                    0,
                                    reinterpret_cast<const uint8_t*>(rtAdjust),
                                    sizeof(rtAdjust));
#endif
        fIntrinsicConstantBufferSlotsUsed = 1;
    }
}

void DawnCommandBuffer::setViewport(const SkRect& viewport) {
    SkASSERT(fActiveRenderPassEncoder);
    fActiveRenderPassEncoder.SetViewport(
            viewport.x(), viewport.y(), viewport.width(), viewport.height(), 0, 1);
}

void DawnCommandBuffer::setBlendConstants(float* blendConstants) {
    SkASSERT(fActiveRenderPassEncoder);
    wgpu::Color blendConst = {
            blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]};
    fActiveRenderPassEncoder.SetBlendConstant(&blendConst);
}

void DawnCommandBuffer::draw(PrimitiveType type,
                             unsigned int baseVertex,
                             unsigned int vertexCount) {
    SkASSERT(fActiveRenderPassEncoder);
    SkASSERT(fActiveGraphicsPipeline->primitiveType() == type);

    this->syncUniformBuffers();

    fActiveRenderPassEncoder.Draw(vertexCount, /*instanceCount=*/1, baseVertex);
}

void DawnCommandBuffer::drawIndexed(PrimitiveType type,
                                    unsigned int baseIndex,
                                    unsigned int indexCount,
                                    unsigned int baseVertex) {
    SkASSERT(fActiveRenderPassEncoder);
    SkASSERT(fActiveGraphicsPipeline->primitiveType() == type);

    this->syncUniformBuffers();

    fActiveRenderPassEncoder.DrawIndexed(indexCount, /*instanceCount=*/1, baseIndex, baseVertex);
}

void DawnCommandBuffer::drawInstanced(PrimitiveType type,
                                      unsigned int baseVertex,
                                      unsigned int vertexCount,
                                      unsigned int baseInstance,
                                      unsigned int instanceCount) {
    SkASSERT(fActiveRenderPassEncoder);
    SkASSERT(fActiveGraphicsPipeline->primitiveType() == type);

    this->syncUniformBuffers();

    fActiveRenderPassEncoder.Draw(vertexCount, instanceCount, baseVertex, baseInstance);
}

void DawnCommandBuffer::drawIndexedInstanced(PrimitiveType type,
                                             unsigned int baseIndex,
                                             unsigned int indexCount,
                                             unsigned int baseVertex,
                                             unsigned int baseInstance,
                                             unsigned int instanceCount) {
    SkASSERT(fActiveRenderPassEncoder);
    SkASSERT(fActiveGraphicsPipeline->primitiveType() == type);

    this->syncUniformBuffers();

    fActiveRenderPassEncoder.DrawIndexed(
            indexCount, instanceCount, baseIndex, baseVertex, baseInstance);
}

void DawnCommandBuffer::drawIndirect(PrimitiveType type) {
    SkASSERT(fActiveRenderPassEncoder);
    SkASSERT(fActiveGraphicsPipeline->primitiveType() == type);
    SkASSERT(fCurrentIndirectBuffer);

    this->syncUniformBuffers();

    fActiveRenderPassEncoder.DrawIndirect(fCurrentIndirectBuffer, fCurrentIndirectBufferOffset);
}

void DawnCommandBuffer::drawIndexedIndirect(PrimitiveType type) {
    SkASSERT(fActiveRenderPassEncoder);
    SkASSERT(fActiveGraphicsPipeline->primitiveType() == type);
    SkASSERT(fCurrentIndirectBuffer);

    this->syncUniformBuffers();

    fActiveRenderPassEncoder.DrawIndexedIndirect(fCurrentIndirectBuffer,
                                                 fCurrentIndirectBufferOffset);
}

void DawnCommandBuffer::beginComputePass() {
    SkASSERT(!fActiveRenderPassEncoder);
    SkASSERT(!fActiveComputePassEncoder);
    fActiveComputePassEncoder = fCommandEncoder.BeginComputePass();
}

void DawnCommandBuffer::bindComputePipeline(const ComputePipeline* computePipeline) {
    SkASSERT(fActiveComputePassEncoder);

    fActiveComputePipeline = static_cast<const DawnComputePipeline*>(computePipeline);
    fActiveComputePassEncoder.SetPipeline(fActiveComputePipeline->dawnComputePipeline());
}

void DawnCommandBuffer::bindDispatchResources(const DispatchGroup& group,
                                              const DispatchGroup::Dispatch& dispatch) {
    SkASSERT(fActiveComputePassEncoder);
    SkASSERT(fActiveComputePipeline);

    // Bind all pipeline resources to a single new bind group at index 0.
    // NOTE: Caching the bind groups here might be beneficial based on the layout and the bound
    // resources (though it's questionable how often a bind group will end up getting reused since
    // the bound objects change often).
    skia_private::TArray<wgpu::BindGroupEntry> entries;
    entries.reserve(dispatch.fBindings.size());

    for (const ResourceBinding& binding : dispatch.fBindings) {
        wgpu::BindGroupEntry& entry = entries.push_back();
        entry.binding = binding.fIndex;
        if (const BufferView* buffer = std::get_if<BufferView>(&binding.fResource)) {
            entry.buffer = static_cast<const DawnBuffer*>(buffer->fInfo.fBuffer)->dawnBuffer();
            entry.offset = buffer->fInfo.fOffset;
            entry.size = buffer->fSize;
        } else if (const TextureIndex* texIdx = std::get_if<TextureIndex>(&binding.fResource)) {
            const DawnTexture* texture =
                    static_cast<const DawnTexture*>(group.getTexture(texIdx->fValue));
            SkASSERT(texture);
            entry.textureView = texture->sampleTextureView();
        } else if (const SamplerIndex* samplerIdx = std::get_if<SamplerIndex>(&binding.fResource)) {
            const DawnSampler* sampler =
                    static_cast<const DawnSampler*>(group.getSampler(samplerIdx->fValue));
            entry.sampler = sampler->dawnSampler();
        } else {
            SK_ABORT("unsupported dispatch resource type");
        }
    }

    wgpu::BindGroupDescriptor desc;
    desc.layout = fActiveComputePipeline->dawnGroupLayout();
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    auto bindGroup = fSharedContext->device().CreateBindGroup(&desc);
    fActiveComputePassEncoder.SetBindGroup(0, bindGroup);
}

void DawnCommandBuffer::dispatchWorkgroups(const WorkgroupSize& globalSize) {
    SkASSERT(fActiveComputePassEncoder);
    SkASSERT(fActiveComputePipeline);

    fActiveComputePassEncoder.DispatchWorkgroups(
            globalSize.fWidth, globalSize.fHeight, globalSize.fDepth);
}

void DawnCommandBuffer::endComputePass() {
    SkASSERT(fActiveComputePassEncoder);
    fActiveComputePassEncoder.End();
    fActiveComputePassEncoder = nullptr;
}

bool DawnCommandBuffer::onCopyBufferToBuffer(const Buffer* srcBuffer,
                                             size_t srcOffset,
                                             const Buffer* dstBuffer,
                                             size_t dstOffset,
                                             size_t size) {
    SkASSERT(!fActiveRenderPassEncoder);
    SkASSERT(!fActiveComputePassEncoder);

    auto& wgpuBufferSrc = static_cast<const DawnBuffer*>(srcBuffer)->dawnBuffer();
    auto& wgpuBufferDst = static_cast<const DawnBuffer*>(dstBuffer)->dawnBuffer();

    fCommandEncoder.CopyBufferToBuffer(wgpuBufferSrc, srcOffset, wgpuBufferDst, dstOffset, size);
    return true;
}

bool DawnCommandBuffer::onCopyTextureToBuffer(const Texture* texture,
                                              SkIRect srcRect,
                                              const Buffer* buffer,
                                              size_t bufferOffset,
                                              size_t bufferRowBytes) {
    SkASSERT(!fActiveRenderPassEncoder);
    SkASSERT(!fActiveComputePassEncoder);

    const auto* wgpuTexture = static_cast<const DawnTexture*>(texture);
    auto& wgpuBuffer = static_cast<const DawnBuffer*>(buffer)->dawnBuffer();

    wgpu::ImageCopyTexture src;
    src.texture = wgpuTexture->dawnTexture();
    src.origin.x = srcRect.x();
    src.origin.y = srcRect.y();
    src.aspect = wgpuTexture->textureInfo().dawnTextureSpec().fAspect;

    wgpu::ImageCopyBuffer dst;
    dst.buffer = wgpuBuffer;
    dst.layout.offset = bufferOffset;
    // Dawn requires buffer's alignment to be multiples of 256.
    // https://b.corp.google.com/issues/259264489
    SkASSERT((bufferRowBytes & 0xFF) == 0);
    dst.layout.bytesPerRow = bufferRowBytes;

    wgpu::Extent3D copySize = {
            static_cast<uint32_t>(srcRect.width()), static_cast<uint32_t>(srcRect.height()), 1};
    fCommandEncoder.CopyTextureToBuffer(&src, &dst, &copySize);

    return true;
}

bool DawnCommandBuffer::onCopyBufferToTexture(const Buffer* buffer,
                                              const Texture* texture,
                                              const BufferTextureCopyData* copyData,
                                              int count) {
    SkASSERT(!fActiveRenderPassEncoder);
    SkASSERT(!fActiveComputePassEncoder);

    auto& wgpuTexture = static_cast<const DawnTexture*>(texture)->dawnTexture();
    auto& wgpuBuffer = static_cast<const DawnBuffer*>(buffer)->dawnBuffer();

    wgpu::ImageCopyBuffer src;
    src.buffer = wgpuBuffer;

    wgpu::ImageCopyTexture dst;
    dst.texture = wgpuTexture;

    for (int i = 0; i < count; ++i) {
        src.layout.offset = copyData[i].fBufferOffset;
        // Dawn requires buffer's alignment to be multiples of 256.
        // https://b.corp.google.com/issues/259264489
        SkASSERT((copyData[i].fBufferRowBytes & 0xFF) == 0);
        src.layout.bytesPerRow = copyData[i].fBufferRowBytes;

        dst.origin.x = copyData[i].fRect.x();
        dst.origin.y = copyData[i].fRect.y();
        dst.mipLevel = copyData[i].fMipLevel;

        wgpu::Extent3D copySize = {static_cast<uint32_t>(copyData[i].fRect.width()),
                                   static_cast<uint32_t>(copyData[i].fRect.height()),
                                   1};
        fCommandEncoder.CopyBufferToTexture(&src, &dst, &copySize);
    }

    return true;
}

bool DawnCommandBuffer::onCopyTextureToTexture(const Texture* src,
                                               SkIRect srcRect,
                                               const Texture* dst,
                                               SkIPoint dstPoint,
                                               int mipLevel) {
    SkASSERT(!fActiveRenderPassEncoder);
    SkASSERT(!fActiveComputePassEncoder);

    auto& wgpuTextureSrc = static_cast<const DawnTexture*>(src)->dawnTexture();
    auto& wgpuTextureDst = static_cast<const DawnTexture*>(dst)->dawnTexture();

    wgpu::ImageCopyTexture srcArgs;
    srcArgs.texture = wgpuTextureSrc;
    srcArgs.origin.x = srcRect.fLeft;
    srcArgs.origin.y = srcRect.fTop;

    wgpu::ImageCopyTexture dstArgs;
    dstArgs.texture = wgpuTextureDst;
    dstArgs.origin.x = dstPoint.fX;
    dstArgs.origin.y = dstPoint.fY;
    dstArgs.mipLevel = mipLevel;

    wgpu::Extent3D copySize = {
            static_cast<uint32_t>(srcRect.width()), static_cast<uint32_t>(srcRect.height()), 1};

    fCommandEncoder.CopyTextureToTexture(&srcArgs, &dstArgs, &copySize);

    return true;
}

bool DawnCommandBuffer::onSynchronizeBufferToCpu(const Buffer* buffer, bool* outDidResultInWork) {
    return true;
}

bool DawnCommandBuffer::onClearBuffer(const Buffer* buffer, size_t offset, size_t size) {
    SkASSERT(!fActiveRenderPassEncoder);
    SkASSERT(!fActiveComputePassEncoder);

    auto& wgpuBuffer = static_cast<const DawnBuffer*>(buffer)->dawnBuffer();
    fCommandEncoder.ClearBuffer(wgpuBuffer, offset, size);

    return true;
}

}  // namespace skgpu::graphite

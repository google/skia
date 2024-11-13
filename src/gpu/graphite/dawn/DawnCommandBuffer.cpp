/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnCommandBuffer.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/dawn/DawnBuffer.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnComputePipeline.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnGraphiteTypesPriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"
#include "src/gpu/graphite/dawn/DawnQueueManager.h"
#include "src/gpu/graphite/dawn/DawnSampler.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"

namespace skgpu::graphite {

// DawnCommandBuffer
// ----------------------------------------------------------------------------
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
        : CommandBuffer(Protected::kNo)  // Dawn doesn't support protected memory
        , fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider) {}

DawnCommandBuffer::~DawnCommandBuffer() {}

wgpu::CommandBuffer DawnCommandBuffer::finishEncoding() {
    SkASSERT(fCommandEncoder);
    wgpu::CommandBuffer cmdBuffer = fCommandEncoder.Finish();

    fCommandEncoder = nullptr;

    return cmdBuffer;
}

void DawnCommandBuffer::onResetCommandBuffer() {
    fActiveGraphicsPipeline = nullptr;
    fActiveRenderPassEncoder = nullptr;
    fActiveComputePassEncoder = nullptr;
    fCommandEncoder = nullptr;

    for (auto& bufferSlot : fBoundUniforms) {
        bufferSlot = {};
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
                                        SkIRect renderPassBounds,
                                        const Texture* colorTexture,
                                        const Texture* resolveTexture,
                                        const Texture* depthStencilTexture,
                                        SkIRect viewport,
                                        const DrawPassList& drawPasses) {
    // `viewport` has already been translated by the replay translation by the base CommandBuffer.
    // All GPU backends support viewports that are defined to extend beyond the render target
    // (allowing for a stable linear transformation from NDC to viewport coordinates as the replay
    // translation pushes the viewport off the final deferred target's edges).
    // However, WebGPU validation layers currently require that the viewport is contained within
    // the attachment so we intersect the viewport before setting the intrinsic constants or
    // viewport state.
    // TODO(https://github.com/gpuweb/gpuweb/issues/373): Hopefully the validation layers can be
    // relaxed and then this extra intersection can be removed.
    if (!viewport.intersect(SkIRect::MakeSize(fColorAttachmentSize))) SK_UNLIKELY {
        // The entire pass is offscreen
        return true;
    }

    // Update the intrinsic constant buffer before starting a render pass.
    if (!this->updateIntrinsicUniforms(viewport)) SK_UNLIKELY {
        return false;
    }

    if (!this->beginRenderPass(renderPassDesc,
                               renderPassBounds,
                               colorTexture,
                               resolveTexture,
                               depthStencilTexture)) SK_UNLIKELY {
        return false;
    }

    this->setViewport(viewport);

    for (const auto& drawPass : drawPasses) {
        if (!this->addDrawPass(drawPass.get())) SK_UNLIKELY {
            this->endRenderPass();
            return false;
        }
    }

    this->endRenderPass();
    return true;
}

bool DawnCommandBuffer::onAddComputePass(DispatchGroupSpan groups) {
    this->beginComputePass();
    for (const auto& group : groups) {
        group->addResourceRefs(this);
        for (const auto& dispatch : group->dispatches()) {
            this->bindComputePipeline(group->getPipeline(dispatch.fPipelineIndex));
            this->bindDispatchResources(*group, dispatch);
            if (const WorkgroupSize* globalSize =
                        std::get_if<WorkgroupSize>(&dispatch.fGlobalSizeOrIndirect)) {
                this->dispatchWorkgroups(*globalSize);
            } else {
                SkASSERT(std::holds_alternative<BindBufferInfo>(dispatch.fGlobalSizeOrIndirect));
                const BindBufferInfo& indirect =
                        *std::get_if<BindBufferInfo>(&dispatch.fGlobalSizeOrIndirect);
                this->dispatchWorkgroupsIndirect(indirect.fBuffer, indirect.fOffset);
            }
        }
    }
    this->endComputePass();
    return true;
}

bool DawnCommandBuffer::beginRenderPass(const RenderPassDesc& renderPassDesc,
                                        SkIRect renderPassBounds,
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
#if !defined(__EMSCRIPTEN__)
    wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled mssaRenderToSingleSampledDesc;
    wgpu::RenderPassDescriptorExpandResolveRect wgpuPartialRect = {};
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
            if (renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad) {
                std::optional<wgpu::LoadOp> resolveLoadOp =
                        fSharedContext->dawnCaps()->resolveTextureLoadOp();
                if (resolveLoadOp.has_value()) {
                    wgpuColorAttachment.loadOp = *resolveLoadOp;
#if !defined(__EMSCRIPTEN__)
                    if (fSharedContext->dawnCaps()->supportsPartialLoadResolve()) {
                        wgpuPartialRect.x = renderPassBounds.x();
                        wgpuPartialRect.y = renderPassBounds.y();
                        wgpuPartialRect.width = renderPassBounds.width();
                        wgpuPartialRect.height = renderPassBounds.height();
                        wgpuRenderPass.nextInChain = &wgpuPartialRect;
                    }
#endif
                } else {
                    // No Dawn built-in support, we need to manually load the resolve texture.
                    loadMSAAFromResolveExplicitly = true;
                }
            }
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
        auto format = TextureInfos::GetDawnViewFormat(dawnDepthStencilTexture->textureInfo());
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

bool DawnCommandBuffer::addDrawPass(const DrawPass* drawPass) {
    drawPass->addResourceRefs(this);
    for (auto [type, cmdPtr] : drawPass->commands()) {
        switch (type) {
            case DrawPassCommands::Type::kBindGraphicsPipeline: {
                auto bgp = static_cast<DrawPassCommands::BindGraphicsPipeline*>(cmdPtr);
                if (!this->bindGraphicsPipeline(drawPass->getPipeline(bgp->fPipelineIndex)))
                    SK_UNLIKELY { return false; }
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
                this->bindTextureAndSamplers(*drawPass, *bts);
                break;
            }
            case DrawPassCommands::Type::kSetScissor: {
                auto ss = static_cast<DrawPassCommands::SetScissor*>(cmdPtr);
                this->setScissor(ss->fScissor);
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

    return true;
}

bool DawnCommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
    SkASSERT(fActiveRenderPassEncoder);

    auto* dawnGraphicsPipeline = static_cast<const DawnGraphicsPipeline*>(graphicsPipeline);
    auto& wgpuPipeline = dawnGraphicsPipeline->dawnRenderPipeline();
    if (!wgpuPipeline) SK_UNLIKELY {
        return false;
    }
    fActiveGraphicsPipeline = dawnGraphicsPipeline;
    fActiveRenderPassEncoder.SetPipeline(wgpuPipeline);
    fBoundUniformBuffersDirty = true;

    if (fActiveGraphicsPipeline->dstReadRequirement() == DstReadRequirement::kTextureCopy &&
        fActiveGraphicsPipeline->numFragTexturesAndSamplers() == 2) {
        // The pipeline has a single paired texture+sampler and uses texture copies for dst reads.
        // This situation comes about when the program requires complex blending but otherwise
        // is not referencing any images. Since there are no other images in play, the DrawPass
        // will not have a BindTexturesAndSamplers command that we can tack the dstCopy on to.
        // Instead we need to set the texture BindGroup ASAP to just the dstCopy.
        // TODO(b/366254117): Once we standardize on a pipeline layout across all backends, the dst
        // copy texture may not go in a group with the regular textures, in which case this binding
        // can hopefully happen in a single place (e.g. here or at the start of the renderpass and
        // not also every other time the textures are changed).
        const auto* texture = static_cast<const DawnTexture*>(fDstCopy.first);
        const auto* sampler = static_cast<const DawnSampler*>(fDstCopy.second);
        wgpu::BindGroup bindGroup =
                fResourceProvider->findOrCreateSingleTextureSamplerBindGroup(sampler, texture);
        fActiveRenderPassEncoder.SetBindGroup(
                DawnGraphicsPipeline::kTextureBindGroupIndex, bindGroup);
    }

    return true;
}

void DawnCommandBuffer::bindUniformBuffer(const BindBufferInfo& info, UniformSlot slot) {
    SkASSERT(fActiveRenderPassEncoder);

    unsigned int bufferIndex;
    switch (slot) {
        case UniformSlot::kRenderStep:
            bufferIndex = DawnGraphicsPipeline::kRenderStepUniformBufferIndex;
            break;
        case UniformSlot::kPaint:
            bufferIndex = DawnGraphicsPipeline::kPaintUniformBufferIndex;
            break;
        case UniformSlot::kGradient:
            bufferIndex = DawnGraphicsPipeline::kGradientBufferIndex;
            break;
    }

    fBoundUniforms[bufferIndex] = info;
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

    // When there's an active graphics pipeline with a texture-copy dstread requirement, add one
    // to account for the intrinsic dstCopy texture we bind here.
    // NOTE: This is in units of pairs of textures and samplers, whereas the value reported by
    // the current pipeline is in net bindings (textures + samplers).
    int numTexturesAndSamplers = command.fNumTexSamplers;
    if (fActiveGraphicsPipeline->dstReadRequirement() == DstReadRequirement::kTextureCopy) {
        numTexturesAndSamplers++;
    }
    SkASSERT(fActiveGraphicsPipeline->numFragTexturesAndSamplers() == 2*numTexturesAndSamplers);

    // If possible, it's ideal to optimize for the common case of using a single texture with one
    // dynamic sampler. When using only one sampler, determine whether it is static or dynamic.
    bool usingSingleStaticSampler = false;
#if !defined(__EMSCRIPTEN__)
    if (command.fNumTexSamplers == 1) {
        const wgpu::YCbCrVkDescriptor& ycbcrDesc =
                TextureInfos::GetDawnTextureSpec(
                        drawPass.getTexture(command.fTextureIndices[0])->textureInfo())
                        .fYcbcrVkDescriptor;
        usingSingleStaticSampler = ycbcrUtils::DawnDescriptorIsValid(ycbcrDesc);
    }
#endif

    wgpu::BindGroup bindGroup;
    // Optimize for single texture with dynamic sampling.
    if (numTexturesAndSamplers == 1 && !usingSingleStaticSampler) {
        SkASSERT(fActiveGraphicsPipeline->numFragTexturesAndSamplers() == 2);
        SkASSERT(fActiveGraphicsPipeline->dstReadRequirement() != DstReadRequirement::kTextureCopy);

        const auto* texture =
                static_cast<const DawnTexture*>(drawPass.getTexture(command.fTextureIndices[0]));
        const auto* sampler =
                static_cast<const DawnSampler*>(drawPass.getSampler(command.fSamplerIndices[0]));
        bindGroup = fResourceProvider->findOrCreateSingleTextureSamplerBindGroup(sampler, texture);
    } else {
        std::vector<wgpu::BindGroupEntry> entries;

        for (int i = 0; i < command.fNumTexSamplers; ++i) {
            const auto* texture = static_cast<const DawnTexture*>(
                    drawPass.getTexture(command.fTextureIndices[i]));
            const auto* sampler = static_cast<const DawnSampler*>(
                    drawPass.getSampler(command.fSamplerIndices[i]));
            auto& wgpuTextureView = texture->sampleTextureView();
            auto& wgpuSampler = sampler->dawnSampler();

#if !defined(__EMSCRIPTEN__)
            // Assuming shader generator assigns binding slot to sampler then texture,
            // then the next sampler and texture, and so on, we need to use
            // 2 * i as base binding index of the sampler and texture.
            // TODO: https://b.corp.google.com/issues/259457090:
            // Better configurable way of assigning samplers and textures' bindings.

            DawnTextureInfo dawnTextureInfo;
            TextureInfos::GetDawnTextureInfo(texture->textureInfo(), &dawnTextureInfo);
            const wgpu::YCbCrVkDescriptor& ycbcrDesc = dawnTextureInfo.fYcbcrVkDescriptor;

            // Only add a sampler as a bind group entry if it's a regular dynamic sampler. A valid
            // YCbCrVkDescriptor indicates the usage of a static sampler, which should not be
            // included here. They should already be fully specified in the bind group layout.
            if (!ycbcrUtils::DawnDescriptorIsValid(ycbcrDesc)) {
#endif
                wgpu::BindGroupEntry samplerEntry;
                samplerEntry.binding = 2 * i;
                samplerEntry.sampler = wgpuSampler;
                entries.push_back(samplerEntry);
#if !defined(__EMSCRIPTEN__)
            }
#endif
            wgpu::BindGroupEntry textureEntry;
            textureEntry.binding = 2 * i + 1;
            textureEntry.textureView = wgpuTextureView;
            entries.push_back(textureEntry);
        }

        if (fActiveGraphicsPipeline->dstReadRequirement() == DstReadRequirement::kTextureCopy) {
            // Append the dstCopy sampler and texture as the very last two bind group entries
            wgpu::BindGroupEntry samplerEntry;
            samplerEntry.binding = 2*numTexturesAndSamplers - 2;
            samplerEntry.sampler = static_cast<const DawnSampler*>(fDstCopy.second)->dawnSampler();
            entries.push_back(samplerEntry);

            wgpu::BindGroupEntry textureEntry;
            textureEntry.binding = 2*numTexturesAndSamplers - 1;
            textureEntry.textureView =
                    static_cast<const DawnTexture*>(fDstCopy.first)->sampleTextureView();
            entries.push_back(textureEntry);
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
    static constexpr int kNumBuffers = DawnGraphicsPipeline::kNumUniformBuffers;

    if (fBoundUniformBuffersDirty) {
        fBoundUniformBuffersDirty = false;

        std::array<uint32_t, kNumBuffers> dynamicOffsets;
        std::array<std::pair<const DawnBuffer*, uint32_t>, kNumBuffers> boundBuffersAndSizes;

        std::array<bool, kNumBuffers> enabled = {
            true,                                         // intrinsic uniforms are always enabled
            fActiveGraphicsPipeline->hasStepUniforms(),   // render step uniforms
            fActiveGraphicsPipeline->hasPaintUniforms(),  // paint uniforms
            fActiveGraphicsPipeline->hasGradientBuffer(), // gradient SSBO
        };

        for (int i = 0; i < kNumBuffers; ++i) {
            if (enabled[i] && fBoundUniforms[i]) {
                boundBuffersAndSizes[i].first =
                        static_cast<const DawnBuffer*>(fBoundUniforms[i].fBuffer);
                boundBuffersAndSizes[i].second = fBoundUniforms[i].fSize;
                dynamicOffsets[i] = fBoundUniforms[i].fOffset;
            } else {
                // Unused or null binding
                boundBuffersAndSizes[i].first = nullptr;
                dynamicOffsets[i] = 0;
            }
        }

        auto bindGroup =
                fResourceProvider->findOrCreateUniformBuffersBindGroup(boundBuffersAndSizes);

        fActiveRenderPassEncoder.SetBindGroup(DawnGraphicsPipeline::kUniformBufferBindGroupIndex,
                                              bindGroup,
                                              dynamicOffsets.size(),
                                              dynamicOffsets.data());
    }
}

void DawnCommandBuffer::setScissor(const Scissor& scissor) {
    SkASSERT(fActiveRenderPassEncoder);
    SkIRect rect = scissor.getRect(fReplayTranslation, fReplayClip);
    fActiveRenderPassEncoder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}

bool DawnCommandBuffer::updateIntrinsicUniforms(SkIRect viewport) {
    UniformManager intrinsicValues{Layout::kStd140};
    CollectIntrinsicUniforms(fSharedContext->caps(), viewport, fDstCopyBounds, &intrinsicValues);

    BindBufferInfo binding = fResourceProvider->findOrCreateIntrinsicBindBufferInfo(
            this, UniformDataBlock::Wrap(&intrinsicValues));
    if (!binding) {
        return false;
    } else if (binding == fBoundUniforms[DawnGraphicsPipeline::kIntrinsicUniformBufferIndex]) {
        return true; // no binding change needed
    }

    fBoundUniforms[DawnGraphicsPipeline::kIntrinsicUniformBufferIndex] = binding;
    fBoundUniformBuffersDirty = true;
    return true;
}

void DawnCommandBuffer::setViewport(SkIRect viewport) {
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
        if (const BindBufferInfo* buffer = std::get_if<BindBufferInfo>(&binding.fResource)) {
            entry.buffer = static_cast<const DawnBuffer*>(buffer->fBuffer)->dawnBuffer();
            entry.offset = buffer->fOffset;
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

void DawnCommandBuffer::dispatchWorkgroupsIndirect(const Buffer* indirectBuffer,
                                                   size_t indirectBufferOffset) {
    SkASSERT(fActiveComputePassEncoder);
    SkASSERT(fActiveComputePipeline);

    auto& wgpuIndirectBuffer = static_cast<const DawnBuffer*>(indirectBuffer)->dawnBuffer();
    fActiveComputePassEncoder.DispatchWorkgroupsIndirect(wgpuIndirectBuffer, indirectBufferOffset);
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
    src.aspect = TextureInfos::GetDawnAspect(wgpuTexture->textureInfo());

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

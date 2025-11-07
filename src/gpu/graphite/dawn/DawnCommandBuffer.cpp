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
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnQueueManager.h"
#include "src/gpu/graphite/dawn/DawnSampler.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/version.h>

namespace wgpu {
using TexelCopyBufferInfo = ImageCopyBuffer;
using TexelCopyTextureInfo = ImageCopyTexture;
}  // namespace wgpu
#endif

namespace skgpu::graphite {

// On emsdk before 3.1.48 the API for RenderPass and ComputePass timestamps was different
// and does not map to current webgpu. We check this in DawnCaps but we also must avoid
// naming the types from the new API because they aren't defined.
#if defined(__EMSCRIPTEN__)                                                                  \
        && ((__EMSCRIPTEN_major__ < 3)                                                       \
         || (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ < 1)                          \
         || (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ == 1 && __EMSCRIPTEN_tiny__ < 48))
    #define WGPU_TIMESTAMP_WRITES_DEFINED 0
#else
    #define WGPU_TIMESTAMP_WRITES_DEFINED 1
#endif

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

bool DawnCommandBuffer::startTimerQuery() {
    wgpu::QuerySet querySet = std::move(fTimestampQuerySet);

    auto buffer = fResourceProvider->findOrCreateDawnBuffer(2 * sizeof(uint64_t),
                                                            BufferType::kQuery,
                                                            AccessPattern::kHostVisible,
                                                            "TimerQuery");
    if (!buffer) {
        SKGPU_LOG_W("Failed to create buffer for resolving results, timer query will "
                    "not be reported.");
        return false;
    }
    sk_sp<DawnBuffer> xferBuffer;
    if (!fSharedContext->caps()->drawBufferCanBeMapped()) {
        xferBuffer = fResourceProvider->findOrCreateDawnBuffer(2 * sizeof(uint64_t),
                                                               BufferType::kXferGpuToCpu,
                                                               AccessPattern::kHostVisible,
                                                               "TimerQueryXfer");
        if (!xferBuffer) {
            SKGPU_LOG_W("Failed to create buffer for transferring timestamp results, timer "
                        "query will not be reported.");
            return false;
        }
    }
    if (!querySet) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = 2;
        descriptor.label = "Command Buffer Timer Query";
        descriptor.type = wgpu::QueryType::Timestamp;
        descriptor.nextInChain = nullptr;
        querySet = fSharedContext->device().CreateQuerySet(&descriptor);
        if (!querySet) {
            SKGPU_LOG_W("Failed to create query set, timer query will not be reported.");
            return false;
        }
    }

    fTimestampQuerySet = std::move(querySet);
    fTimestampQueryBuffer = std::move(buffer);
    fTimestampQueryXferBuffer = std::move(xferBuffer);

    if (fSharedContext->dawnCaps()->supportsCommandBufferTimestamps()) {
        // In native Dawn we use the writeTimeStamp method on CommandBuffer to bracket
        // the whole command buffer.
        fCommandEncoder.WriteTimestamp(fTimestampQuerySet, 0);
    } else {
        // On WebGPU the best we can do is add timestamps to each render/compute pass as we record
        // them. Since we don't know a priori how many passes there will be we write a begin
        // timestamp on the first pass and an end on every pass, overwriting the second query in the
        // set.
        fWroteFirstPassTimestamps = false;
    }

    return true;
}

void DawnCommandBuffer::endTimerQuery() {
    SkASSERT(fTimestampQuerySet);
    SkASSERT(fTimestampQueryBuffer);
    if (fSharedContext->dawnCaps()->supportsCommandBufferTimestamps()) {
        fCommandEncoder.WriteTimestamp(fTimestampQuerySet, 1);
    } else if (!fWroteFirstPassTimestamps) {
        // If we didn't have any passes then we can't report anything.
        fTimestampQueryBuffer = nullptr;
        fTimestampQueryXferBuffer = nullptr;
        return;
    }
    // This resolve covers both timestamp code paths (command encoder and render/compute pass).
    fCommandEncoder.ResolveQuerySet(
            fTimestampQuerySet, 0, 2, fTimestampQueryBuffer->dawnBuffer(), 0);
    if (fTimestampQueryXferBuffer) {
        SkASSERT(fTimestampQueryBuffer->size() == fTimestampQueryBuffer->size());
        fCommandEncoder.CopyBufferToBuffer(fTimestampQueryBuffer->dawnBuffer(),
                                           /*sourceOffset=*/0,
                                           fTimestampQueryXferBuffer->dawnBuffer(),
                                           /*destinationOffset=*/0,
                                           /*size=*/fTimestampQueryBuffer->size());
        fTimestampQueryBuffer.reset();
    }
    if (fSharedContext->caps()->bufferMapsAreAsync()) {
        sk_sp<Buffer> buffer =
                fTimestampQueryXferBuffer ? fTimestampQueryXferBuffer : fTimestampQueryBuffer;
        this->addBuffersToAsyncMapOnSubmit({&buffer, 1});
    }
}

std::optional<GpuStats> DawnCommandBuffer::gpuStats() {
    auto& buffer = fTimestampQueryXferBuffer ? fTimestampQueryXferBuffer : fTimestampQueryBuffer;
    if (!buffer) {
        return {};
    }
    if (fSharedContext->caps()->bufferMapsAreAsync()) {
        if (!buffer->isMapped()) {
            return {};
        }
    }
    uint64_t* results = static_cast<uint64_t*>(buffer->map());
    if (!results) {
        SKGPU_LOG_W("Failed to get timer query results because buffer couldn't be mapped.");
        return {};
    }
    if (results[1] < results[0]) {
        return {};
    }
    GpuStats stats;
    stats.elapsedTime = results[1] - results[0];
    return stats;
}

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
    if (fTimestampQueryBuffer && fTimestampQueryBuffer->isUnmappable()) {
        fTimestampQueryBuffer->unmap();
    }
    if (fTimestampQueryXferBuffer && fTimestampQueryXferBuffer->isUnmappable()) {
        fTimestampQueryXferBuffer->unmap();
    }
    fTimestampQueryBuffer = {};
    fTimestampQueryXferBuffer = {};
    fWroteFirstPassTimestamps = false;
}

bool DawnCommandBuffer::setNewCommandBufferResources() {
    SkASSERT(!fCommandEncoder);
    fCommandEncoder = fSharedContext->device().CreateCommandEncoder();
    SkASSERT(fCommandEncoder);

    return true;
}

// Requests a sampler. Dynamic samplers live in the global cache, requiring no tracking, but
// immutable samplers are created on the current graphics pipeline, and may outlive it, requiring
// further tracking.
const DawnSampler* DawnCommandBuffer::getSampler(
        const DrawPassCommands::BindTexturesAndSamplers& command, int32_t index) {
    auto desc = command.fSamplers[index];
    if (desc.isImmutable()) {
        const DawnSampler* immutableSampler = fActiveGraphicsPipeline->immutableSampler(index);
        if (immutableSampler) {
            this->trackCommandBufferResource(sk_ref_sp<Sampler>(immutableSampler));
        }
        return immutableSampler;
    } else {
        // Use the shared Sampler held in the global cache
        return static_cast<const DawnSampler*>(
                fSharedContext->globalCache()->getDynamicSampler(desc));
    }
}

bool DawnCommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                        SkIRect renderPassBounds,
                                        const Texture* colorTexture,
                                        const Texture* resolveTexture,
                                        const Texture* depthStencilTexture,
                                        SkIPoint resolveOffset,
                                        SkIRect viewport,
                                        const DrawPassList& drawPasses) {
    // `viewport` has already been translated by the replay translation by the base CommandBuffer
    if (!SkIRect::Intersects(viewport, fRenderPassBounds)) SK_UNLIKELY {
            // The entire pass is offscreen
            return true;
        }


    UniformManager intrinsicValues{Layout::kStd140};
    CollectIntrinsicUniforms(fSharedContext->caps(), viewport, fDstReadBounds, &intrinsicValues);
    const auto uniformData = UniformDataBlock::Wrap(&intrinsicValues);
    const bool usePushConstant = fSharedContext->dawnCaps()
                                         ->resourceBindingRequirements()
                                         .fUsePushConstantsForIntrinsicConstants;

    // If push constant is not supported, update the intrinsic constant buffer before starting a
    // render pass.
    if (!usePushConstant && !this->updateIntrinsicUniformsAsUBO(uniformData)) SK_UNLIKELY {
        return false;
    }

    if (!this->beginRenderPass(renderPassDesc,
                               resolveOffset,
                               renderPassBounds,
                               colorTexture,
                               resolveTexture,
                               depthStencilTexture)) SK_UNLIKELY {
        return false;
    }

    // If push constant is supported, update the intrinsic constants after starting a render pass.
    if (usePushConstant && !this->updateIntrinsicUniformsAsPushConstant(uniformData)) SK_UNLIKELY {
        return false;
    }

    this->setViewport(viewport);

    for (const auto& drawPass : drawPasses) {
        if (!this->addDrawPass(drawPass.get())) SK_UNLIKELY {
            this->endRenderPass();
            return false;
        }
    }

    return this->endRenderPass();
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
                                        const SkIPoint& resolveOffset,
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
    wgpu::RenderPassDescriptorResolveRect wgpuPartialRect = {};
#endif

#if WGPU_TIMESTAMP_WRITES_DEFINED
#if defined(__EMSCRIPTEN__)
    wgpu::RenderPassTimestampWrites wgpuTimestampWrites;
#else
    wgpu::PassTimestampWrites wgpuTimestampWrites;
#endif
    if (!fSharedContext->dawnCaps()->supportsCommandBufferTimestamps() && fTimestampQueryBuffer) {
        SkASSERT(fTimestampQuerySet);
        wgpuTimestampWrites.querySet = fTimestampQuerySet;
        if (!fWroteFirstPassTimestamps) {
            wgpuTimestampWrites.beginningOfPassWriteIndex = 0;
            fWroteFirstPassTimestamps = true;
        }
        wgpuTimestampWrites.endOfPassWriteIndex = 1;
        wgpuRenderPass.timestampWrites = &wgpuTimestampWrites;
    }
#else
    SkASSERT(!fTimestampQueryBuffer);
#endif

    // Validate attachment descs and textures
    const auto& colorInfo = renderPassDesc.fColorAttachment;
    const auto& resolveInfo = renderPassDesc.fColorResolveAttachment;
    const auto& depthStencilInfo = renderPassDesc.fDepthStencilAttachment;
    SkASSERT(colorTexture ? colorInfo.isCompatible(colorTexture->textureInfo())
                          : colorInfo.fFormat == TextureFormat::kUnsupported);
    SkASSERT(resolveTexture ? resolveInfo.isCompatible(resolveTexture->textureInfo())
                            : resolveInfo.fFormat == TextureFormat::kUnsupported);
    SkASSERT(depthStencilTexture ? depthStencilInfo.isCompatible(depthStencilTexture->textureInfo())
                                 : depthStencilInfo.fFormat == TextureFormat::kUnsupported);

    // Set up color attachment
    bool emulateLoadStoreResolveTexture = false;
    if (colorTexture) {
        wgpuRenderPass.colorAttachments = &wgpuColorAttachment;
        wgpuRenderPass.colorAttachmentCount = 1;

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
            SkASSERT(resolveInfo.fStoreOp == StoreOp::kStore);

            const auto* dawnResolveTexture = static_cast<const DawnTexture*>(resolveTexture);
            SkASSERT(dawnResolveTexture->renderTextureView());
            wgpuColorAttachment.resolveTarget = dawnResolveTexture->renderTextureView();

            // Inclusion of a resolve texture implies the client wants to finish the
            // renderpass with a resolve.
            SkASSERT(wgpuColorAttachment.storeOp == wgpu::StoreOp::Discard);
            // But it also means we might have to load the resolve texture into the MSAA color attachment

            if (fSharedContext->dawnCaps()->emulateLoadStoreResolve()) {
                emulateLoadStoreResolveTexture = true;
            } else if (resolveInfo.fLoadOp == LoadOp::kLoad) {
                std::optional<wgpu::LoadOp> resolveLoadOp =
                        fSharedContext->dawnCaps()->resolveTextureLoadOp();
                if (resolveLoadOp.has_value()) {
                    wgpuColorAttachment.loadOp = *resolveLoadOp;
                } else {
                    // No Dawn built-in support, we need to manually load the resolve texture.
                    emulateLoadStoreResolveTexture = true;
                }
            }

            if (!emulateLoadStoreResolveTexture) {
#if !defined(__EMSCRIPTEN__)
                if (fSharedContext->dawnCaps()->supportsPartialLoadResolve()) {
                    SkIRect msaaArea = renderPassBounds;
                    SkAssertResult(msaaArea.intersect(SkIRect::MakeSize(
                            colorTexture->dimensions())));
                    wgpuPartialRect.colorOffsetX = msaaArea.x();
                    wgpuPartialRect.colorOffsetY = msaaArea.y();

                    SkIRect resolveArea = renderPassBounds;
                    resolveArea.offset(resolveOffset);
                    SkAssertResult(resolveArea.intersect(SkIRect::MakeSize(
                            resolveTexture->dimensions())));
                    wgpuPartialRect.resolveOffsetX = resolveArea.x();
                    wgpuPartialRect.resolveOffsetY = resolveArea.y();
                    wgpuPartialRect.width = resolveArea.width();
                    wgpuPartialRect.height = resolveArea.height();
                    wgpuRenderPass.nextInChain = &wgpuPartialRect;
                } else
#endif
                {
                    SkASSERT(resolveOffset.isZero());
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
    if (depthStencilTexture) {
        const auto* dawnDepthStencilTexture = static_cast<const DawnTexture*>(depthStencilTexture);

        SkASSERT(dawnDepthStencilTexture->renderTextureView());
        wgpuDepthStencilAttachment.view = dawnDepthStencilTexture->renderTextureView();

        if (TextureFormatHasDepth(depthStencilInfo.fFormat)) {
            wgpuDepthStencilAttachment.depthClearValue = renderPassDesc.fClearDepth;
            wgpuDepthStencilAttachment.depthLoadOp =
                    wgpuLoadActionMap[static_cast<int>(depthStencilInfo.fLoadOp)];
            wgpuDepthStencilAttachment.depthStoreOp =
                    wgpuStoreActionMap[static_cast<int>(depthStencilInfo.fStoreOp)];
        }

        if (TextureFormatHasStencil(depthStencilInfo.fFormat)) {
            wgpuDepthStencilAttachment.stencilClearValue = renderPassDesc.fClearStencil;
            wgpuDepthStencilAttachment.stencilLoadOp =
                    wgpuLoadActionMap[static_cast<int>(depthStencilInfo.fLoadOp)];
            wgpuDepthStencilAttachment.stencilStoreOp =
                    wgpuStoreActionMap[static_cast<int>(depthStencilInfo.fStoreOp)];
        }

        wgpuRenderPass.depthStencilAttachment = &wgpuDepthStencilAttachment;
    }

    if (emulateLoadStoreResolveTexture) {
        if (!this->emulateLoadMSAAFromResolveAndBeginRenderPassEncoder(
                    renderPassDesc,
                    wgpuRenderPass,
                    resolveOffset,
                    renderPassBounds,
                    static_cast<const DawnTexture*>(colorTexture),
                    static_cast<const DawnTexture*>(resolveTexture))) {
            return false;
        }
    }
    else {
        fActiveRenderPassEncoder = fCommandEncoder.BeginRenderPass(&wgpuRenderPass);
    }

    return true;
}

bool DawnCommandBuffer::emulateLoadMSAAFromResolveAndBeginRenderPassEncoder(
        const RenderPassDesc& intendedRenderPassDesc,
        const wgpu::RenderPassDescriptor& intendedDawnRenderPassDesc,
        const SkIPoint& resolveOffset,
        const SkIRect& renderPassBounds,
        const DawnTexture* msaaTexture,
        const DawnTexture* resolveTexture) {
    SkASSERT(!fActiveRenderPassEncoder);

    const bool loadResolve =
            intendedRenderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;

    // Override the render pass to exclude the resolve texture. We will emulate the loading &
    // resolve via blit. The resovle step will be done separately after endRenderPass()
    RenderPassDesc renderPassWithoutResolveDesc = intendedRenderPassDesc;
    renderPassWithoutResolveDesc.fColorResolveAttachment = {};

    wgpu::RenderPassColorAttachment dawnColorAttachmentWithoutResolve =
            intendedDawnRenderPassDesc.colorAttachments[0];
    dawnColorAttachmentWithoutResolve.resolveTarget = nullptr;
    dawnColorAttachmentWithoutResolve.storeOp = wgpu::StoreOp::Store;

    if (loadResolve) {
        // If we intend to load the resolve texture, then override the loadOp of the MSAA attachment
        // to Load instead of Clear.
        // This path is intended to be used when the device doesn't support transient attachments.
        // Which most likely means it is a non-tiled GPU. On non-tiled GPUs, load is a no-op so it
        // should be faster than clearing the whole MSAA attachment. Note: Dawn doesn't have any
        // DontCare loadOp.
        dawnColorAttachmentWithoutResolve.loadOp = wgpu::LoadOp::Load;
    }

    wgpu::RenderPassDescriptor dawnRenderPassDescWithoutResolve = intendedDawnRenderPassDesc;
    dawnRenderPassDescWithoutResolve.colorAttachments = &dawnColorAttachmentWithoutResolve;
    dawnRenderPassDescWithoutResolve.colorAttachmentCount = 1;

    auto renderPassEncoder = fCommandEncoder.BeginRenderPass(&dawnRenderPassDescWithoutResolve);

    SkIRect msaaArea = renderPassBounds;
    msaaArea.intersect(SkIRect::MakeSize(msaaTexture->dimensions()));
    SkIRect resolveArea = renderPassBounds;
    resolveArea.offset(resolveOffset);
    resolveArea.intersect(SkIRect::MakeSize(resolveTexture->dimensions()));

    if (loadResolve) {
        // Blit from the resolve texture to the MSAA texture
        if (!this->doBlitWithDraw(renderPassEncoder,
                                  renderPassWithoutResolveDesc,
                                  /*srcTextureView=*/resolveTexture->renderTextureView(),
                                  /*srcSampleCount=*/1,
                                  /*srcOffset=*/resolveArea.topLeft(),
                                  /*dstBounds=*/msaaArea)) {
            renderPassEncoder.End();
            return false;
        }
    }

    fActiveRenderPassEncoder = renderPassEncoder;

    fResolveStepEmulationInfo = {msaaTexture, resolveTexture, msaaArea.topLeft(), resolveArea};

    return true;
}

bool DawnCommandBuffer::doBlitWithDraw(const wgpu::RenderPassEncoder& renderEncoder,
                                       const RenderPassDesc& frontendRenderPassDescKey,
                                       const wgpu::TextureView& srcTextureView,
                                       int srcSampleCount,
                                       const SkIPoint& srcOffset,
                                       const SkIRect& dstBounds) {
    DawnResourceProvider::BlitWithDrawEncoder blit =
            fResourceProvider->findOrCreateBlitWithDrawEncoder(frontendRenderPassDescKey,
                                                               srcSampleCount);
    if (!blit) {
        SKGPU_LOG_E("Unable to create pipeline to blit with draw");
        return false;
    }

    SkASSERT(renderEncoder);

    blit.EncodeBlit(fSharedContext->device(), renderEncoder, srcTextureView, srcOffset, dstBounds);

    return true;
}

bool DawnCommandBuffer::endRenderPass() {
    SkASSERT(fActiveRenderPassEncoder);
    fActiveRenderPassEncoder.End();
    fActiveRenderPassEncoder = nullptr;

    // Return early if no resolve step's emulation is needed.
    if (!fResolveStepEmulationInfo.has_value()) {
        return true;
    }

    // Creating an intermediate render pass to copy from the MSAA texture -> resolve texture.
    RenderPassDesc intermediateRenderPassDesc = {};
    intermediateRenderPassDesc.fColorAttachment = {
            TextureInfoPriv::ViewFormat(fResolveStepEmulationInfo->fResolveTexture->textureInfo()),
            LoadOp::kLoad,
            StoreOp::kStore,
            /*fSampleCount=*/1 };

    wgpu::RenderPassColorAttachment dawnIntermediateColorAttachment;
    dawnIntermediateColorAttachment.loadOp = wgpu::LoadOp::Load;
    dawnIntermediateColorAttachment.storeOp = wgpu::StoreOp::Store;
    dawnIntermediateColorAttachment.view =
            fResolveStepEmulationInfo->fResolveTexture->renderTextureView();

    wgpu::RenderPassDescriptor dawnIntermediateRenderPassDesc;
    dawnIntermediateRenderPassDesc.colorAttachmentCount = 1;
    dawnIntermediateRenderPassDesc.colorAttachments = &dawnIntermediateColorAttachment;

    auto renderPassEncoder = fCommandEncoder.BeginRenderPass(&dawnIntermediateRenderPassDesc);

    bool blitSucceeded = this->doBlitWithDraw(
            renderPassEncoder,
            intermediateRenderPassDesc,
            /*srcTextureView=*/fResolveStepEmulationInfo->fMSAATexture->renderTextureView(),
            /*srcSampleCount=*/fResolveStepEmulationInfo->fMSAATexture->textureInfo().numSamples(),
            /*srcOffset=*/fResolveStepEmulationInfo->fMSAAAOffset,
            /*dstBounds=*/fResolveStepEmulationInfo->fResolveArea);

    renderPassEncoder.End();

    fResolveStepEmulationInfo.reset();

    return blitSucceeded;
}

bool DawnCommandBuffer::addDrawPass(DrawPass* drawPass) {
    // If there is gradient data to bind, it must be done prior to draws.
    if (drawPass->floatStorageManager()->hasData()) {
        this->bindUniformBuffer(drawPass->floatStorageManager()->getBufferInfo(),
                                UniformSlot::kGradient);
    }

    if (!drawPass->addResourceRefs(fResourceProvider, this)) SK_UNLIKELY {
        return false;
    }

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
            case DrawPassCommands::Type::kBindStaticDataBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindStaticDataBuffer*>(cmdPtr);
                this->bindInputBuffer(bdb->fStaticData.fBuffer, bdb->fStaticData.fOffset,
                                      DawnGraphicsPipeline::kStaticDataBufferIndex);
                break;
            }
            case DrawPassCommands::Type::kBindAppendDataBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindAppendDataBuffer*>(cmdPtr);
                this->bindInputBuffer(bdb->fAppendData.fBuffer, bdb->fAppendData.fOffset,
                                      DawnGraphicsPipeline::kAppendDataBufferIndex);
                break;
            }
            case DrawPassCommands::Type::kBindIndexBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindIndexBuffer*>(cmdPtr);
                this->bindIndexBuffer(
                        bdb->fIndices.fBuffer, bdb->fIndices.fOffset);
                break;
            }
            case DrawPassCommands::Type::kBindIndirectBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindIndirectBuffer*>(cmdPtr);
                this->bindIndirectBuffer(
                        bdb->fIndirect.fBuffer, bdb->fIndirect.fOffset);
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
            case DrawPassCommands::Type::kAddBarrier: {
                SKGPU_LOG_E("DawnCommandBuffer does not support the addition of barriers.");
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

    if (fActiveGraphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy &&
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

void DawnCommandBuffer::bindInputBuffer(const Buffer* buffer, size_t offset,
                                        uint32_t bindingIndex) {
    SkASSERT(fActiveRenderPassEncoder);
    if (buffer) {
        auto dawnBuffer = static_cast<const DawnBuffer*>(buffer)->dawnBuffer();
        fActiveRenderPassEncoder.SetVertexBuffer(bindingIndex, dawnBuffer, offset);
    }
}

void DawnCommandBuffer::bindIndexBuffer(const Buffer* indexBuffer, size_t offset) {
    SkASSERT(fActiveRenderPassEncoder);
    if (indexBuffer) {
        auto dawnBuffer = static_cast<const DawnBuffer*>(indexBuffer)->dawnBuffer();
        fActiveRenderPassEncoder.SetIndexBuffer(
                dawnBuffer, wgpu::IndexFormat::Uint16, offset);
    }
}

void DawnCommandBuffer::bindIndirectBuffer(const Buffer* indirectBuffer, size_t offset) {
    if (indirectBuffer) {
        fCurrentIndirectBuffer = static_cast<const DawnBuffer*>(indirectBuffer)->dawnBuffer();
        fCurrentIndirectBufferOffset = offset;
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
    if (fActiveGraphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy) {
        numTexturesAndSamplers++;
    }
    SkASSERT(fActiveGraphicsPipeline->numFragTexturesAndSamplers() == 2*numTexturesAndSamplers);

    // If possible, it's ideal to optimize for the common case of using a single texture with one
    // dynamic sampler. When using only one sampler, determine whether it is static or dynamic.
    bool usingSingleStaticSampler = false;
#if !defined(__EMSCRIPTEN__)
    if (command.fNumTexSamplers == 1) {
        const wgpu::YCbCrVkDescriptor& ycbcrDesc =
                TextureInfoPriv::Get<DawnTextureInfo>(
                    command.fTextures[0]->texture()->textureInfo())
                        .fYcbcrVkDescriptor;
        usingSingleStaticSampler = DawnDescriptorIsValid(ycbcrDesc);
    }
#endif

    wgpu::BindGroup bindGroup;
    // Optimize for single texture with dynamic sampling.
    if (numTexturesAndSamplers == 1 && !usingSingleStaticSampler) {
        SkASSERT(fActiveGraphicsPipeline->numFragTexturesAndSamplers() == 2);
        SkASSERT(fActiveGraphicsPipeline->dstReadStrategy() != DstReadStrategy::kTextureCopy);

        const auto* texture = static_cast<const DawnTexture*>(command.fTextures[0]->texture());
        const auto* sampler = this->getSampler(command, 0);
        bindGroup = fResourceProvider->findOrCreateSingleTextureSamplerBindGroup(sampler, texture);
    } else {
        std::vector<wgpu::BindGroupEntry> entries;

        for (int i = 0; i < command.fNumTexSamplers; ++i) {
            const auto* texture = static_cast<const DawnTexture*>(command.fTextures[i]->texture());
            const auto* sampler = this->getSampler(command, i);
            auto& wgpuTextureView = texture->sampleTextureView();
            auto& wgpuSampler = sampler->dawnSampler();

#if !defined(__EMSCRIPTEN__)
            // Assuming shader generator assigns binding slot to sampler then texture,
            // then the next sampler and texture, and so on, we need to use
            // 2 * i as base binding index of the sampler and texture.
            // TODO: https://b.corp.google.com/issues/259457090:
            // Better configurable way of assigning samplers and textures' bindings.
            const wgpu::YCbCrVkDescriptor& ycbcrDesc =
                    texture->dawnTextureInfo().fYcbcrVkDescriptor;

            // Only add a sampler as a bind group entry if it's a regular dynamic sampler. A valid
            // YCbCrVkDescriptor indicates the usage of a static sampler, which should not be
            // included here. They should already be fully specified in the bind group layout.
            if (!DawnDescriptorIsValid(ycbcrDesc)) {
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

        if (fActiveGraphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy) {
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
                !fSharedContext->dawnCaps()
                         ->resourceBindingRequirements()
                         .fUsePushConstantsForIntrinsicConstants,  // intrinsic uniforms
                fActiveGraphicsPipeline->hasStepUniforms(),            // render step uniforms
                fActiveGraphicsPipeline->hasPaintUniforms(),           // paint uniforms
                fActiveGraphicsPipeline->hasGradientBuffer(),          // gradient SSBO
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
    SkIRect rect = scissor.getRect(fReplayTranslation, fRenderPassBounds);
    fActiveRenderPassEncoder.SetScissorRect(rect.x(), rect.y(), rect.width(), rect.height());
}

bool DawnCommandBuffer::updateIntrinsicUniformsAsUBO(UniformDataBlock uniformData) {
    BindBufferInfo binding =
            fResourceProvider->findOrCreateIntrinsicBindBufferInfo(this, uniformData);

    if (!binding) {
        return false;
    } else if (binding == fBoundUniforms[DawnGraphicsPipeline::kIntrinsicUniformBufferIndex]) {
        return true; // no binding change needed
    }

    fBoundUniforms[DawnGraphicsPipeline::kIntrinsicUniformBufferIndex] = binding;
    fBoundUniformBuffersDirty = true;
    return true;
}

bool DawnCommandBuffer::updateIntrinsicUniformsAsPushConstant(UniformDataBlock uniformData) {
#if !defined(__EMSCRIPTEN__)
    SkASSERT(fActiveRenderPassEncoder);
    SkASSERT(uniformData.size() <= DawnGraphicsPipeline::kIntrinsicUniformSize);
    fActiveRenderPassEncoder.SetImmediates(0, uniformData.data(), uniformData.size());
    return true;
#else
    SkASSERT(false); // No push constant support in WASM yet
    return false;
#endif
}

void DawnCommandBuffer::setViewport(SkIRect viewport) {
    SkASSERT(fActiveRenderPassEncoder);
    fActiveRenderPassEncoder.SetViewport(
            viewport.x(), viewport.y(), viewport.width(), viewport.height(), 0, 1);
}

void DawnCommandBuffer::setBlendConstants(std::array<float, 4> blendConstants) {
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
    wgpu::ComputePassDescriptor wgpuComputePassDescriptor = {};
#if WGPU_TIMESTAMP_WRITES_DEFINED
#if defined(__EMSCRIPTEN__)
    wgpu::ComputePassTimestampWrites wgpuTimestampWrites;
#else
    wgpu::PassTimestampWrites wgpuTimestampWrites;
#endif
    if (!fSharedContext->dawnCaps()->supportsCommandBufferTimestamps() && fTimestampQueryBuffer) {
        SkASSERT(fTimestampQuerySet);
        wgpuTimestampWrites.querySet = fTimestampQuerySet;
        if (!fWroteFirstPassTimestamps) {
            wgpuTimestampWrites.beginningOfPassWriteIndex = 0;
            fWroteFirstPassTimestamps = true;
        }
        wgpuTimestampWrites.endOfPassWriteIndex = 1;
        wgpuComputePassDescriptor.timestampWrites = &wgpuTimestampWrites;
    }
#else
    SkASSERT(!fTimestampQueryBuffer);
#endif
    fActiveComputePassEncoder = fCommandEncoder.BeginComputePass(&wgpuComputePassDescriptor);
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

    wgpu::TexelCopyTextureInfo src;
    src.texture = wgpuTexture->dawnTexture();
    src.origin.x = srcRect.x();
    src.origin.y = srcRect.y();
    src.aspect = wgpuTexture->dawnTextureInfo().fAspect;

    wgpu::TexelCopyBufferInfo dst;
    dst.buffer = wgpuBuffer;
    dst.layout.offset = bufferOffset;
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

    wgpu::TexelCopyBufferInfo src;
    src.buffer = wgpuBuffer;

    wgpu::TexelCopyTextureInfo dst;
    dst.texture = wgpuTexture;

    for (int i = 0; i < count; ++i) {
        src.layout.offset = copyData[i].fBufferOffset;
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

    wgpu::TexelCopyTextureInfo srcArgs;
    srcArgs.texture = wgpuTextureSrc;
    srcArgs.origin.x = srcRect.fLeft;
    srcArgs.origin.y = srcRect.fTop;

    wgpu::TexelCopyTextureInfo dstArgs;
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

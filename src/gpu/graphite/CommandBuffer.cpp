/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/CommandBuffer.h"

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/Sampler.h"  // IWYU pragma: keep
#include "src/gpu/graphite/Texture.h"

#include <algorithm>
#include <atomic>
#include <cstdint>

namespace skgpu::graphite {

CommandBuffer::CommandBuffer(Protected isProtected) : fIsProtected(isProtected) {}

CommandBuffer::~CommandBuffer() {
    this->releaseResources();
}

void CommandBuffer::releaseResources() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    fTrackedUsageResources.clear();
    fCommandBufferResources.clear();
}

void CommandBuffer::resetCommandBuffer() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    // The dst copy texture and sampler are kept alive by the tracked resources, so reset these
    // before we release their refs. Assuming we don't go idle and free lots of resources, we'll
    // get the same cached sampler the next time we need a dst copy.
    fDstCopy = {nullptr, nullptr};
    this->releaseResources();
    this->onResetCommandBuffer();
    fBuffersToAsyncMap.clear();
}

void CommandBuffer::trackResource(sk_sp<Resource> resource) {
    fTrackedUsageResources.push_back(std::move(resource));
}

void CommandBuffer::trackCommandBufferResource(sk_sp<Resource> resource) {
    fCommandBufferResources.push_back(std::move(resource));
}

void CommandBuffer::addFinishedProc(sk_sp<RefCntedCallback> finishedProc) {
    fFinishedProcs.push_back(std::move(finishedProc));
}

void CommandBuffer::callFinishedProcs(bool success) {
    if (!success) {
        for (int i = 0; i < fFinishedProcs.size(); ++i) {
            fFinishedProcs[i]->setFailureResult();
        }
    } else {
        if (auto stats = this->gpuStats()) {
            for (int i = 0; i < fFinishedProcs.size(); ++i) {
                if (fFinishedProcs[i]->receivesGpuStats()) {
                    fFinishedProcs[i]->setStats(*stats);
                }
            }
        }
    }
    fFinishedProcs.clear();
}

void CommandBuffer::addBuffersToAsyncMapOnSubmit(SkSpan<const sk_sp<Buffer>> buffers) {
    for (size_t i = 0; i < buffers.size(); ++i) {
        SkASSERT(buffers[i]);
        fBuffersToAsyncMap.push_back(buffers[i]);
    }
}

SkSpan<const sk_sp<Buffer>> CommandBuffer::buffersToAsyncMapOnSubmit() const {
    return fBuffersToAsyncMap;
}

bool CommandBuffer::addRenderPass(const RenderPassDesc& renderPassDesc,
                                  sk_sp<Texture> colorTexture,
                                  sk_sp<Texture> resolveTexture,
                                  sk_sp<Texture> depthStencilTexture,
                                  const Texture* dstCopy,
                                  SkIRect dstReadBounds,
                                  SkIPoint resolveOffset,
                                  SkISize viewportDims,
                                  const DrawPassList& drawPasses) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    SkIRect renderPassBounds;
    for (const auto& drawPass : drawPasses) {
        renderPassBounds.join(drawPass->bounds());
    }
    if (renderPassDesc.fColorAttachment.fLoadOp == LoadOp::kClear) {
        renderPassBounds.join(fRenderPassBounds);
    }
    renderPassBounds.offset(fReplayTranslation.x(), fReplayTranslation.y());
    if (!renderPassBounds.intersect(fRenderPassBounds)) {
        // The entire RenderPass is offscreen given the replay translation so skip adding the pass
        // at all
        return true;
    }

    dstReadBounds.offset(fReplayTranslation);
    if (!dstReadBounds.intersect(fRenderPassBounds)) {
        // The draws within the RenderPass that would sample from the dstCopy have been translated
        // off screen. Set the bounds to empty and let the GPU clipping do its job.
        dstReadBounds = SkIRect::MakeEmpty();
    }
    // Save the dstCopy texture so that it can be embedded into texture bind commands later on.
    // Stash the texture's full dimensions on the rect so we can calculate normalized coords later.
    fDstCopy.first = dstCopy;
    fDstReadBounds = dstCopy ? SkIRect::MakePtSize(dstReadBounds.topLeft(), dstCopy->dimensions())
                             : SkIRect::MakeEmpty();
    if (dstCopy && !fDstCopy.second) {
        // Only lookup the sampler the first time we require a dstCopy. The texture can change
        // on subsequent passes but it will always use the same nearest neighbor sampling.
        sk_sp<Sampler> nearestNeighbor = this->resourceProvider()->findOrCreateCompatibleSampler(
                {SkFilterMode::kNearest, SkTileMode::kClamp});
        fDstCopy.second = nearestNeighbor.get();
        this->trackResource(std::move(nearestNeighbor));
    }

    // We don't intersect the viewport with the render pass bounds or target size because it just
    // defines a linear transform, which we don't want to change just because a portion of it maps
    // to a region that gets clipped.
    SkIRect viewport = SkIRect::MakePtSize(fReplayTranslation, viewportDims);
    if (!this->onAddRenderPass(renderPassDesc,
                               renderPassBounds,
                               colorTexture.get(),
                               resolveTexture.get(),
                               depthStencilTexture.get(),
                               resolveOffset,
                               viewport,
                               drawPasses)) {
        return false;
    }

    if (colorTexture) {
        this->trackCommandBufferResource(std::move(colorTexture));
    }
    if (resolveTexture) {
        this->trackCommandBufferResource(std::move(resolveTexture));
    }
    if (depthStencilTexture) {
        this->trackCommandBufferResource(std::move(depthStencilTexture));
    }
    // We just assume if you are adding a render pass that the render pass will actually do work. In
    // theory we could have a discard load that doesn't submit any draws, clears, etc. But hopefully
    // something so trivial would be caught before getting here.
    SkDEBUGCODE(fHasWork = true;)

    return true;
}

bool CommandBuffer::addComputePass(DispatchGroupSpan dispatchGroups) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    if (!this->onAddComputePass(dispatchGroups)) {
        return false;
    }

    SkDEBUGCODE(fHasWork = true;)

    return true;
}

bool CommandBuffer::copyBufferToBuffer(const Buffer* srcBuffer,
                                       size_t srcOffset,
                                       sk_sp<Buffer> dstBuffer,
                                       size_t dstOffset,
                                       size_t size) {
    SkASSERT(srcBuffer);
    SkASSERT(dstBuffer);

    if (!this->onCopyBufferToBuffer(srcBuffer, srcOffset, dstBuffer.get(), dstOffset, size)) {
        return false;
    }

    this->trackResource(std::move(dstBuffer));

    SkDEBUGCODE(fHasWork = true;)

    return true;
}

bool CommandBuffer::copyTextureToBuffer(sk_sp<Texture> texture,
                                        SkIRect srcRect,
                                        sk_sp<Buffer> buffer,
                                        size_t bufferOffset,
                                        size_t bufferRowBytes) {
    SkASSERT(texture);
    SkASSERT(buffer);

    if (!this->onCopyTextureToBuffer(texture.get(), srcRect, buffer.get(), bufferOffset,
                                     bufferRowBytes)) {
        return false;
    }

    this->trackCommandBufferResource(std::move(texture));
    this->trackResource(std::move(buffer));

    SkDEBUGCODE(fHasWork = true;)

    return true;
}

bool CommandBuffer::copyBufferToTexture(const Buffer* buffer,
                                        sk_sp<Texture> texture,
                                        const BufferTextureCopyData* copyData,
                                        int count) {
    SkASSERT(buffer);
    SkASSERT(texture);
    SkASSERT(count > 0 && copyData);

    if (!this->onCopyBufferToTexture(buffer, texture.get(), copyData, count)) {
        return false;
    }

    this->trackCommandBufferResource(std::move(texture));

    SkDEBUGCODE(fHasWork = true;)

    return true;
}

bool CommandBuffer::copyTextureToTexture(sk_sp<Texture> src,
                                         SkIRect srcRect,
                                         sk_sp<Texture> dst,
                                         SkIPoint dstPoint,
                                         int mipLevel) {
    SkASSERT(src);
    SkASSERT(dst);
    if (src->textureInfo().isProtected() == Protected::kYes &&
        dst->textureInfo().isProtected() != Protected::kYes) {
        SKGPU_LOG_E("Can't copy from protected memory to non-protected");
        return false;
    }

    if (!this->onCopyTextureToTexture(src.get(), srcRect, dst.get(), dstPoint, mipLevel)) {
        return false;
    }

    this->trackCommandBufferResource(std::move(src));
    this->trackCommandBufferResource(std::move(dst));

    SkDEBUGCODE(fHasWork = true;)

    return true;
}

bool CommandBuffer::synchronizeBufferToCpu(sk_sp<Buffer> buffer) {
    SkASSERT(buffer);

    bool didResultInWork = false;
    if (!this->onSynchronizeBufferToCpu(buffer.get(), &didResultInWork)) {
        return false;
    }

    if (didResultInWork) {
        this->trackResource(std::move(buffer));
        SkDEBUGCODE(fHasWork = true;)
    }

    return true;
}

bool CommandBuffer::clearBuffer(const Buffer* buffer, size_t offset, size_t size) {
    SkASSERT(buffer);

    if (!this->onClearBuffer(buffer, offset, size)) {
        return false;
    }

    SkDEBUGCODE(fHasWork = true;)

    return true;
}

bool CommandBuffer::setReplayTranslationAndClip(const SkIVector& translation,
                                                const SkIRect& clip,
                                                const SkIRect& renderTargetBounds) {
    fReplayTranslation = translation;
    fRenderPassBounds = renderTargetBounds;

    // If a replay clip is defined, we intersect it with the render target bounds.
    if (!clip.isEmpty()) {
        if (!fRenderPassBounds.intersect(clip.makeOffset(translation))) {
            return false;
        }
    }

    return true;
}

} // namespace skgpu::graphite

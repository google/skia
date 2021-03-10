/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlGpu.h"

#include "include/private/GrTypesPriv.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrThreadSafePipelineBuilder.h"
#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlOpsRenderPass.h"
#include "src/gpu/mtl/GrMtlPipelineStateBuilder.h"
#include "src/gpu/mtl/GrMtlSemaphore.h"
#include "src/gpu/mtl/GrMtlTexture.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#import <simd/simd.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

static bool get_feature_set(id<MTLDevice> device, MTLFeatureSet* featureSet) {
    // Mac OSX
#ifdef SK_BUILD_FOR_MAC
    if (@available(macOS 10.12, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_OSX_GPUFamily1_v2]) {
            *featureSet = MTLFeatureSet_OSX_GPUFamily1_v2;
            return true;
        }
    }
    if ([device supportsFeatureSet:MTLFeatureSet_OSX_GPUFamily1_v1]) {
        *featureSet = MTLFeatureSet_OSX_GPUFamily1_v1;
        return true;
    }
#endif

    // iOS Family group 3
#ifdef SK_BUILD_FOR_IOS
    if (@available(iOS 10.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v2]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily3_v2;
            return true;
        }
    }
    if (@available(iOS 9.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily3_v1;
            return true;
        }
    }

    // iOS Family group 2
    if (@available(iOS 10.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v3]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily2_v3;
            return true;
        }
    }
    if (@available(iOS 9.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v2]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily2_v2;
            return true;
        }
    }
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily2_v1;
        return true;
    }

    // iOS Family group 1
    if (@available(iOS 10.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v3]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily1_v3;
            return true;
        }
    }
    if (@available(iOS 9.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v2]) {
            *featureSet = MTLFeatureSet_iOS_GPUFamily1_v2;
            return true;
        }
    }
    if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v1]) {
        *featureSet = MTLFeatureSet_iOS_GPUFamily1_v1;
        return true;
    }
#endif
    // No supported feature sets were found
    return false;
}

sk_sp<GrGpu> GrMtlGpu::Make(const GrMtlBackendContext& context, const GrContextOptions& options,
                            GrDirectContext* direct) {
    if (!context.fDevice || !context.fQueue) {
        return nullptr;
    }
    if (@available(macOS 10.14, iOS 10.0, *)) {
        // no warning needed
    } else {
        SkDebugf("*** Warning ***: this OS version is deprecated and will no longer be supported " \
                 "in future releases.\n");
#ifdef SK_BUILD_FOR_IOS
        SkDebugf("Minimum recommended version is iOS 10.0.\n");
#else
        SkDebugf("Minimum recommended version is MacOS 10.14.\n");
#endif
    }

    id<MTLDevice> device = (__bridge id<MTLDevice>)(context.fDevice.get());
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)(context.fQueue.get());
    MTLFeatureSet featureSet;
    if (!get_feature_set(device, &featureSet)) {
        return nullptr;
    }
    return sk_sp<GrGpu>(new GrMtlGpu(direct, options, device, queue, context.fBinaryArchive.get(),
                                     featureSet));
}

// This constant determines how many OutstandingCommandBuffers are allocated together as a block in
// the deque. As such it needs to balance allocating too much memory vs. incurring
// allocation/deallocation thrashing. It should roughly correspond to the max number of outstanding
// command buffers we expect to see.
static const int kDefaultOutstandingAllocCnt = 8;

GrMtlGpu::GrMtlGpu(GrDirectContext* direct, const GrContextOptions& options,
                   id<MTLDevice> device, id<MTLCommandQueue> queue, GrMTLHandle binaryArchive,
                   MTLFeatureSet featureSet)
        : INHERITED(direct)
        , fDevice(device)
        , fQueue(queue)
        , fOutstandingCommandBuffers(sizeof(OutstandingCommandBuffer), kDefaultOutstandingAllocCnt)
        , fResourceProvider(this)
        , fStagingBufferManager(this)
        , fDisconnected(false) {
    fMtlCaps.reset(new GrMtlCaps(options, fDevice, featureSet));
    this->initCapsAndCompiler(fMtlCaps);
    fCurrentCmdBuffer = GrMtlCommandBuffer::Make(fQueue);
#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, *)) {
        fBinaryArchive = (__bridge id<MTLBinaryArchive>)(binaryArchive);
    }
#endif
}

GrMtlGpu::~GrMtlGpu() {
    if (!fDisconnected) {
        this->destroyResources();
    }
}

void GrMtlGpu::disconnect(DisconnectType type) {
    INHERITED::disconnect(type);

    if (!fDisconnected) {
        this->destroyResources();
        fDisconnected = true;
    }
}

GrThreadSafePipelineBuilder* GrMtlGpu::pipelineBuilder() {
    return nullptr;
}

sk_sp<GrThreadSafePipelineBuilder> GrMtlGpu::refPipelineBuilder() {
    return nullptr;
}

void GrMtlGpu::destroyResources() {
    this->submitCommandBuffer(SyncQueue::kForce_SyncQueue);

    // We used a placement new for each object in fOutstandingCommandBuffers, so we're responsible
    // for calling the destructor on each of them as well.
    while (!fOutstandingCommandBuffers.empty()) {
        OutstandingCommandBuffer* buffer =
                (OutstandingCommandBuffer*)fOutstandingCommandBuffers.front();
        // make sure we remove before deleting as deletion might try to kick off another submit
        fOutstandingCommandBuffers.pop_front();
        buffer->~OutstandingCommandBuffer();
    }

    fStagingBufferManager.reset();

    fResourceProvider.destroyResources();

    fQueue = nil;
    fDevice = nil;
}

GrOpsRenderPass* GrMtlGpu::onGetOpsRenderPass(
            GrRenderTarget* renderTarget, GrAttachment*,
            GrSurfaceOrigin origin, const SkIRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
            const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
            const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
            GrXferBarrierFlags renderPassXferBarriers) {
    return new GrMtlOpsRenderPass(this, renderTarget, origin, colorInfo, stencilInfo);
}

GrMtlCommandBuffer* GrMtlGpu::commandBuffer() {
    if (!fCurrentCmdBuffer) {
        // Create a new command buffer for the next submit
        fCurrentCmdBuffer = GrMtlCommandBuffer::Make(fQueue);
    }

    SkASSERT(fCurrentCmdBuffer);
    return fCurrentCmdBuffer.get();
}

void GrMtlGpu::takeOwnershipOfBuffer(sk_sp<GrGpuBuffer> buffer) {
    SkASSERT(fCurrentCmdBuffer);
    fCurrentCmdBuffer->addGrBuffer(std::move(buffer));
}

void GrMtlGpu::submit(GrOpsRenderPass* renderPass) {
    GrMtlOpsRenderPass* mtlRenderPass = reinterpret_cast<GrMtlOpsRenderPass*>(renderPass);
    mtlRenderPass->submit();
    delete renderPass;
}

bool GrMtlGpu::submitCommandBuffer(SyncQueue sync) {
    if (!fCurrentCmdBuffer || !fCurrentCmdBuffer->hasWork()) {
        if (sync == SyncQueue::kForce_SyncQueue) {
            this->finishOutstandingGpuWork();
            this->checkForFinishedCommandBuffers();
        }
        // We need to manually call the finishedCallbacks since we don't add this
        // to the OutstandingCommandBuffer list
        if (fCurrentCmdBuffer) {
            fCurrentCmdBuffer->callFinishedCallbacks();
        }
        return true;
    }

    SkASSERT(fCurrentCmdBuffer);
    new (fOutstandingCommandBuffers.push_back()) OutstandingCommandBuffer(fCurrentCmdBuffer);

    if (!fCurrentCmdBuffer->commit(sync == SyncQueue::kForce_SyncQueue)) {
        return false;
    }

    // We don't create a new command buffer here because we may end up using it
    // in the next frame, and that confuses the GPU debugger. Instead we
    // create when we next need one.
    fCurrentCmdBuffer = nullptr;

    // If the freeing of any resources held by a finished command buffer causes us to send
    // a new command to the gpu (like changing the resource state) we'll create the new
    // command buffer in commandBuffer(), above.
    this->checkForFinishedCommandBuffers();

    return true;
}

void GrMtlGpu::checkForFinishedCommandBuffers() {
    // Iterate over all the outstanding command buffers to see if any have finished. The command
    // buffers are in order from oldest to newest, so we start at the front to check if their fence
    // has signaled. If so we pop it off and move onto the next.
    // Repeat till we find a command list that has not finished yet (and all others afterwards are
    // also guaranteed to not have finished).
    OutstandingCommandBuffer* front = (OutstandingCommandBuffer*)fOutstandingCommandBuffers.front();
    while (front && (*front)->isCompleted()) {
        // Make sure we remove before deleting as deletion might try to kick off another submit
        fOutstandingCommandBuffers.pop_front();
        // Since we used placement new we are responsible for calling the destructor manually.
        front->~OutstandingCommandBuffer();
        front = (OutstandingCommandBuffer*)fOutstandingCommandBuffers.front();
    }
}

void GrMtlGpu::finishOutstandingGpuWork() {
    // wait for the last command buffer we've submitted to finish
    OutstandingCommandBuffer* back =
            (OutstandingCommandBuffer*)fOutstandingCommandBuffers.back();
    if (back) {
        (*back)->waitUntilCompleted();
    }
}

void GrMtlGpu::addFinishedProc(GrGpuFinishedProc finishedProc,
                               GrGpuFinishedContext finishedContext) {
    SkASSERT(finishedProc);
    this->addFinishedCallback(GrRefCntedCallback::Make(finishedProc, finishedContext));
}

void GrMtlGpu::addFinishedCallback(sk_sp<GrRefCntedCallback> finishedCallback) {
    SkASSERT(finishedCallback);
    // Besides the current commandbuffer, we also add the finishedCallback to the newest outstanding
    // commandbuffer. Our contract for calling the proc is that all previous submitted cmdbuffers
    // have finished when we call it. However, if our current command buffer has no work when it is
    // flushed it will drop its ref to the callback immediately. But the previous work may not have
    // finished. It is safe to only add the proc to the newest outstanding commandbuffer cause that
    // must finish after all previously submitted command buffers.
    OutstandingCommandBuffer* back = (OutstandingCommandBuffer*)fOutstandingCommandBuffers.back();
    if (back) {
        (*back)->addFinishedCallback(finishedCallback);
    }
    commandBuffer()->addFinishedCallback(std::move(finishedCallback));
}

bool GrMtlGpu::onSubmitToGpu(bool syncCpu) {
    if (syncCpu) {
        return this->submitCommandBuffer(kForce_SyncQueue);
    } else {
        return this->submitCommandBuffer(kSkip_SyncQueue);
    }
}

std::unique_ptr<GrSemaphore> GrMtlGpu::prepareTextureForCrossContextUsage(GrTexture*) {
    this->submitToGpu(false);
    return nullptr;
}

sk_sp<GrGpuBuffer> GrMtlGpu::onCreateBuffer(size_t size, GrGpuBufferType type,
                                            GrAccessPattern accessPattern, const void* data) {
    return GrMtlBuffer::Make(this, size, type, accessPattern, data);
}

static bool check_max_blit_width(int widthInPixels) {
    if (widthInPixels > 32767) {
        SkASSERT(false); // surfaces should not be this wide anyway
        return false;
    }
    return true;
}

bool GrMtlGpu::uploadToTexture(GrMtlTexture* tex, int left, int top, int width, int height,
                               GrColorType dataColorType, const GrMipLevel texels[],
                               int mipLevelCount) {
    SkASSERT(this->caps()->isFormatTexturable(tex->backendFormat()));
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(1 == mipLevelCount ||
             (0 == left && 0 == top && width == tex->width() && height == tex->height()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
    SkASSERT(1 == mipLevelCount || mipLevelCount == (tex->maxMipmapLevel() + 1));

    if (!check_max_blit_width(width)) {
        return false;
    }
    if (width == 0 || height == 0) {
        return false;
    }

    SkASSERT(this->mtlCaps().surfaceSupportsWritePixels(tex));
    SkASSERT(this->mtlCaps().areColorTypeAndFormatCompatible(dataColorType, tex->backendFormat()));

    id<MTLTexture> mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);
    // Either upload only the first miplevel or all miplevels
    SkASSERT(1 == mipLevelCount || mipLevelCount == (int)mtlTexture.mipmapLevelCount);

    if (1 == mipLevelCount && !texels[0].fPixels) {
        return true;   // no data to upload
    }

    for (int i = 0; i < mipLevelCount; ++i) {
        // We do not allow any gaps in the mip data
        if (!texels[i].fPixels) {
            return false;
        }
    }

    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);

    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    size_t combinedBufferSize = GrComputeTightCombinedBufferSize(
            bpp, {width, height}, &individualMipOffsets, mipLevelCount);
    SkASSERT(combinedBufferSize);


    // offset value must be a multiple of the destination texture's pixel size in bytes
#ifdef SK_BUILD_FOR_MAC
    static const size_t kMinAlignment = 4;
#else
    static const size_t kMinAlignment = 1;
#endif
    size_t alignment = std::max(bpp, kMinAlignment);
    GrStagingBufferManager::Slice slice = fStagingBufferManager.allocateStagingBufferSlice(
            combinedBufferSize, alignment);
    if (!slice.fBuffer) {
        return false;
    }
    char* bufferData = (char*)slice.fOffsetMapPtr;
    GrMtlBuffer* mtlBuffer = static_cast<GrMtlBuffer*>(slice.fBuffer);

    int currentWidth = width;
    int currentHeight = height;
    int layerHeight = tex->height();
    MTLOrigin origin = MTLOriginMake(left, top, 0);

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (texels[currentMipLevel].fPixels) {
            SkASSERT(1 == mipLevelCount || currentHeight == layerHeight);
            const size_t trimRowBytes = currentWidth * bpp;
            const size_t rowBytes = texels[currentMipLevel].fRowBytes;

            // copy data into the buffer, skipping any trailing bytes
            char* dst = bufferData + individualMipOffsets[currentMipLevel];
            const char* src = (const char*)texels[currentMipLevel].fPixels;
            SkRectMemcpy(dst, trimRowBytes, src, rowBytes, trimRowBytes, currentHeight);

            [blitCmdEncoder copyFromBuffer: mtlBuffer->mtlBuffer()
                              sourceOffset: slice.fOffset + individualMipOffsets[currentMipLevel]
                         sourceBytesPerRow: trimRowBytes
                       sourceBytesPerImage: trimRowBytes*currentHeight
                                sourceSize: MTLSizeMake(currentWidth, currentHeight, 1)
                                 toTexture: mtlTexture
                          destinationSlice: 0
                          destinationLevel: currentMipLevel
                         destinationOrigin: origin];
        }
        currentWidth = std::max(1, currentWidth/2);
        currentHeight = std::max(1, currentHeight/2);
        layerHeight = currentHeight;
    }
#ifdef SK_BUILD_FOR_MAC
    [mtlBuffer->mtlBuffer() didModifyRange: NSMakeRange(slice.fOffset, combinedBufferSize)];
#endif

    if (mipLevelCount < (int) tex->mtlTexture().mipmapLevelCount) {
        tex->markMipmapsDirty();
    }

    return true;
}

bool GrMtlGpu::clearTexture(GrMtlTexture* tex, size_t bpp, uint32_t levelMask) {
    SkASSERT(this->mtlCaps().isFormatTexturable(tex->backendFormat()));

    if (!levelMask) {
        return true;
    }

    id<MTLTexture> mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);
    // Either upload only the first miplevel or all miplevels
    int mipLevelCount = (int)mtlTexture.mipmapLevelCount;

    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    size_t combinedBufferSize = 0;
    int currentWidth = tex->width();
    int currentHeight = tex->height();

    // The alignment must be at least 4 bytes and a multiple of the bytes per pixel of the image
    // config. This works with the assumption that the bytes in pixel config is always a power of 2.
    // TODO: can we just copy from a single buffer the size of the largest cleared level w/o a perf
    // penalty?
    SkASSERT((bpp & (bpp - 1)) == 0);
    const size_t alignmentMask = 0x3 | (bpp - 1);
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (levelMask & (1 << currentMipLevel)) {
            const size_t trimmedSize = currentWidth * bpp * currentHeight;
            const size_t alignmentDiff = combinedBufferSize & alignmentMask;
            if (alignmentDiff != 0) {
                combinedBufferSize += alignmentMask - alignmentDiff + 1;
            }
            individualMipOffsets.push_back(combinedBufferSize);
            combinedBufferSize += trimmedSize;
        }
        currentWidth = std::max(1, currentWidth/2);
        currentHeight = std::max(1, currentHeight/2);
    }
    SkASSERT(combinedBufferSize > 0 && !individualMipOffsets.empty());

    // TODO: Create GrMtlTransferBuffer
    NSUInteger options = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        options |= MTLResourceStorageModePrivate;
    }
    id<MTLBuffer> transferBuffer = [fDevice newBufferWithLength: combinedBufferSize
                                                        options: options];
    if (nil == transferBuffer) {
        return false;
    }

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    // clear the buffer to transparent black
    NSRange clearRange;
    clearRange.location = 0;
    clearRange.length = combinedBufferSize;
    [blitCmdEncoder fillBuffer: transferBuffer
                         range: clearRange
                         value: 0];

    // now copy buffer to texture
    currentWidth = tex->width();
    currentHeight = tex->height();
    MTLOrigin origin = MTLOriginMake(0, 0, 0);
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (levelMask & (1 << currentMipLevel)) {
            const size_t rowBytes = currentWidth * bpp;

            [blitCmdEncoder copyFromBuffer: transferBuffer
                              sourceOffset: individualMipOffsets[currentMipLevel]
                         sourceBytesPerRow: rowBytes
                       sourceBytesPerImage: rowBytes * currentHeight
                                sourceSize: MTLSizeMake(currentWidth, currentHeight, 1)
                                 toTexture: mtlTexture
                          destinationSlice: 0
                          destinationLevel: currentMipLevel
                         destinationOrigin: origin];
        }
        currentWidth = std::max(1, currentWidth/2);
        currentHeight = std::max(1, currentHeight/2);
    }

    if (mipLevelCount < (int) tex->mtlTexture().mipmapLevelCount) {
        tex->markMipmapsDirty();
    }

    return true;
}

sk_sp<GrAttachment> GrMtlGpu::makeStencilAttachmentForRenderTarget(
        const GrRenderTarget* rt, SkISize dimensions, int numStencilSamples) {
    SkASSERT(numStencilSamples == rt->numSamples());
    SkASSERT(dimensions.width() >= rt->width());
    SkASSERT(dimensions.height() >= rt->height());

    int samples = rt->numSamples();

    MTLPixelFormat sFmt = this->mtlCaps().preferredStencilFormat();

    fStats.incStencilAttachmentCreates();
    return GrMtlAttachment::GrMtlAttachment::MakeStencil(this, dimensions, samples, sFmt);
}

sk_sp<GrTexture> GrMtlGpu::onCreateTexture(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           GrRenderable renderable,
                                           int renderTargetSampleCnt,
                                           SkBudgeted budgeted,
                                           GrProtected isProtected,
                                           int mipLevelCount,
                                           uint32_t levelClearMask) {
    // We don't support protected textures in Metal.
    if (isProtected == GrProtected::kYes) {
        return nullptr;
    }
    SkASSERT(mipLevelCount > 0);

    MTLPixelFormat mtlPixelFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(mtlPixelFormat != MTLPixelFormatInvalid);
    SkASSERT(!this->caps()->isFormatCompressed(format));

    sk_sp<GrMtlTexture> tex;
    // This TexDesc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this TexDesc describes the resolved texture. Therefore we always have samples
    // set to 1.
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    texDesc.textureType = MTLTextureType2D;
    texDesc.pixelFormat = mtlPixelFormat;
    texDesc.width = dimensions.fWidth;
    texDesc.height = dimensions.fHeight;
    texDesc.depth = 1;
    texDesc.mipmapLevelCount = mipLevelCount;
    texDesc.sampleCount = 1;
    texDesc.arrayLength = 1;
    // Make all textures have private gpu only access. We can use transfer buffers or textures
    // to copy to them.
    if (@available(macOS 10.11, iOS 9.0, *)) {
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.usage = MTLTextureUsageShaderRead;
        texDesc.usage |= (renderable == GrRenderable::kYes) ? MTLTextureUsageRenderTarget : 0;
    }

    GrMipmapStatus mipmapStatus =
            mipLevelCount > 1 ? GrMipmapStatus::kDirty : GrMipmapStatus::kNotAllocated;
    if (renderable == GrRenderable::kYes) {
        tex = GrMtlTextureRenderTarget::MakeNewTextureRenderTarget(
                this, budgeted, dimensions, renderTargetSampleCnt, texDesc, mipmapStatus);
    } else {
        tex = GrMtlTexture::MakeNewTexture(this, budgeted, dimensions, texDesc, mipmapStatus);
    }

    if (!tex) {
        return nullptr;
    }

    if (levelClearMask) {
        this->clearTexture(tex.get(), GrMtlFormatBytesPerBlock(mtlPixelFormat), levelClearMask);
    }

    return std::move(tex);
}

sk_sp<GrTexture> GrMtlGpu::onCreateCompressedTexture(SkISize dimensions,
                                                     const GrBackendFormat& format,
                                                     SkBudgeted budgeted,
                                                     GrMipmapped mipMapped,
                                                     GrProtected isProtected,
                                                     const void* data, size_t dataSize) {
    // We don't support protected textures in Metal.
    if (isProtected == GrProtected::kYes) {
        return nullptr;
    }

    SkASSERT(this->caps()->isFormatTexturable(format));
    SkASSERT(data);

    if (!check_max_blit_width(dimensions.width())) {
        return nullptr;
    }

    MTLPixelFormat mtlPixelFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(this->caps()->isFormatCompressed(format));

    int numMipLevels = 1;
    if (mipMapped == GrMipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    // This TexDesc refers to the texture that will be read by the client. Thus even if msaa is
    // requested, this TexDesc describes the resolved texture. Therefore we always have samples
    // set to 1.
    // Compressed textures with MIP levels or multiple samples are not supported as of now.
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    texDesc.textureType = MTLTextureType2D;
    texDesc.pixelFormat = mtlPixelFormat;
    texDesc.width = dimensions.width();
    texDesc.height = dimensions.height();
    texDesc.depth = 1;
    texDesc.mipmapLevelCount = numMipLevels;
    texDesc.sampleCount = 1;
    texDesc.arrayLength = 1;
    // Make all textures have private gpu only access. We can use transfer buffers or textures
    // to copy to them.
    if (@available(macOS 10.11, iOS 9.0, *)) {
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.usage = MTLTextureUsageShaderRead;
    }

    GrMipmapStatus mipmapStatus = (mipMapped == GrMipmapped::kYes)
                                                                ? GrMipmapStatus::kValid
                                                                : GrMipmapStatus::kNotAllocated;

    auto tex = GrMtlTexture::MakeNewTexture(this, budgeted, dimensions, texDesc, mipmapStatus);
    if (!tex) {
        return nullptr;
    }

    // Upload to texture
    id<MTLTexture> mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);

    auto compressionType = GrBackendFormatToCompressionType(format);
    SkASSERT(compressionType != SkImage::CompressionType::kNone);

    SkTArray<size_t> individualMipOffsets(numMipLevels);
    SkDEBUGCODE(size_t combinedBufferSize =) SkCompressedDataSize(compressionType, dimensions,
                                                                  &individualMipOffsets,
                                                                  mipMapped == GrMipmapped::kYes);
    SkASSERT(individualMipOffsets.count() == numMipLevels);
    SkASSERT(dataSize == combinedBufferSize);

    // offset value must be a multiple of the destination texture's pixel size in bytes
    // for compressed textures, this is the block size
    size_t alignment = SkCompressedBlockSize(compressionType);
    GrStagingBufferManager::Slice slice = fStagingBufferManager.allocateStagingBufferSlice(
            dataSize, alignment);
    if (!slice.fBuffer) {
        return nullptr;
    }
    char* bufferData = (char*)slice.fOffsetMapPtr;
    GrMtlBuffer* mtlBuffer = static_cast<GrMtlBuffer*>(slice.fBuffer);

    MTLOrigin origin = MTLOriginMake(0, 0, 0);

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();

    // copy data into the buffer, skipping any trailing bytes
    memcpy(bufferData, data, dataSize);

    SkISize levelDimensions = dimensions;
    for (int currentMipLevel = 0; currentMipLevel < numMipLevels; currentMipLevel++) {
        const size_t levelRowBytes = GrCompressedRowBytes(compressionType, levelDimensions.width());
        size_t levelSize = SkCompressedDataSize(compressionType, levelDimensions, nullptr, false);

        // TODO: can this all be done in one go?
        [blitCmdEncoder copyFromBuffer: mtlBuffer->mtlBuffer()
                          sourceOffset: slice.fOffset + individualMipOffsets[currentMipLevel]
                     sourceBytesPerRow: levelRowBytes
                   sourceBytesPerImage: levelSize
                            sourceSize: MTLSizeMake(levelDimensions.width(),
                                                    levelDimensions.height(), 1)
                             toTexture: mtlTexture
                      destinationSlice: 0
                      destinationLevel: currentMipLevel
                     destinationOrigin: origin];

        levelDimensions = {std::max(1, levelDimensions.width() /2),
                           std::max(1, levelDimensions.height()/2)};
    }
#ifdef SK_BUILD_FOR_MAC
    [mtlBuffer->mtlBuffer() didModifyRange: NSMakeRange(slice.fOffset, dataSize)];
#endif

    return std::move(tex);
}

static id<MTLTexture> get_texture_from_backend(const GrBackendTexture& backendTex) {
    GrMtlTextureInfo textureInfo;
    if (!backendTex.getMtlTextureInfo(&textureInfo)) {
        return nil;
    }
    return GrGetMTLTexture(textureInfo.fTexture.get());
}

static id<MTLTexture> get_texture_from_backend(const GrBackendRenderTarget& backendRT) {
    GrMtlTextureInfo textureInfo;
    if (!backendRT.getMtlTextureInfo(&textureInfo)) {
        return nil;
    }
    return GrGetMTLTexture(textureInfo.fTexture.get());
}

sk_sp<GrTexture> GrMtlGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                                GrWrapOwnership,
                                                GrWrapCacheable cacheable,
                                                GrIOType ioType) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex);
    if (!mtlTexture) {
        return nullptr;
    }
    // We don't currently support sampling from a MSAA texture in shaders.
    if (mtlTexture.sampleCount != 1) {
        return nullptr;
    }

    return GrMtlTexture::MakeWrappedTexture(this, backendTex.dimensions(), mtlTexture, cacheable,
                                            ioType);
}

sk_sp<GrTexture> GrMtlGpu::onWrapCompressedBackendTexture(const GrBackendTexture& backendTex,
                                                          GrWrapOwnership,
                                                          GrWrapCacheable cacheable) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex);
    if (!mtlTexture) {
        return nullptr;
    }
    // We don't currently support sampling from a MSAA texture in shaders.
    if (mtlTexture.sampleCount != 1) {
        return nullptr;
    }

    return GrMtlTexture::MakeWrappedTexture(this, backendTex.dimensions(), mtlTexture, cacheable,
                                            kRead_GrIOType);
}

sk_sp<GrTexture> GrMtlGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                          int sampleCnt,
                                                          GrWrapOwnership,
                                                          GrWrapCacheable cacheable) {
    id<MTLTexture> mtlTexture = get_texture_from_backend(backendTex);
    if (!mtlTexture) {
        return nullptr;
    }
    // We don't currently support sampling from a MSAA texture in shaders.
    if (mtlTexture.sampleCount != 1) {
        return nullptr;
    }

    const GrMtlCaps& caps = this->mtlCaps();

    MTLPixelFormat format = mtlTexture.pixelFormat;
    if (!caps.isFormatRenderable(format, sampleCnt)) {
        return nullptr;
    }

    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(MTLTextureUsageRenderTarget & mtlTexture.usage);
    }

    sampleCnt = caps.getRenderTargetSampleCount(sampleCnt, format);
    SkASSERT(sampleCnt);

    return GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
            this, backendTex.dimensions(), sampleCnt, mtlTexture, cacheable);
}

sk_sp<GrRenderTarget> GrMtlGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT) {
    if (!this->caps()->isFormatRenderable(backendRT.getBackendFormat(), backendRT.sampleCnt())) {
        return nullptr;
    }

    id<MTLTexture> mtlTexture = get_texture_from_backend(backendRT);
    if (!mtlTexture) {
        return nullptr;
    }

    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(MTLTextureUsageRenderTarget & mtlTexture.usage);
    }

    return GrMtlRenderTarget::MakeWrappedRenderTarget(this, backendRT.dimensions(),
                                                      backendRT.sampleCnt(), mtlTexture);
}

bool GrMtlGpu::onRegenerateMipMapLevels(GrTexture* texture) {
    GrMtlTexture* grMtlTexture = static_cast<GrMtlTexture*>(texture);
    id<MTLTexture> mtlTexture = grMtlTexture->mtlTexture();

    // Automatic mipmap generation is only supported by color-renderable formats
    if (!fMtlCaps->isFormatRenderable(mtlTexture.pixelFormat, 1) &&
        // We have pixel configs marked as textureable-only that use RGBA8 as the internal format
        MTLPixelFormatRGBA8Unorm != mtlTexture.pixelFormat) {
        return false;
    }

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder generateMipmapsForTexture: mtlTexture];

    return true;
}

// Used to "clear" a backend texture to a constant color by transferring.
static GrColorType mtl_format_to_backend_tex_clear_colortype(MTLPixelFormat format) {
    switch(format) {
        case MTLPixelFormatA8Unorm:         return GrColorType::kAlpha_8;
        case MTLPixelFormatR8Unorm:         return GrColorType::kR_8;

#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatB5G6R5Unorm:     return GrColorType::kBGR_565;
        case MTLPixelFormatABGR4Unorm:      return GrColorType::kABGR_4444;
#endif
        case MTLPixelFormatRGBA8Unorm:      return GrColorType::kRGBA_8888;
        case MTLPixelFormatRGBA8Unorm_sRGB: return GrColorType::kRGBA_8888_SRGB;

        case MTLPixelFormatRG8Unorm:        return GrColorType::kRG_88;
        case MTLPixelFormatBGRA8Unorm:      return GrColorType::kBGRA_8888;
        case MTLPixelFormatRGB10A2Unorm:    return GrColorType::kRGBA_1010102;
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBGR10A2Unorm:    return GrColorType::kBGRA_1010102;
#endif
        case MTLPixelFormatR16Float:        return GrColorType::kR_F16;
        case MTLPixelFormatRGBA16Float:     return GrColorType::kRGBA_F16;
        case MTLPixelFormatR16Unorm:        return GrColorType::kR_16;
        case MTLPixelFormatRG16Unorm:       return GrColorType::kRG_1616;
        case MTLPixelFormatRGBA16Unorm:     return GrColorType::kRGBA_16161616;
        case MTLPixelFormatRG16Float:       return GrColorType::kRG_F16;
        default:                            return GrColorType::kUnknown;
    }

    SkUNREACHABLE;
}

void copy_src_data(char* dst,
                   size_t bytesPerPixel,
                   const SkTArray<size_t>& individualMipOffsets,
                   const GrPixmap srcData[],
                   int numMipLevels,
                   size_t bufferSize) {
    SkASSERT(srcData && numMipLevels);
    SkASSERT(individualMipOffsets.count() == numMipLevels);

    for (int level = 0; level < numMipLevels; ++level) {
        const size_t trimRB = srcData[level].width() * bytesPerPixel;
        SkASSERT(individualMipOffsets[level] + trimRB * srcData[level].height() <= bufferSize);
        SkRectMemcpy(dst + individualMipOffsets[level], trimRB,
                     srcData[level].addr(), srcData[level].rowBytes(),
                     trimRB, srcData[level].height());
    }
}

bool GrMtlGpu::createMtlTextureForBackendSurface(MTLPixelFormat mtlFormat,
                                                 SkISize dimensions,
                                                 int sampleCnt,
                                                 GrTexturable texturable,
                                                 GrRenderable renderable,
                                                 GrMipmapped mipMapped,
                                                 GrMtlTextureInfo* info) {
    SkASSERT(texturable == GrTexturable::kYes || renderable == GrRenderable::kYes);

    if (texturable == GrTexturable::kYes && !fMtlCaps->isFormatTexturable(mtlFormat)) {
        return false;
    }
    if (renderable == GrRenderable::kYes && !fMtlCaps->isFormatRenderable(mtlFormat, 1)) {
        return false;
    }

    if (!check_max_blit_width(dimensions.width())) {
        return false;
    }

    auto desc = [[MTLTextureDescriptor alloc] init];
    desc.pixelFormat = mtlFormat;
    desc.width = dimensions.width();
    desc.height = dimensions.height();
    if (mipMapped == GrMipMapped::kYes) {
        desc.mipmapLevelCount = 1 + SkPrevLog2(std::max(dimensions.width(), dimensions.height()));
    }
    if (@available(macOS 10.11, iOS 9.0, *)) {
        desc.storageMode = MTLStorageModePrivate;
        MTLTextureUsage usage = texturable == GrTexturable::kYes ? MTLTextureUsageShaderRead : 0;
        usage |= renderable == GrRenderable::kYes ? MTLTextureUsageRenderTarget : 0;
        desc.usage = usage;
    }
    if (sampleCnt != 1) {
        desc.sampleCount = sampleCnt;
        desc.textureType = MTLTextureType2DMultisample;
    }
    id<MTLTexture> testTexture = [fDevice newTextureWithDescriptor: desc];
    info->fTexture.reset(GrRetainPtrFromId(testTexture));
    return true;
}

GrBackendTexture GrMtlGpu::onCreateBackendTexture(SkISize dimensions,
                                                  const GrBackendFormat& format,
                                                  GrRenderable renderable,
                                                  GrMipmapped mipMapped,
                                                  GrProtected isProtected) {
    const MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);

    GrMtlTextureInfo info;
    if (!this->createMtlTextureForBackendSurface(mtlFormat, dimensions, 1, GrTexturable::kYes,
                                                 renderable, mipMapped, &info)) {
        return {};
    }

    GrBackendTexture backendTex(dimensions.width(), dimensions.height(), mipMapped, info);
    return backendTex;
}

bool GrMtlGpu::onUpdateBackendTexture(const GrBackendTexture& backendTexture,
                                      sk_sp<GrRefCntedCallback> finishedCallback,
                                      const BackendTextureData* data) {
    GrMtlTextureInfo info;
    SkAssertResult(backendTexture.getMtlTextureInfo(&info));

    id<MTLTexture> mtlTexture = GrGetMTLTexture(info.fTexture.get());

    const MTLPixelFormat mtlFormat = mtlTexture.pixelFormat;

    int numMipLevels = mtlTexture.mipmapLevelCount;
    GrMipmapped mipMapped = numMipLevels > 1 ? GrMipmapped::kYes : GrMipmapped::kNo;

    SkImage::CompressionType compression = GrBackendFormatToCompressionType(
            backendTexture.getBackendFormat());

    // Create a transfer buffer and fill with data.
    size_t bytesPerPixel = GrMtlFormatBytesPerBlock(mtlFormat);
    SkSTArray<16, size_t> individualMipOffsets;
    size_t combinedBufferSize;

    if (data->type() == BackendTextureData::Type::kColor &&
        compression == SkImage::CompressionType::kNone) {
        combinedBufferSize = bytesPerPixel*backendTexture.width()*backendTexture.height();
        // Reuse the same buffer for all levels. Should be ok since we made the row bytes tight.
        individualMipOffsets.push_back_n(numMipLevels, (size_t)0);
    } else if (compression == SkImage::CompressionType::kNone) {
        combinedBufferSize = GrComputeTightCombinedBufferSize(bytesPerPixel,
                                                              backendTexture.dimensions(),
                                                              &individualMipOffsets,
                                                              numMipLevels);
    } else {
        combinedBufferSize = SkCompressedDataSize(compression, backendTexture.dimensions(),
                                                  &individualMipOffsets,
                                                  mipMapped == GrMipmapped::kYes);
    }
    SkASSERT(individualMipOffsets.count() == numMipLevels);

#ifdef SK_BUILD_FOR_MAC
    static const size_t kMinAlignment = 4;
#else
    static const size_t kMinAlignment = 1;
#endif
    size_t alignment;
    if (data->type() == BackendTextureData::Type::kCompressed) {
        alignment = std::max(SkCompressedBlockSize(compression), kMinAlignment);
    } else {
        alignment = std::max(bytesPerPixel, kMinAlignment);
    }
    GrStagingBufferManager::Slice slice = fStagingBufferManager.allocateStagingBufferSlice(
            combinedBufferSize, alignment);
    if (!slice.fBuffer) {
        return false;
    }
    char* buffer = (char*)slice.fOffsetMapPtr;

    if (data->type() == BackendTextureData::Type::kPixmaps) {
        copy_src_data(buffer, bytesPerPixel, individualMipOffsets, data->pixmaps(),
                      numMipLevels, combinedBufferSize);
    } else if (data->type() == BackendTextureData::Type::kCompressed) {
        memcpy(buffer, data->compressedData(), data->compressedSize());
    } else {
        SkASSERT(data->type() == BackendTextureData::Type::kColor);
        if (compression == SkImage::CompressionType::kNone) {
            auto colorType = mtl_format_to_backend_tex_clear_colortype(mtlFormat);
            if (colorType == GrColorType::kUnknown) {
                return false;
            }
            GrImageInfo ii(colorType, kUnpremul_SkAlphaType, nullptr, backendTexture.dimensions());
            auto rb = ii.minRowBytes();
            SkASSERT(rb == bytesPerPixel*backendTexture.width());
            if (!GrClearImage(ii, buffer, rb, data->color())) {
                return false;
            }
        } else {
            GrFillInCompressedData(compression, backendTexture.dimensions(), mipMapped, buffer,
                                   data->color());
        }
    }

    // Transfer buffer contents to texture
    MTLOrigin origin = MTLOriginMake(0, 0, 0);

    GrMtlCommandBuffer* cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    GrMtlBuffer* mtlBuffer = static_cast<GrMtlBuffer*>(slice.fBuffer);

    SkISize levelDimensions(backendTexture.dimensions());
    for (int currentMipLevel = 0; currentMipLevel < numMipLevels; currentMipLevel++) {
        size_t levelRowBytes;
        size_t levelSize;

        if (compression == SkImage::CompressionType::kNone) {
            levelRowBytes = levelDimensions.width() * bytesPerPixel;
            levelSize = levelRowBytes * levelDimensions.height();
        } else {
            levelRowBytes = GrCompressedRowBytes(compression, levelDimensions.width());
            levelSize = SkCompressedDataSize(compression, levelDimensions, nullptr, false);
        }

        // TODO: can this all be done in one go?
        [blitCmdEncoder copyFromBuffer: mtlBuffer->mtlBuffer()
                          sourceOffset: slice.fOffset + individualMipOffsets[currentMipLevel]
                     sourceBytesPerRow: levelRowBytes
                   sourceBytesPerImage: levelSize
                            sourceSize: MTLSizeMake(levelDimensions.width(),
                                                    levelDimensions.height(), 1)
                             toTexture: mtlTexture
                      destinationSlice: 0
                      destinationLevel: currentMipLevel
                     destinationOrigin: origin];

        levelDimensions = { std::max(1, levelDimensions.width() / 2),
                            std::max(1, levelDimensions.height() / 2) };
    }
#ifdef SK_BUILD_FOR_MAC
    [mtlBuffer->mtlBuffer() didModifyRange: NSMakeRange(slice.fOffset, combinedBufferSize)];
#endif

    if (finishedCallback) {
        this->addFinishedCallback(std::move(finishedCallback));
    }

    return true;
}

GrBackendTexture GrMtlGpu::onCreateCompressedBackendTexture(
        SkISize dimensions, const GrBackendFormat& format, GrMipmapped mipMapped,
        GrProtected isProtected) {
    const MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);

    GrMtlTextureInfo info;
    if (!this->createMtlTextureForBackendSurface(mtlFormat, dimensions, 1, GrTexturable::kYes,
                                                 GrRenderable::kNo, mipMapped, &info)) {
        return {};
    }

    return GrBackendTexture(dimensions.width(), dimensions.height(), mipMapped, info);
}

bool GrMtlGpu::onUpdateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                                sk_sp<GrRefCntedCallback> finishedCallback,
                                                const BackendTextureData* data) {
    return this->onUpdateBackendTexture(backendTexture, std::move(finishedCallback), data);
}

void GrMtlGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kMetal == tex.backend());
    // Nothing to do here, will get cleaned up when the GrBackendTexture object goes away
}

bool GrMtlGpu::compile(const GrProgramDesc& desc, const GrProgramInfo& programInfo) {

    GrThreadSafePipelineBuilder::Stats::ProgramCacheResult stat;

    auto pipelineState = this->resourceProvider().findOrCreateCompatiblePipelineState(
                                 desc, programInfo, &stat);
    if (!pipelineState) {
        return false;
    }

    return stat != GrThreadSafePipelineBuilder::Stats::ProgramCacheResult::kHit;
}

bool GrMtlGpu::precompileShader(const SkData& key, const SkData& data) {
    return this->resourceProvider().precompileShader(key, data);
}

#if GR_TEST_UTILS
bool GrMtlGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(GrBackendApi::kMetal == tex.backend());

    GrMtlTextureInfo info;
    if (!tex.getMtlTextureInfo(&info)) {
        return false;
    }
    id<MTLTexture> mtlTexture = GrGetMTLTexture(info.fTexture.get());
    if (!mtlTexture) {
        return false;
    }
    if (@available(macOS 10.11, iOS 9.0, *)) {
        return mtlTexture.usage & MTLTextureUsageShaderRead;
    } else {
        return true; // best we can do
    }
}

GrBackendRenderTarget GrMtlGpu::createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                                     GrColorType ct,
                                                                     int sampleCnt,
                                                                     GrProtected isProtected) {
    if (dimensions.width()  > this->caps()->maxRenderTargetSize() ||
        dimensions.height() > this->caps()->maxRenderTargetSize()) {
        return {};
    }
    if (isProtected == GrProtected::kYes) {
        return {};
    }

    MTLPixelFormat format = this->mtlCaps().getFormatFromColorType(ct);
    sampleCnt = this->mtlCaps().getRenderTargetSampleCount(sampleCnt, format);
    if (sampleCnt == 0) {
        return {};
    }

    GrMtlTextureInfo info;
    if (!this->createMtlTextureForBackendSurface(format, dimensions, sampleCnt, GrTexturable::kNo,
                                                 GrRenderable::kYes, GrMipmapped::kNo, &info)) {
        return {};
    }

    return GrBackendRenderTarget(dimensions.width(), dimensions.height(), info);
}

void GrMtlGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    SkASSERT(GrBackendApi::kMetal == rt.backend());

    GrMtlTextureInfo info;
    if (rt.getMtlTextureInfo(&info)) {
        this->submitToGpu(true);
        // Nothing else to do here, will get cleaned up when the GrBackendRenderTarget
        // is deleted.
    }
}
#endif // GR_TEST_UTILS

void GrMtlGpu::copySurfaceAsResolve(GrSurface* dst, GrSurface* src) {
    // TODO: Add support for subrectangles
    GrMtlRenderTarget* srcRT = static_cast<GrMtlRenderTarget*>(src->asRenderTarget());
    GrRenderTarget* dstRT = dst->asRenderTarget();
    id<MTLTexture> dstTexture;
    if (dstRT) {
        GrMtlRenderTarget* mtlRT = static_cast<GrMtlRenderTarget*>(dstRT);
        dstTexture = mtlRT->mtlColorTexture();
    } else {
        SkASSERT(dst->asTexture());
        dstTexture = static_cast<GrMtlTexture*>(dst->asTexture())->mtlTexture();
    }

    this->resolveTexture(dstTexture, srcRT->mtlColorTexture());
}

void GrMtlGpu::copySurfaceAsBlit(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                 const SkIPoint& dstPoint) {
#ifdef SK_DEBUG
    SkASSERT(this->mtlCaps().canCopyAsBlit(dst, src, srcRect, dstPoint));
#endif
    id<MTLTexture> dstTex = GrGetMTLTextureFromSurface(dst);
    id<MTLTexture> srcTex = GrGetMTLTextureFromSurface(src);

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder copyFromTexture: srcTex
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(srcRect.x(), srcRect.y(), 0)
                         sourceSize: MTLSizeMake(srcRect.width(), srcRect.height(), 1)
                          toTexture: dstTex
                   destinationSlice: 0
                   destinationLevel: 0
                  destinationOrigin: MTLOriginMake(dstPoint.fX, dstPoint.fY, 0)];
}

bool GrMtlGpu::onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                             const SkIPoint& dstPoint) {
    SkASSERT(!src->isProtected() && !dst->isProtected());

    bool success = false;
    if (this->mtlCaps().canCopyAsBlit(dst, src, srcRect, dstPoint)) {
       this->copySurfaceAsBlit(dst, src, srcRect, dstPoint);
       success = true;
    } else if (this->mtlCaps().canCopyAsResolve(dst, src, srcRect, dstPoint)) {
        this->copySurfaceAsResolve(dst, src);
        success = true;
    }
    if (success) {
        SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.x(), dstPoint.y(),
                                            srcRect.width(), srcRect.height());
        // The rect is already in device space so we pass in kTopLeft so no flip is done.
        this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
    }
    return success;
}

bool GrMtlGpu::onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                             GrColorType surfaceColorType, GrColorType srcColorType,
                             const GrMipLevel texels[], int mipLevelCount,
                             bool prepForTexSampling) {
    GrMtlTexture* mtlTexture = static_cast<GrMtlTexture*>(surface->asTexture());
    // TODO: In principle we should be able to support pure rendertargets as well, but
    // until we find a use case we'll only support texture rendertargets.
    if (!mtlTexture) {
        return false;
    }
    if (!mipLevelCount) {
        return false;
    }
#ifdef SK_DEBUG
    for (int i = 0; i < mipLevelCount; i++) {
        SkASSERT(texels[i].fPixels);
    }
#endif
    return this->uploadToTexture(mtlTexture, left, top, width, height, srcColorType, texels,
                                 mipLevelCount);
}

bool GrMtlGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                            GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                            size_t rowBytes) {
    SkASSERT(surface);

    if (surfaceColorType != dstColorType) {
        return false;
    }

    int bpp = GrColorTypeBytesPerPixel(dstColorType);
    size_t transBufferRowBytes = bpp * width;
    size_t transBufferImageBytes = transBufferRowBytes * height;

    // TODO: implement some way of reusing buffers instead of making a new one every time.
    NSUInteger options = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
#ifdef SK_BUILD_FOR_MAC
        options |= MTLResourceStorageModeManaged;
#else
        options |= MTLResourceStorageModeShared;
#endif
    }

    id<MTLBuffer> transferBuffer = [fDevice newBufferWithLength: transBufferImageBytes
                                                        options: options];

    if (!this->readOrTransferPixels(surface, left, top, width, height, dstColorType, transferBuffer,
                                    0, transBufferImageBytes, transBufferRowBytes)) {
        return false;
    }
    this->submitCommandBuffer(kForce_SyncQueue);

    const void* mappedMemory = transferBuffer.contents;

    SkRectMemcpy(buffer, rowBytes, mappedMemory, transBufferRowBytes, transBufferRowBytes, height);

    return true;
}

bool GrMtlGpu::onTransferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                                  GrColorType textureColorType, GrColorType bufferColorType,
                                  sk_sp<GrGpuBuffer> transferBuffer, size_t offset,
                                  size_t rowBytes) {
    SkASSERT(texture);
    SkASSERT(transferBuffer);
    if (textureColorType != bufferColorType) {
        return false;
    }

    GrMtlTexture* grMtlTexture = static_cast<GrMtlTexture*>(texture);
    id<MTLTexture> mtlTexture = grMtlTexture->mtlTexture();
    SkASSERT(mtlTexture);

    GrMtlBuffer* grMtlBuffer = static_cast<GrMtlBuffer*>(transferBuffer.get());
    id<MTLBuffer> mtlBuffer = grMtlBuffer->mtlBuffer();
    SkASSERT(mtlBuffer);

    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    if (offset % bpp) {
        return false;
    }
    if (GrBackendFormatBytesPerPixel(texture->backendFormat()) != bpp) {
        return false;
    }

    MTLOrigin origin = MTLOriginMake(left, top, 0);

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder copyFromBuffer: mtlBuffer
                      sourceOffset: offset + grMtlBuffer->offset()
                 sourceBytesPerRow: rowBytes
               sourceBytesPerImage: rowBytes*height
                        sourceSize: MTLSizeMake(width, height, 1)
                         toTexture: mtlTexture
                  destinationSlice: 0
                  destinationLevel: 0
                 destinationOrigin: origin];

    return true;
}

bool GrMtlGpu::onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                                    GrColorType surfaceColorType, GrColorType bufferColorType,
                                    sk_sp<GrGpuBuffer> transferBuffer, size_t offset) {
    SkASSERT(surface);
    SkASSERT(transferBuffer);

    if (surfaceColorType != bufferColorType) {
        return false;
    }

    // Metal only supports offsets that are aligned to a pixel.
    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    if (offset % bpp) {
        return false;
    }
    if (GrBackendFormatBytesPerPixel(surface->backendFormat()) != bpp) {
        return false;
    }

    GrMtlBuffer* grMtlBuffer = static_cast<GrMtlBuffer*>(transferBuffer.get());

    size_t transBufferRowBytes = bpp * width;
    size_t transBufferImageBytes = transBufferRowBytes * height;

    return this->readOrTransferPixels(surface, left, top, width, height, bufferColorType,
                                      grMtlBuffer->mtlBuffer(), offset + grMtlBuffer->offset(),
                                      transBufferImageBytes, transBufferRowBytes);
}

bool GrMtlGpu::readOrTransferPixels(GrSurface* surface, int left, int top, int width, int height,
                                    GrColorType dstColorType, id<MTLBuffer> transferBuffer,
                                    size_t offset, size_t imageBytes, size_t rowBytes) {
    if (!check_max_blit_width(width)) {
        return false;
    }

    id<MTLTexture> mtlTexture;
    if (GrMtlRenderTarget* rt = static_cast<GrMtlRenderTarget*>(surface->asRenderTarget())) {
        if (rt->numSamples() > 1) {
            SkASSERT(rt->requiresManualMSAAResolve());  // msaa-render-to-texture not yet supported.
            mtlTexture = rt->mtlResolveTexture();
        } else {
            SkASSERT(!rt->requiresManualMSAAResolve());
            mtlTexture = rt->mtlColorTexture();
        }
    } else if (GrMtlTexture* texture = static_cast<GrMtlTexture*>(surface->asTexture())) {
        mtlTexture = texture->mtlTexture();
    }
    if (!mtlTexture) {
        return false;
    }

    id<MTLBlitCommandEncoder> blitCmdEncoder = this->commandBuffer()->getBlitCommandEncoder();
    [blitCmdEncoder copyFromTexture: mtlTexture
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(left, top, 0)
                         sourceSize: MTLSizeMake(width, height, 1)
                           toBuffer: transferBuffer
                  destinationOffset: offset
             destinationBytesPerRow: rowBytes
           destinationBytesPerImage: imageBytes];
#ifdef SK_BUILD_FOR_MAC
    // Sync GPU data back to the CPU
    [blitCmdEncoder synchronizeResource: transferBuffer];
#endif

    return true;
}

GrFence SK_WARN_UNUSED_RESULT GrMtlGpu::insertFence() {
    GrMtlCommandBuffer* cmdBuffer = this->commandBuffer();
    // We create a semaphore and signal it within the current
    // command buffer's completion handler.
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    cmdBuffer->addCompletedHandler(^(id <MTLCommandBuffer>commandBuffer) {
        dispatch_semaphore_signal(semaphore);
    });

    const void* cfFence = (__bridge_retained const void*) semaphore;
    return (GrFence) cfFence;
}

bool GrMtlGpu::waitFence(GrFence fence) {
    const void* cfFence = (const void*) fence;
    dispatch_semaphore_t semaphore = (__bridge dispatch_semaphore_t)cfFence;

    long result = dispatch_semaphore_wait(semaphore, 0);

    return !result;
}

void GrMtlGpu::deleteFence(GrFence fence) const {
    const void* cfFence = (const void*) fence;
    // In this case it's easier to release in CoreFoundation than depend on ARC
    CFRelease(cfFence);
}

std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT GrMtlGpu::makeSemaphore(bool /*isOwned*/) {
    SkASSERT(this->caps()->semaphoreSupport());
    return GrMtlSemaphore::Make(this);
}

std::unique_ptr<GrSemaphore> GrMtlGpu::wrapBackendSemaphore(
        const GrBackendSemaphore& semaphore,
        GrResourceProvider::SemaphoreWrapType wrapType,
        GrWrapOwnership /*ownership*/) {
    SkASSERT(this->caps()->semaphoreSupport());
    return GrMtlSemaphore::MakeWrapped(semaphore.mtlSemaphore(), semaphore.mtlValue());
}

void GrMtlGpu::insertSemaphore(GrSemaphore* semaphore) {
    if (@available(macOS 10.14, iOS 12.0, *)) {
        SkASSERT(semaphore);
        GrMtlSemaphore* mtlSem = static_cast<GrMtlSemaphore*>(semaphore);

        this->commandBuffer()->encodeSignalEvent(mtlSem->event(), mtlSem->value());
    }
}

void GrMtlGpu::waitSemaphore(GrSemaphore* semaphore) {
    if (@available(macOS 10.14, iOS 12.0, *)) {
        SkASSERT(semaphore);
        GrMtlSemaphore* mtlSem = static_cast<GrMtlSemaphore*>(semaphore);

        this->commandBuffer()->encodeWaitForEvent(mtlSem->event(), mtlSem->value());
    }
}

void GrMtlGpu::onResolveRenderTarget(GrRenderTarget* target, const SkIRect&) {
    this->resolveTexture(static_cast<GrMtlRenderTarget*>(target)->mtlResolveTexture(),
                         static_cast<GrMtlRenderTarget*>(target)->mtlColorTexture());
}

void GrMtlGpu::resolveTexture(id<MTLTexture> resolveTexture, id<MTLTexture> colorTexture) {
    auto renderPassDesc = [[MTLRenderPassDescriptor alloc] init];
    auto colorAttachment = renderPassDesc.colorAttachments[0];
    colorAttachment.texture = colorTexture;
    colorAttachment.resolveTexture = resolveTexture;
    colorAttachment.loadAction = MTLLoadActionLoad;
    colorAttachment.storeAction = MTLStoreActionMultisampleResolve;

    id<MTLRenderCommandEncoder> cmdEncoder =
            this->commandBuffer()->getRenderCommandEncoder(renderPassDesc, nullptr, nullptr);
    SkASSERT(nil != cmdEncoder);
    cmdEncoder.label = @"resolveTexture";
}

#if GR_TEST_UTILS
void GrMtlGpu::testingOnly_startCapture() {
    if (@available(macOS 10.13, iOS 11.0, *)) {
        // TODO: add Metal 3 interface as well
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        [captureManager startCaptureWithDevice: fDevice];
    }
}

void GrMtlGpu::testingOnly_endCapture() {
    if (@available(macOS 10.13, iOS 11.0, *)) {
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        [captureManager stopCapture];
    }
}
#endif

#ifdef SK_ENABLE_DUMP_GPU
#include "src/utils/SkJSONWriter.h"
void GrMtlGpu::onDumpJSON(SkJSONWriter* writer) const {
    // We are called by the base class, which has already called beginObject(). We choose to nest
    // all of our caps information in a named sub-object.
    writer->beginObject("Metal GPU");

    writer->beginObject("Device");
    writer->appendString("name", fDevice.name.UTF8String);
#ifdef SK_BUILD_FOR_MAC
    if (@available(macOS 10.11, *)) {
        writer->appendBool("isHeadless", fDevice.isHeadless);
        writer->appendBool("isLowPower", fDevice.isLowPower);
    }
    if (@available(macOS 10.13, *)) {
        writer->appendBool("isRemovable", fDevice.isRemovable);
    }
#endif
    if (@available(macOS 10.13, iOS 11.0, *)) {
        writer->appendU64("registryID", fDevice.registryID);
    }
#if defined(SK_BUILD_FOR_MAC) && __MAC_OS_X_VERSION_MAX_ALLOWED >= 101500
    if (@available(macOS 10.15, *)) {
        switch (fDevice.location) {
            case MTLDeviceLocationBuiltIn:
                writer->appendString("location", "builtIn");
                break;
            case MTLDeviceLocationSlot:
                writer->appendString("location", "slot");
                break;
            case MTLDeviceLocationExternal:
                writer->appendString("location", "external");
                break;
            case MTLDeviceLocationUnspecified:
                writer->appendString("location", "unspecified");
                break;
            default:
                writer->appendString("location", "unknown");
                break;
        }
        writer->appendU64("locationNumber", fDevice.locationNumber);
        writer->appendU64("maxTransferRate", fDevice.maxTransferRate);
    }
#endif  // SK_BUILD_FOR_MAC
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101500 || __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
    if (@available(macOS 10.15, iOS 13.0, *)) {
        writer->appendBool("hasUnifiedMemory", fDevice.hasUnifiedMemory);
    }
#endif
#ifdef SK_BUILD_FOR_MAC
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101500
    if (@available(macOS 10.15, *)) {
        writer->appendU64("peerGroupID", fDevice.peerGroupID);
        writer->appendU32("peerCount", fDevice.peerCount);
        writer->appendU32("peerIndex", fDevice.peerIndex);
    }
#endif
    if (@available(macOS 10.12, *)) {
        writer->appendU64("recommendedMaxWorkingSetSize", fDevice.recommendedMaxWorkingSetSize);
    }
#endif  // SK_BUILD_FOR_MAC
    if (@available(macOS 10.13, iOS 11.0, *)) {
        writer->appendU64("currentAllocatedSize", fDevice.currentAllocatedSize);
        writer->appendU64("maxThreadgroupMemoryLength", fDevice.maxThreadgroupMemoryLength);
    }

    if (@available(macOS 10.11, iOS 9.0, *)) {
        writer->beginObject("maxThreadsPerThreadgroup");
        writer->appendU64("width", fDevice.maxThreadsPerThreadgroup.width);
        writer->appendU64("height", fDevice.maxThreadsPerThreadgroup.height);
        writer->appendU64("depth", fDevice.maxThreadsPerThreadgroup.depth);
        writer->endObject();
    }

    if (@available(macOS 10.13, iOS 11.0, *)) {
        writer->appendBool("areProgrammableSamplePositionsSupported",
                           fDevice.areProgrammableSamplePositionsSupported);
        writer->appendBool("areRasterOrderGroupsSupported",
                           fDevice.areRasterOrderGroupsSupported);
    }
#ifdef SK_BUILD_FOR_MAC
    if (@available(macOS 10.11, *)) {
        writer->appendBool("isDepth24Stencil8PixelFormatSupported",
                           fDevice.isDepth24Stencil8PixelFormatSupported);

    }
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101500
    if (@available(macOS 10.15, *)) {
        writer->appendBool("areBarycentricCoordsSupported",
                           fDevice.areBarycentricCoordsSupported);
        writer->appendBool("supportsShaderBarycentricCoordinates",
                           fDevice.supportsShaderBarycentricCoordinates);
    }
#endif
#endif  // SK_BUILD_FOR_MAC
    if (@available(macOS 10.14, iOS 12.0, *)) {
        writer->appendU64("maxBufferLength", fDevice.maxBufferLength);
    }
    if (@available(macOS 10.13, iOS 11.0, *)) {
        switch (fDevice.readWriteTextureSupport) {
            case MTLReadWriteTextureTier1:
                writer->appendString("readWriteTextureSupport", "tier1");
                break;
            case MTLReadWriteTextureTier2:
                writer->appendString("readWriteTextureSupport", "tier2");
                break;
            case MTLReadWriteTextureTierNone:
                writer->appendString("readWriteTextureSupport", "tierNone");
                break;
            default:
                writer->appendString("readWriteTextureSupport", "unknown");
                break;
        }
        switch (fDevice.argumentBuffersSupport) {
            case MTLArgumentBuffersTier1:
                writer->appendString("argumentBuffersSupport", "tier1");
                break;
            case MTLArgumentBuffersTier2:
                writer->appendString("argumentBuffersSupport", "tier2");
                break;
            default:
                writer->appendString("argumentBuffersSupport", "unknown");
                break;
        }
    }
    if (@available(macOS 10.14, iOS 12.0, *)) {
        writer->appendU64("maxArgumentBufferSamplerCount", fDevice.maxArgumentBufferSamplerCount);
    }
#ifdef SK_BUILD_FOR_IOS
    if (@available(iOS 13.0, *)) {
        writer->appendU64("sparseTileSizeInBytes", fDevice.sparseTileSizeInBytes);
    }
#endif
    writer->endObject();

    writer->appendString("queue", fQueue.label.UTF8String);
    writer->appendBool("disconnected", fDisconnected);

    writer->endObject();
}
#endif

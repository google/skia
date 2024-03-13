/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlGpu.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendSemaphore.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h"
#include "src/gpu/ganesh/mtl/GrMtlBuffer.h"
#include "src/gpu/ganesh/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/ganesh/mtl/GrMtlOpsRenderPass.h"
#include "src/gpu/ganesh/mtl/GrMtlPipelineStateBuilder.h"
#include "src/gpu/ganesh/mtl/GrMtlRenderCommandEncoder.h"
#include "src/gpu/ganesh/mtl/GrMtlSemaphore.h"
#include "src/gpu/ganesh/mtl/GrMtlTexture.h"
#include "src/gpu/ganesh/mtl/GrMtlTextureRenderTarget.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#import <simd/simd.h>

using namespace skia_private;

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

#if defined(GR_TEST_UTILS)
// set to 1 if you want to do GPU capture of each commandBuffer
#define GR_METAL_CAPTURE_COMMANDBUFFER 0
#endif

std::unique_ptr<GrGpu> GrMtlGpu::Make(const GrMtlBackendContext& context,
                                      const GrContextOptions& options,
                                      GrDirectContext* direct) {
    if (!context.fDevice || !context.fQueue) {
        return nullptr;
    }
    if (@available(macOS 10.14, iOS 10.0, tvOS 10.0, *)) {
        // no warning needed
    } else {
        SkDebugf("*** Error ***: Skia's Metal backend no longer supports this OS version.\n");
#ifdef SK_BUILD_FOR_IOS
        SkDebugf("Minimum supported version is iOS 10.0.\n");
#else
        SkDebugf("Minimum supported version is MacOS 10.14.\n");
#endif
        return nullptr;
    }

    id<MTLDevice> GR_NORETAIN device = (__bridge id<MTLDevice>)(context.fDevice.get());
    id<MTLCommandQueue> GR_NORETAIN queue = (__bridge id<MTLCommandQueue>)(context.fQueue.get());

    return std::unique_ptr<GrGpu>(new GrMtlGpu(direct,
                                               options,
                                               device,
                                               queue,
                                               context.fBinaryArchive.get()));
}

// This constant determines how many OutstandingCommandBuffers are allocated together as a block in
// the deque. As such it needs to balance allocating too much memory vs. incurring
// allocation/deallocation thrashing. It should roughly correspond to the max number of outstanding
// command buffers we expect to see.
static const int kDefaultOutstandingAllocCnt = 8;

GrMtlGpu::GrMtlGpu(GrDirectContext* direct, const GrContextOptions& options,
                   id<MTLDevice> device, id<MTLCommandQueue> queue, GrMTLHandle binaryArchive)
        : INHERITED(direct)
        , fDevice(device)
        , fQueue(queue)
        , fOutstandingCommandBuffers(sizeof(OutstandingCommandBuffer), kDefaultOutstandingAllocCnt)
        , fResourceProvider(this)
        , fStagingBufferManager(this)
        , fUniformsRingBuffer(this, 128 * 1024, 256, GrGpuBufferType::kUniform)
        , fDisconnected(false) {
    fMtlCaps.reset(new GrMtlCaps(options, fDevice));
    this->initCaps(fMtlCaps);
#if GR_METAL_CAPTURE_COMMANDBUFFER
    this->testingOnly_startCapture();
#endif
    fCurrentCmdBuffer = GrMtlCommandBuffer::Make(fQueue);
#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
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
    // if there's no work we won't release the command buffer, so we do it here
    fCurrentCmdBuffer = nil;

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
            GrRenderTarget* renderTarget, bool useMSAASurface, GrAttachment* stencil,
            GrSurfaceOrigin origin, const SkIRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
            const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
            const TArray<GrSurfaceProxy*, true>& sampledProxies,
            GrXferBarrierFlags renderPassXferBarriers) {
    // For the given render target and requested render pass features we need to find a compatible
    // framebuffer to use.
    GrMtlRenderTarget* mtlRT = static_cast<GrMtlRenderTarget*>(renderTarget);

    // TODO: support DMSAA
    SkASSERT(!useMSAASurface ||
             (renderTarget->numSamples() > 1));

    bool withResolve = false;

    // Figure out if we can use a Resolve store action for this render pass. When we set up
    // the render pass we'll update the color load/store ops since we don't want to ever load
    // or store the msaa color attachment, but may need to for the resolve attachment.
    if (useMSAASurface && this->mtlCaps().renderTargetSupportsDiscardableMSAA(mtlRT)) {
        withResolve = true;
    }

    sk_sp<GrMtlFramebuffer> framebuffer =
            sk_ref_sp(mtlRT->getFramebuffer(withResolve, SkToBool(stencil)));
    if (!framebuffer) {
        return nullptr;
    }

    return new GrMtlOpsRenderPass(this, renderTarget, std::move(framebuffer), origin, colorInfo,
                                  stencilInfo);
}

GrMtlCommandBuffer* GrMtlGpu::commandBuffer() {
    if (!fCurrentCmdBuffer) {
#if GR_METAL_CAPTURE_COMMANDBUFFER
        this->testingOnly_startCapture();
#endif
        // Create a new command buffer for the next submit
        fCurrentCmdBuffer = GrMtlCommandBuffer::Make(fQueue);
    }

    SkASSERT(fCurrentCmdBuffer);
    return fCurrentCmdBuffer.get();
}

void GrMtlGpu::takeOwnershipOfBuffer(sk_sp<GrGpuBuffer> buffer) {
    SkASSERT(buffer);
    this->commandBuffer()->addGrBuffer(std::move(buffer));
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
    bool didCommit = fCurrentCmdBuffer->commit(sync == SyncQueue::kForce_SyncQueue);
    if (didCommit) {
        new (fOutstandingCommandBuffers.push_back()) OutstandingCommandBuffer(fCurrentCmdBuffer);
    }

    // We don't create a new command buffer here because we may end up using it
    // in the next frame, and that confuses the GPU debugger. Instead we
    // create when we next need one.
    fCurrentCmdBuffer.reset();

    // If the freeing of any resources held by a finished command buffer causes us to send
    // a new command to the gpu we'll create the new command buffer in commandBuffer(), above.
    this->checkForFinishedCommandBuffers();

#if GR_METAL_CAPTURE_COMMANDBUFFER
    this->testingOnly_stopCapture();
#endif
    return didCommit;
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
    this->addFinishedCallback(skgpu::RefCntedCallback::Make(finishedProc, finishedContext));
}

void GrMtlGpu::addFinishedCallback(sk_sp<skgpu::RefCntedCallback> finishedCallback) {
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

bool GrMtlGpu::onSubmitToGpu(GrSyncCpu sync) {
    if (sync == GrSyncCpu::kYes) {
        return this->submitCommandBuffer(kForce_SyncQueue);
    } else {
        return this->submitCommandBuffer(kSkip_SyncQueue);
    }
}

std::unique_ptr<GrSemaphore> GrMtlGpu::prepareTextureForCrossContextUsage(GrTexture*) {
    this->submitToGpu(GrSyncCpu::kNo);
    return nullptr;
}

sk_sp<GrGpuBuffer> GrMtlGpu::onCreateBuffer(size_t size,
                                            GrGpuBufferType type,
                                            GrAccessPattern accessPattern) {
    return GrMtlBuffer::Make(this, size, type, accessPattern);
}

static bool check_max_blit_width(int widthInPixels) {
    if (widthInPixels > 32767) {
        SkASSERT(false); // surfaces should not be this wide anyway
        return false;
    }
    return true;
}

bool GrMtlGpu::uploadToTexture(GrMtlTexture* tex,
                               SkIRect rect,
                               GrColorType dataColorType,
                               const GrMipLevel texels[],
                               int mipLevelCount) {
    SkASSERT(this->mtlCaps().isFormatTexturable(tex->mtlTexture().pixelFormat));
    // The assumption is either that we have no mipmaps, or that our rect is the entire texture
    SkASSERT(mipLevelCount == 1 || rect == SkIRect::MakeSize(tex->dimensions()));

    // We assume that if the texture has mip levels, we either upload to all the levels or just the
    // first.
    SkASSERT(mipLevelCount == 1 || mipLevelCount == (tex->maxMipmapLevel() + 1));

    if (!check_max_blit_width(rect.width())) {
        return false;
    }
    if (rect.isEmpty()) {
        return false;
    }

    SkASSERT(this->mtlCaps().surfaceSupportsWritePixels(tex));
    SkASSERT(this->mtlCaps().areColorTypeAndFormatCompatible(dataColorType, tex->backendFormat()));

    id<MTLTexture> GR_NORETAIN mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);
    // Either upload only the first miplevel or all miplevels
    SkASSERT(1 == mipLevelCount || mipLevelCount == (int)mtlTexture.mipmapLevelCount);

    if (mipLevelCount == 1 && !texels[0].fPixels) {
        return true;   // no data to upload
    }

    for (int i = 0; i < mipLevelCount; ++i) {
        // We do not allow any gaps in the mip data
        if (!texels[i].fPixels) {
            return false;
        }
    }

    size_t bpp = GrColorTypeBytesPerPixel(dataColorType);

    TArray<size_t> individualMipOffsets(mipLevelCount);
    size_t combinedBufferSize = GrComputeTightCombinedBufferSize(bpp,
                                                                 rect.size(),
                                                                 &individualMipOffsets,
                                                                 mipLevelCount);
    SkASSERT(combinedBufferSize);


    // offset value must be a multiple of the destination texture's pixel size in bytes
    size_t alignment = std::max(bpp, this->mtlCaps().getMinBufferAlignment());
    GrStagingBufferManager::Slice slice = fStagingBufferManager.allocateStagingBufferSlice(
            combinedBufferSize, alignment);
    if (!slice.fBuffer) {
        return false;
    }
    char* bufferData = (char*)slice.fOffsetMapPtr;
    GrMtlBuffer* mtlBuffer = static_cast<GrMtlBuffer*>(slice.fBuffer);

    int currentWidth = rect.width();
    int currentHeight = rect.height();
    SkDEBUGCODE(int layerHeight = tex->height());
    MTLOrigin origin = MTLOriginMake(rect.left(), rect.top(), 0);

    auto cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"uploadToTexture"];
#endif
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
        SkDEBUGCODE(layerHeight = currentHeight);
    }
#ifdef SK_BUILD_FOR_MAC
    if (this->mtlCaps().isMac()) {
        [mtlBuffer->mtlBuffer() didModifyRange: NSMakeRange(slice.fOffset, combinedBufferSize)];
    }
#endif
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder popDebugGroup];
#endif

    if (mipLevelCount < (int) tex->mtlTexture().mipmapLevelCount) {
        tex->markMipmapsDirty();
    }

    return true;
}

bool GrMtlGpu::clearTexture(GrMtlTexture* tex, size_t bpp, uint32_t levelMask) {
    SkASSERT(this->mtlCaps().isFormatTexturable(tex->mtlTexture().pixelFormat));

    if (!levelMask) {
        return true;
    }

    id<MTLTexture> GR_NORETAIN mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);
    // Either upload only the first miplevel or all miplevels
    int mipLevelCount = (int)mtlTexture.mipmapLevelCount;

    TArray<size_t> individualMipOffsets(mipLevelCount);
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

    size_t alignment = std::max(bpp, this->mtlCaps().getMinBufferAlignment());
    GrStagingBufferManager::Slice slice = fStagingBufferManager.allocateStagingBufferSlice(
            combinedBufferSize, alignment);
    if (!slice.fBuffer) {
        return false;
    }
    GrMtlBuffer* mtlBuffer = static_cast<GrMtlBuffer*>(slice.fBuffer);
    id<MTLBuffer> transferBuffer = mtlBuffer->mtlBuffer();

    auto cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"clearTexture"];
#endif
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
    // Don't need didModifyRange: here because fillBuffer: happens on the GPU
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder popDebugGroup];
#endif

    if (mipLevelCount < (int) tex->mtlTexture().mipmapLevelCount) {
        tex->markMipmapsDirty();
    }

    return true;
}

sk_sp<GrAttachment> GrMtlGpu::makeStencilAttachment(const GrBackendFormat& /*colorFormat*/,
                                                    SkISize dimensions, int numStencilSamples) {
    MTLPixelFormat sFmt = this->mtlCaps().preferredStencilFormat();

    fStats.incStencilAttachmentCreates();
    return GrMtlAttachment::GrMtlAttachment::MakeStencil(this, dimensions, numStencilSamples, sFmt);
}

sk_sp<GrAttachment> GrMtlGpu::makeMSAAAttachment(SkISize dimensions,
                                                 const GrBackendFormat& format,
                                                 int numSamples,
                                                 GrProtected isProtected,
                                                 GrMemoryless isMemoryless) {
    // Metal doesn't support protected textures
    SkASSERT(isProtected == GrProtected::kNo);
    // TODO: add memoryless support
    SkASSERT(isMemoryless == GrMemoryless::kNo);

    MTLPixelFormat pixelFormat = (MTLPixelFormat) format.asMtlFormat();
    SkASSERT(pixelFormat != MTLPixelFormatInvalid);
    SkASSERT(!skgpu::MtlFormatIsCompressed(pixelFormat));
    SkASSERT(this->mtlCaps().isFormatRenderable(pixelFormat, numSamples));

    fStats.incMSAAAttachmentCreates();
    return GrMtlAttachment::MakeMSAA(this, dimensions, numSamples, pixelFormat);
}

sk_sp<GrTexture> GrMtlGpu::onCreateTexture(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           GrRenderable renderable,
                                           int renderTargetSampleCnt,
                                           skgpu::Budgeted budgeted,
                                           GrProtected isProtected,
                                           int mipLevelCount,
                                           uint32_t levelClearMask,
                                           std::string_view label) {
    // We don't support protected textures in Metal.
    if (isProtected == GrProtected::kYes) {
        return nullptr;
    }
    SkASSERT(mipLevelCount > 0);

    MTLPixelFormat mtlPixelFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(mtlPixelFormat != MTLPixelFormatInvalid);
    SkASSERT(!this->caps()->isFormatCompressed(format));

    sk_sp<GrMtlTexture> tex;
    GrMipmapStatus mipmapStatus =
            mipLevelCount > 1 ? GrMipmapStatus::kDirty : GrMipmapStatus::kNotAllocated;
    if (renderable == GrRenderable::kYes) {
        tex = GrMtlTextureRenderTarget::MakeNewTextureRenderTarget(
                this, budgeted, dimensions, renderTargetSampleCnt, mtlPixelFormat, mipLevelCount,
                mipmapStatus, label);
    } else {
        tex = GrMtlTexture::MakeNewTexture(this, budgeted, dimensions, mtlPixelFormat,
                                           mipLevelCount, mipmapStatus, label);
    }

    if (!tex) {
        return nullptr;
    }

    if (levelClearMask) {
        this->clearTexture(tex.get(),
                           skgpu::MtlFormatBytesPerBlock(mtlPixelFormat),
                           levelClearMask);
    }

    return std::move(tex);
}

sk_sp<GrTexture> GrMtlGpu::onCreateCompressedTexture(SkISize dimensions,
                                                     const GrBackendFormat& format,
                                                     skgpu::Budgeted budgeted,
                                                     skgpu::Mipmapped mipmapped,
                                                     GrProtected isProtected,
                                                     const void* data,
                                                     size_t dataSize) {
    // We don't support protected textures in Metal.
    if (isProtected == GrProtected::kYes) {
        return nullptr;
    }

    SkASSERT(this->caps()->isFormatTexturable(format, GrTextureType::k2D));
    SkASSERT(data);

    if (!check_max_blit_width(dimensions.width())) {
        return nullptr;
    }

    MTLPixelFormat mtlPixelFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(this->caps()->isFormatCompressed(format));

    int numMipLevels = 1;
    if (mipmapped == skgpu::Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    GrMipmapStatus mipmapStatus = (mipmapped == skgpu::Mipmapped::kYes)
                                          ? GrMipmapStatus::kValid
                                          : GrMipmapStatus::kNotAllocated;

    auto tex = GrMtlTexture::MakeNewTexture(this, budgeted, dimensions, mtlPixelFormat,
                                            numMipLevels, mipmapStatus,
                                            /*label=*/"MtlGpu_CreateCompressedTexture");
    if (!tex) {
        return nullptr;
    }

    // Upload to texture
    id<MTLTexture> GR_NORETAIN mtlTexture = tex->mtlTexture();
    SkASSERT(mtlTexture);

    auto compressionType = GrBackendFormatToCompressionType(format);
    SkASSERT(compressionType != SkTextureCompressionType::kNone);

    TArray<size_t> individualMipOffsets(numMipLevels);
    SkDEBUGCODE(size_t combinedBufferSize =)
            SkCompressedDataSize(compressionType,
                                 dimensions,
                                 &individualMipOffsets,
                                 mipmapped == skgpu::Mipmapped::kYes);
    SkASSERT(individualMipOffsets.size() == numMipLevels);
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

    auto cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return nullptr;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"onCreateCompressedTexture"];
#endif

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
    if (this->mtlCaps().isMac()) {
        [mtlBuffer->mtlBuffer() didModifyRange: NSMakeRange(slice.fOffset, dataSize)];
    }
#endif
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder popDebugGroup];
#endif

    return std::move(tex);
}

// TODO: Extra retain/release can't be avoided here because of getMtlTextureInfo copying the
// sk_cfp. It would be useful to have a (possibly-internal-only?) API to get the raw pointer.
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

    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
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

    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        SkASSERT(MTLTextureUsageRenderTarget & mtlTexture.usage);
    }

    return GrMtlRenderTarget::MakeWrappedRenderTarget(this, backendRT.dimensions(),
                                                      backendRT.sampleCnt(), mtlTexture);
}

bool GrMtlGpu::onRegenerateMipMapLevels(GrTexture* texture) {
    GrMtlTexture* grMtlTexture = static_cast<GrMtlTexture*>(texture);
    id<MTLTexture> GR_NORETAIN mtlTexture = grMtlTexture->mtlTexture();

    // Automatic mipmap generation is only supported by color-renderable formats
    if (!fMtlCaps->isFormatRenderable(mtlTexture.pixelFormat, 1) &&
        // We have pixel configs marked as textureable-only that use RGBA8 as the internal format
        MTLPixelFormatRGBA8Unorm != mtlTexture.pixelFormat) {
        return false;
    }

    auto cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }
    [blitCmdEncoder generateMipmapsForTexture: mtlTexture];
    this->commandBuffer()->addGrSurface(sk_ref_sp<const GrSurface>(grMtlTexture->attachment()));

    return true;
}

// Used to "clear" a backend texture to a constant color by transferring.
static GrColorType mtl_format_to_backend_tex_clear_colortype(MTLPixelFormat format) {
    switch(format) {
        case MTLPixelFormatA8Unorm:         return GrColorType::kAlpha_8;
        case MTLPixelFormatR8Unorm:         return GrColorType::kR_8;
        case MTLPixelFormatB5G6R5Unorm:     return GrColorType::kBGR_565;
        case MTLPixelFormatABGR4Unorm:      return GrColorType::kABGR_4444;
        case MTLPixelFormatRGBA8Unorm:      return GrColorType::kRGBA_8888;
        case MTLPixelFormatRGBA8Unorm_sRGB: return GrColorType::kRGBA_8888_SRGB;

        case MTLPixelFormatRG8Unorm:        return GrColorType::kRG_88;
        case MTLPixelFormatBGRA8Unorm:      return GrColorType::kBGRA_8888;
        case MTLPixelFormatRGB10A2Unorm:    return GrColorType::kRGBA_1010102;
        case MTLPixelFormatBGR10A2Unorm:    return GrColorType::kBGRA_1010102;
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
                   const TArray<size_t>& individualMipOffsets,
                   const GrPixmap srcData[],
                   int numMipLevels,
                   size_t bufferSize) {
    SkASSERT(srcData && numMipLevels);
    SkASSERT(individualMipOffsets.size() == numMipLevels);

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
                                                 skgpu::Mipmapped mipmapped,
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
    if (mipmapped == skgpu::Mipmapped::kYes) {
        desc.mipmapLevelCount = 1 + SkPrevLog2(std::max(dimensions.width(), dimensions.height()));
    }
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
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
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    testTexture.label = @"testTexture";
#endif
    info->fTexture.reset(GrRetainPtrFromId(testTexture));
    return true;
}

GrBackendTexture GrMtlGpu::onCreateBackendTexture(SkISize dimensions,
                                                  const GrBackendFormat& format,
                                                  GrRenderable renderable,
                                                  skgpu::Mipmapped mipmapped,
                                                  GrProtected isProtected,
                                                  std::string_view label) {
    const MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);

    GrMtlTextureInfo info;
    if (!this->createMtlTextureForBackendSurface(mtlFormat, dimensions, 1, GrTexturable::kYes,
                                                 renderable, mipmapped, &info)) {
        return {};
    }

    GrBackendTexture backendTex(dimensions.width(), dimensions.height(), mipmapped, info);
    return backendTex;
}

bool GrMtlGpu::onClearBackendTexture(const GrBackendTexture& backendTexture,
                                     sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                     std::array<float, 4> color) {
    GrMtlTextureInfo info;
    SkAssertResult(backendTexture.getMtlTextureInfo(&info));

    id<MTLTexture> GR_NORETAIN mtlTexture = GrGetMTLTexture(info.fTexture.get());

    const MTLPixelFormat mtlFormat = mtlTexture.pixelFormat;

    // Create a transfer buffer and fill with data.
    size_t bytesPerPixel = skgpu::MtlFormatBytesPerBlock(mtlFormat);
    size_t combinedBufferSize;

    // Reuse the same buffer for all levels. Should be ok since we made the row bytes tight.
    combinedBufferSize = bytesPerPixel*backendTexture.width()*backendTexture.height();

    size_t alignment = std::max(bytesPerPixel, this->mtlCaps().getMinBufferAlignment());
    GrStagingBufferManager::Slice slice = fStagingBufferManager.allocateStagingBufferSlice(
            combinedBufferSize, alignment);
    if (!slice.fBuffer) {
        return false;
    }
    char* buffer = (char*)slice.fOffsetMapPtr;

    auto colorType = mtl_format_to_backend_tex_clear_colortype(mtlFormat);
    if (colorType == GrColorType::kUnknown) {
        return false;
    }
    GrImageInfo ii(colorType, kUnpremul_SkAlphaType, nullptr, backendTexture.dimensions());
    auto rb = ii.minRowBytes();
    SkASSERT(rb == bytesPerPixel*backendTexture.width());
    if (!GrClearImage(ii, buffer, rb, color)) {
        return false;
    }

    // Transfer buffer contents to texture
    MTLOrigin origin = MTLOriginMake(0, 0, 0);

    GrMtlCommandBuffer* cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"onClearBackendTexture"];
#endif
    GrMtlBuffer* mtlBuffer = static_cast<GrMtlBuffer*>(slice.fBuffer);

    SkISize levelDimensions(backendTexture.dimensions());
    int numMipLevels = mtlTexture.mipmapLevelCount;
    for (int currentMipLevel = 0; currentMipLevel < numMipLevels; currentMipLevel++) {
        size_t levelRowBytes;
        size_t levelSize;

        levelRowBytes = levelDimensions.width() * bytesPerPixel;
        levelSize = levelRowBytes * levelDimensions.height();

        // TODO: can this all be done in one go?
        [blitCmdEncoder copyFromBuffer: mtlBuffer->mtlBuffer()
                          sourceOffset: slice.fOffset
                     sourceBytesPerRow: levelRowBytes
                   sourceBytesPerImage: levelSize
                            sourceSize: MTLSizeMake(levelDimensions.width(),
                                                    levelDimensions.height(),
                                                    1)
                             toTexture: mtlTexture
                      destinationSlice: 0
                      destinationLevel: currentMipLevel
                     destinationOrigin: origin];

        levelDimensions = {std::max(1, levelDimensions.width() / 2),
                           std::max(1, levelDimensions.height() / 2)};
    }
#ifdef SK_BUILD_FOR_MAC
    if (this->mtlCaps().isMac()) {
        [mtlBuffer->mtlBuffer() didModifyRange: NSMakeRange(slice.fOffset, combinedBufferSize)];
    }
#endif
    [blitCmdEncoder popDebugGroup];

    if (finishedCallback) {
        this->addFinishedCallback(std::move(finishedCallback));
    }

    return true;
}

GrBackendTexture GrMtlGpu::onCreateCompressedBackendTexture(SkISize dimensions,
                                                            const GrBackendFormat& format,
                                                            skgpu::Mipmapped mipmapped,
                                                            GrProtected isProtected) {
    const MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);

    GrMtlTextureInfo info;
    if (!this->createMtlTextureForBackendSurface(mtlFormat, dimensions, 1, GrTexturable::kYes,
                                                 GrRenderable::kNo, mipmapped, &info)) {
        return {};
    }

    return GrBackendTexture(dimensions.width(), dimensions.height(), mipmapped, info);
}

bool GrMtlGpu::onUpdateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                                sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                                const void* data,
                                                size_t size) {
    GrMtlTextureInfo info;
    SkAssertResult(backendTexture.getMtlTextureInfo(&info));

    id<MTLTexture> mtlTexture = GrGetMTLTexture(info.fTexture.get());

    int numMipLevels = mtlTexture.mipmapLevelCount;
    skgpu::Mipmapped mipmapped = numMipLevels > 1 ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;

    SkTextureCompressionType compression =
            GrBackendFormatToCompressionType(backendTexture.getBackendFormat());
    SkASSERT(compression != SkTextureCompressionType::kNone);

    // Create a transfer buffer and fill with data.
    STArray<16, size_t> individualMipOffsets;
    size_t combinedBufferSize;
    combinedBufferSize = SkCompressedDataSize(compression,
                                              backendTexture.dimensions(),
                                              &individualMipOffsets,
                                              mipmapped == skgpu::Mipmapped::kYes);
    SkASSERT(individualMipOffsets.size() == numMipLevels);

    size_t alignment = std::max(SkCompressedBlockSize(compression),
                                this->mtlCaps().getMinBufferAlignment());
    GrStagingBufferManager::Slice slice =
            fStagingBufferManager.allocateStagingBufferSlice(combinedBufferSize, alignment);
    if (!slice.fBuffer) {
        return false;
    }
    char* buffer = (char*)slice.fOffsetMapPtr;

    memcpy(buffer, data, size);

    // Transfer buffer contents to texture
    MTLOrigin origin = MTLOriginMake(0, 0, 0);

    GrMtlCommandBuffer* cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"onUpdateCompressedBackendTexture"];
#endif
    GrMtlBuffer* mtlBuffer = static_cast<GrMtlBuffer*>(slice.fBuffer);

    SkISize levelDimensions(backendTexture.dimensions());
    for (int currentMipLevel = 0; currentMipLevel < numMipLevels; currentMipLevel++) {
        size_t levelRowBytes;
        size_t levelSize;

        levelRowBytes = GrCompressedRowBytes(compression, levelDimensions.width());
        levelSize = SkCompressedDataSize(compression, levelDimensions, nullptr, false);

        // TODO: can this all be done in one go?
        [blitCmdEncoder copyFromBuffer: mtlBuffer->mtlBuffer()
                          sourceOffset: slice.fOffset + individualMipOffsets[currentMipLevel]
                     sourceBytesPerRow: levelRowBytes
                   sourceBytesPerImage: levelSize
                            sourceSize: MTLSizeMake(levelDimensions.width(),
                                                    levelDimensions.height(),
                                                    1)
                             toTexture: mtlTexture
                      destinationSlice: 0
                      destinationLevel: currentMipLevel
                     destinationOrigin: origin];

        levelDimensions = {std::max(1, levelDimensions.width() / 2),
                           std::max(1, levelDimensions.height() / 2)};
    }
#ifdef SK_BUILD_FOR_MAC
    if (this->mtlCaps().isMac()) {
        [mtlBuffer->mtlBuffer() didModifyRange:NSMakeRange(slice.fOffset, combinedBufferSize)];
    }
#endif
    [blitCmdEncoder popDebugGroup];

    if (finishedCallback) {
        this->addFinishedCallback(std::move(finishedCallback));
    }

    return true;
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

#if defined(GR_TEST_UTILS)
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
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
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
    if (!this->createMtlTextureForBackendSurface(format,
                                                 dimensions,
                                                 sampleCnt,
                                                 GrTexturable::kNo,
                                                 GrRenderable::kYes,
                                                 skgpu::Mipmapped::kNo,
                                                 &info)) {
        return {};
    }

    return GrBackendRenderTarget(dimensions.width(), dimensions.height(), info);
}

void GrMtlGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    SkASSERT(GrBackendApi::kMetal == rt.backend());

    GrMtlTextureInfo info;
    if (rt.getMtlTextureInfo(&info)) {
        this->submitToGpu(GrSyncCpu::kYes);
        // Nothing else to do here, will get cleaned up when the GrBackendRenderTarget
        // is deleted.
    }
}
#endif // defined(GR_TEST_UTILS)

void GrMtlGpu::copySurfaceAsResolve(GrSurface* dst, GrSurface* src) {
    // TODO: Add support for subrectangles
    GrMtlRenderTarget* srcRT = static_cast<GrMtlRenderTarget*>(src->asRenderTarget());
    GrRenderTarget* dstRT = dst->asRenderTarget();
    GrMtlAttachment* dstAttachment;
    if (dstRT) {
        GrMtlRenderTarget* mtlRT = static_cast<GrMtlRenderTarget*>(dstRT);
        dstAttachment = mtlRT->colorAttachment();
    } else {
        SkASSERT(dst->asTexture());
        dstAttachment = static_cast<GrMtlTexture*>(dst->asTexture())->attachment();
    }

    this->resolve(dstAttachment, srcRT->colorAttachment());
}

void GrMtlGpu::copySurfaceAsBlit(GrSurface* dst, GrSurface* src,
                                 GrMtlAttachment* dstAttachment, GrMtlAttachment* srcAttachment,
                                 const SkIRect& srcRect, const SkIPoint& dstPoint) {
#ifdef SK_DEBUG
    SkASSERT(this->mtlCaps().canCopyAsBlit(dstAttachment->mtlFormat(), dstAttachment->numSamples(),
                                           srcAttachment->mtlFormat(), dstAttachment->numSamples(),
                                           srcRect, dstPoint, dst == src));
#endif
    id<MTLTexture> GR_NORETAIN dstTex = dstAttachment->mtlTexture();
    id<MTLTexture> GR_NORETAIN srcTex = srcAttachment->mtlTexture();

    auto cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"copySurfaceAsBlit"];
#endif
    [blitCmdEncoder copyFromTexture: srcTex
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(srcRect.x(), srcRect.y(), 0)
                         sourceSize: MTLSizeMake(srcRect.width(), srcRect.height(), 1)
                          toTexture: dstTex
                   destinationSlice: 0
                   destinationLevel: 0
                  destinationOrigin: MTLOriginMake(dstPoint.fX, dstPoint.fY, 0)];
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder popDebugGroup];
#endif
    cmdBuffer->addGrSurface(sk_ref_sp<const GrSurface>(dst));
    cmdBuffer->addGrSurface(sk_ref_sp<const GrSurface>(src));
}

bool GrMtlGpu::onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                             GrSurface* src, const SkIRect& srcRect,
                             GrSamplerState::Filter) {
    SkASSERT(!src->isProtected() && !dst->isProtected());

    if (srcRect.size() != dstRect.size()) {
        return false;
    }

    GrMtlAttachment* dstAttachment;
    GrMtlAttachment* srcAttachment;
    GrRenderTarget* dstRT = dst->asRenderTarget();
    if (dstRT) {
        GrMtlRenderTarget* mtlRT = static_cast<GrMtlRenderTarget*>(dstRT);
        // This will technically return true for single sample rts that used DMSAA in which case we
        // don't have to pick the resolve attachment. But in that case the resolve and color
        // attachments will be the same anyways.
        if (this->mtlCaps().renderTargetSupportsDiscardableMSAA(mtlRT)) {
            dstAttachment = mtlRT->resolveAttachment();
        } else {
            dstAttachment = mtlRT->colorAttachment();
        }
    } else if (dst->asTexture()) {
        dstAttachment = static_cast<GrMtlTexture*>(dst->asTexture())->attachment();
    } else {
        // The surface in a GrAttachment already
        dstAttachment = static_cast<GrMtlAttachment*>(dst);
    }
    GrRenderTarget* srcRT = src->asRenderTarget();
    if (srcRT) {
        GrMtlRenderTarget* mtlRT = static_cast<GrMtlRenderTarget*>(srcRT);
        // This will technically return true for single sample rts that used DMSAA in which case we
        // don't have to pick the resolve attachment. But in that case the resolve and color
        // attachments will be the same anyways.
        if (this->mtlCaps().renderTargetSupportsDiscardableMSAA(mtlRT)) {
            srcAttachment = mtlRT->resolveAttachment();
        } else {
            srcAttachment = mtlRT->colorAttachment();
        }
    } else if (src->asTexture()) {
        SkASSERT(src->asTexture());
        srcAttachment = static_cast<GrMtlTexture*>(src->asTexture())->attachment();
    } else {
        // The surface in a GrAttachment already
        srcAttachment = static_cast<GrMtlAttachment*>(src);
    }

    MTLPixelFormat dstFormat = dstAttachment->mtlFormat();
    MTLPixelFormat srcFormat = srcAttachment->mtlFormat();

    int dstSampleCnt = dstAttachment->sampleCount();
    int srcSampleCnt = srcAttachment->sampleCount();

    const SkIPoint dstPoint = dstRect.topLeft();
    if (this->mtlCaps().canCopyAsResolve(dstFormat, dstSampleCnt,
                                         srcFormat, srcSampleCnt,
                                         SkToBool(srcRT), src->dimensions(),
                                         srcRect, dstPoint,
                                         dstAttachment == srcAttachment)) {
        this->copySurfaceAsResolve(dst, src);
        return true;
    }

    if (srcAttachment->framebufferOnly() || dstAttachment->framebufferOnly()) {
        return false;
    }

    if (this->mtlCaps().canCopyAsBlit(dstFormat, dstSampleCnt, srcFormat, srcSampleCnt,
                                      srcRect, dstPoint, dstAttachment == srcAttachment)) {
        this->copySurfaceAsBlit(dst, src, dstAttachment, srcAttachment, srcRect, dstPoint);
        return true;
    }

    return false;
}

bool GrMtlGpu::onWritePixels(GrSurface* surface,
                             SkIRect rect,
                             GrColorType surfaceColorType,
                             GrColorType srcColorType,
                             const GrMipLevel texels[],
                             int mipLevelCount,
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
    return this->uploadToTexture(mtlTexture, rect, srcColorType, texels, mipLevelCount);
}

bool GrMtlGpu::onReadPixels(GrSurface* surface,
                            SkIRect rect,
                            GrColorType surfaceColorType,
                            GrColorType dstColorType,
                            void* buffer,
                            size_t rowBytes) {
    SkASSERT(surface);

    if (surfaceColorType != dstColorType) {
        return false;
    }

    int bpp = GrColorTypeBytesPerPixel(dstColorType);
    size_t transBufferRowBytes = bpp*rect.width();
    size_t transBufferImageBytes = transBufferRowBytes*rect.height();

    GrResourceProvider* resourceProvider = this->getContext()->priv().resourceProvider();
    sk_sp<GrGpuBuffer> transferBuffer = resourceProvider->createBuffer(
            transBufferImageBytes,
            GrGpuBufferType::kXferGpuToCpu,
            kDynamic_GrAccessPattern,
            GrResourceProvider::ZeroInit::kNo);

    if (!transferBuffer) {
        return false;
    }

    GrMtlBuffer* grMtlBuffer = static_cast<GrMtlBuffer*>(transferBuffer.get());
    if (!this->readOrTransferPixels(surface,
                                    rect,
                                    dstColorType,
                                    grMtlBuffer->mtlBuffer(),
                                    0,
                                    transBufferImageBytes,
                                    transBufferRowBytes)) {
        return false;
    }
    this->submitCommandBuffer(kForce_SyncQueue);

    const void* mappedMemory = grMtlBuffer->mtlBuffer().contents;

    SkRectMemcpy(buffer,
                 rowBytes,
                 mappedMemory,
                 transBufferRowBytes,
                 transBufferRowBytes,
                 rect.height());

    return true;
}

bool GrMtlGpu::onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                            size_t srcOffset,
                                            sk_sp<GrGpuBuffer> dst,
                                            size_t dstOffset,
                                            size_t size) {
    id<MTLBuffer> GR_NORETAIN mtlSrc =  static_cast<GrMtlBuffer*>(src.get())->mtlBuffer();
    id<MTLBuffer> GR_NORETAIN mtlDst =  static_cast<GrMtlBuffer*>(dst.get())->mtlBuffer();
    SkASSERT(mtlSrc);
    SkASSERT(mtlDst);

    auto cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }

#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"onTransferFromBufferToBuffer"];
#endif
    [blitCmdEncoder copyFromBuffer: mtlSrc
                      sourceOffset: srcOffset
                          toBuffer: mtlDst
                 destinationOffset: dstOffset
                              size: size];
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder popDebugGroup];
#endif

    cmdBuffer->addGrBuffer(std::move(src));
    cmdBuffer->addGrBuffer(std::move(dst));

    return true;
}

bool GrMtlGpu::onTransferPixelsTo(GrTexture* texture,
                                  SkIRect rect,
                                  GrColorType textureColorType,
                                  GrColorType bufferColorType,
                                  sk_sp<GrGpuBuffer> transferBuffer,
                                  size_t offset,
                                  size_t rowBytes) {
    SkASSERT(texture);
    SkASSERT(transferBuffer);
    if (textureColorType != bufferColorType) {
        return false;
    }

    GrMtlTexture* grMtlTexture = static_cast<GrMtlTexture*>(texture);
    id<MTLTexture> GR_NORETAIN mtlTexture = grMtlTexture->mtlTexture();
    SkASSERT(mtlTexture);

    GrMtlBuffer* grMtlBuffer = static_cast<GrMtlBuffer*>(transferBuffer.get());
    id<MTLBuffer> GR_NORETAIN mtlBuffer = grMtlBuffer->mtlBuffer();
    SkASSERT(mtlBuffer);

    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    if (offset % bpp) {
        return false;
    }
    if (GrBackendFormatBytesPerPixel(texture->backendFormat()) != bpp) {
        return false;
    }

    MTLOrigin origin = MTLOriginMake(rect.left(), rect.top(), 0);

    auto cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"onTransferPixelsTo"];
#endif
    [blitCmdEncoder copyFromBuffer: mtlBuffer
                      sourceOffset: offset
                 sourceBytesPerRow: rowBytes
               sourceBytesPerImage: rowBytes*rect.height()
                        sourceSize: MTLSizeMake(rect.width(), rect.height(), 1)
                         toTexture: mtlTexture
                  destinationSlice: 0
                  destinationLevel: 0
                 destinationOrigin: origin];
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder popDebugGroup];
#endif

    return true;
}

bool GrMtlGpu::onTransferPixelsFrom(GrSurface* surface,
                                    SkIRect rect,
                                    GrColorType surfaceColorType,
                                    GrColorType bufferColorType,
                                    sk_sp<GrGpuBuffer> transferBuffer,
                                    size_t offset) {
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

    size_t transBufferRowBytes = bpp*rect.width();
    size_t transBufferImageBytes = transBufferRowBytes*rect.height();

    return this->readOrTransferPixels(surface,
                                      rect,
                                      bufferColorType,
                                      grMtlBuffer->mtlBuffer(),
                                      offset,
                                      transBufferImageBytes,
                                      transBufferRowBytes);
}

bool GrMtlGpu::readOrTransferPixels(GrSurface* surface,
                                    SkIRect rect,
                                    GrColorType dstColorType,
                                    id<MTLBuffer> transferBuffer,
                                    size_t offset,
                                    size_t imageBytes,
                                    size_t rowBytes) {
    if (!check_max_blit_width(rect.width())) {
        return false;
    }

    id<MTLTexture> mtlTexture;
    if (GrMtlRenderTarget* rt = static_cast<GrMtlRenderTarget*>(surface->asRenderTarget())) {
        if (rt->numSamples() > 1) {
            SkASSERT(rt->requiresManualMSAAResolve());  // msaa-render-to-texture not yet supported.
            mtlTexture = rt->resolveMTLTexture();
        } else {
            SkASSERT(!rt->requiresManualMSAAResolve());
            mtlTexture = rt->colorMTLTexture();
        }
    } else if (GrMtlTexture* texture = static_cast<GrMtlTexture*>(surface->asTexture())) {
        mtlTexture = texture->mtlTexture();
    }
    if (!mtlTexture) {
        return false;
    }

    auto cmdBuffer = this->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder pushDebugGroup:@"readOrTransferPixels"];
#endif
    [blitCmdEncoder copyFromTexture: mtlTexture
                        sourceSlice: 0
                        sourceLevel: 0
                       sourceOrigin: MTLOriginMake(rect.left(), rect.top(), 0)
                         sourceSize: MTLSizeMake(rect.width(), rect.height(), 1)
                           toBuffer: transferBuffer
                  destinationOffset: offset
             destinationBytesPerRow: rowBytes
           destinationBytesPerImage: imageBytes];
#ifdef SK_BUILD_FOR_MAC
    if (this->mtlCaps().isMac()) {
        // Sync GPU data back to the CPU
        [blitCmdEncoder synchronizeResource: transferBuffer];
    }
#endif
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    [blitCmdEncoder popDebugGroup];
#endif

    return true;
}

[[nodiscard]] std::unique_ptr<GrSemaphore> GrMtlGpu::makeSemaphore(bool /*isOwned*/) {
    SkASSERT(this->caps()->semaphoreSupport());
    return GrMtlSemaphore::Make(this);
}

std::unique_ptr<GrSemaphore> GrMtlGpu::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                            GrSemaphoreWrapType /* wrapType */,
                                                            GrWrapOwnership /*ownership*/) {
    SkASSERT(this->caps()->backendSemaphoreSupport());
    return GrMtlSemaphore::MakeWrapped(GrBackendSemaphores::GetMtlHandle(semaphore),
                                       GrBackendSemaphores::GetMtlValue(semaphore));
}

void GrMtlGpu::insertSemaphore(GrSemaphore* semaphore) {
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        SkASSERT(semaphore);
        GrMtlSemaphore* mtlSem = static_cast<GrMtlSemaphore*>(semaphore);

        this->commandBuffer()->encodeSignalEvent(mtlSem->event(), mtlSem->value());
    }
}

void GrMtlGpu::waitSemaphore(GrSemaphore* semaphore) {
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        SkASSERT(semaphore);
        GrMtlSemaphore* mtlSem = static_cast<GrMtlSemaphore*>(semaphore);

        this->commandBuffer()->encodeWaitForEvent(mtlSem->event(), mtlSem->value());
    }
}

void GrMtlGpu::onResolveRenderTarget(GrRenderTarget* target, const SkIRect&) {
    SkASSERT(target->numSamples() > 1);
    GrMtlRenderTarget* rt = static_cast<GrMtlRenderTarget*>(target);

    if (rt->resolveAttachment() && this->mtlCaps().renderTargetSupportsDiscardableMSAA(rt)) {
        // We would have resolved the RT during the render pass.
        return;
    }

    this->resolve(static_cast<GrMtlRenderTarget*>(target)->resolveAttachment(),
                  static_cast<GrMtlRenderTarget*>(target)->colorAttachment());
}

void GrMtlGpu::resolve(GrMtlAttachment* resolveAttachment,
                       GrMtlAttachment* msaaAttachment) {
    auto renderPassDesc = [[MTLRenderPassDescriptor alloc] init];
    auto colorAttachment = renderPassDesc.colorAttachments[0];
    colorAttachment.texture = msaaAttachment->mtlTexture();
    colorAttachment.resolveTexture = resolveAttachment->mtlTexture();
    colorAttachment.loadAction = MTLLoadActionLoad;
    colorAttachment.storeAction = MTLStoreActionMultisampleResolve;

    GrMtlRenderCommandEncoder* cmdEncoder =
            this->commandBuffer()->getRenderCommandEncoder(renderPassDesc, nullptr, nullptr);
    if (cmdEncoder) {
        cmdEncoder->setLabel(@"resolveTexture");
        this->commandBuffer()->addGrSurface(sk_ref_sp<const GrSurface>(resolveAttachment));
        this->commandBuffer()->addGrSurface(sk_ref_sp<const GrSurface>(msaaAttachment));
    }
}

GrMtlRenderCommandEncoder* GrMtlGpu::loadMSAAFromResolve(
        GrAttachment* dst, GrMtlAttachment* src, const SkIRect& srcRect,
        MTLRenderPassStencilAttachmentDescriptor* stencil) {
    if (!dst) {
        return nil;
    }
    if (!src || src->framebufferOnly()) {
        return nil;
    }

    GrMtlAttachment* mtlDst = static_cast<GrMtlAttachment*>(dst);

    MTLPixelFormat stencilFormat = stencil.texture.pixelFormat;
    auto renderPipeline = this->resourceProvider().findOrCreateMSAALoadPipeline(mtlDst->mtlFormat(),
                                                                                dst->numSamples(),
                                                                                stencilFormat);

    // Set up rendercommandencoder
    auto renderPassDesc = [MTLRenderPassDescriptor new];
    auto colorAttachment = renderPassDesc.colorAttachments[0];
    colorAttachment.texture = mtlDst->mtlTexture();
    colorAttachment.loadAction = MTLLoadActionDontCare;
    colorAttachment.storeAction = MTLStoreActionMultisampleResolve;
    colorAttachment.resolveTexture = src->mtlTexture();

    renderPassDesc.stencilAttachment = stencil;

    // We know in this case that the preceding renderCommandEncoder will not be compatible.
    // Either it's using a different rendertarget, or we are reading from the resolve and
    // hence we need to let the previous resolve finish. So we create a new one without checking.
    auto renderCmdEncoder =
                this->commandBuffer()->getRenderCommandEncoder(renderPassDesc, nullptr);
    if (!renderCmdEncoder) {
        return nullptr;
    }

    // Bind pipeline
    renderCmdEncoder->setRenderPipelineState(renderPipeline->mtlPipelineState());
    this->commandBuffer()->addResource(sk_ref_sp(renderPipeline));

    // Bind src as input texture
    renderCmdEncoder->setFragmentTexture(src->mtlTexture(), 0);
    // No sampler needed
    this->commandBuffer()->addGrSurface(sk_ref_sp<GrSurface>(src));

    // Scissor and viewport should default to size of color attachment

    // Update and bind uniform data
    int w = srcRect.width();
    int h = srcRect.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dst->width();
    int dh = dst->height();
    float dx0 = 2.f * srcRect.fLeft / dw - 1.f;
    float dx1 = 2.f * (srcRect.fLeft + w) / dw - 1.f;
    float dy0 = 2.f * srcRect.fTop / dh - 1.f;
    float dy1 = 2.f * (srcRect.fTop + h) / dh - 1.f;

    struct {
        float posXform[4];
        int textureSize[2];
        int pad[2];
    } uniData = {{dx1 - dx0, dy1 - dy0, dx0, dy0}, {dw, dh}, {0, 0}};

    constexpr size_t uniformSize = 32;
    if (@available(macOS 10.11, iOS 8.3, tvOS 9.0, *)) {
        SkASSERT(uniformSize <= this->caps()->maxPushConstantsSize());
        renderCmdEncoder->setVertexBytes(&uniData, uniformSize, 0);
    } else {
        // upload the data
        GrRingBuffer::Slice slice = this->uniformsRingBuffer()->suballocate(uniformSize);
        GrMtlBuffer* buffer = (GrMtlBuffer*) slice.fBuffer;
        char* destPtr = static_cast<char*>(slice.fBuffer->map()) + slice.fOffset;
        memcpy(destPtr, &uniData, uniformSize);

        renderCmdEncoder->setVertexBuffer(buffer->mtlBuffer(), slice.fOffset, 0);
    }

    renderCmdEncoder->drawPrimitives(MTLPrimitiveTypeTriangleStrip, (NSUInteger)0, (NSUInteger)4);

    return renderCmdEncoder;
}

#if defined(GR_TEST_UTILS)
void GrMtlGpu::testingOnly_startCapture() {
    if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
        // TODO: add Metal 3 interface as well
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        if (captureManager.isCapturing) {
            return;
        }
        if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
            MTLCaptureDescriptor* captureDescriptor = [[MTLCaptureDescriptor alloc] init];
            captureDescriptor.captureObject = fQueue;

            NSError *error;
            if (![captureManager startCaptureWithDescriptor: captureDescriptor error:&error])
            {
                NSLog(@"Failed to start capture, error %@", error);
            }
        } else {
            [captureManager startCaptureWithCommandQueue: fQueue];
        }
     }
}

void GrMtlGpu::testingOnly_stopCapture() {
    if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        if (captureManager.isCapturing) {
            [captureManager stopCapture];
        }
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
    writer->appendCString("name", fDevice.name.UTF8String);
#ifdef SK_BUILD_FOR_MAC
    if (@available(macOS 10.11, *)) {
        writer->appendBool("isHeadless", fDevice.isHeadless);
        writer->appendBool("isLowPower", fDevice.isLowPower);
    }
    if (@available(macOS 10.13, *)) {
        writer->appendBool("isRemovable", fDevice.isRemovable);
    }
#endif
    if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
        writer->appendU64("registryID", fDevice.registryID);
    }
#if defined(SK_BUILD_FOR_MAC) && __MAC_OS_X_VERSION_MAX_ALLOWED >= 101500
    if (@available(macOS 10.15, *)) {
        switch (fDevice.location) {
            case MTLDeviceLocationBuiltIn:
                writer->appendNString("location", "builtIn");
                break;
            case MTLDeviceLocationSlot:
                writer->appendNString("location", "slot");
                break;
            case MTLDeviceLocationExternal:
                writer->appendNString("location", "external");
                break;
            case MTLDeviceLocationUnspecified:
                writer->appendNString("location", "unspecified");
                break;
            default:
                writer->appendNString("location", "unknown");
                break;
        }
        writer->appendU64("locationNumber", fDevice.locationNumber);
        writer->appendU64("maxTransferRate", fDevice.maxTransferRate);
    }
#endif  // SK_BUILD_FOR_MAC
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101500 || __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
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
    if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
        writer->appendU64("currentAllocatedSize", fDevice.currentAllocatedSize);
        writer->appendU64("maxThreadgroupMemoryLength", fDevice.maxThreadgroupMemoryLength);
    }

    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        writer->beginObject("maxThreadsPerThreadgroup");
        writer->appendU64("width", fDevice.maxThreadsPerThreadgroup.width);
        writer->appendU64("height", fDevice.maxThreadsPerThreadgroup.height);
        writer->appendU64("depth", fDevice.maxThreadsPerThreadgroup.depth);
        writer->endObject();
    }

    if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
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
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        writer->appendU64("maxBufferLength", fDevice.maxBufferLength);
    }
    if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
        switch (fDevice.readWriteTextureSupport) {
            case MTLReadWriteTextureTier1:
                writer->appendNString("readWriteTextureSupport", "tier1");
                break;
            case MTLReadWriteTextureTier2:
                writer->appendNString("readWriteTextureSupport", "tier2");
                break;
            case MTLReadWriteTextureTierNone:
                writer->appendNString("readWriteTextureSupport", "tierNone");
                break;
            default:
                writer->appendNString("readWriteTextureSupport", "unknown");
                break;
        }
        switch (fDevice.argumentBuffersSupport) {
            case MTLArgumentBuffersTier1:
                writer->appendNString("argumentBuffersSupport", "tier1");
                break;
            case MTLArgumentBuffersTier2:
                writer->appendNString("argumentBuffersSupport", "tier2");
                break;
            default:
                writer->appendNString("argumentBuffersSupport", "unknown");
                break;
        }
    }
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        writer->appendU64("maxArgumentBufferSamplerCount", fDevice.maxArgumentBufferSamplerCount);
    }
#ifdef SK_BUILD_FOR_IOS
    if (@available(iOS 13.0, tvOS 13.0, *)) {
        writer->appendU64("sparseTileSizeInBytes", fDevice.sparseTileSizeInBytes);
    }
#endif
    writer->endObject();

    writer->appendCString("queue", fQueue.label.UTF8String);
    writer->appendBool("disconnected", fDisconnected);

    writer->endObject();
}
#endif

GR_NORETAIN_END

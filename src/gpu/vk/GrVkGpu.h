/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkGpu_DEFINED
#define GrVkGpu_DEFINED

#include "GrGpu.h"
#include "GrGpuFactory.h"
#include "vk/GrVkBackendContext.h"
#include "GrVkCaps.h"
#include "GrVkCopyManager.h"
#include "GrVkIndexBuffer.h"
#include "GrVkMemory.h"
#include "GrVkResourceProvider.h"
#include "GrVkSemaphore.h"
#include "GrVkVertexBuffer.h"
#include "GrVkUtil.h"
#include "vk/GrVkDefines.h"

class GrPipeline;

class GrVkBufferImpl;
class GrVkPipeline;
class GrVkPipelineState;
class GrVkPrimaryCommandBuffer;
class GrVkRenderPass;
class GrVkSecondaryCommandBuffer;
class GrVkTexture;
struct GrVkInterface;

namespace SkSL {
    class Compiler;
}

class GrVkGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(GrBackendContext backendContext, const GrContextOptions&, GrContext*);
    static sk_sp<GrGpu> Make(sk_sp<const GrVkBackendContext>, const GrContextOptions&, GrContext*);

    ~GrVkGpu() override;

    void disconnect(DisconnectType) override;

    const GrVkInterface* vkInterface() const { return fBackendContext->fInterface.get(); }
    const GrVkCaps& vkCaps() const { return *fVkCaps; }

    VkDevice device() const { return fDevice; }
    VkQueue  queue() const { return fQueue; }
    uint32_t  queueIndex() const { return fBackendContext->fGraphicsQueueIndex; }
    VkCommandPool cmdPool() const { return fCmdPool; }
    VkPhysicalDeviceProperties physicalDeviceProperties() const {
        return fPhysDevProps;
    }
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties() const {
        return fPhysDevMemProps;
    }

    GrVkResourceProvider& resourceProvider() { return fResourceProvider; }

    GrVkPrimaryCommandBuffer* currentCommandBuffer() { return fCurrentCmdBuffer; }

    enum SyncQueue {
        kForce_SyncQueue,
        kSkip_SyncQueue
    };

    bool onGetReadPixelsInfo(GrSurface* srcSurface, GrSurfaceOrigin srcOrigin,
                             int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override;

    bool onGetWritePixelsInfo(GrSurface* dstSurface, GrSurfaceOrigin dstOrigin,
                              int width, int height,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override;

    void onQueryMultisampleSpecs(GrRenderTarget*, GrSurfaceOrigin, const GrStencilSettings&,
                                 int* effectiveSampleCnt, SamplePattern*) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    GrBackendTexture createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                     GrPixelConfig config,
                                                     bool isRenderTarget,
                                                     GrMipMapped) override;
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;
    void deleteTestingOnlyBackendTexture(GrBackendTexture*, bool abandonTexture = false) override;

    void testingOnly_flushGpuAndSync() override;

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override;

    void clearStencil(GrRenderTarget* target, int clearValue) override;

    GrGpuRTCommandBuffer* createCommandBuffer(
            GrRenderTarget*, GrSurfaceOrigin,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) override;

    GrGpuTextureCommandBuffer* createCommandBuffer(GrTexture*, GrSurfaceOrigin) override;

    void addMemoryBarrier(VkPipelineStageFlags srcStageMask,
                          VkPipelineStageFlags dstStageMask,
                          bool byRegion,
                          VkMemoryBarrier* barrier) const;
    void addBufferMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                VkPipelineStageFlags dstStageMask,
                                bool byRegion,
                                VkBufferMemoryBarrier* barrier) const;
    void addImageMemoryBarrier(VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask,
                               bool byRegion,
                               VkImageMemoryBarrier* barrier) const;

    SkSL::Compiler* shaderCompiler() const {
        return fCompiler;
    }

    void onResolveRenderTarget(GrRenderTarget* target, GrSurfaceOrigin origin) override {
        this->internalResolveRenderTarget(target, origin, true);
    }

    void submitSecondaryCommandBuffer(const SkTArray<GrVkSecondaryCommandBuffer*>&,
                                      const GrVkRenderPass*,
                                      const VkClearValue* colorClear,
                                      GrVkRenderTarget*, GrSurfaceOrigin,
                                      const SkIRect& bounds);

    GrFence SK_WARN_UNUSED_RESULT insertFence() override;
    bool waitFence(GrFence, uint64_t timeout) override;
    void deleteFence(GrFence) const override;

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned) override;
    sk_sp<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrResourceProvider::SemaphoreWrapType wrapType,
                                            GrWrapOwnership ownership) override;
    void insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) override;
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override;

    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    void generateMipmap(GrVkTexture* tex, GrSurfaceOrigin texOrigin);

    void copyBuffer(GrVkBuffer* srcBuffer, GrVkBuffer* dstBuffer, VkDeviceSize srcOffset,
                    VkDeviceSize dstOffset, VkDeviceSize size);
    bool updateBuffer(GrVkBuffer* buffer, const void* src, VkDeviceSize offset, VkDeviceSize size);

    // Heaps
    enum Heap {
        kLinearImage_Heap = 0,
        // We separate out small (i.e., <= 16K) images to reduce fragmentation
        // in the main heap.
        kOptimalImage_Heap,
        kSmallOptimalImage_Heap,
        // We have separate vertex and image heaps, because it's possible that
        // a given Vulkan driver may allocate them separately.
        kVertexBuffer_Heap,
        kIndexBuffer_Heap,
        kUniformBuffer_Heap,
        kTexelBuffer_Heap,
        kCopyReadBuffer_Heap,
        kCopyWriteBuffer_Heap,

        kLastHeap = kCopyWriteBuffer_Heap
    };
    static const int kHeapCount = kLastHeap + 1;

    GrVkHeap* getHeap(Heap heap) const { return fHeaps[heap].get(); }

private:
    GrVkGpu(GrContext*, const GrContextOptions&, sk_sp<const GrVkBackendContext> backendContext);

    void onResetContext(uint32_t resetBits) override {}

    void destroyResources();

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                     const GrMipLevel texels[], int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override;

    GrBuffer* onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern,
                             const void* data) override;

    bool onReadPixels(GrSurface* surface, GrSurfaceOrigin,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface, GrSurfaceOrigin,
                       int left, int top, int width, int height,
                       GrPixelConfig config, const GrMipLevel texels[], int mipLevelCount) override;

    bool onTransferPixels(GrTexture*,
                          int left, int top, int width, int height,
                          GrPixelConfig config, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override;

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin, GrSurface* src,
                       GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
                       const SkIPoint& dstPoint, bool canDiscardOutsideDstRect) override;

    void onFinishFlush(bool insertedSemaphores) override;

    // Ends and submits the current command buffer to the queue and then creates a new command
    // buffer and begins it. If sync is set to kForce_SyncQueue, the function will wait for all
    // work in the queue to finish before returning. If this GrVkGpu object has any semaphores in
    // fSemaphoreToSignal, we will add those signal semaphores to the submission of this command
    // buffer. If this GrVkGpu object has any semaphores in fSemaphoresToWaitOn, we will add those
    // wait semaphores to the submission of this command buffer.
    void submitCommandBuffer(SyncQueue sync);

    void internalResolveRenderTarget(GrRenderTarget*, GrSurfaceOrigin origin, bool requiresSubmit);

    void copySurfaceAsCopyImage(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                GrSurface* src, GrSurfaceOrigin srcOrigin,
                                GrVkImage* dstImage, GrVkImage* srcImage,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint);

    void copySurfaceAsBlit(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                           GrSurface* src, GrSurfaceOrigin srcOrigin,
                           GrVkImage* dstImage, GrVkImage* srcImage,
                           const SkIRect& srcRect,
                           const SkIPoint& dstPoint);

    void copySurfaceAsResolve(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                              GrSurface* src, GrSurfaceOrigin srcOrigin,
                              const SkIRect& srcRect,
                              const SkIPoint& dstPoint);

    // helpers for onCreateTexture and writeTexturePixels
    bool uploadTexDataLinear(GrVkTexture* tex, GrSurfaceOrigin texOrigin,
                             int left, int top, int width, int height,
                             GrPixelConfig dataConfig,
                             const void* data,
                             size_t rowBytes);
    bool uploadTexDataOptimal(GrVkTexture* tex, GrSurfaceOrigin texOrigin,
                              int left, int top, int width, int height,
                              GrPixelConfig dataConfig,
                              const GrMipLevel texels[], int mipLevelCount);

    void resolveImage(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                      GrVkRenderTarget* src, GrSurfaceOrigin srcOrigin,
                      const SkIRect& srcRect, const SkIPoint& dstPoint);

    sk_sp<const GrVkBackendContext> fBackendContext;
    sk_sp<GrVkCaps>                 fVkCaps;

    // These Vulkan objects are provided by the client, and also stored in fBackendContext.
    // They're copied here for convenient access.
    VkDevice                                     fDevice;
    VkQueue                                      fQueue;    // Must be Graphics queue

    // Created by GrVkGpu
    GrVkResourceProvider                         fResourceProvider;
    VkCommandPool                                fCmdPool;

    GrVkPrimaryCommandBuffer*                    fCurrentCmdBuffer;

    SkSTArray<1, GrVkSemaphore::Resource*>       fSemaphoresToWaitOn;
    SkSTArray<1, GrVkSemaphore::Resource*>       fSemaphoresToSignal;

    VkPhysicalDeviceProperties                   fPhysDevProps;
    VkPhysicalDeviceMemoryProperties             fPhysDevMemProps;

    std::unique_ptr<GrVkHeap>                    fHeaps[kHeapCount];

    GrVkCopyManager                              fCopyManager;

#ifdef SK_ENABLE_VK_LAYERS
    // For reporting validation layer errors
    VkDebugReportCallbackEXT               fCallback;
#endif

    // compiler used for compiling sksl into spirv. We only want to create the compiler once since
    // there is significant overhead to the first compile of any compiler.
    SkSL::Compiler* fCompiler;

    // We need a bool to track whether or not we've already disconnected all the gpu resources from
    // vulkan context.
    bool fDisconnected;

    typedef GrGpu INHERITED;
};

#endif

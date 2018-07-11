/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkGpu_DEFINED
#define GrVkGpu_DEFINED

#include "GrGpu.h"
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
class GrVkMemoryAllocator;
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
    static sk_sp<GrGpu> Make(const GrVkBackendContext&, const GrContextOptions&, GrContext*);

    ~GrVkGpu() override;

    void disconnect(DisconnectType) override;

    const GrVkInterface* vkInterface() const { return fInterface.get(); }
    const GrVkCaps& vkCaps() const { return *fVkCaps; }

    GrVkMemoryAllocator* memoryAllocator() const { return fMemoryAllocator.get(); }

    VkDevice device() const { return fDevice; }
    VkQueue  queue() const { return fQueue; }
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

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

#if GR_TEST_UTILS
    GrBackendTexture createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                     GrPixelConfig config, bool isRenderTarget,
                                                     GrMipMapped) override;
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;
    void deleteTestingOnlyBackendTexture(const GrBackendTexture&) override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType,
                                                               GrSRGBEncoded) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override;
#endif

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

    bool onRegenerateMipMapLevels(GrTexture* tex) override;

    void onResolveRenderTarget(GrRenderTarget* target) override {
        this->internalResolveRenderTarget(target, true);
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

    void copyBuffer(GrVkBuffer* srcBuffer, GrVkBuffer* dstBuffer, VkDeviceSize srcOffset,
                    VkDeviceSize dstOffset, VkDeviceSize size);
    bool updateBuffer(GrVkBuffer* buffer, const void* src, VkDeviceSize offset, VkDeviceSize size);

private:
    GrVkGpu(GrContext*, const GrContextOptions&, const GrVkBackendContext& backendContext,
            sk_sp<const GrVkInterface>);

    void onResetContext(uint32_t resetBits) override {}

    void destroyResources();

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc&, SkBudgeted, const GrMipLevel[],
                                     int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override;

    GrBuffer* onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern,
                             const void* data) override;

    bool onReadPixels(GrSurface* surface, int left, int top, int width, int height, GrColorType,
                      void* buffer, size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface, int left, int top, int width, int height, GrColorType,
                       const GrMipLevel texels[], int mipLevelCount) override;

    bool onTransferPixels(GrTexture*, int left, int top, int width, int height, GrColorType,
                          GrBuffer* transferBuffer, size_t offset, size_t rowBytes) override;

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

    void internalResolveRenderTarget(GrRenderTarget*, bool requiresSubmit);

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
    bool uploadTexDataLinear(GrVkTexture* tex, int left, int top, int width, int height,
                             GrColorType colorType, const void* data, size_t rowBytes);
    bool uploadTexDataOptimal(GrVkTexture* tex, int left, int top, int width, int height,
                              GrColorType colorType, const GrMipLevel texels[], int mipLevelCount);

    void resolveImage(GrSurface* dst, GrVkRenderTarget* src, const SkIRect& srcRect,
                      const SkIPoint& dstPoint);

#if GR_TEST_UTILS
    bool createTestingOnlyVkImage(GrPixelConfig config, int w, int h, bool texturable,
                                  bool renderable, GrMipMapped mipMapped, const void* srcData,
                                  GrVkImageInfo* info);
#endif

    sk_sp<const GrVkInterface>             fInterface;
    sk_sp<GrVkMemoryAllocator>             fMemoryAllocator;
    sk_sp<GrVkCaps>                        fVkCaps;

    VkInstance                             fInstance;
    VkDevice                               fDevice;
    VkQueue                                fQueue;    // Must be Graphics queue

    // Created by GrVkGpu
    GrVkResourceProvider                   fResourceProvider;
    VkCommandPool                          fCmdPool;

    GrVkPrimaryCommandBuffer*              fCurrentCmdBuffer;

    SkSTArray<1, GrVkSemaphore::Resource*> fSemaphoresToWaitOn;
    SkSTArray<1, GrVkSemaphore::Resource*> fSemaphoresToSignal;

    VkPhysicalDeviceProperties             fPhysDevProps;
    VkPhysicalDeviceMemoryProperties       fPhysDevMemProps;

    GrVkCopyManager                        fCopyManager;

    // compiler used for compiling sksl into spirv. We only want to create the compiler once since
    // there is significant overhead to the first compile of any compiler.
    SkSL::Compiler*                        fCompiler;

    // We need a bool to track whether or not we've already disconnected all the gpu resources from
    // vulkan context.
    bool                                   fDisconnected;

    typedef GrGpu INHERITED;
};

#endif

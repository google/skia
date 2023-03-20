/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkGpu_DEFINED
#define GrVkGpu_DEFINED

#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrStagingBufferManager.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkMSAALoadManager.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"
#include "src/gpu/ganesh/vk/GrVkSemaphore.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"

class GrDirectContext;
class GrPipeline;
class GrVkBuffer;
class GrVkCommandPool;
class GrVkFramebuffer;
class GrVkOpsRenderPass;
class GrVkPipeline;
class GrVkPipelineState;
class GrVkPrimaryCommandBuffer;
class GrVkRenderPass;
class GrVkSecondaryCommandBuffer;
class GrVkTexture;
enum class SkTextureCompressionType;

namespace skgpu { struct VulkanInterface; }

namespace skgpu { class VulkanMemoryAllocator; }

class GrVkGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(const GrVkBackendContext&, const GrContextOptions&, GrDirectContext*);

    ~GrVkGpu() override;

    void disconnect(DisconnectType) override;
    bool disconnected() const { return fDisconnected; }

    void releaseUnlockedBackendObjects() override {
        fResourceProvider.releaseUnlockedBackendObjects();
    }

    GrThreadSafePipelineBuilder* pipelineBuilder() override;
    sk_sp<GrThreadSafePipelineBuilder> refPipelineBuilder() override;

    const skgpu::VulkanInterface* vkInterface() const { return fInterface.get(); }
    const GrVkCaps& vkCaps() const { return *fVkCaps; }

    GrStagingBufferManager* stagingBufferManager() override { return &fStagingBufferManager; }
    void takeOwnershipOfBuffer(sk_sp<GrGpuBuffer>) override;

    bool isDeviceLost() const override { return fDeviceIsLost; }

    skgpu::VulkanMemoryAllocator* memoryAllocator() const { return fMemoryAllocator.get(); }

    VkPhysicalDevice physicalDevice() const { return fPhysicalDevice; }
    VkDevice device() const { return fDevice; }
    VkQueue  queue() const { return fQueue; }
    uint32_t  queueIndex() const { return fQueueIndex; }
    GrVkCommandPool* cmdPool() const { return fMainCmdPool; }
    const VkPhysicalDeviceProperties& physicalDeviceProperties() const {
        return fPhysDevProps;
    }
    const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties() const {
        return fPhysDevMemProps;
    }
    bool protectedContext() const { return fProtectedContext == skgpu::Protected::kYes; }

    GrVkResourceProvider& resourceProvider() { return fResourceProvider; }

    GrVkPrimaryCommandBuffer* currentCommandBuffer() const { return fMainCmdBuffer; }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override;

    bool setBackendTextureState(const GrBackendTexture&,
                                const skgpu::MutableTextureState&,
                                skgpu::MutableTextureState* previousState,
                                sk_sp<skgpu::RefCntedCallback> finishedCallback) override;

    bool setBackendRenderTargetState(const GrBackendRenderTarget&,
                                     const skgpu::MutableTextureState&,
                                     skgpu::MutableTextureState* previousState,
                                     sk_sp<skgpu::RefCntedCallback> finishedCallback) override;

    void deleteBackendTexture(const GrBackendTexture&) override;

    bool compile(const GrProgramDesc&, const GrProgramInfo&) override;

#if GR_TEST_UTILS
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                               GrColorType,
                                                               int sampleCnt,
                                                               GrProtected) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void resetShaderCacheForTesting() const override {
        fResourceProvider.resetShaderCacheForTesting();
    }
#endif

    sk_sp<GrAttachment> makeStencilAttachment(const GrBackendFormat& /*colorFormat*/,
                                              SkISize dimensions, int numStencilSamples) override;

    GrBackendFormat getPreferredStencilFormat(const GrBackendFormat&) override {
        return GrBackendFormat::MakeVk(this->vkCaps().preferredStencilFormat());
    }

    sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           int numSamples,
                                           GrProtected isProtected,
                                           GrMemoryless isMemoryless) override;

    void addBufferMemoryBarrier(const GrManagedResource*,
                                VkPipelineStageFlags srcStageMask,
                                VkPipelineStageFlags dstStageMask,
                                bool byRegion,
                                VkBufferMemoryBarrier* barrier) const;
    void addBufferMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                VkPipelineStageFlags dstStageMask,
                                bool byRegion,
                                VkBufferMemoryBarrier* barrier) const;
    void addImageMemoryBarrier(const GrManagedResource*,
                               VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask,
                               bool byRegion,
                               VkImageMemoryBarrier* barrier) const;

    bool loadMSAAFromResolve(GrVkCommandBuffer* commandBuffer,
                             const GrVkRenderPass& renderPass,
                             GrAttachment* dst,
                             GrVkImage* src,
                             const SkIRect& srcRect);

    bool onRegenerateMipMapLevels(GrTexture* tex) override;

    void onResolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect) override;

    void submitSecondaryCommandBuffer(std::unique_ptr<GrVkSecondaryCommandBuffer>);

    void submit(GrOpsRenderPass*) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override;
    bool waitFence(GrFence) override;
    void deleteFence(GrFence) override;

    std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned) override;
    std::unique_ptr<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore&,
                                                      GrSemaphoreWrapType,
                                                      GrWrapOwnership) override;
    void insertSemaphore(GrSemaphore* semaphore) override;
    void waitSemaphore(GrSemaphore* semaphore) override;

    // These match the definitions in SkDrawable, from whence they came
    typedef void* SubmitContext;
    typedef void (*SubmitProc)(SubmitContext submitContext);

    // Adds an SkDrawable::GpuDrawHandler that we will delete the next time we submit the primary
    // command buffer to the gpu.
    void addDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable);

    void checkFinishProcs() override { fResourceProvider.checkCommandBuffers(); }
    void finishOutstandingGpuWork() override;

    std::unique_ptr<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    bool updateBuffer(sk_sp<GrVkBuffer> buffer, const void* src, VkDeviceSize offset,
                      VkDeviceSize size);

    bool zeroBuffer(sk_sp<GrGpuBuffer>);

    enum PersistentCacheKeyType : uint32_t {
        kShader_PersistentCacheKeyType = 0,
        kPipelineCache_PersistentCacheKeyType = 1,
    };

    void storeVkPipelineCacheData() override;

    bool beginRenderPass(const GrVkRenderPass*,
                         sk_sp<const GrVkFramebuffer>,
                         const VkClearValue* colorClear,
                         const GrSurface*,
                         const SkIRect& renderPassBounds,
                         bool forSecondaryCB);
    void endRenderPass(GrRenderTarget* target, GrSurfaceOrigin origin, const SkIRect& bounds);

    // Returns true if VkResult indicates success and also checks for device lost or OOM. Every
    // Vulkan call (and skgpu::VulkanMemoryAllocator call that returns VkResult) made on behalf of
    // the GrVkGpu should be processed by this function so that we respond to OOMs and lost devices.
    bool checkVkResult(VkResult);

private:
    enum SyncQueue {
        kForce_SyncQueue,
        kSkip_SyncQueue
    };

    GrVkGpu(GrDirectContext*,
            const GrVkBackendContext&,
            const sk_sp<GrVkCaps> caps,
            sk_sp<const skgpu::VulkanInterface>,
            uint32_t instanceVersion,
            uint32_t physicalDeviceVersion,
            sk_sp<skgpu::VulkanMemoryAllocator>);

    void destroyResources();

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            GrMipmapped,
                                            GrProtected,
                                            std::string_view label) override;
    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      GrMipmapped,
                                                      GrProtected) override;

    bool onClearBackendTexture(const GrBackendTexture&,
                               sk_sp<skgpu::RefCntedCallback> finishedCallback,
                               std::array<float, 4> color) override;

    bool onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                          sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                          const void* data,
                                          size_t length) override;

    bool setBackendSurfaceState(GrVkImageInfo info,
                                sk_sp<skgpu::MutableTextureStateRef> currentState,
                                SkISize dimensions,
                                const skgpu::VulkanMutableTextureState& newState,
                                skgpu::MutableTextureState* previousState,
                                sk_sp<skgpu::RefCntedCallback> finishedCallback);

    sk_sp<GrTexture> onCreateTexture(SkISize,
                                     const GrBackendFormat&,
                                     GrRenderable,
                                     int renderTargetSampleCnt,
                                     skgpu::Budgeted,
                                     GrProtected,
                                     int mipLevelCount,
                                     uint32_t levelClearMask,
                                     std::string_view label) override;
    sk_sp<GrTexture> onCreateCompressedTexture(SkISize dimensions,
                                               const GrBackendFormat&,
                                               skgpu::Budgeted,
                                               GrMipmapped,
                                               GrProtected,
                                               const void* data,
                                               size_t dataSize) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&,
                                          GrWrapOwnership,
                                          GrWrapCacheable,
                                          GrIOType) override;
    sk_sp<GrTexture> onWrapCompressedBackendTexture(const GrBackendTexture&,
                                                    GrWrapOwnership,
                                                    GrWrapCacheable) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership,
                                                    GrWrapCacheable) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrRenderTarget> onWrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo&,
                                                                const GrVkDrawableInfo&) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t size, GrGpuBufferType type, GrAccessPattern) override;

    bool onReadPixels(GrSurface*,
                      SkIRect,
                      GrColorType surfaceColorType,
                      GrColorType dstColorType,
                      void* buffer,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface*,
                       SkIRect,
                       GrColorType surfaceColorType,
                       GrColorType srcColorType,
                       const GrMipLevel[],
                       int mipLevelCount,
                       bool prepForTexSampling) override;

    bool onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                      size_t srcOffset,
                                      sk_sp<GrGpuBuffer> dst,
                                      size_t dstOffset,
                                      size_t size) override;

    bool onTransferPixelsTo(GrTexture*,
                            SkIRect,
                            GrColorType textureColorType,
                            GrColorType bufferColorType,
                            sk_sp<GrGpuBuffer>,
                            size_t offset,
                            size_t rowBytes) override;

    bool onTransferPixelsFrom(GrSurface*,
                              SkIRect,
                              GrColorType surfaceColorType,
                              GrColorType bufferColorType,
                              sk_sp<GrGpuBuffer>,
                              size_t offset) override;

    bool onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                       GrSurface* src, const SkIRect& srcRect,
                       GrSamplerState::Filter) override;

    void addFinishedProc(GrGpuFinishedProc finishedProc,
                         GrGpuFinishedContext finishedContext) override;

    void addFinishedCallback(sk_sp<skgpu::RefCntedCallback> finishedCallback);

    GrOpsRenderPass* onGetOpsRenderPass(GrRenderTarget*,
                                        bool useMSAASurface,
                                        GrAttachment* stencil,
                                        GrSurfaceOrigin,
                                        const SkIRect&,
                                        const GrOpsRenderPass::LoadAndStoreInfo&,
                                        const GrOpsRenderPass::StencilLoadAndStoreInfo&,
                                        const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
                                        GrXferBarrierFlags renderPassXferBarriers) override;

    void prepareSurfacesForBackendAccessAndStateUpdates(
            SkSpan<GrSurfaceProxy*> proxies,
            SkSurface::BackendSurfaceAccess access,
            const skgpu::MutableTextureState* newState) override;

    bool onSubmitToGpu(bool syncCpu) override;

    void onReportSubmitHistograms() override;

    // Ends and submits the current command buffer to the queue and then creates a new command
    // buffer and begins it. If sync is set to kForce_SyncQueue, the function will wait for all
    // work in the queue to finish before returning. If this GrVkGpu object has any semaphores in
    // fSemaphoreToSignal, we will add those signal semaphores to the submission of this command
    // buffer. If this GrVkGpu object has any semaphores in fSemaphoresToWaitOn, we will add those
    // wait semaphores to the submission of this command buffer.
    bool submitCommandBuffer(SyncQueue sync);

    void copySurfaceAsCopyImage(GrSurface* dst,
                                GrSurface* src,
                                GrVkImage* dstImage,
                                GrVkImage* srcImage,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint);

    void copySurfaceAsBlit(GrSurface* dst,
                           GrSurface* src,
                           GrVkImage* dstImage,
                           GrVkImage* srcImage,
                           const SkIRect& srcRect,
                           const SkIRect& dstRect,
                           GrSamplerState::Filter filter);

    void copySurfaceAsResolve(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                              const SkIPoint& dstPoint);

    // helpers for onCreateTexture and writeTexturePixels
    bool uploadTexDataLinear(GrVkImage* tex,
                             SkIRect rect,
                             GrColorType colorType,
                             const void* data,
                             size_t rowBytes);
    bool uploadTexDataOptimal(GrVkImage* tex,
                              SkIRect rect,
                              GrColorType colorType,
                              const GrMipLevel texels[],
                              int mipLevelCount);
    bool uploadTexDataCompressed(GrVkImage* tex, SkTextureCompressionType compression,
                                 VkFormat vkFormat, SkISize dimensions, GrMipmapped mipmapped,
                                 const void* data, size_t dataSize);
    void resolveImage(GrSurface* dst, GrVkRenderTarget* src, const SkIRect& srcRect,
                      const SkIPoint& dstPoint);

    bool createVkImageForBackendSurface(VkFormat,
                                        SkISize dimensions,
                                        int sampleCnt,
                                        GrTexturable,
                                        GrRenderable,
                                        GrMipmapped,
                                        GrVkImageInfo*,
                                        GrProtected);

    sk_sp<const skgpu::VulkanInterface>                   fInterface;
    sk_sp<skgpu::VulkanMemoryAllocator>                   fMemoryAllocator;
    sk_sp<GrVkCaps>                                       fVkCaps;
    bool                                                  fDeviceIsLost = false;

    VkPhysicalDevice                                      fPhysicalDevice;
    VkDevice                                              fDevice;
    VkQueue                                               fQueue;    // Must be Graphics queue
    uint32_t                                              fQueueIndex;

    // Created by GrVkGpu
    GrVkResourceProvider                                  fResourceProvider;
    GrStagingBufferManager                                fStagingBufferManager;

    GrVkMSAALoadManager                                   fMSAALoadManager;

    GrVkCommandPool*                                      fMainCmdPool;
    // just a raw pointer; object's lifespan is managed by fCmdPool
    GrVkPrimaryCommandBuffer*                             fMainCmdBuffer;

    SkSTArray<1, GrVkSemaphore::Resource*>                fSemaphoresToWaitOn;
    SkSTArray<1, GrVkSemaphore::Resource*>                fSemaphoresToSignal;

    SkTArray<std::unique_ptr<SkDrawable::GpuDrawHandler>> fDrawables;

    VkPhysicalDeviceProperties                            fPhysDevProps;
    VkPhysicalDeviceMemoryProperties                      fPhysDevMemProps;

    // We need a bool to track whether or not we've already disconnected all the gpu resources from
    // vulkan context.
    bool                                                  fDisconnected;

    skgpu::Protected                                      fProtectedContext;

    std::unique_ptr<GrVkOpsRenderPass>                    fCachedOpsRenderPass;

    using INHERITED = GrGpu;
};

#endif

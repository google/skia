/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkGpu_DEFINED
#define GrVkGpu_DEFINED

#include "include/core/SkDrawable.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrOpsRenderPass.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrStagingBufferManager.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkMSAALoadManager.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"
#include "src/gpu/ganesh/vk/GrVkSemaphore.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

class GrAttachment;
class GrBackendSemaphore;
class GrDirectContext;
class GrGpuBuffer;
class GrManagedResource;
class GrProgramDesc;
class GrProgramInfo;
class GrRenderTarget;
class GrSemaphore;
class GrSurface;
class GrSurfaceProxy;
class GrTexture;
class GrThreadSafePipelineBuilder;
class GrVkBuffer;
class GrVkCommandBuffer;
class GrVkCommandPool;
class GrVkFramebuffer;
class GrVkImage;
class GrVkOpsRenderPass;
class GrVkPrimaryCommandBuffer;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkSecondaryCommandBuffer;
enum class SkTextureCompressionType;
struct GrContextOptions;
struct GrVkDrawableInfo;
struct GrVkImageInfo;
struct SkIPoint;
struct SkIRect;
struct SkISize;
struct SkImageInfo;

namespace SkSurfaces {
enum class BackendSurfaceAccess;
}

namespace skgpu {
class MutableTextureState;
class VulkanMemoryAllocator;
struct VulkanBackendContext;
struct VulkanInterface;
}  // namespace skgpu

class GrVkGpu : public GrGpu {
public:
    static std::unique_ptr<GrGpu> Make(const skgpu::VulkanBackendContext&,
                                       const GrContextOptions&,
                                       GrDirectContext*);

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

#if defined(GPU_TEST_UTILS)
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
        return GrBackendFormats::MakeVk(this->vkCaps().preferredStencilFormat());
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

    [[nodiscard]] std::unique_ptr<GrSemaphore> makeSemaphore(bool isOwned) override;
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

    void checkFinishedCallbacks() override { fResourceProvider.checkCommandBuffers(); }
    void finishOutstandingGpuWork() override;

    std::unique_ptr<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    bool updateBuffer(sk_sp<GrVkBuffer> buffer, const void* src, VkDeviceSize offset,
                      VkDeviceSize size);

    bool zeroBuffer(sk_sp<GrGpuBuffer>);

    enum PersistentCacheKeyType : uint32_t {
        kShader_PersistentCacheKeyType = 0,
        kPipelineCache_PersistentCacheKeyType = 1,
    };

    void pipelineCompileWasRequired();
    bool canDetectNewVkPipelineCacheData() const override;
    bool hasNewVkPipelineCacheData() const override;
    void storeVkPipelineCacheData(size_t maxSize) override;

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
    GrVkGpu(GrDirectContext*,
            const skgpu::VulkanBackendContext&,
            const sk_sp<GrVkCaps> caps,
            sk_sp<const skgpu::VulkanInterface>,
            uint32_t instanceVersion,
            uint32_t physicalDeviceVersion,
            sk_sp<skgpu::VulkanMemoryAllocator>);

    void destroyResources();

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            skgpu::Mipmapped,
                                            GrProtected,
                                            std::string_view label) override;
    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      skgpu::Mipmapped,
                                                      GrProtected) override;

    bool onClearBackendTexture(const GrBackendTexture&,
                               sk_sp<skgpu::RefCntedCallback> finishedCallback,
                               std::array<float, 4> color) override;

    bool onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                          sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                          const void* data,
                                          size_t length) override;

    bool setBackendSurfaceState(GrVkImageInfo info,
                                sk_sp<skgpu::MutableTextureState> currentState,
                                SkISize dimensions,
                                VkImageLayout newLayout,
                                uint32_t newQueueFamilyIndex,
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
                                               skgpu::Mipmapped,
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

    void addFinishedCallback(skgpu::AutoCallback callback,
                             std::optional<GrTimerQuery> timerQuery) override {
        SkASSERT(!timerQuery);
        this->addFinishedCallback(skgpu::RefCntedCallback::Make(std::move(callback)));
    }

    void addFinishedCallback(sk_sp<skgpu::RefCntedCallback> finishedCallback);

    GrOpsRenderPass* onGetOpsRenderPass(GrRenderTarget*,
                                        bool useMSAASurface,
                                        GrAttachment* stencil,
                                        GrSurfaceOrigin,
                                        const SkIRect&,
                                        const GrOpsRenderPass::LoadAndStoreInfo&,
                                        const GrOpsRenderPass::StencilLoadAndStoreInfo&,
                                        const skia_private::TArray<GrSurfaceProxy*, true>& sampledProxies,
                                        GrXferBarrierFlags renderPassXferBarriers) override;

    void prepareSurfacesForBackendAccessAndStateUpdates(
            SkSpan<GrSurfaceProxy*> proxies,
            SkSurfaces::BackendSurfaceAccess access,
            const skgpu::MutableTextureState* newState) override;

    bool onSubmitToGpu(const GrSubmitInfo& info) override;

    void onReportSubmitHistograms() override;

    // Ends and submits the current command buffer to the queue and then creates a new command
    // buffer and begins it. If fSync in the submitInfo is set to GrSyncCpu::kYes, the function will
    // wait for all work in the queue to finish before returning. If this GrVkGpu object has any
    // semaphores in fSemaphoreToSignal, we will add those signal semaphores to the submission of
    // this command buffer. If this GrVkGpu object has any semaphores in fSemaphoresToWaitOn, we
    // will add those wait semaphores to the submission of this command buffer.
    //
    // If fMarkBoundary in submitInfo is GrMarkFrameBoundary::kYes, then we will mark the end of a
    // frame if the VK_EXT_frame_boundary extension is available.
    bool submitCommandBuffer(const GrSubmitInfo& submitInfo);

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
    bool uploadTexDataCompressed(GrVkImage* tex,
                                 SkTextureCompressionType compression,
                                 VkFormat vkFormat,
                                 SkISize dimensions,
                                 skgpu::Mipmapped mipmapped,
                                 const void* data,
                                 size_t dataSize);
    void resolveImage(GrSurface* dst, GrVkRenderTarget* src, const SkIRect& srcRect,
                      const SkIPoint& dstPoint);

    bool createVkImageForBackendSurface(VkFormat,
                                        SkISize dimensions,
                                        int sampleCnt,
                                        GrTexturable,
                                        GrRenderable,
                                        skgpu::Mipmapped,
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

    skia_private::STArray<1, GrVkSemaphore::Resource*>    fSemaphoresToWaitOn;
    skia_private::STArray<1, GrVkSemaphore::Resource*>    fSemaphoresToSignal;

    skia_private::TArray<std::unique_ptr<SkDrawable::GpuDrawHandler>> fDrawables;

    VkPhysicalDeviceProperties                            fPhysDevProps;
    VkPhysicalDeviceMemoryProperties                      fPhysDevMemProps;

    // We need a bool to track whether or not we've already disconnected all the gpu resources from
    // vulkan context.
    bool                                                  fDisconnected;

    bool                                                  fHasNewVkPipelineCacheData;

    skgpu::Protected                                      fProtectedContext;

    std::unique_ptr<GrVkOpsRenderPass>                    fCachedOpsRenderPass;

    skgpu::VulkanDeviceLostContext                        fDeviceLostContext;
    skgpu::VulkanDeviceLostProc                           fDeviceLostProc;

    using INHERITED = GrGpu;
};

#endif

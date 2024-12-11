/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkImage_DEFINED
#define GrVkImage_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/ganesh/vk/GrVkTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/GpuRefCnt.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrManagedResource.h"
#include "src/gpu/ganesh/vk/GrVkDescriptorSet.h"  // IWYU pragma: keep
#include "src/gpu/vk/VulkanMutableTextureStatePriv.h"

#include <cinttypes>
#include <cstdint>
#include <string_view>

class GrVkGpu;
class GrVkImageView;
struct SkISize;

class GrVkImage : public GrAttachment {
private:
    class Resource;

public:
    static sk_sp<GrVkImage> MakeStencil(GrVkGpu* gpu,
                                        SkISize dimensions,
                                        int sampleCnt,
                                        VkFormat format);

    static sk_sp<GrVkImage> MakeMSAA(GrVkGpu* gpu,
                                     SkISize dimensions,
                                     int numSamples,
                                     VkFormat format,
                                     GrProtected isProtected,
                                     GrMemoryless memoryless);

    static sk_sp<GrVkImage> MakeTexture(GrVkGpu* gpu,
                                        SkISize dimensions,
                                        VkFormat format,
                                        uint32_t mipLevels,
                                        GrRenderable renderable,
                                        int numSamples,
                                        skgpu::Budgeted budgeted,
                                        GrProtected isProtected);

    static sk_sp<GrVkImage> MakeWrapped(GrVkGpu* gpu,
                                        SkISize dimensions,
                                        const GrVkImageInfo&,
                                        sk_sp<skgpu::MutableTextureState>,
                                        UsageFlags attachmentUsages,
                                        GrWrapOwnership,
                                        GrWrapCacheable,
                                        std::string_view label,
                                        bool forSecondaryCB = false);

    ~GrVkImage() override;

    VkImage image() const {
        // Should only be called when we have a real fResource object, i.e. never when being used as
        // a RT in an external secondary command buffer.
        SkASSERT(fResource);
        return fInfo.fImage;
    }
    const skgpu::VulkanAlloc& alloc() const {
        // Should only be called when we have a real fResource object, i.e. never when being used as
        // a RT in an external secondary command buffer.
        SkASSERT(fResource);
        return fInfo.fAlloc;
    }
    const GrVkImageInfo& vkImageInfo() const { return fInfo; }
    VkFormat imageFormat() const { return fInfo.fFormat; }
    GrBackendFormat backendFormat() const override {
        bool usesDRMModifier =
                this->vkImageInfo().fImageTiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
        if (fResource && this->ycbcrConversionInfo().isValid()) {
            SkASSERT(this->imageFormat() == this->ycbcrConversionInfo().fFormat);
            return GrBackendFormats::MakeVk(this->ycbcrConversionInfo(), usesDRMModifier);
        }
        SkASSERT(this->imageFormat() != VK_FORMAT_UNDEFINED);
        return GrBackendFormats::MakeVk(this->imageFormat(), usesDRMModifier);
    }
    uint32_t mipLevels() const { return fInfo.fLevelCount; }
    const skgpu::VulkanYcbcrConversionInfo& ycbcrConversionInfo() const {
        // Should only be called when we have a real fResource object, i.e. never when being used as
        // a RT in an external secondary command buffer.
        SkASSERT(fResource);
        return fInfo.fYcbcrConversionInfo;
    }
    VkImageUsageFlags vkUsageFlags() { return fInfo.fImageUsageFlags; }
    bool supportsInputAttachmentUsage() const {
        return fInfo.fImageUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    const GrVkImageView* framebufferView() const { return fFramebufferView.get(); }
    const GrVkImageView* textureView() const { return fTextureView.get(); }

    // So that we don't need to rewrite descriptor sets each time, we keep cached input descriptor
    // sets on the attachment and simply reuse those descriptor sets for this attachment only. These
    // calls will fail if the attachment does not support being used as an input attachment. These
    // calls do not ref the GrVkDescriptorSet so they called will need to manually ref them if they
    // need to be kept alive.
    gr_rp<const GrVkDescriptorSet> inputDescSetForBlending(GrVkGpu* gpu);
    // Input descripotr set used when needing to read a resolve attachment to load data into a
    // discardable msaa attachment.
    gr_rp<const GrVkDescriptorSet> inputDescSetForMSAALoad(GrVkGpu* gpu);

    const Resource* resource() const {
        SkASSERT(fResource);
        return fResource;
    }
    bool isLinearTiled() const {
        // Should only be called when we have a real fResource object, i.e. never when being used as
        // a RT in an external secondary command buffer.
        SkASSERT(fResource);
        return SkToBool(VK_IMAGE_TILING_LINEAR == fInfo.fImageTiling);
    }
    bool isBorrowed() const { return fIsBorrowed; }

    sk_sp<skgpu::MutableTextureState> getMutableState() const { return fMutableState; }

    VkImageLayout currentLayout() const {
        return skgpu::MutableTextureStates::GetVkImageLayout(fMutableState.get());
    }

    void setImageLayoutAndQueueIndex(const GrVkGpu* gpu,
                                     VkImageLayout newLayout,
                                     VkAccessFlags dstAccessMask,
                                     VkPipelineStageFlags dstStageMask,
                                     bool byRegion,
                                     uint32_t newQueueFamilyIndex);

    void setImageLayout(const GrVkGpu* gpu,
                        VkImageLayout newLayout,
                        VkAccessFlags dstAccessMask,
                        VkPipelineStageFlags dstStageMask,
                        bool byRegion) {
        this->setImageLayoutAndQueueIndex(gpu, newLayout, dstAccessMask, dstStageMask, byRegion,
                                          VK_QUEUE_FAMILY_IGNORED);
    }

    uint32_t currentQueueFamilyIndex() const {
        return skgpu::MutableTextureStates::GetVkQueueFamilyIndex(fMutableState.get());
    }

    void setQueueFamilyIndex(uint32_t queueFamilyIndex) {
        skgpu::MutableTextureStates::SetVkQueueFamilyIndex(fMutableState.get(), queueFamilyIndex);
    }

    // Returns the image to its original queue family and changes the layout to present if the queue
    // family is not external or foreign.
    void prepareForPresent(GrVkGpu* gpu);

    // Returns the image to its original queue family
    void prepareForExternal(GrVkGpu* gpu);

    // This simply updates our tracking of the image layout and does not actually do any gpu work.
    // This is only used for mip map generation where we are manually changing the layouts as we
    // blit each layer, and then at the end need to update our tracking.
    void updateImageLayout(VkImageLayout newLayout) {
        // Should only be called when we have a real fResource object, i.e. never when being used as
        // a RT in an external secondary command buffer.
        SkASSERT(fResource);
        skgpu::MutableTextureStates::SetVkImageLayout(fMutableState.get(), newLayout);
    }

    struct ImageDesc {
        VkImageType         fImageType;
        VkFormat            fFormat;
        uint32_t            fWidth;
        uint32_t            fHeight;
        uint32_t            fLevels;
        uint32_t            fSamples;
        VkImageTiling       fImageTiling;
        VkImageUsageFlags   fUsageFlags;
        VkFlags             fMemProps;
        GrProtected         fIsProtected;

        ImageDesc()
                : fImageType(VK_IMAGE_TYPE_2D)
                , fFormat(VK_FORMAT_UNDEFINED)
                , fWidth(0)
                , fHeight(0)
                , fLevels(1)
                , fSamples(1)
                , fImageTiling(VK_IMAGE_TILING_OPTIMAL)
                , fUsageFlags(0)
                , fMemProps(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                , fIsProtected(GrProtected::kNo) {}
    };

    static bool InitImageInfo(GrVkGpu* gpu, const ImageDesc& imageDesc, GrVkImageInfo*);
    // Destroys the internal VkImage and VkDeviceMemory in the GrVkImageInfo
    static void DestroyImageInfo(const GrVkGpu* gpu, GrVkImageInfo*);

    // These match the definitions in SkImage, for whence they came
    typedef void* ReleaseCtx;
    typedef void (*ReleaseProc)(ReleaseCtx);

    void setResourceRelease(sk_sp<RefCntedReleaseProc> releaseHelper);

    // Helpers to use for setting the layout of the VkImage
    static VkPipelineStageFlags LayoutToPipelineSrcStageFlags(const VkImageLayout layout);
    static VkAccessFlags LayoutToSrcAccessMask(const VkImageLayout layout);

#if defined(GPU_TEST_UTILS)
    void setCurrentQueueFamilyToGraphicsQueue(GrVkGpu* gpu);
#endif

private:
    static sk_sp<GrVkImage> Make(GrVkGpu* gpu,
                                 SkISize dimensions,
                                 UsageFlags attachmentUsages,
                                 int sampleCnt,
                                 VkFormat format,
                                 uint32_t mipLevels,
                                 VkImageUsageFlags vkUsageFlags,
                                 GrProtected isProtected,
                                 GrMemoryless,
                                 skgpu::Budgeted);

    GrVkImage(GrVkGpu* gpu,
              SkISize dimensions,
              UsageFlags supportedUsages,
              const GrVkImageInfo&,
              sk_sp<skgpu::MutableTextureState> mutableState,
              sk_sp<const GrVkImageView> framebufferView,
              sk_sp<const GrVkImageView> textureView,
              skgpu::Budgeted,
              std::string_view label);

    GrVkImage(GrVkGpu* gpu,
              SkISize dimensions,
              UsageFlags supportedUsages,
              const GrVkImageInfo&,
              sk_sp<skgpu::MutableTextureState> mutableState,
              sk_sp<const GrVkImageView> framebufferView,
              sk_sp<const GrVkImageView> textureView,
              GrBackendObjectOwnership,
              GrWrapCacheable,
              bool forSecondaryCB,
              std::string_view label);

    void init(GrVkGpu*, bool forSecondaryCB);

    void onRelease() override;
    void onAbandon() override;

    void releaseImage();
    bool hasResource() const { return fResource; }

    GrVkGpu* getVkGpu() const;

    GrVkImageInfo                        fInfo;
    uint32_t                             fInitialQueueFamily;
    sk_sp<skgpu::MutableTextureState> fMutableState;

    sk_sp<const GrVkImageView>           fFramebufferView;
    sk_sp<const GrVkImageView>           fTextureView;

    bool fIsBorrowed;

    // Descriptor set used when this is used as an input attachment for reading the dst in blending.
    gr_rp<const GrVkDescriptorSet> fCachedBlendingInputDescSet;
    // Descriptor set used when this is used as an input attachment for loading an msaa attachment.
    gr_rp<const GrVkDescriptorSet> fCachedMSAALoadInputDescSet;

    class Resource : public GrTextureResource {
    public:
        explicit Resource(const GrVkGpu* gpu)
                : fGpu(gpu)
                , fImage(VK_NULL_HANDLE) {
            fAlloc.fMemory = VK_NULL_HANDLE;
            fAlloc.fOffset = 0;
        }

        Resource(const GrVkGpu* gpu,
                 VkImage image,
                 const skgpu::VulkanAlloc& alloc,
                 VkImageTiling tiling)
            : fGpu(gpu)
            , fImage(image)
            , fAlloc(alloc) {}

        ~Resource() override {}

#ifdef SK_TRACE_MANAGED_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrVkImage: %" PRIdPTR " (%d refs)\n", (intptr_t)fImage, this->getRefCnt());
        }
#endif

#ifdef SK_DEBUG
        const GrManagedResource* asVkImageResource() const override { return this; }
#endif

    private:
        void freeGPUData() const override;

        const GrVkGpu*     fGpu;
        VkImage            fImage;
        skgpu::VulkanAlloc fAlloc;

        using INHERITED = GrTextureResource;
    };

    // for wrapped textures
    class BorrowedResource : public Resource {
    public:
        BorrowedResource(const GrVkGpu* gpu, VkImage image, const skgpu::VulkanAlloc& alloc,
                         VkImageTiling tiling)
            : Resource(gpu, image, alloc, tiling) {
        }
    private:
        void freeGPUData() const override;
    };

    Resource* fResource;

    friend class GrVkRenderTarget;
};

#endif

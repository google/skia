/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkImage_DEFINED
#define GrVkImage_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrManagedResource.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/vk/GrVkImageLayout.h"

class GrVkGpu;
class GrVkTexture;

class GrVkImage : SkNoncopyable {
private:
    class Resource;

public:
    GrVkImage(const GrVkGpu* gpu, const GrVkImageInfo& info, sk_sp<GrVkImageLayout> layout,
              GrBackendObjectOwnership ownership, bool forSecondaryCB = false)
            : fInfo(info)
            , fInitialQueueFamily(info.fCurrentQueueFamily)
            , fLayout(std::move(layout))
            , fIsBorrowed(GrBackendObjectOwnership::kBorrowed == ownership) {
        SkASSERT(fLayout->getImageLayout() == fInfo.fImageLayout);
        if (forSecondaryCB) {
            fResource = nullptr;
        } else if (fIsBorrowed) {
            fResource = new BorrowedResource(gpu, info.fImage, info.fAlloc, info.fImageTiling);
        } else {
            SkASSERT(VK_NULL_HANDLE != info.fAlloc.fMemory);
            fResource = new Resource(gpu, info.fImage, info.fAlloc, info.fImageTiling);
        }
    }
    virtual ~GrVkImage();

    VkImage image() const {
        // Should only be called when we have a real fResource object, i.e. never when being used as
        // a RT in an external secondary command buffer.
        SkASSERT(fResource);
        return fInfo.fImage;
    }
    const GrVkAlloc& alloc() const {
        // Should only be called when we have a real fResource object, i.e. never when being used as
        // a RT in an external secondary command buffer.
        SkASSERT(fResource);
        return fInfo.fAlloc;
    }
    VkFormat imageFormat() const { return fInfo.fFormat; }
    GrBackendFormat getBackendFormat() const {
        if (fResource && this->ycbcrConversionInfo().isValid()) {
            SkASSERT(this->imageFormat() == this->ycbcrConversionInfo().fFormat);
            return GrBackendFormat::MakeVk(this->ycbcrConversionInfo());
        }
        SkASSERT(this->imageFormat() != VK_FORMAT_UNDEFINED);
        return GrBackendFormat::MakeVk(this->imageFormat());
    }
    uint32_t mipLevels() const { return fInfo.fLevelCount; }
    const GrVkYcbcrConversionInfo& ycbcrConversionInfo() const {
        // Should only be called when we have a real fResource object, i.e. never when being used as
        // a RT in an external secondary command buffer.
        SkASSERT(fResource);
        return fInfo.fYcbcrConversionInfo;
    }
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

    sk_sp<GrVkImageLayout> grVkImageLayout() const { return fLayout; }

    VkImageLayout currentLayout() const {
        return fLayout->getImageLayout();
    }

    void setImageLayout(const GrVkGpu* gpu,
                        VkImageLayout newLayout,
                        VkAccessFlags dstAccessMask,
                        VkPipelineStageFlags dstStageMask,
                        bool byRegion,
                        bool releaseFamilyQueue = false);

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
        fLayout->setImageLayout(newLayout);
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

    void setResourceRelease(sk_sp<GrRefCntedCallback> releaseHelper);

    // Helpers to use for setting the layout of the VkImage
    static VkPipelineStageFlags LayoutToPipelineSrcStageFlags(const VkImageLayout layout);
    static VkAccessFlags LayoutToSrcAccessMask(const VkImageLayout layout);

#if GR_TEST_UTILS
    void setCurrentQueueFamilyToGraphicsQueue(GrVkGpu* gpu);
#endif

protected:
    void releaseImage(GrVkGpu* gpu);
    bool hasResource() const { return fResource; }

    GrVkImageInfo          fInfo;
    uint32_t               fInitialQueueFamily;
    sk_sp<GrVkImageLayout> fLayout;
    bool                   fIsBorrowed;

private:
    class Resource : public GrTextureResource {
    public:
        explicit Resource(const GrVkGpu* gpu)
                : fGpu(gpu)
                , fImage(VK_NULL_HANDLE) {
            fAlloc.fMemory = VK_NULL_HANDLE;
            fAlloc.fOffset = 0;
        }

        Resource(const GrVkGpu* gpu, VkImage image, const GrVkAlloc& alloc, VkImageTiling tiling)
            : fGpu(gpu)
            , fImage(image)
            , fAlloc(alloc)
            , fImageTiling(tiling) {}

        ~Resource() override {}

#ifdef SK_TRACE_MANAGED_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrVkImage: %d (%d refs)\n", fImage, this->getRefCnt());
        }
#endif

    private:
        void freeGPUData() const override;

        const GrVkGpu* fGpu;
        VkImage        fImage;
        GrVkAlloc      fAlloc;
        VkImageTiling  fImageTiling;

        typedef GrTextureResource INHERITED;
    };

    // for wrapped textures
    class BorrowedResource : public Resource {
    public:
        BorrowedResource(const GrVkGpu* gpu, VkImage image, const GrVkAlloc& alloc,
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

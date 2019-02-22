/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkImage_DEFINED
#define GrVkImage_DEFINED

#include "GrBackendSurface.h"
#include "GrTexture.h"
#include "GrTypesPriv.h"
#include "GrVkImageLayout.h"
#include "GrVkResource.h"
#include "SkTypes.h"
#include "vk/GrVkTypes.h"

class GrVkGpu;
class GrVkTexture;

class GrVkImage : SkNoncopyable {
private:
    class Resource;

public:
    GrVkImage(const GrVkImageInfo& info, sk_sp<GrVkImageLayout> layout,
              GrBackendObjectOwnership ownership, bool forSecondaryCB = false)
            : fInfo(info)
            , fInitialQueueFamily(info.fCurrentQueueFamily)
            , fLayout(std::move(layout))
            , fIsBorrowed(GrBackendObjectOwnership::kBorrowed == ownership) {
        SkASSERT(fLayout->getImageLayout() == fInfo.fImageLayout);
        if (forSecondaryCB) {
            fResource = nullptr;
        } else if (fIsBorrowed) {
            fResource = new BorrowedResource(info.fImage, info.fAlloc, info.fImageTiling);
        } else {
            fResource = new Resource(info.fImage, info.fAlloc, info.fImageTiling);
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
            SkASSERT(this->imageFormat() == VK_FORMAT_UNDEFINED);
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

        ImageDesc()
            : fImageType(VK_IMAGE_TYPE_2D)
            , fFormat(VK_FORMAT_UNDEFINED)
            , fWidth(0)
            , fHeight(0)
            , fLevels(1)
            , fSamples(1)
            , fImageTiling(VK_IMAGE_TILING_OPTIMAL)
            , fUsageFlags(0)
            , fMemProps(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {}
    };

    static bool InitImageInfo(const GrVkGpu* gpu, const ImageDesc& imageDesc, GrVkImageInfo*);
    // Destroys the internal VkImage and VkDeviceMemory in the GrVkImageInfo
    static void DestroyImageInfo(const GrVkGpu* gpu, GrVkImageInfo*);

    // These match the definitions in SkImage, for whence they came
    typedef void* ReleaseCtx;
    typedef void (*ReleaseProc)(ReleaseCtx);

    void setResourceRelease(sk_sp<GrReleaseProcHelper> releaseHelper);

    // Helpers to use for setting the layout of the VkImage
    static VkPipelineStageFlags LayoutToPipelineSrcStageFlags(const VkImageLayout layout);
    static VkAccessFlags LayoutToSrcAccessMask(const VkImageLayout layout);

protected:
    void releaseImage(GrVkGpu* gpu);
    void abandonImage();
    bool hasResource() const { return fResource; }

    GrVkImageInfo          fInfo;
    uint32_t               fInitialQueueFamily;
    sk_sp<GrVkImageLayout> fLayout;
    bool                   fIsBorrowed;

private:
    class Resource : public GrVkResource {
    public:
        Resource()
                : fImage(VK_NULL_HANDLE) {
            fAlloc.fMemory = VK_NULL_HANDLE;
            fAlloc.fOffset = 0;
        }

        Resource(VkImage image, const GrVkAlloc& alloc, VkImageTiling tiling)
            : fImage(image)
            , fAlloc(alloc)
            , fImageTiling(tiling) {}

        ~Resource() override {
            SkASSERT(!fReleaseHelper);
        }

#ifdef SK_TRACE_VK_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrVkImage: %d (%d refs)\n", fImage, this->getRefCnt());
        }
#endif
        void setRelease(sk_sp<GrReleaseProcHelper> releaseHelper) {
            fReleaseHelper = std::move(releaseHelper);
        }

        /**
         * These are used to coordinate calling the idle proc between the GrVkTexture and the
         * Resource. If the GrVkTexture becomes purgeable and if there are no command buffers
         * referring to the Resource then it calls the proc. Otherwise, the Resource calls it
         * when the last command buffer reference goes away and the GrVkTexture is purgeable.
         */
        void setIdleProc(GrVkTexture* owner, GrTexture::IdleProc, void* context) const;
        void removeOwningTexture() const;

        /**
         * We track how many outstanding references this Resource has in command buffers and
         * when the count reaches zero we call the idle proc.
         */
        void notifyAddedToCommandBuffer() const override;
        void notifyRemovedFromCommandBuffer() const override;
        bool isOwnedByCommandBuffer() const { return fNumCommandBufferOwners > 0; }

    protected:
        mutable sk_sp<GrReleaseProcHelper> fReleaseHelper;

        void invokeReleaseProc() const {
            if (fReleaseHelper) {
                // Depending on the ref count of fReleaseHelper this may or may not actually trigger
                // the ReleaseProc to be called.
                fReleaseHelper.reset();
            }
        }

    private:
        void freeGPUData(GrVkGpu* gpu) const override;
        void abandonGPUData() const override {
            this->invokeReleaseProc();
            SkASSERT(!fReleaseHelper);
        }

        VkImage        fImage;
        GrVkAlloc      fAlloc;
        VkImageTiling  fImageTiling;
        mutable int fNumCommandBufferOwners = 0;
        mutable GrTexture::IdleProc* fIdleProc = nullptr;
        mutable void* fIdleProcContext = nullptr;
        mutable GrVkTexture* fOwningTexture = nullptr;

        typedef GrVkResource INHERITED;
    };

    // for wrapped textures
    class BorrowedResource : public Resource {
    public:
        BorrowedResource(VkImage image, const GrVkAlloc& alloc, VkImageTiling tiling)
            : Resource(image, alloc, tiling) {
        }
    private:
        void freeGPUData(GrVkGpu* gpu) const override;
        void abandonGPUData() const override;
    };

    Resource* fResource;

    friend class GrVkRenderTarget;
};

#endif

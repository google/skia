/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkImage_DEFINED
#define GrVkImage_DEFINED

#include "GrVkResource.h"

#include "GrTypesPriv.h"
#include "GrVkImageLayout.h"
#include "SkTypes.h"

#include "vk/GrVkDefines.h"
#include "vk/GrVkTypes.h"

class GrVkGpu;

class GrVkImage : SkNoncopyable {
private:
    class Resource;

public:
    GrVkImage(const GrVkImageInfo& info, sk_sp<GrVkImageLayout> layout,
              GrBackendObjectOwnership ownership)
            : fInfo(info)
            , fLayout(std::move(layout))
            , fIsBorrowed(GrBackendObjectOwnership::kBorrowed == ownership) {
        SkASSERT(fLayout->getImageLayout() == fInfo.fImageLayout);
        if (fIsBorrowed) {
            fResource = new BorrowedResource(info.fImage, info.fAlloc, info.fImageTiling);
        } else {
            fResource = new Resource(info.fImage, info.fAlloc, info.fImageTiling);
        }
    }
    virtual ~GrVkImage();

    VkImage image() const { return fInfo.fImage; }
    const GrVkAlloc& alloc() const { return fInfo.fAlloc; }
    VkFormat imageFormat() const { return fInfo.fFormat; }
    uint32_t mipLevels() const { return fInfo.fLevelCount; }
    const Resource* resource() const { return fResource; }
    bool isLinearTiled() const {
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
                        bool byRegion);

    // This simply updates our tracking of the image layout and does not actually do any gpu work.
    // This is only used for mip map generation where we are manually changing the layouts as we
    // blit each layer, and then at the end need to update our tracking.
    void updateImageLayout(VkImageLayout newLayout) {
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
    static VkPipelineStageFlags LayoutToPipelineStageFlags(const VkImageLayout layout);
    static VkAccessFlags LayoutToSrcAccessMask(const VkImageLayout layout);

protected:
    void releaseImage(const GrVkGpu* gpu);
    void abandonImage();

    void setNewResource(VkImage image, const GrVkAlloc& alloc, VkImageTiling tiling);

    GrVkImageInfo          fInfo;
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
    protected:
        mutable sk_sp<GrReleaseProcHelper> fReleaseHelper;

    private:
        void freeGPUData(const GrVkGpu* gpu) const override;
        void abandonGPUData() const override {
            SkASSERT(!fReleaseHelper);
        }

        VkImage        fImage;
        GrVkAlloc      fAlloc;
        VkImageTiling  fImageTiling;

        typedef GrVkResource INHERITED;
    };

    // for wrapped textures
    class BorrowedResource : public Resource {
    public:
        BorrowedResource(VkImage image, const GrVkAlloc& alloc, VkImageTiling tiling)
            : Resource(image, alloc, tiling) {
        }
    private:
        void invokeReleaseProc() const {
            if (fReleaseHelper) {
                // Depending on the ref count of fReleaseHelper this may or may not actually trigger
                // the ReleaseProc to be called.
                fReleaseHelper.reset();
            }
        }

        void freeGPUData(const GrVkGpu* gpu) const override;
        void abandonGPUData() const override;
    };

    Resource* fResource;

    friend class GrVkRenderTarget;
};

#endif

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
#include "SkTypes.h"

#include "vk/GrVkDefines.h"

class GrVkGpu;

class GrVkImage : SkNoncopyable {
public:
    // unlike GrVkBuffer, this needs to be public so GrVkStencilAttachment can use it
    class Resource : public GrVkResource {
    public:
        enum Flags {
            kNo_Flags = 0,
            kLinearTiling_Flag = 0x01,
            kBorrowed_Flag = 0x02
        };

        VkImage                  fImage;
        VkDeviceMemory           fAlloc;
        VkFormat                 fFormat;
        uint32_t                 fLevelCount;
        uint32_t                 fFlags;

        Resource()
            : INHERITED()
            , fImage(VK_NULL_HANDLE)
            , fAlloc(VK_NULL_HANDLE)
            , fFormat(VK_FORMAT_UNDEFINED)
            , fLevelCount(0)
            , fFlags(kNo_Flags) {}

        Resource(VkImage image, VkDeviceMemory alloc, VkFormat format, uint32_t levelCount,
                 uint32_t flags)
            : fImage(image), fAlloc(alloc), fFormat(format), fLevelCount(levelCount)
            , fFlags(flags) {}

        ~Resource() override {}

    private:
        void freeGPUData(const GrVkGpu* gpu) const override;

        typedef GrVkResource INHERITED;
    };

    // for wrapped textures
    class BorrowedResource : public Resource {
    public:
        BorrowedResource(VkImage image, VkDeviceMemory alloc, VkFormat format, uint32_t levelCount,
                         uint32_t flags)
            : Resource(image, alloc, format, levelCount, (flags | kBorrowed_Flag)) {
        }
    private:
        void freeGPUData(const GrVkGpu* gpu) const override;
    };

    GrVkImage(const Resource* imageResource) : fResource(imageResource) {
        if (imageResource->fFlags & Resource::kLinearTiling_Flag) {
            fCurrentLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        } else {
            fCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
        imageResource->ref();
    }

    virtual ~GrVkImage();

    VkImage textureImage() const { return fResource->fImage; }
    VkDeviceMemory textureMemory() const { return fResource->fAlloc; }
    const Resource* resource() const { return fResource; }
    bool isLinearTiled() const {
        return SkToBool(fResource->fFlags & Resource::kLinearTiling_Flag);
    }

    VkImageLayout currentLayout() const { return fCurrentLayout; }

    void setImageLayout(const GrVkGpu* gpu,
                        VkImageLayout newLayout,
                        VkAccessFlags srcAccessMask,
                        VkAccessFlags dstAccessMask,
                        VkPipelineStageFlags srcStageMask,
                        VkPipelineStageFlags dstStageMask,
                        bool byRegion);

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

    static const Resource* CreateResource(const GrVkGpu* gpu, const ImageDesc& imageDesc);

protected:

    void releaseImage(const GrVkGpu* gpu);
    void abandonImage();

    const Resource* fResource;

    VkImageLayout   fCurrentLayout;
};

#endif

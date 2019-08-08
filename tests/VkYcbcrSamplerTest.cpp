/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "tests/Test.h"
#include "tools/gpu/vk/VkTestUtils.h"

const size_t kImageWidth = 8;
const size_t kImageHeight = 8;

static int getY(size_t x, size_t y) {
    return 16 + (x + y) * 219 / (kImageWidth + kImageHeight - 2);
}
static int getU(size_t x, size_t y) { return 16 + x * 224 / (kImageWidth - 1); }
static int getV(size_t x, size_t y) { return 16 + y * 224 / (kImageHeight - 1); }

#define DECLARE_VK_PROC(name) PFN_vk##name fVk##name

#define ACQUIRE_INST_VK_PROC(name)                                                           \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, fBackendContext.fInstance,\
                                                       VK_NULL_HANDLE));                     \
    if (fVk##name == nullptr) {                                                              \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);            \
        return false;                                                                        \
    }

#define ACQUIRE_DEVICE_VK_PROC(name)                                                          \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, VK_NULL_HANDLE, fDevice)); \
    if (fVk##name == nullptr) {                                                               \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);             \
        return false;                                                                         \
    }

class VkYcbcrSamplerTestHelper {
public:
    VkYcbcrSamplerTestHelper() {}
    ~VkYcbcrSamplerTestHelper();

    bool init(skiatest::Reporter* reporter);

    sk_sp<SkImage> createI420Image(skiatest::Reporter* reporter);

    GrContext* getGrContext() { return fGrContext.get(); }

private:
    GrVkExtensions fExtensions;
    VkPhysicalDeviceFeatures2 fFeatures = {};
    VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;

    DECLARE_VK_PROC(DestroyInstance);
    DECLARE_VK_PROC(DeviceWaitIdle);
    DECLARE_VK_PROC(DestroyDevice);

    DECLARE_VK_PROC(GetPhysicalDeviceFormatProperties);
    DECLARE_VK_PROC(GetPhysicalDeviceMemoryProperties);

    DECLARE_VK_PROC(CreateImage);
    DECLARE_VK_PROC(DestroyImage);
    DECLARE_VK_PROC(GetImageMemoryRequirements);
    DECLARE_VK_PROC(AllocateMemory);
    DECLARE_VK_PROC(FreeMemory);
    DECLARE_VK_PROC(BindImageMemory);
    DECLARE_VK_PROC(MapMemory);
    DECLARE_VK_PROC(UnmapMemory);
    DECLARE_VK_PROC(FlushMappedMemoryRanges);
    DECLARE_VK_PROC(GetImageSubresourceLayout);

    VkDevice fDevice = VK_NULL_HANDLE;

    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugCallback = nullptr;

    GrVkBackendContext fBackendContext;
    sk_sp<GrContext> fGrContext;

    VkImage fImage = VK_NULL_HANDLE;
    VkDeviceMemory fImageMemory = VK_NULL_HANDLE;
    GrBackendTexture texture;
};

VkYcbcrSamplerTestHelper::~VkYcbcrSamplerTestHelper() {
    fGrContext.reset();

    if (fImage != VK_NULL_HANDLE) {
        fVkDestroyImage(fDevice, fImage, nullptr);
        fImage = VK_NULL_HANDLE;
    }
    if (fImageMemory != VK_NULL_HANDLE) {
        fVkFreeMemory(fDevice, fImageMemory, nullptr);
        fImageMemory = VK_NULL_HANDLE;
    }

    fBackendContext.fMemoryAllocator.reset();
    if (fDevice != VK_NULL_HANDLE) {
        fVkDeviceWaitIdle(fDevice);
        fVkDestroyDevice(fDevice, nullptr);
        fDevice = VK_NULL_HANDLE;
    }
    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback(fBackendContext.fInstance, fDebugCallback, nullptr);
    }
    if (fBackendContext.fInstance != VK_NULL_HANDLE) {
        fVkDestroyInstance(fBackendContext.fInstance, nullptr);
        fBackendContext.fInstance = VK_NULL_HANDLE;
    }

    sk_gpu_test::FreeVulkanFeaturesStructs(&fFeatures);
}

bool VkYcbcrSamplerTestHelper::init(skiatest::Reporter* reporter) {
    PFN_vkGetInstanceProcAddr instProc;
    PFN_vkGetDeviceProcAddr devProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc, &devProc)) {
        ERRORF(reporter, "Failed to load Vulkan");
        return false;
    }
    auto getProc = [&instProc, &devProc](const char* proc_name,
                                         VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return devProc(device, proc_name);
        }
        return instProc(instance, proc_name);
    };

    fFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    fFeatures.pNext = nullptr;

    fBackendContext.fInstance = VK_NULL_HANDLE;
    fBackendContext.fDevice = VK_NULL_HANDLE;

    if (!sk_gpu_test::CreateVkBackendContext(getProc, &fBackendContext, &fExtensions, &fFeatures,
                                             &fDebugCallback, nullptr, sk_gpu_test::CanPresentFn(),
                                             false)) {
        return false;
    }
    fDevice = fBackendContext.fDevice;

    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                instProc(fBackendContext.fInstance, "vkDestroyDebugReportCallbackEXT"));
    }
    ACQUIRE_INST_VK_PROC(DestroyInstance)
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle)
    ACQUIRE_INST_VK_PROC(DestroyDevice)

    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceFormatProperties)
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceMemoryProperties)

    ACQUIRE_DEVICE_VK_PROC(CreateImage)
    ACQUIRE_DEVICE_VK_PROC(DestroyImage)
    ACQUIRE_DEVICE_VK_PROC(GetImageMemoryRequirements)
    ACQUIRE_DEVICE_VK_PROC(AllocateMemory)
    ACQUIRE_DEVICE_VK_PROC(FreeMemory)
    ACQUIRE_DEVICE_VK_PROC(BindImageMemory)
    ACQUIRE_DEVICE_VK_PROC(MapMemory)
    ACQUIRE_DEVICE_VK_PROC(UnmapMemory)
    ACQUIRE_DEVICE_VK_PROC(FlushMappedMemoryRanges)
    ACQUIRE_DEVICE_VK_PROC(GetImageSubresourceLayout)

    if (!fExtensions.hasExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, 1)) {
        return false;
    }

    fGrContext = GrContext::MakeVulkan(fBackendContext);
    if (!fGrContext) {
        return false;
    }

    return true;
}

sk_sp<SkImage> VkYcbcrSamplerTestHelper::createI420Image(skiatest::Reporter* reporter) {
    VkImageCreateInfo vkImageInfo = {};
    vkImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageInfo.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    vkImageInfo.extent = VkExtent3D{kImageWidth, kImageHeight, 1};
    vkImageInfo.mipLevels = 1;
    vkImageInfo.arrayLayers = 1;
    vkImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageInfo.tiling = VK_IMAGE_TILING_LINEAR;
    vkImageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    REPORTER_ASSERT(reporter, fImage == VK_NULL_HANDLE);
    if (fVkCreateImage(fDevice, &vkImageInfo, nullptr, &fImage) != VK_SUCCESS) {
        ERRORF(reporter, "Failed to allocate I420 image");
        return nullptr;
    }

    VkMemoryRequirements requirements;
    fVkGetImageMemoryRequirements(fDevice, fImage, &requirements);

    uint32_t memoryTypeIndex = 0;
    bool foundHeap = false;
    VkPhysicalDeviceMemoryProperties phyDevMemProps;
    fVkGetPhysicalDeviceMemoryProperties(fBackendContext.fPhysicalDevice, &phyDevMemProps);
    for (uint32_t i = 0; i < phyDevMemProps.memoryTypeCount && !foundHeap; ++i) {
        if (requirements.memoryTypeBits & (1 << i)) {
            // Map host-visible memory.
            if (phyDevMemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                memoryTypeIndex = i;
                foundHeap = true;
            }
        }
    }
    if (!foundHeap) {
        ERRORF(reporter, "Failed to find valid heap for imported memory");
        return nullptr;
    }

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    REPORTER_ASSERT(reporter, fImageMemory == VK_NULL_HANDLE);
    if (fVkAllocateMemory(fDevice, &allocInfo, nullptr, &fImageMemory) != VK_SUCCESS) {
        ERRORF(reporter, "Failed to allocate VkDeviceMemory.");
        return nullptr;
    }

    void* mappedBuffer;
    if (fVkMapMemory(fDevice, fImageMemory, 0u, requirements.size, 0u, &mappedBuffer) !=
        VK_SUCCESS) {
        ERRORF(reporter, "Failed to map Vulkan memory.");
        return nullptr;
    }

    // Write Y channel.
    VkImageSubresource subresource;
    subresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    VkSubresourceLayout yLayout;
    fVkGetImageSubresourceLayout(fDevice, fImage, &subresource, &yLayout);
    uint8_t* bufferData = reinterpret_cast<uint8_t*>(mappedBuffer) + yLayout.offset;
    for (size_t y = 0; y < kImageHeight; ++y) {
        for (size_t x = 0; x < kImageWidth; ++x) {
            bufferData[y * yLayout.rowPitch + x] = getY(x, y);
        }
    }

    // Write UV channels.
    subresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    VkSubresourceLayout uvLayout;
    fVkGetImageSubresourceLayout(fDevice, fImage, &subresource, &uvLayout);
    bufferData = reinterpret_cast<uint8_t*>(mappedBuffer) + uvLayout.offset;
    for (size_t y = 0; y < kImageHeight / 2; ++y) {
        for (size_t x = 0; x < kImageWidth / 2; ++x) {
            bufferData[y * uvLayout.rowPitch + x * 2] = getU(x * 2, y * 2);
            bufferData[y * uvLayout.rowPitch + x * 2 + 1] = getV(x * 2, y * 2);
        }
    }

    VkMappedMemoryRange flushRange;
    flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    flushRange.pNext = nullptr;
    flushRange.memory = fImageMemory;
    flushRange.offset = 0;
    flushRange.size = VK_WHOLE_SIZE;
    if (fVkFlushMappedMemoryRanges(fDevice, 1, &flushRange) != VK_SUCCESS) {
        ERRORF(reporter, "Failed to flush buffer memory.");
        return nullptr;
    }
    fVkUnmapMemory(fDevice, fImageMemory);

    // Bind image memory.
    if (fVkBindImageMemory(fDevice, fImage, fImageMemory, 0u) != VK_SUCCESS) {
        ERRORF(reporter, "Failed to bind VkImage memory.");
        return nullptr;
    }

    // Get format flags for the image format.
    VkFormatProperties formatProperties;
    fVkGetPhysicalDeviceFormatProperties(fBackendContext.fPhysicalDevice, vkImageInfo.format,
                                         &formatProperties);

    // Wrap the image into SkImage.
    GrVkYcbcrConversionInfo ycbcrInfo(vkImageInfo.format,
                                      /*externalFormat=*/0,
                                      VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                                      VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                                      VK_CHROMA_LOCATION_COSITED_EVEN,
                                      VK_CHROMA_LOCATION_COSITED_EVEN,
                                      VK_FILTER_LINEAR,
                                      false,
                                      formatProperties.linearTilingFeatures);
    GrVkAlloc alloc(fImageMemory, 0 /* offset */, requirements.size, 0 /* flags */);
    GrVkImageInfo imageInfo(fImage, alloc, VK_IMAGE_TILING_LINEAR, VK_IMAGE_LAYOUT_UNDEFINED,
                            vkImageInfo.format, 1 /* levelCount */, VK_QUEUE_FAMILY_IGNORED,
                            GrProtected::kNo, ycbcrInfo);

    texture = GrBackendTexture(kImageWidth, kImageHeight, imageInfo);
    sk_sp<SkImage> image = SkImage::MakeFromTexture(fGrContext.get(),
                                                    texture,
                                                    kTopLeft_GrSurfaceOrigin,
                                                    kRGB_888x_SkColorType,
                                                    kPremul_SkAlphaType,
                                                    nullptr);

    if (!image) {
        ERRORF(reporter, "Failed to wrap VkImage with SkImage");
        return nullptr;
    }

    return image;
}

static int round_and_clamp(float x) {
    int r = static_cast<int>(round(x));
    if (r > 255) return 255;
    if (r < 0) return 0;
    return r;
}

DEF_GPUTEST(VkYCbcrSampler_DrawImageWithYcbcrSampler, reporter, options) {
    VkYcbcrSamplerTestHelper helper;
    if (!helper.init(reporter)) {
        return;
    }

    sk_sp<SkImage> srcImage = helper.createI420Image(reporter);
    if (!srcImage) {
        return;
    }

    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(
            helper.getGrContext(), SkBudgeted::kNo,
            SkImageInfo::Make(kImageWidth, kImageHeight, kN32_SkColorType, kPremul_SkAlphaType));
    if (!surface) {
        ERRORF(reporter, "Failed to create target SkSurface");
        return;
    }
    surface->getCanvas()->drawImage(srcImage, 0, 0);
    surface->flush();

    std::vector<uint8_t> readbackData(kImageWidth * kImageHeight * 4);
    if (!surface->readPixels(SkImageInfo::Make(kImageWidth, kImageHeight, kRGBA_8888_SkColorType,
                                               kOpaque_SkAlphaType),
                             readbackData.data(), kImageWidth * 4, 0, 0)) {
        ERRORF(reporter, "Readback failed");
        return;
    }

    // Allow resulting color to be off by 1 in each channel as some Vulkan implementations do not
    // round YCbCr sampler result properly.
    const int kColorTolerance = 1;

    // Verify results only for pixels with even coordinates, since others use
    // interpolated U & V channels.
    for (size_t y = 0; y < kImageHeight; y += 2) {
        for (size_t x = 0; x < kImageWidth; x += 2) {
            // createI420Image() initializes the image with VK_SAMPLER_YCBCR_RANGE_ITU_NARROW.
            float yChannel = (static_cast<float>(getY(x, y)) - 16.0) / 219.0;
            float uChannel = (static_cast<float>(getU(x, y)) - 128.0) / 224.0;
            float vChannel = (static_cast<float>(getV(x, y)) - 128.0) / 224.0;

            // BR.709 conversion as specified in
            // https://www.khronos.org/registry/DataFormat/specs/1.2/dataformat.1.2.html#MODEL_YUV
            int expectedR = round_and_clamp((yChannel + 1.5748f * vChannel) * 255.0);
            int expectedG = round_and_clamp((yChannel - 0.13397432f / 0.7152f * uChannel -
                                             0.33480248f / 0.7152f * vChannel) *
                                            255.0);
            int expectedB = round_and_clamp((yChannel + 1.8556f * uChannel) * 255.0);

            int r = readbackData[(y * kImageWidth + x) * 4];
            if (abs(r - expectedR) > kColorTolerance) {
                ERRORF(reporter, "R should be %d, but is %d at (%d, %d)", expectedR, r, x, y);
            }

            int g = readbackData[(y * kImageWidth + x) * 4 + 1];
            if (abs(g - expectedG) > kColorTolerance) {
                ERRORF(reporter, "G should be %d, but is %d at (%d, %d)", expectedG, g, x, y);
            }

            int b = readbackData[(y * kImageWidth + x) * 4 + 2];
            if (abs(b - expectedB) > kColorTolerance) {
                ERRORF(reporter, "B should be %d, but is %d at (%d, %d)", expectedB, b, x, y);
            }
        }
    }
}

// Verifies that it's not possible to allocate Ycbcr texture directly.
DEF_GPUTEST(VkYCbcrSampler_NoYcbcrSurface, reporter, options) {
    VkYcbcrSamplerTestHelper helper;
    if (!helper.init(reporter)) {
        return;
    }

    GrBackendTexture texture = helper.getGrContext()->createBackendTexture(
            kImageWidth, kImageHeight, GrBackendFormat::MakeVk(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM),
            GrMipMapped::kNo, GrRenderable::kNo, GrProtected::kNo);
    if (texture.isValid()) {
        ERRORF(reporter,
               "GrContext::createBackendTexture() didn't fail as expected for Ycbcr format.");
    }
}

#endif  // SK_SUPPORT_GPU && defined(SK_VULKAN)

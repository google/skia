/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef VulkanWindowContext_DEFINED
#define VulkanWindowContext_DEFINED

#include "include/core/SkTypes.h"

#include "include/core/SkSurface.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "tools/gpu/vk/VkTestUtils.h"
#include "tools/window/WindowContext.h"

class GrRenderTarget;

namespace skgpu { struct VulkanInterface; }
namespace skwindow::internal {

class VulkanWindowContext : public WindowContext {
public:
    ~VulkanWindowContext() override;

    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return fDevice != VK_NULL_HANDLE; }

    void resize(int w, int h) override { this->createSwapchain(w, h); }

    void setDisplayParams(std::unique_ptr<const DisplayParams> params) override {
        this->destroyContext();
        fDisplayParams = std::move(params);
        this->initializeContext();
    }

    /** Platform specific function that creates a VkSurfaceKHR for a window */
    using CreateVkSurfaceFn = std::function<VkSurfaceKHR(VkInstance)>;
    /** Platform specific function that determines whether presentation will succeed. */
    using CanPresentFn = sk_gpu_test::CanPresentFn;

    VulkanWindowContext(std::unique_ptr<const DisplayParams>,
                        CreateVkSurfaceFn,
                        CanPresentFn,
                        PFN_vkGetInstanceProcAddr);

private:
    void initializeContext();
    void destroyContext();

    bool createSwapchain(int width, int height);
    bool populateSwapchainImages(VkFormat, VkImageUsageFlags, SkColorType, VkSharingMode);

    /**
     * Swap backbuffers/frames, presenting the next available image.
     */
    void onSwapBuffers() override;

    /**
     * Define private method to submit work to the GPU (rather than using Window::submitToGpu) in
     * order to properly configure semaphores. Submits work to render the image at
     * fCurrentImageIndex.
     */
    void submitToGpu();

    /**
     * Reset the container of images (and destroy each image's associated rendering semaphore).
     */
    void resetSwapchainImages();

    /**
     * Define a struct which represents a swapchain image to be presented. Each SwapchainImage
     * contains the native VkImage, its image layout while not being used as a color attachment
     * (which we initialize as VK_IMAGE_LAYOUT_UNDEFINED), a semaphore for signaling render
     * completion, and an SkSurface the client renders to.
     */
    struct SwapchainImage {
        VkImage fVkImage = VK_NULL_HANDLE;
        VkImageLayout fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkSemaphore fRenderCompletionSemaphore = VK_NULL_HANDLE;
        sk_sp<SkSurface> fSurface = nullptr; /* May not be based on rts */
    };

    skia_private::AutoTArray<SwapchainImage> fImages;
    uint32_t fCurrentImageIndex; /* Index of image currently being presented */
    VkSemaphore fAcquireSemaphore = VK_NULL_HANDLE; /* Semaphore to signal image acquisition */
    sk_sp<const skgpu::VulkanInterface> fInterface;

    /* Vulkan driver structs + info */
    VkInstance fInstance = VK_NULL_HANDLE;
    VkPhysicalDevice fPhysicalDevice = VK_NULL_HANDLE;
    VkDevice fDevice = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT fDebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR fDeviceSurface;
    VkSwapchainKHR fSwapchain;
    uint32_t fGraphicsQueueIndex;
    VkQueue fGraphicsQueue;
    uint32_t fPresentQueueIndex;
    VkQueue fPresentQueue;

    /* Create functions */
    CreateVkSurfaceFn fCreateVkSurfaceFn;
    CanPresentFn fCanPresentFn;
    PFN_vkGetInstanceProcAddr fGetInstanceProcAddr = nullptr;

    /* WSI interface functions */
    PFN_vkDestroySurfaceKHR fDestroySurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR fGetPhysicalDeviceSurfaceSupportKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fGetPhysicalDeviceSurfaceCapabilitiesKHR =
            nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fGetPhysicalDeviceSurfacePresentModesKHR =
            nullptr;
    PFN_vkCreateSwapchainKHR fCreateSwapchainKHR = nullptr;
    PFN_vkDestroySwapchainKHR fDestroySwapchainKHR = nullptr;
    PFN_vkGetSwapchainImagesKHR fGetSwapchainImagesKHR = nullptr;
    PFN_vkAcquireNextImageKHR fAcquireNextImageKHR = nullptr;
    PFN_vkQueuePresentKHR fQueuePresentKHR = nullptr;

    PFN_vkDestroyInstance fDestroyInstance = nullptr;
    PFN_vkDeviceWaitIdle fDeviceWaitIdle = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT fDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkQueueWaitIdle fQueueWaitIdle = nullptr;
    PFN_vkDestroyDevice fDestroyDevice = nullptr;
    PFN_vkGetDeviceQueue fGetDeviceQueue = nullptr;
};

}  // namespace skwindow::internal

#endif


/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef VulkanWindowContext_DEFINED
#define VulkanWindowContext_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#include "include/gpu/vk/GrVkVulkan.h"

#include "include/gpu/vk/GrVkBackendContext.h"
#include "src/gpu/vk/GrVkInterface.h"
#include "tools/gpu/vk/VkTestUtils.h"
#include "tools/sk_app/WindowContext.h"

class GrRenderTarget;

namespace sk_app {

class VulkanWindowContext : public WindowContext {
public:
    ~VulkanWindowContext() override;

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    bool isValid() override { return fDevice != VK_NULL_HANDLE; }

    void resize(int w, int h) override {
        this->createSwapchain(w, h, fDisplayParams);
    }

    void setDisplayParams(const DisplayParams& params) override {
        this->destroyContext();
        fDisplayParams = params;
        this->initializeContext();
    }

    /** Platform specific function that creates a VkSurfaceKHR for a window */
    using CreateVkSurfaceFn = std::function<VkSurfaceKHR(VkInstance)>;
    /** Platform specific function that determines whether presentation will succeed. */
    using CanPresentFn = sk_gpu_test::CanPresentFn;

    VulkanWindowContext(const DisplayParams&, CreateVkSurfaceFn, CanPresentFn,
                        PFN_vkGetInstanceProcAddr, PFN_vkGetDeviceProcAddr);

private:
    void initializeContext();
    void destroyContext();

    struct BackbufferInfo {
        uint32_t        fImageIndex;          // image this is associated with
        VkSemaphore     fRenderSemaphore;     // we wait on this for rendering to be done
    };

    BackbufferInfo* getAvailableBackbuffer();
    bool createSwapchain(int width, int height, const DisplayParams& params);
    void createBuffers(VkFormat format, SkColorType colorType);
    void destroyBuffers();

    VkInstance fInstance = VK_NULL_HANDLE;
    VkPhysicalDevice fPhysicalDevice = VK_NULL_HANDLE;
    VkDevice fDevice = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;

    // Create functions
    CreateVkSurfaceFn fCreateVkSurfaceFn;
    CanPresentFn      fCanPresentFn;

    // Vulkan GetProcAddr functions
    PFN_vkGetInstanceProcAddr fGetInstanceProcAddr = nullptr;
    PFN_vkGetDeviceProcAddr fGetDeviceProcAddr = nullptr;

    // WSI interface functions
    PFN_vkDestroySurfaceKHR fDestroySurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR fGetPhysicalDeviceSurfaceSupportKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fGetPhysicalDeviceSurfaceCapabilitiesKHR =nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fGetPhysicalDeviceSurfacePresentModesKHR =nullptr;

    PFN_vkCreateSwapchainKHR fCreateSwapchainKHR = nullptr;
    PFN_vkDestroySwapchainKHR fDestroySwapchainKHR = nullptr;
    PFN_vkGetSwapchainImagesKHR fGetSwapchainImagesKHR = nullptr;
    PFN_vkAcquireNextImageKHR fAcquireNextImageKHR = nullptr;
    PFN_vkQueuePresentKHR fQueuePresentKHR = nullptr;

    PFN_vkDestroyInstance fDestroyInstance = nullptr;
    PFN_vkDeviceWaitIdle fDeviceWaitIdle = nullptr;
    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugReportCallbackEXT = nullptr;
    PFN_vkQueueWaitIdle fQueueWaitIdle = nullptr;
    PFN_vkDestroyDevice fDestroyDevice = nullptr;
    PFN_vkGetDeviceQueue fGetDeviceQueue = nullptr;

    sk_sp<const GrVkInterface> fInterface;

    VkSurfaceKHR      fSurface;
    VkSwapchainKHR    fSwapchain;
    uint32_t          fGraphicsQueueIndex;
    VkQueue           fGraphicsQueue;
    uint32_t          fPresentQueueIndex;
    VkQueue           fPresentQueue;

    uint32_t               fImageCount;
    VkImage*               fImages;         // images in the swapchain
    VkImageLayout*         fImageLayouts;   // layouts of these images when not color attachment
    sk_sp<SkSurface>*      fSurfaces;       // surfaces client renders to (may not be based on rts)
    BackbufferInfo*        fBackbuffers;
    uint32_t               fCurrentBackbufferIndex;
};

}   // namespace sk_app

#endif // SK_VULKAN

#endif


/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef VulkanWindowContext_DEFINED
#define VulkanWindowContext_DEFINED

#include "SkTypes.h" // required to pull in any SkUserConfig defines

#ifdef SK_VULKAN

#include "vk/GrVkBackendContext.h"
#include "WindowContext.h"

class GrRenderTarget;

namespace sk_app {

class VulkanWindowContext : public WindowContext {
public:
    ~VulkanWindowContext() override;

    // each platform will have to implement these in its CPP file
    static VkSurfaceKHR createVkSurface(VkInstance, void* platformData);
    static bool canPresent(VkInstance, VkPhysicalDevice, uint32_t queueFamilyIndex,
                           void* platformData);

    static VulkanWindowContext* Create(void* platformData, const DisplayParams& params) {
        VulkanWindowContext* ctx = new VulkanWindowContext(platformData, params);
        if (!ctx->isValid()) {
            delete ctx;
            return nullptr;
        }
        return ctx;
    }

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    bool isValid() override { return SkToBool(fBackendContext.get()); }

    void resize(uint32_t w, uint32_t h) override {
        this->createSwapchain(w, h, fDisplayParams);
    }

    void setDisplayParams(const DisplayParams& params) override {
        this->createSwapchain(fWidth, fHeight, params);
    }

    GrBackendContext getBackendContext() override { 
        return (GrBackendContext) fBackendContext.get(); 
    }

private:
    VulkanWindowContext(void*, const DisplayParams&);
    void initializeContext(void*, const DisplayParams&);
    void destroyContext();

    struct BackbufferInfo {
        uint32_t        fImageIndex;          // image this is associated with
        VkSemaphore     fAcquireSemaphore;    // we signal on this for acquisition of image
        VkSemaphore     fRenderSemaphore;     // we wait on this for rendering to be done
        VkCommandBuffer fTransitionCmdBuffers[2]; // to transition layout between present and render
        VkFence         fUsageFences[2];      // used to ensure this data is no longer used on GPU
    };

    BackbufferInfo* getAvailableBackbuffer();
    bool createSwapchain(uint32_t width, uint32_t height, const DisplayParams& params);
    void createBuffers(VkFormat format);
    void destroyBuffers();

    SkAutoTUnref<const GrVkBackendContext> fBackendContext;

    // simple wrapper class that exists only to initialize a pointer to NULL
    template <typename FNPTR_TYPE> class VkPtr {
    public:
        VkPtr() : fPtr(NULL) {}
        VkPtr operator=(FNPTR_TYPE ptr) { fPtr = ptr; return *this; }
        operator FNPTR_TYPE() const { return fPtr; }
    private:
        FNPTR_TYPE fPtr;
    };

    // WSI interface functions
    VkPtr<PFN_vkDestroySurfaceKHR> fDestroySurfaceKHR;
    VkPtr<PFN_vkGetPhysicalDeviceSurfaceSupportKHR> fGetPhysicalDeviceSurfaceSupportKHR;
    VkPtr<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR> fGetPhysicalDeviceSurfaceCapabilitiesKHR;
    VkPtr<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR> fGetPhysicalDeviceSurfaceFormatsKHR;
    VkPtr<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR> fGetPhysicalDeviceSurfacePresentModesKHR;

    VkPtr<PFN_vkCreateSwapchainKHR> fCreateSwapchainKHR;
    VkPtr<PFN_vkDestroySwapchainKHR> fDestroySwapchainKHR;
    VkPtr<PFN_vkGetSwapchainImagesKHR> fGetSwapchainImagesKHR;
    VkPtr<PFN_vkAcquireNextImageKHR> fAcquireNextImageKHR;
    VkPtr<PFN_vkQueuePresentKHR> fQueuePresentKHR;
    VkPtr<PFN_vkCreateSharedSwapchainsKHR> fCreateSharedSwapchainsKHR;

    VkSurfaceKHR      fSurface;
    VkSwapchainKHR    fSwapchain;
    uint32_t          fPresentQueueIndex;
    VkQueue           fPresentQueue;

    uint32_t               fImageCount;
    VkImage*               fImages;         // images in the swapchain
    VkImageLayout*         fImageLayouts;   // layouts of these images when not color attachment
    sk_sp<GrRenderTarget>* fRenderTargets;  // wrapped rendertargets for those images
    sk_sp<SkSurface>*      fSurfaces;       // surfaces client renders to (may not be based on rts)
    VkCommandPool          fCommandPool;
    BackbufferInfo*        fBackbuffers;
    uint32_t               fCurrentBackbufferIndex;
};

}   // namespace sk_app

#endif // SK_VULKAN

#endif

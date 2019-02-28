
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkVulkan.h"

#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"

#include "vk/VkTestUtils.h"

#include "WindowContextFactory_mac.h"
#include "../VulkanWindowContext.h"
#include "vulkan/vulkan_macos.h"

#import <MetalKit/MetalKit.h>

namespace sk_app {

class VulkanWindowContext_mac : public VulkanWindowContext {
public:
    VulkanWindowContext_mac(MTKView* mtkView,
                            const DisplayParams& params,
                            CreateVkSurfaceFn createVkSurface,
                            CanPresentFn canPresent,
                            PFN_vkGetInstanceProcAddr instProc,
                            PFN_vkGetDeviceProcAddr devProc);

    ~VulkanWindowContext_mac() override;

    void resize(int w, int h) override;

private:
    MTKView*              fMTKView;

    typedef VulkanWindowContext INHERITED;
};

VulkanWindowContext_mac::VulkanWindowContext_mac(MTKView* mtkView,
                                                 const DisplayParams& params,
                                                 CreateVkSurfaceFn createVkSurface,
                                                 CanPresentFn canPresent,
                                                 PFN_vkGetInstanceProcAddr instProc,
                                                 PFN_vkGetDeviceProcAddr devProc)
    : INHERITED(params, createVkSurface, canPresent, instProc, devProc)
    , fMTKView(mtkView) {

    // any config code here (particularly for msaa)?
}

VulkanWindowContext_mac::~VulkanWindowContext_mac() {
    [fMTKView removeFromSuperview];
    [fMTKView release];
    fMTKView = nil;
}

void VulkanWindowContext_mac::resize(int w, int h) {
    CGSize newSize;
    newSize.width = w;
    newSize.height = h;
    fMTKView.drawableSize = newSize;
    this->INHERITED::resize(w, h);
}

namespace window_context_factory {

WindowContext* NewVulkanForMac(const MacWindowInfo& info, const DisplayParams& displayParams) {
    PFN_vkGetInstanceProcAddr instProc;
    PFN_vkGetDeviceProcAddr devProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc, &devProc)) {
        return nullptr;
    }

    // Create mtkview
    NSRect rect = info.fMainView.frame;
    MTKView* mtkView = [[MTKView alloc] initWithFrame:rect device:nil];
    if (nil == mtkView) {
        return nullptr;
    }

    mtkView.autoResizeDrawable = NO;
    mtkView.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    mtkView.drawableSize = rect.size;

//    if (fDisplayParams.fMSAASampleCount > 1) {
//        if (![fDevice supportsTextureSampleCount:fDisplayParams.fMSAASampleCount]) {
//            return nullptr;
//        }
//    }
//    mtkView.sampleCount = fDisplayParams.fMSAASampleCount;

    // attach Metal view to main view
    [mtkView setTranslatesAutoresizingMaskIntoConstraints:NO];

    [info.fMainView addSubview:mtkView];
    NSDictionary *views = NSDictionaryOfVariableBindings(mtkView);

    [info.fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[mtkView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    [info.fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[mtkView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    auto createVkSurface = [mtkView, instProc](VkInstance instance) -> VkSurfaceKHR {
        static PFN_vkCreateMacOSSurfaceMVK createMacOSSurfaceMVK = nullptr;
        if (!createMacOSSurfaceMVK) {
            createMacOSSurfaceMVK =
                    (PFN_vkCreateMacOSSurfaceMVK) instProc(instance, "vkCreateMacOSSurfaceMVK");
        }

        VkSurfaceKHR surface;

        VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(VkMacOSSurfaceCreateInfoMVK));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
        surfaceCreateInfo.pNext = nullptr;
        surfaceCreateInfo.flags = 0;
        surfaceCreateInfo.pView = (__bridge void*)mtkView;

        VkResult res = createMacOSSurfaceMVK(instance, &surfaceCreateInfo, nullptr, &surface);
        if (VK_SUCCESS != res) {
            return VK_NULL_HANDLE;
        }

        return surface;
    };

    auto canPresent = [](VkInstance instance, VkPhysicalDevice physDev, uint32_t queueFamilyIndex) {
        return true;
    };
    WindowContext* context = new VulkanWindowContext_mac(mtkView, displayParams, createVkSurface,
                                                         canPresent, instProc, devProc);
    if (!context->isValid()) {
        delete context;
        [mtkView removeFromSuperview];
        [mtkView release];
        return nullptr;
    }
    return context;
}

}  // namespace VulkanWindowContextFactory

}  // namespace sk_app

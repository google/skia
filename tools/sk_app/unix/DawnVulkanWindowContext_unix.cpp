/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/DawnWindowContext.h"
#include "tools/sk_app/unix/WindowContextFactory_unix.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/VulkanBackend.h"
#include "src/ports/SkOSLibrary.h"
#include "tools/gpu/vk/VkTestUtils.h"

#include <vulkan/vulkan.h>
#include <X11/Xlib-xcb.h>

using sk_app::window_context_factory::XlibWindowInfo;
using sk_app::DisplayParams;
using sk_app::DawnWindowContext;

namespace sk_app {

class DawnVulkanWindowContext_xlib : public DawnWindowContext {
public:
    DawnVulkanWindowContext_xlib(const XlibWindowInfo&, const DisplayParams&);
    ~DawnVulkanWindowContext_xlib() override {}
    wgpu::Device onInitializeContext() override;
    void onDestroyContext() override {}
    DawnSwapChainImplementation createSwapChainImplementation(
            int width, int height, const DisplayParams& params) override;
    void onSwapBuffers() override {}

private:
    Display*     fDisplay;
    XWindow      fWindow;
    VkSurfaceKHR fVkSurface = nullptr;

    using INHERITED = DawnWindowContext;
};

DawnVulkanWindowContext_xlib::DawnVulkanWindowContext_xlib(const XlibWindowInfo& winInfo,
                                                           const DisplayParams& params)
        : INHERITED(params, wgpu::TextureFormat::BGRA8Unorm)
        , fDisplay(winInfo.fDisplay)
        , fWindow(winInfo.fWindow) {
    XWindow root;
    int x, y;
    unsigned int border_width, depth;
    unsigned int width, height;
    XGetGeometry(fDisplay, fWindow, &root, &x, &y, &width, &height, &border_width, &depth);
    this->initializeContext(width, height);
}

DawnSwapChainImplementation DawnVulkanWindowContext_xlib::createSwapChainImplementation(
        int width, int height, const DisplayParams& params) {
    return dawn_native::vulkan::CreateNativeSwapChainImpl(fDevice.Get(), fVkSurface);
}

wgpu::Device DawnVulkanWindowContext_xlib::onInitializeContext() {
    wgpu::Device device = this->createDevice(wgpu::BackendType::Vulkan);
    if (!device) {
        return nullptr;
    }

    void *vkLib = SkLoadDynamicLibrary("libvulkan.so.1");
    if (!vkLib) {
        return nullptr;
    }
    VkInstance instance = dawn_native::vulkan::GetInstance(device.Get());
    if (!instance) {
        return nullptr;
    }
    auto createXcbSurfaceKHR =
        reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(SkGetProcedureAddress(vkLib,
                                                                        "vkCreateXcbSurfaceKHR"));
    if (!createXcbSurfaceKHR) {
        printf("couldn't get extensions :(\n");
        return nullptr;
    }

    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(VkXcbSurfaceCreateInfoKHR));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.connection = XGetXCBConnection(fDisplay);
    surfaceCreateInfo.window = fWindow;

    createXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &fVkSurface);
    return device;
}

namespace window_context_factory {

std::unique_ptr<WindowContext> MakeDawnVulkanForXlib(const XlibWindowInfo& winInfo,
                                                     const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new DawnVulkanWindowContext_xlib(winInfo, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}

}  // namespace sk_app

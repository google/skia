/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../NXTWindowContext.h"
#include "WindowContextFactory_unix.h"
#include "SkOSLibrary.h"
#include "nxt/GrNXTBackendContext.h"
#include "vk/VkTestUtils.h"

#include <vulkan/vulkan.h>
#include <X11/Xlib-xcb.h>

using sk_app::window_context_factory::XlibWindowInfo;
using sk_app::DisplayParams;
using sk_app::NXTWindowContext;

namespace backend {
    namespace vulkan {
        void Init(nxtProcTable* procs,
                  nxtDevice* device,
                  const std::vector<const char*>& requiredInstanceExtensions);
        nxtSwapChainImplementation CreateNativeSwapChainImpl(nxtDevice device, uint64_t surface);
        VkInstance GetInstance(nxtDevice device);
    }
}

namespace sk_app {

class NXTVulkanWindowContext_xlib : public NXTWindowContext {
public:
    NXTVulkanWindowContext_xlib(const XlibWindowInfo&, const DisplayParams&);
    ~NXTVulkanWindowContext_xlib() override {}
    sk_sp<GrNXTBackendContext> onInitializeContext() override;
    void onDestroyContext() override {}
    nxtSwapChainImplementation createSwapChainImplementation(int width, int height, const DisplayParams& params) override;
    void onSwapBuffers() override {}

private:
    Display*     fDisplay;
    XWindow      fWindow;
    VkSurfaceKHR fVkSurface = nullptr;

    typedef NXTWindowContext INHERITED;
};

NXTVulkanWindowContext_xlib::NXTVulkanWindowContext_xlib(const XlibWindowInfo& winInfo, const DisplayParams& params)
        : INHERITED(params, nxt::TextureFormat::B8G8R8A8Unorm)
        , fDisplay(winInfo.fDisplay)
        , fWindow(winInfo.fWindow) {
    XWindow root;
    int x, y;
    unsigned int border_width, depth;
    unsigned int width, height;
    XGetGeometry(fDisplay, fWindow, &root, &x, &y, &width, &height, &border_width, &depth);
    this->initializeContext(width, height);
}

nxtSwapChainImplementation NXTVulkanWindowContext_xlib::createSwapChainImplementation(int width, int height, const DisplayParams& params) {
    return backend::vulkan::CreateNativeSwapChainImpl(fDevice.Get(), reinterpret_cast<uint64_t>(fVkSurface));
}

sk_sp<GrNXTBackendContext> NXTVulkanWindowContext_xlib::onInitializeContext() {
    nxtDevice backendDevice;
    nxtProcTable backendProcs;
    std::vector<const char*> requiredInstanceExtensions;
    requiredInstanceExtensions.push_back("VK_KHR_xcb_surface");

    backend::vulkan::Init(&backendProcs, &backendDevice, requiredInstanceExtensions);
    nxtSetProcs(&backendProcs);
    nxtQueue backendQueue = nxtDeviceCreateQueue(backendDevice);
    sk_sp<GrNXTBackendContext> ctx(new GrNXTBackendContext(backendDevice, backendQueue));

    void *vkLib = DynamicLoadLibrary("libvulkan.so.1");
    if (!vkLib) {
        return nullptr;
    }
#if 0
    auto instProc =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcedureAddress(vkLib, "vkGetInstanceProcAddr"));
    if (!instProc) {
        return nullptr;
    }
#endif
    VkInstance instance = backend::vulkan::GetInstance(backendDevice);
    if (!instance) {
        return nullptr;
    }
#if 0
    auto createXcbSurfaceKHR =
        reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(instProc(nullptr, "vkCreateXcbSurfaceKHR"));
#endif
    auto createXcbSurfaceKHR =
        reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(GetProcedureAddress(vkLib, "vkCreateXcbSurfaceKHR"));
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
    return ctx;
}

namespace window_context_factory {

WindowContext* NewNXTVulkanForXlib(const XlibWindowInfo& winInfo, const DisplayParams& params) {
    WindowContext* ctx = new NXTVulkanWindowContext_xlib(winInfo, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}

}  // namespace sk_app

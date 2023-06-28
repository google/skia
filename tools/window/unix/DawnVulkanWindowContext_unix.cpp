/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/DawnWindowContext.h"
#include "tools/window/unix/WindowContextFactory_unix.h"
#include "dawn/native/DawnNative.h"

using skwindow::XlibWindowInfo;
using skwindow::DisplayParams;
using skwindow::internal::DawnWindowContext;

namespace {

class DawnVulkanWindowContext_xlib : public DawnWindowContext {
public:
    DawnVulkanWindowContext_xlib(const XlibWindowInfo&, const DisplayParams&);
    ~DawnVulkanWindowContext_xlib() override {}
    wgpu::Device onInitializeContext() override;
    void onDestroyContext() override {}

private:
    Display*     fDisplay;
    XWindow      fWindow;
};

DawnVulkanWindowContext_xlib::DawnVulkanWindowContext_xlib(const XlibWindowInfo& winInfo,
                                                           const DisplayParams& params)
        : DawnWindowContext(params, wgpu::TextureFormat::BGRA8Unorm)
        , fDisplay(winInfo.fDisplay)
        , fWindow(winInfo.fWindow) {
    XWindow root;
    int x, y;
    unsigned int border_width, depth;
    unsigned int width, height;
    XGetGeometry(fDisplay, fWindow, &root, &x, &y, &width, &height, &border_width, &depth);
    this->initializeContext(width, height);
}

wgpu::Device DawnVulkanWindowContext_xlib::onInitializeContext() {
    wgpu::SurfaceDescriptorFromXlibWindow xlibDesc;
    xlibDesc.display = fDisplay;
    xlibDesc.window = fWindow;

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &xlibDesc;

    fDawnSurface = wgpu::Instance(fInstance->Get()).CreateSurface(&surfaceDesc);
    SkASSERT(fDawnSurface);

    return this->createDevice(wgpu::BackendType::Vulkan);
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeDawnVulkanForXlib(const XlibWindowInfo& winInfo,
                                                     const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new DawnVulkanWindowContext_xlib(winInfo, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

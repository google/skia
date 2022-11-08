/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/GraphiteDawnWindowContext.h"
#include "tools/sk_app/unix/WindowContextFactory_unix.h"

using sk_app::window_context_factory::XlibWindowInfo;
using sk_app::DisplayParams;
using sk_app::GraphiteDawnWindowContext;

namespace {

class GraphiteDawnVulkanWindowContext_unix : public GraphiteDawnWindowContext {
public:
    GraphiteDawnVulkanWindowContext_unix(const XlibWindowInfo& info, const DisplayParams& params);

    ~GraphiteDawnVulkanWindowContext_unix() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;
    void onSwapBuffers() override;
    void resize(int w, int h) override;

private:
    Display*     fDisplay;
    XWindow      fWindow;

    using INHERITED = GraphiteDawnWindowContext;
};

GraphiteDawnVulkanWindowContext_unix::GraphiteDawnVulkanWindowContext_unix(
    const XlibWindowInfo& info,
    const DisplayParams& params)
        : INHERITED(params, wgpu::TextureFormat::BGRA8Unorm)
        , fDisplay(info.fDisplay)
        , fWindow(info.fWindow)  {
    XWindow root;
    int x, y;
    unsigned int border_width, depth;
    unsigned int width, height;
    XGetGeometry(fDisplay, fWindow, &root, &x, &y, &width, &height, &border_width, &depth);
    this->initializeContext(width, height);
}

GraphiteDawnVulkanWindowContext_unix::~GraphiteDawnVulkanWindowContext_unix() {
    this->destroyContext();
}

bool GraphiteDawnVulkanWindowContext_unix::onInitializeContext() {
    SkASSERT(!!fWindow);

    auto device = this->createDevice(wgpu::BackendType::Vulkan);
    if (!device) {
        SkASSERT(device);
        return false;
    }

    wgpu::SurfaceDescriptorFromXlibWindow surfaceChainedDesc;
    surfaceChainedDesc.display = fDisplay;
    surfaceChainedDesc.window = fWindow;

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &surfaceChainedDesc;

    auto surface = wgpu::Instance(fInstance->Get()).CreateSurface(&surfaceDesc);
    if (!surface) {
        SkASSERT(false);
        return false;
    }

    fDevice = std::move(device);
    fSurface = std::move(surface);
    fSwapChain = this->createSwapChain();

    return true;
}

void GraphiteDawnVulkanWindowContext_unix::onDestroyContext() {}

void GraphiteDawnVulkanWindowContext_unix::onSwapBuffers() {}

void GraphiteDawnVulkanWindowContext_unix::resize(int w, int h) {
    fSwapChain = this->createSwapChain();
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeGraphiteDawnVulkanForXlib(const XlibWindowInfo& info,
                                                             const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GraphiteDawnVulkanWindowContext_unix(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app

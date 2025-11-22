/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// Important to put this first because webgpu_cpp.h and X.h don't get along.
// Include these first, before X11 defines None, Success, Status etc.
#include "dawn/native/DawnNative.h"  // NO_G3_REWRITE
#include "webgpu/webgpu_cpp.h"       // NO_G3_REWRITE

#include "tools/window/unix/GraphiteDawnXlibWindowContext_unix.h"

#include "tools/window/GraphiteDawnWindowContext.h"
#include "tools/window/unix/XlibWindowInfo.h"

using skwindow::XlibWindowInfo;
using skwindow::DisplayParams;
using skwindow::internal::GraphiteDawnWindowContext;

namespace {

wgpu::BackendType ToDawnBackendType(sk_app::Window::BackendType backendType) {
    switch (backendType) {
        case sk_app::Window::kGraphiteDawnVulkan_BackendType:
            return wgpu::BackendType::Vulkan;
        case sk_app::Window::kGraphiteDawnOpenGLES_BackendType:
            return wgpu::BackendType::OpenGLES;
        default:
            SkASSERT(false);
            return wgpu::BackendType::Vulkan;
    }
}

wgpu::TextureFormat GetPreferredFormat(sk_app::Window::BackendType backendType) {
    if (backendType == sk_app::Window::kGraphiteDawnOpenGLES_BackendType) {
        return wgpu::TextureFormat::RGBA8Unorm;
    }
    return wgpu::TextureFormat::BGRA8Unorm;
}

class GraphiteDawnXlibWindowContext_unix : public GraphiteDawnWindowContext {
public:
    GraphiteDawnXlibWindowContext_unix(const XlibWindowInfo& info,
                                       std::unique_ptr<const DisplayParams> params,
                                       sk_app::Window::BackendType backendType);

    ~GraphiteDawnXlibWindowContext_unix() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;
    void resize(int w, int h) override;

private:
    Display*           fDisplay;
    XWindow            fWindow;
    wgpu::BackendType  fBackendType;
};

GraphiteDawnXlibWindowContext_unix::GraphiteDawnXlibWindowContext_unix(
        const XlibWindowInfo& info,
        std::unique_ptr<const DisplayParams> params,
        sk_app::Window::BackendType backendType)
        : GraphiteDawnWindowContext(std::move(params), GetPreferredFormat(backendType))
        , fDisplay(info.fDisplay)
        , fWindow(info.fWindow)
        , fBackendType(ToDawnBackendType(backendType)) {
    XWindow root;
    int x, y;
    unsigned int border_width, depth;
    unsigned int width, height;
    XGetGeometry(fDisplay, fWindow, &root, &x, &y, &width, &height, &border_width, &depth);
    this->initializeContext(width, height);
}

GraphiteDawnXlibWindowContext_unix::~GraphiteDawnXlibWindowContext_unix() {
    this->destroyContext();
}

bool GraphiteDawnXlibWindowContext_unix::onInitializeContext() {
    SkASSERT(!!fWindow);

    auto device = this->createDevice(fBackendType);
    if (!device) {
        SkASSERT(device);
        return false;
    }

    wgpu::SurfaceSourceXlibWindow surfaceChainedDesc;
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
    configureSurface();

    return true;
}

void GraphiteDawnXlibWindowContext_unix::onDestroyContext() {}

void GraphiteDawnXlibWindowContext_unix::resize(int w, int h) {
    configureSurface();
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGraphiteDawnForXlib(
        const XlibWindowInfo& info,
        std::unique_ptr<const DisplayParams> params,
        sk_app::Window::BackendType backendType) {
    std::unique_ptr<WindowContext> ctx(
            new GraphiteDawnXlibWindowContext_unix(info, std::move(params), backendType));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/android/WindowContextFactory_android.h"
#include "tools/window/GraphiteDawnWindowContext.h"

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

class GraphiteDawnWindowContext_android : public GraphiteDawnWindowContext {
public:
    GraphiteDawnWindowContext_android(ANativeWindow* window,
                                      std::unique_ptr<const DisplayParams> params,
                                      sk_app::Window::BackendType backendType);

    ~GraphiteDawnWindowContext_android() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;
    void resize(int w, int h) override;

private:
    ANativeWindow*     fWindow;
    wgpu::BackendType  fBackendType;
};

GraphiteDawnWindowContext_android::GraphiteDawnWindowContext_android(
        ANativeWindow* window,
        std::unique_ptr<const DisplayParams> params,
        sk_app::Window::BackendType backendType)
        : GraphiteDawnWindowContext(std::move(params), wgpu::TextureFormat::RGBA8Unorm)
        , fWindow(window)
        , fBackendType(ToDawnBackendType(backendType)) {
    auto width = ANativeWindow_getWidth(window);
    auto height = ANativeWindow_getHeight(window);

    this->initializeContext(width, height);
}

GraphiteDawnWindowContext_android::~GraphiteDawnWindowContext_android() {
    this->destroyContext();
}

bool GraphiteDawnWindowContext_android::onInitializeContext() {
    SkASSERT(!!fWindow);

    auto device = this->createDevice(fBackendType);
    if (!device) {
        SkASSERT(device);
        return false;
    }

    wgpu::SurfaceSourceAndroidNativeWindow surfaceChainedDesc;
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

void GraphiteDawnWindowContext_android::onDestroyContext() {}

void GraphiteDawnWindowContext_android::resize(int w, int h) {
    configureSurface();
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGraphiteDawnForAndroid(
        ANativeWindow* window, std::unique_ptr<const DisplayParams> params,
        sk_app::Window::BackendType backendType) {
    std::unique_ptr<WindowContext> ctx(
            new GraphiteDawnWindowContext_android(window, std::move(params), backendType));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

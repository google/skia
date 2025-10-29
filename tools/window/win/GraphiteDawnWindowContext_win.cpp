/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/GraphiteDawnWindowContext.h"
#include "tools/window/win/WindowContextFactory_win.h"

using skwindow::DisplayParams;
using skwindow::internal::GraphiteDawnWindowContext;

namespace {

wgpu::BackendType ToDawnBackendType(sk_app::Window::BackendType backendType) {
    switch (backendType) {
        case sk_app::Window::kGraphiteDawnD3D11_BackendType:
            return wgpu::BackendType::D3D11;
        case sk_app::Window::kGraphiteDawnD3D12_BackendType:
            return wgpu::BackendType::D3D12;
        default:
            SkASSERT(false);
            return wgpu::BackendType::D3D12;
    }
}

class GraphiteDawnWindowContext_win : public GraphiteDawnWindowContext {
public:
    GraphiteDawnWindowContext_win(HWND hwnd, std::unique_ptr<const DisplayParams> params,
                                  sk_app::Window::BackendType backendType);

    ~GraphiteDawnWindowContext_win() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;
    void resize(int w, int h) override;

private:
    HWND              fWindow;
    wgpu::BackendType fBackendType;
};

GraphiteDawnWindowContext_win::GraphiteDawnWindowContext_win(
        HWND hwnd,
        std::unique_ptr<const DisplayParams> params,
        sk_app::Window::BackendType backendType)
        : GraphiteDawnWindowContext(std::move(params), wgpu::TextureFormat::BGRA8Unorm)
        , fWindow(hwnd)
        , fBackendType(ToDawnBackendType(backendType)) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    this->initializeContext(rect.right - rect.left, rect.bottom - rect.top);
}

GraphiteDawnWindowContext_win::~GraphiteDawnWindowContext_win() {
    this->destroyContext();
}

bool GraphiteDawnWindowContext_win::onInitializeContext() {
    SkASSERT(!!fWindow);

    auto device = this->createDevice(fBackendType);
    if (!device) {
        SkASSERT(device);
        return false;
    }

    wgpu::SurfaceSourceWindowsHWND surfaceChainedDesc;
    surfaceChainedDesc.hwnd = fWindow;
    surfaceChainedDesc.hinstance = GetModuleHandle(nullptr);
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

void GraphiteDawnWindowContext_win::onDestroyContext() {}

void GraphiteDawnWindowContext_win::resize(int w, int h) {
    configureSurface();
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGraphiteDawnForWin(
        HWND hwnd, std::unique_ptr<const DisplayParams> params, sk_app::Window::BackendType backendType) {
    std::unique_ptr<WindowContext> ctx(
            new GraphiteDawnWindowContext_win(hwnd, std::move(params), backendType));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

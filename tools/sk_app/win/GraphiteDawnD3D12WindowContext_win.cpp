/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/GraphiteDawnWindowContext.h"
#include "tools/sk_app/win/WindowContextFactory_win.h"

using sk_app::DisplayParams;
using sk_app::GraphiteDawnWindowContext;

namespace {

class GraphiteDawnD3D12WindowContext_win : public GraphiteDawnWindowContext {
public:
    GraphiteDawnD3D12WindowContext_win(HWND hwnd, const DisplayParams& params);

    ~GraphiteDawnD3D12WindowContext_win() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;
    void onSwapBuffers() override;
    void resize(int w, int h) override;

private:
    HWND fWindow;
};

GraphiteDawnD3D12WindowContext_win::GraphiteDawnD3D12WindowContext_win(HWND hwnd,
                                                                       const DisplayParams& params)
        : GraphiteDawnWindowContext(params, wgpu::TextureFormat::BGRA8Unorm), fWindow(hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    this->initializeContext(rect.right - rect.left, rect.bottom - rect.top);
}

GraphiteDawnD3D12WindowContext_win::~GraphiteDawnD3D12WindowContext_win() {
    this->destroyContext();
}

bool GraphiteDawnD3D12WindowContext_win::onInitializeContext() {
    SkASSERT(!!fWindow);

    auto device = this->createDevice(wgpu::BackendType::D3D12);
    if (!device) {
        SkASSERT(device);
        return false;
    }

    wgpu::SurfaceDescriptorFromWindowsHWND surfaceChainedDesc;
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
    fSwapChain = this->createSwapChain();

    return true;
}

void GraphiteDawnD3D12WindowContext_win::onDestroyContext() {}

void GraphiteDawnD3D12WindowContext_win::onSwapBuffers() {}

void GraphiteDawnD3D12WindowContext_win::resize(int w, int h) {
    fSwapChain = this->createSwapChain();
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeGraphiteDawnD3D12ForWin(HWND hwnd, const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GraphiteDawnD3D12WindowContext_win(hwnd, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app

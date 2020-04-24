/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/DawnWindowContext.h"
#include "tools/sk_app/win/WindowContextFactory_win.h"
#include "dawn/webgpu_cpp.h"
#include "dawn/dawn_wsi.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/D3D12Backend.h"
#include "common/SwapChainUtils.h"

namespace sk_app {

class DawnD3D12WindowContext : public DawnWindowContext {
public:
    DawnD3D12WindowContext(HWND hwnd, const DisplayParams& params);
    ~DawnD3D12WindowContext() override;
    wgpu::Device onInitializeContext() override;
    void onDestroyContext() override;
    DawnSwapChainImplementation createSwapChainImplementation(
            int width, int height, const DisplayParams& params) override;
    void onSwapBuffers() override;
private:
    HWND                 fWindow;
};

// NOTE: this texture format must match the one in D3D12's swap chain impl
DawnD3D12WindowContext::DawnD3D12WindowContext(HWND hwnd, const DisplayParams& params)
    : DawnWindowContext(params, wgpu::TextureFormat::RGBA8Unorm)
    , fWindow(hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    this->initializeContext(rect.right - rect.left, rect.bottom - rect.top);
}

DawnD3D12WindowContext::~DawnD3D12WindowContext() {
    this->destroyContext();
}

DawnSwapChainImplementation DawnD3D12WindowContext::createSwapChainImplementation(
        int width, int height, const DisplayParams& params) {
    return dawn_native::d3d12::CreateNativeSwapChainImpl(fDevice.Get(), fWindow);
}

wgpu::Device DawnD3D12WindowContext::onInitializeContext() {
    return this->createDevice(dawn_native::BackendType::D3D12);
}

void DawnD3D12WindowContext::onDestroyContext() {
}

void DawnD3D12WindowContext::onSwapBuffers() {
}

namespace window_context_factory {

std::unique_ptr<WindowContext> MakeDawnD3D12ForWin(HWND hwnd,
                                                   const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new DawnD3D12WindowContext(hwnd, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}

}   //namespace sk_app

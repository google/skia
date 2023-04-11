/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/DawnWindowContext.h"
#include "tools/sk_app/win/WindowContextFactory_win.h"
#include "webgpu/webgpu_cpp.h"
#include "dawn/native/DawnNative.h"

namespace sk_app {

class DawnD3D12WindowContext : public DawnWindowContext {
public:
    DawnD3D12WindowContext(HWND hwnd, const DisplayParams& params);
    ~DawnD3D12WindowContext() override;
    wgpu::Device onInitializeContext() override;
    void onDestroyContext() override;
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

wgpu::Device DawnD3D12WindowContext::onInitializeContext() {
    wgpu::SurfaceDescriptorFromWindowsHWND hwndDesc;
    hwndDesc.hwnd = fWindow;
    hwndDesc.hinstance = GetModuleHandle(nullptr);

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &hwndDesc;

    fDawnSurface = wgpu::Instance(fInstance->Get()).CreateSurface(&surfaceDesc);
    SkASSERT(fDawnSurface);

    return this->createDevice(wgpu::BackendType::D3D12);
}

void DawnD3D12WindowContext::onDestroyContext() {
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

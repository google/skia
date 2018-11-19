/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../DawnWindowContext.h"
#include "WindowContextFactory_win.h"
#include "dawn/GrDawnBackendContext.h"
#include "dawn/dawncpp.h"
#include "dawn/dawn_wsi.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/D3D12Backend.h"
#include "common/SwapChainUtils.h"

namespace sk_app {

class DawnD3D12WindowContext : public DawnWindowContext {
public:
    DawnD3D12WindowContext(HWND hwnd, const DisplayParams& params);
    virtual ~DawnD3D12WindowContext();
    sk_sp<GrDawnBackendContext> onInitializeContext() override;
    void onDestroyContext() override;
    dawnSwapChainImplementation createSwapChainImplementation(int width, int height, const DisplayParams& params) override;
    void onSwapBuffers() override;
private:
    HWND                 fWindow;
};

// NOTE: this texture format must match the one in D3D12's swap chain impl
DawnD3D12WindowContext::DawnD3D12WindowContext(HWND hwnd, const DisplayParams& params)
    : DawnWindowContext(params, dawn::TextureFormat::R8G8B8A8Unorm)
    , fWindow(hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    this->initializeContext(rect.right - rect.left, rect.bottom - rect.top);
}

DawnD3D12WindowContext::~DawnD3D12WindowContext() {
    this->destroyContext();
}

dawnSwapChainImplementation DawnD3D12WindowContext::createSwapChainImplementation(int width, int height, const DisplayParams& params) {
    return dawn_native::d3d12::CreateNativeSwapChainImpl(fDevice.Get(), fWindow);
}

sk_sp<GrDawnBackendContext> DawnD3D12WindowContext::onInitializeContext() {

    dawnProcTable backendProcs = dawn_native::GetProcs();
    dawnDevice backendDevice = dawn_native::d3d12::CreateDevice();
    dawnSetProcs(&backendProcs);
    dawnQueue backendQueue = dawnDeviceCreateQueue(backendDevice);
    sk_sp<GrDawnBackendContext> ctx(new GrDawnBackendContext(backendDevice, backendQueue));

    return ctx;
}

void DawnD3D12WindowContext::onDestroyContext() {
}

void DawnD3D12WindowContext::onSwapBuffers() {
}

namespace window_context_factory {

WindowContext* NewDawnD3D12ForWin(HWND hwnd, const DisplayParams& params) {
    WindowContext* ctx = new DawnD3D12WindowContext(hwnd, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}

}   //namespace sk_app

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../NXTWindowContext.h"
#include "GrNXTBackendContext.h"
#include "WindowContextFactory_win.h"
#include "nxt/nxtcpp.h"
#include "nxt/nxt_wsi.h"
#include "common/SwapChainUtils.h"

namespace backend {
    namespace d3d12 {
        void Init(nxtProcTable* procs, nxtDevice* device);
        nxtSwapChainImplementation CreateNativeSwapChainImpl(nxtDevice device, HWND);
    }
}

namespace sk_app {

class NXTD3D12WindowContext : public NXTWindowContext {
public:
    NXTD3D12WindowContext(HWND hwnd, const DisplayParams& params);
    virtual ~NXTD3D12WindowContext();
    sk_sp<GrNXTBackendContext> onInitializeContext() override;
    void onDestroyContext() override;
    nxtSwapChainImplementation createSwapChainImplementation(int width, int height, const DisplayParams& params) override;
    void onSwapBuffers() override;
private:
    HWND                 fWindow;
    nxt::Device          fDevice;
};

// NOTE: this texture format must match the one in D3D12's swap chain impl
NXTD3D12WindowContext::NXTD3D12WindowContext(HWND hwnd, const DisplayParams& params)
    : NXTWindowContext(params, nxt::TextureFormat::R8G8B8A8Unorm)
    , fWindow(hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    this->initializeContext(rect.right - rect.left, rect.bottom - rect.top);
}

NXTD3D12WindowContext::~NXTD3D12WindowContext() {
    this->destroyContext();
}

nxtSwapChainImplementation NXTD3D12WindowContext::createSwapChainImplementation(int width, int height, const DisplayParams& params) {
    return backend::d3d12::CreateNativeSwapChainImpl(fDevice.Get(), fWindow);
}

sk_sp<GrNXTBackendContext> NXTD3D12WindowContext::onInitializeContext() {

    nxtDevice backendDevice;
    nxtProcTable backendProcs;

    backend::d3d12::Init(&backendProcs, &backendDevice);
    nxtSetProcs(&backendProcs);
    fDevice = nxt::Device::Acquire(backendDevice);
    nxt::Queue queue = fDevice.CreateQueueBuilder().GetResult();
    sk_sp<GrNXTBackendContext> ctx(new GrNXTBackendContext(fDevice.Clone(), queue.Clone()));

    return ctx;
}

void NXTD3D12WindowContext::onDestroyContext() {
}

void NXTD3D12WindowContext::onSwapBuffers() {
}

namespace window_context_factory {

WindowContext* NewNXTD3D12ForWin(HWND hwnd, const DisplayParams& params) {
    WindowContext* ctx = new NXTD3D12WindowContext(hwnd, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}

}   //namespace sk_app

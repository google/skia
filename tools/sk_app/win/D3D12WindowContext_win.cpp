/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/WindowContext.h"
#include "tools/sk_app/win/WindowContextFactory_win.h"

#include "tools/gpu/d3d/D3DTestUtils.h"

#include "include/core/SkSurface.h"
#include "include/gpu/d3d/GrD3DBackendContext.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

namespace sk_app {

class D3D12WindowContext : public WindowContext {
public:
    D3D12WindowContext(HWND hwnd, const DisplayParams& params);
    ~D3D12WindowContext() override;
    void initializeContext();
    void destroyContext();

    bool isValid() {
        return fDevice.get() != nullptr;
    }

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    void resize(int width, int height) override { /* TODO */ }
    void setDisplayParams(const DisplayParams& params) override { /* TODO */ }
private:
    static constexpr int kNumFrames = 2;

    HWND fWindow;
    gr_cp<ID3D12Device> fDevice;
    gr_cp<ID3D12CommandQueue> fQueue;
    gr_cp<IDXGISwapChain3> fSwapChain;
    gr_cp<ID3D12Resource> fBuffers[kNumFrames];
    sk_sp<SkSurface> fSurfaces[kNumFrames];

    // Synchronization objects.
    unsigned int fBufferIndex;
    HANDLE fFenceEvent;
    gr_cp<ID3D12Fence> fFence;
    uint64_t fFenceValues[kNumFrames];
};

D3D12WindowContext::D3D12WindowContext(HWND hwnd, const DisplayParams& params)
    : WindowContext(params)
    , fWindow(hwnd) {

    this->initializeContext();

}

D3D12WindowContext::~D3D12WindowContext() {
    this->destroyContext();
}

void D3D12WindowContext::initializeContext() {
    GrD3DBackendContext backendContext;
    sk_gpu_test::CreateD3DBackendContext(&backendContext);
    fDevice = backendContext.fDevice;
    fQueue = backendContext.fQueue;

    fContext = GrContext::MakeDirect3D(backendContext, fDisplayParams.fGrContextOptions);

    // Make the swapchain
    RECT windowRect;
    GetWindowRect(fWindow, &windowRect);
    unsigned int width = windowRect.right - windowRect.left;
    unsigned int height = windowRect.bottom - windowRect.top;

    UINT dxgiFactoryFlags = 0;
    SkDEBUGCODE(dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;)

    ComPtr<IDXGIFactory4> factory;
    HRESULT result = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
    SkASSERT(SUCCEEDED(result));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = kNumFrames;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1; // TODO: support MSAA

    gr_cp<IDXGISwapChain1> swapChain;
    result = factory->CreateSwapChainForHwnd(
            fQueue.get(), fWindow, &swapChainDesc, nullptr, nullptr, &swapChain);
    SkASSERT(SUCCEEDED(result));

    // We don't support fullscreen transitions.
    result = factory->MakeWindowAssociation(fWindow, DXGI_MWA_NO_ALT_ENTER);

    result = swapChain->QueryInterface(IID_PPV_ARGS(&fSwapChain));
    SkASSERT(SUCCEEDED(result));

    fBufferIndex = fSwapChain->GetCurrentBackBufferIndex();

    // set up base resource info
    GrD3DTextureResourceInfo info(nullptr,
                                  D3D12_RESOURCE_STATE_PRESENT,
                                  DXGI_FORMAT_R8G8B8A8_UNORM,
                                  1,
                                  0);
    for (int i = 0; i < kNumFrames; ++i) {
        result = fSwapChain->GetBuffer(i, IID_PPV_ARGS(&fBuffers[i]));
        SkASSERT(SUCCEEDED(result));

        info.fResource = fBuffers[i];

        // TODO: support MSAA
        GrBackendRenderTarget backendRT(width, height, 1, info);
        fSurfaces[i] = SkSurface::MakeFromBackendRenderTarget(
            fContext.get(), backendRT, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
            fDisplayParams.fColorSpace, &fDisplayParams.fSurfaceProps);
    }

    result = fDevice->CreateFence(fFenceValues[fBufferIndex], D3D12_FENCE_FLAG_NONE,
                                  IID_PPV_ARGS(&fFence));
    SkASSERT(SUCCEEDED(result));
    fFenceValues[fBufferIndex]++;

    fFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    SkASSERT(fFenceEvent);

    fWidth = width;
    fHeight = height;
}

void D3D12WindowContext::destroyContext() {
    // TODO
    // probably need to teardown swapchain here
}

sk_sp<SkSurface> D3D12WindowContext::getBackbufferSurface() {
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = fFenceValues[fBufferIndex];
    HRESULT result = fQueue->Signal(fFence.get(), currentFenceValue);
    SkASSERT(SUCCEEDED(result));

    // Update the frame index.
    fBufferIndex = fSwapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (fFence->GetCompletedValue() < fFenceValues[fBufferIndex]) {
        result = fFence->SetEventOnCompletion(fFenceValues[fBufferIndex], fFenceEvent);
        SkASSERT(SUCCEEDED(result));
        WaitForSingleObjectEx(fFenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    fFenceValues[fBufferIndex] = currentFenceValue + 1;

    return fSurfaces[fBufferIndex];
}

void D3D12WindowContext::swapBuffers() {
    SkSurface* surface = fSurfaces[fBufferIndex].get();

    GrFlushInfo info;
    info.fNumSemaphores = 0;
    info.fSignalSemaphores = nullptr;
    info.fFlags = kForce_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kPresent, info);

    HRESULT result = fSwapChain->Present(1, 0);
    SkASSERT(SUCCEEDED(result));
}

namespace window_context_factory {

std::unique_ptr<WindowContext> MakeD3D12ForWin(HWND hwnd, const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new D3D12WindowContext(hwnd, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}

}   //namespace sk_app

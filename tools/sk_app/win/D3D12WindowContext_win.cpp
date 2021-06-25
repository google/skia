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
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/d3d/GrD3DBackendContext.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#define GR_D3D_CALL_ERRCHECK(X)                                         \
    do {                                                                \
        HRESULT result = X;                                             \
        SkASSERT(SUCCEEDED(result));                                    \
        if (!SUCCEEDED(result)) {                                       \
            SkDebugf("Failed Direct3D call. Error: 0x%08lx\n", result); \
        }                                                               \
    } while (false)

using namespace Microsoft::WRL;

namespace sk_app {

class D3D12WindowContext : public WindowContext {
public:
    D3D12WindowContext(HWND hwnd, const DisplayParams& params);
    ~D3D12WindowContext() override;
    void initializeContext();
    void destroyContext();
    void setupSurfaces(int width, int height);

    bool isValid() override {
        return fDevice.get() != nullptr;
    }

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    void resize(int width, int height) override;
    void setDisplayParams(const DisplayParams& params) override;
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

    fContext = GrDirectContext::MakeDirect3D(backendContext, fDisplayParams.fGrContextOptions);
    SkASSERT(fContext);

    // Make the swapchain
    RECT windowRect;
    GetWindowRect(fWindow, &windowRect);
    unsigned int width = windowRect.right - windowRect.left;
    unsigned int height = windowRect.bottom - windowRect.top;

    UINT dxgiFactoryFlags = 0;
    SkDEBUGCODE(dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;)

    gr_cp<IDXGIFactory4> factory;
    GR_D3D_CALL_ERRCHECK(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = kNumFrames;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    gr_cp<IDXGISwapChain1> swapChain;
    GR_D3D_CALL_ERRCHECK(factory->CreateSwapChainForHwnd(
            fQueue.get(), fWindow, &swapChainDesc, nullptr, nullptr, &swapChain));

    // We don't support fullscreen transitions.
    GR_D3D_CALL_ERRCHECK(factory->MakeWindowAssociation(fWindow, DXGI_MWA_NO_ALT_ENTER));

    GR_D3D_CALL_ERRCHECK(swapChain->QueryInterface(IID_PPV_ARGS(&fSwapChain)));

    fBufferIndex = fSwapChain->GetCurrentBackBufferIndex();

    fSampleCount = fDisplayParams.fMSAASampleCount;

    this->setupSurfaces(width, height);

    for (int i = 0; i < kNumFrames; ++i) {
        fFenceValues[i] = 10000;   // use a high value to make it easier to track these in PIX
    }
    GR_D3D_CALL_ERRCHECK(fDevice->CreateFence(fFenceValues[fBufferIndex], D3D12_FENCE_FLAG_NONE,
                                              IID_PPV_ARGS(&fFence)));

    fFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    SkASSERT(fFenceEvent);

    fWidth = width;
    fHeight = height;
}

void D3D12WindowContext::setupSurfaces(int width, int height) {
    // set up base resource info
    GrD3DTextureResourceInfo info(nullptr,
                                  nullptr,
                                  D3D12_RESOURCE_STATE_PRESENT,
                                  DXGI_FORMAT_R8G8B8A8_UNORM,
                                  1,
                                  1,
                                  0);
    for (int i = 0; i < kNumFrames; ++i) {
        GR_D3D_CALL_ERRCHECK(fSwapChain->GetBuffer(i, IID_PPV_ARGS(&fBuffers[i])));

        SkASSERT(fBuffers[i]->GetDesc().Width == (UINT64)width &&
                 fBuffers[i]->GetDesc().Height == (UINT64)height);

        info.fResource = fBuffers[i];
        if (fSampleCount > 1) {
            GrBackendTexture backendTexture(width, height, info);
            fSurfaces[i] = SkSurface::MakeFromBackendTexture(
                fContext.get(), backendTexture, kTopLeft_GrSurfaceOrigin, fSampleCount,
                kRGBA_8888_SkColorType, fDisplayParams.fColorSpace, &fDisplayParams.fSurfaceProps);
        } else {
            GrBackendRenderTarget backendRT(width, height, info);
            fSurfaces[i] = SkSurface::MakeFromBackendRenderTarget(
                fContext.get(), backendRT, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                fDisplayParams.fColorSpace, &fDisplayParams.fSurfaceProps);
        }
    }
}

void D3D12WindowContext::destroyContext() {
    CloseHandle(fFenceEvent);
    fFence.reset(nullptr);

    for (int i = 0; i < kNumFrames; ++i) {
        fSurfaces[i].reset(nullptr);
        fBuffers[i].reset(nullptr);
    }

    fSwapChain.reset(nullptr);
    fQueue.reset(nullptr);
    fDevice.reset(nullptr);
}

sk_sp<SkSurface> D3D12WindowContext::getBackbufferSurface() {
    // Update the frame index.
    const UINT64 currentFenceValue = fFenceValues[fBufferIndex];
    fBufferIndex = fSwapChain->GetCurrentBackBufferIndex();

    // If the last frame for this buffer index is not done, wait until it is ready.
    if (fFence->GetCompletedValue() < fFenceValues[fBufferIndex]) {
        GR_D3D_CALL_ERRCHECK(fFence->SetEventOnCompletion(fFenceValues[fBufferIndex], fFenceEvent));
        WaitForSingleObjectEx(fFenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    fFenceValues[fBufferIndex] = currentFenceValue + 1;

    return fSurfaces[fBufferIndex];
}

void D3D12WindowContext::swapBuffers() {
    SkSurface* surface = fSurfaces[fBufferIndex].get();

    GrFlushInfo info;
    surface->flush(SkSurface::BackendSurfaceAccess::kPresent, info);
    fContext->submit();

    GR_D3D_CALL_ERRCHECK(fSwapChain->Present(1, 0));

    // Schedule a Signal command in the queue.
    GR_D3D_CALL_ERRCHECK(fQueue->Signal(fFence.get(), fFenceValues[fBufferIndex]));
}

void D3D12WindowContext::resize(int width, int height) {
    // Clean up any outstanding resources in command lists
    fContext->flush({});
    fContext->submit(true);

    // release the previous surface and backbuffer resources
    for (int i = 0; i < kNumFrames; ++i) {
        // Let present complete
        if (fFence->GetCompletedValue() < fFenceValues[i]) {
            GR_D3D_CALL_ERRCHECK(fFence->SetEventOnCompletion(fFenceValues[i], fFenceEvent));
            WaitForSingleObjectEx(fFenceEvent, INFINITE, FALSE);
        }
        fSurfaces[i].reset(nullptr);
        fBuffers[i].reset(nullptr);
    }

    GR_D3D_CALL_ERRCHECK(fSwapChain->ResizeBuffers(0, width, height,
                                                   DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    this->setupSurfaces(width, height);

    fWidth = width;
    fHeight = height;
}

void D3D12WindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext();
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

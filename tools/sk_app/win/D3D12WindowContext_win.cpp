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
        return false;
    }

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    void resize(int width, int height) override { /* TODO */ }
    void setDisplayParams(const DisplayParams& params) override { /* TODO */ }
private:
    HWND fWindow;
    bool fDisableVsync;
    gr_cp<ID3D12Device> fDevice;
    gr_cp<ID3D12CommandQueue> fQueue;
    gr_cp<IDXGISwapChain3> fSwapChain;
    ComPtr<ID3D12DescriptorHeap> fDescriptorHeap;
    ComPtr<ID3D12Resource> fBackBufferRenderTarget[2];
    unsigned int fBufferIndex;
    ComPtr<ID3D12CommandAllocator> fCommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> fCommandList;
    ComPtr<ID3D12PipelineState> fPipelineState;
    ComPtr<ID3D12Fence> fFence;
    HANDLE fFenceEvent;
    unsigned long long fFenceValue;

};

static std::pair<unsigned int, unsigned int> get_refresh_rate(IDXGIAdapter1* adapter,
                                                              unsigned int width,
                                                              unsigned int height) {
    unsigned int numerator = 0;
    unsigned int denominator = 1;
    HRESULT result;
    IDXGIOutput* output;
    unsigned int numModes;
    DXGI_ADAPTER_DESC adapterDesc;

    result = adapter->EnumOutputs(0, &output);
    SkASSERT(SUCCEEDED(result));

    result = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED,
                                        &numModes, NULL);
    SkASSERT(SUCCEEDED(result));

    SkAutoTArray<DXGI_MODE_DESC> modeDescs(numModes);
    result = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED,
                                        &numModes, modeDescs.get());
    SkASSERT(SUCCEEDED(result));

    // seems like this will only work for fullscreen
    for (int i = 0; i < numModes; ++i) {
        if (modeDescs[i].Width == width && modeDescs[i].Height == height) {
            numerator = modeDescs[i].RefreshRate.Numerator;
            denominator = modeDescs[i].RefreshRate.Denominator;
        }
    }

    return std::make_pair(numerator, denominator);
}

D3D12WindowContext::D3D12WindowContext(HWND hwnd, const DisplayParams& params)
    : WindowContext(params)
    , fWindow(hwnd)
    , fDisableVsync(params.fDisableVsync)
    , fFenceValue(0) {

    GrD3DBackendContext backendContext;
    sk_gpu_test::CreateD3DBackendContext(&backendContext);
    fDevice = backendContext.fDevice;
    fQueue = backendContext.fQueue;

    fContext = GrContext::MakeDirect3D(backendContext, fDisplayParams.fGrContextOptions);

    RECT windowRect;
    GetWindowRect(fWindow, &windowRect);
    unsigned int width = windowRect.right - windowRect.left;
    unsigned int height = windowRect.bottom - windowRect.top;
    unsigned int numerator, denominator;
    std::tie(numerator, denominator) = get_refresh_rate(backendContext.fAdapter.get(),
                                                        width, height);

    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Width = width;
    swapDesc.BufferDesc.Height = height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.OutputWindow = fWindow;
    swapDesc.Windowed = true;
    if (fDisableVsync) {
        swapDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    } else {
        swapDesc.BufferDesc.RefreshRate.Numerator = numerator;
        swapDesc.BufferDesc.RefreshRate.Denominator = denominator;
    }

    // no MSAA for now
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;

    swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapDesc.Flags = 0;

    IDXGISwapChain* swapChain;
    HRESULT result = backendContext.fFactory->CreateSwapChain(fQueue.get(), &swapDesc, &swapChain);
    SkASSERT(SUCCEEDED(result));
    result = swapChain->QueryInterface(IID_PPV_ARGS(&fSwapChain));
    SkASSERT(SUCCEEDED(result));
}

D3D12WindowContext::~D3D12WindowContext() {
    this->destroyContext();
}

void D3D12WindowContext::initializeContext() {
    // TODO
    // probably need to create swapchain here
}

void D3D12WindowContext::destroyContext() {
    // TODO
    // probably need to teardown swapchain here
}

sk_sp<SkSurface> D3D12WindowContext::getBackbufferSurface() {
    return nullptr;
}

void D3D12WindowContext::swapBuffers() {
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

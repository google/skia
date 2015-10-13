//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// NativeWindow.cpp: Handler for managing HWND native window types.

#include "libANGLE/renderer/d3d/d3d11/NativeWindow.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

#include "common/debug.h"

namespace rx
{

NativeWindow::NativeWindow(EGLNativeWindowType window, const egl::Config *)
    : mWindow(window)
{
}

bool NativeWindow::initialize()
{
    return true;
}

bool NativeWindow::getClientRect(LPRECT rect)
{
    return GetClientRect(mWindow, rect) == TRUE;
}

bool NativeWindow::isIconic()
{
    return IsIconic(mWindow) == TRUE;
}

bool NativeWindow::isValidNativeWindow(EGLNativeWindowType window)
{
    return IsWindow(window) == TRUE;
}

HRESULT NativeWindow::createSwapChain(ID3D11Device* device, DXGIFactory* factory,
                                      DXGI_FORMAT format, unsigned int width, unsigned int height,
                                      DXGISwapChain** swapChain)
{
    if (device == NULL || factory == NULL || swapChain == NULL || width == 0 || height == 0)
    {
        return E_INVALIDARG;
    }

    // Use IDXGIFactory2::CreateSwapChainForHwnd if DXGI 1.2 is available to create a DXGI_SWAP_EFFECT_SEQUENTIAL swap chain.
    IDXGIFactory2 *factory2 = d3d11::DynamicCastComObject<IDXGIFactory2>(factory);
    if (factory2 != nullptr)
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = format;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
        swapChainDesc.BufferCount = 1;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags = 0;
        IDXGISwapChain1 *swapChain1 = nullptr;
        HRESULT result = factory2->CreateSwapChainForHwnd(device, mWindow, &swapChainDesc, nullptr, nullptr, &swapChain1);
        if (SUCCEEDED(result))
        {
            *swapChain = static_cast<DXGISwapChain*>(swapChain1);
        }
        SafeRelease(factory2);
        return result;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = format;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.Flags = 0;
    swapChainDesc.OutputWindow = mWindow;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    return factory->CreateSwapChain(device, &swapChainDesc, swapChain);
}
}

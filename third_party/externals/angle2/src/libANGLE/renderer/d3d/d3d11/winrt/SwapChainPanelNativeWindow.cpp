//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SwapChainPanelNativeWindow.cpp: NativeWindow for managing ISwapChainPanel native window types.

#include "libANGLE/renderer/d3d/d3d11/winrt/SwapChainPanelNativeWindow.h"

#include <algorithm>
#include <math.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::Xaml;
using namespace Microsoft::WRL;

namespace rx
{
SwapChainPanelNativeWindow::~SwapChainPanelNativeWindow()
{
    unregisterForSizeChangeEvents();
}

template <typename T>
struct AddFtmBase
{
    typedef Implements<RuntimeClassFlags<ClassicCom>, T, FtmBase> Type;
};

template <typename CODE>
HRESULT RunOnUIThread(CODE &&code, const ComPtr<ICoreDispatcher> &dispatcher)
{
    ComPtr<IAsyncAction> asyncAction;
    HRESULT result = S_OK;

    boolean hasThreadAccess;
    result = dispatcher->get_HasThreadAccess(&hasThreadAccess);
    if (FAILED(result))
    {
        return result;
    }

    if (hasThreadAccess)
    {
        return code();
    }
    else
    {
        Event waitEvent(CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
        if (!waitEvent.IsValid())
        {
            return E_FAIL;
        }

        HRESULT codeResult = E_FAIL;
        auto handler =
            Callback<AddFtmBase<IDispatchedHandler>::Type>([&codeResult, &code, &waitEvent]
                                                           {
                                                               codeResult = code();
                                                               SetEvent(waitEvent.Get());
                                                               return S_OK;
                                                           });

        result = dispatcher->RunAsync(CoreDispatcherPriority_Normal, handler.Get(),
                                      asyncAction.GetAddressOf());
        if (FAILED(result))
        {
            return result;
        }

        auto waitResult = WaitForSingleObjectEx(waitEvent.Get(), 10 * 1000, true);
        if (waitResult != WAIT_OBJECT_0)
        {
            // Wait 10 seconds before giving up. At this point, the application is in an
            // unrecoverable state (probably deadlocked). We therefore terminate the application
            // entirely. This also prevents stack corruption if the async operation is eventually
            // run.
            ERR("Timeout waiting for async action on UI thread. The UI thread might be blocked.");
            std::terminate();
            return E_FAIL;
        }

        return codeResult;
    }
}

bool SwapChainPanelNativeWindow::initialize(EGLNativeWindowType window, IPropertySet *propertySet)
{
    ComPtr<IPropertySet> props = propertySet;
    ComPtr<IInspectable> win = window;
    SIZE swapChainSize = {};
    HRESULT result = S_OK;

    // IPropertySet is an optional parameter and can be null.
    // If one is specified, cache as an IMap and read the properties
    // used for initial host initialization.
    if (propertySet)
    {
        result = props.As(&mPropertyMap);
        if (FAILED(result))
        {
            return false;
        }

        // The EGLRenderSurfaceSizeProperty is optional and may be missing. The IPropertySet
        // was prevalidated to contain the EGLNativeWindowType before being passed to
        // this host.
        result = GetOptionalSizePropertyValue(mPropertyMap, EGLRenderSurfaceSizeProperty, &swapChainSize, &mSwapChainSizeSpecified);
        if (FAILED(result))
        {
            return false;
        }

        // The EGLRenderResolutionScaleProperty is optional and may be missing. The IPropertySet
        // was prevalidated to contain the EGLNativeWindowType before being passed to
        // this host.
        result = GetOptionalSinglePropertyValue(mPropertyMap, EGLRenderResolutionScaleProperty, &mSwapChainScale, &mSwapChainScaleSpecified);
        if (FAILED(result))
        {
            return false;
        }

        if (!mSwapChainScaleSpecified)
        {
            // Default value for the scale is 1.0f
            mSwapChainScale = 1.0f;
        }

        // A EGLRenderSurfaceSizeProperty and a EGLRenderResolutionScaleProperty can't both be specified
        if (mSwapChainScaleSpecified && mSwapChainSizeSpecified)
        {
            ERR("It is invalid to specify both an EGLRenderSurfaceSizeProperty and a EGLRenderResolutionScaleProperty.");
            return false;
        }
    }

    if (SUCCEEDED(result))
    {
        result = win.As(&mSwapChainPanel);
    }

    ComPtr<IDependencyObject> swapChainPanelDependencyObject;
    if (SUCCEEDED(result))
    {
        result = mSwapChainPanel.As(&swapChainPanelDependencyObject);
    }

    if (SUCCEEDED(result))
    {
        result = swapChainPanelDependencyObject->get_Dispatcher(
            mSwapChainPanelDispatcher.GetAddressOf());
    }

    if (SUCCEEDED(result))
    {
        // If a swapchain size is specfied, then the automatic resize
        // behaviors implemented by the host should be disabled.  The swapchain
        // will be still be scaled when being rendered to fit the bounds
        // of the host.
        // Scaling of the swapchain output needs to be handled by the
        // host for swapchain panels even though the scaling mode setting
        // DXGI_SCALING_STRETCH is configured on the swapchain.
        if (mSwapChainSizeSpecified)
        {
            mClientRect = { 0, 0, swapChainSize.cx, swapChainSize.cy };
        }
        else
        {
            SIZE swapChainPanelSize;
            result = GetSwapChainPanelSize(mSwapChainPanel, mSwapChainPanelDispatcher,
                                           &swapChainPanelSize);

            if (SUCCEEDED(result))
            {
                // Update the client rect to account for any swapchain scale factor
                mClientRect = { 0, 0, static_cast<long>(swapChainPanelSize.cx * mSwapChainScale), static_cast<long>(swapChainPanelSize.cy * mSwapChainScale) };
            }
        }
    }

    if (SUCCEEDED(result))
    {
        mNewClientRect = mClientRect;
        mClientRectChanged = false;
        return registerForSizeChangeEvents();
    }

    return false;
}

bool SwapChainPanelNativeWindow::registerForSizeChangeEvents()
{
    ComPtr<ISizeChangedEventHandler> sizeChangedHandler;
    ComPtr<IFrameworkElement> frameworkElement;
    HRESULT result = Microsoft::WRL::MakeAndInitialize<SwapChainPanelSizeChangedHandler>(sizeChangedHandler.ReleaseAndGetAddressOf(), this->shared_from_this());

    if (SUCCEEDED(result))
    {
        result = mSwapChainPanel.As(&frameworkElement);
    }

    if (SUCCEEDED(result))
    {
        result = RunOnUIThread(
            [this, frameworkElement, sizeChangedHandler]
            {
                return frameworkElement->add_SizeChanged(sizeChangedHandler.Get(),
                                                         &mSizeChangedEventToken);
            },
            mSwapChainPanelDispatcher);
    }

    if (SUCCEEDED(result))
    {
        return true;
    }

    return false;
}

void SwapChainPanelNativeWindow::unregisterForSizeChangeEvents()
{
    ComPtr<IFrameworkElement> frameworkElement;
    if (mSwapChainPanel && SUCCEEDED(mSwapChainPanel.As(&frameworkElement)))
    {
        RunOnUIThread(
            [this, frameworkElement]
            {
                return frameworkElement->remove_SizeChanged(mSizeChangedEventToken);
            },
            mSwapChainPanelDispatcher);
    }

    mSizeChangedEventToken.value = 0;
}

HRESULT SwapChainPanelNativeWindow::createSwapChain(ID3D11Device *device,
                                                    DXGIFactory *factory,
                                                    DXGI_FORMAT format,
                                                    unsigned int width,
                                                    unsigned int height,
                                                    bool containsAlpha,
                                                    DXGISwapChain **swapChain)
{
    if (device == NULL || factory == NULL || swapChain == NULL || width == 0 || height == 0)
    {
        return E_INVALIDARG;
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = format;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.AlphaMode =
        containsAlpha ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE;

    *swapChain = nullptr;

    ComPtr<IDXGISwapChain1> newSwapChain;
    ComPtr<ISwapChainPanelNative> swapChainPanelNative;
    SIZE currentPanelSize = {};

    HRESULT result = factory->CreateSwapChainForComposition(device, &swapChainDesc, nullptr, newSwapChain.ReleaseAndGetAddressOf());

    if (SUCCEEDED(result))
    {
        result = mSwapChainPanel.As(&swapChainPanelNative);
    }

    if (SUCCEEDED(result))
    {
        result = RunOnUIThread(
            [swapChainPanelNative, newSwapChain]
            {
                return swapChainPanelNative->SetSwapChain(newSwapChain.Get());
            },
            mSwapChainPanelDispatcher);
    }

    if (SUCCEEDED(result))
    {
        // The swapchain panel host requires an instance of the swapchain set on the SwapChainPanel
        // to perform the runtime-scale behavior.  This swapchain is cached here because there are
        // no methods for retreiving the currently configured on from ISwapChainPanelNative.
        mSwapChain = newSwapChain;
        result = newSwapChain.CopyTo(swapChain);
    }

    // If the host is responsible for scaling the output of the swapchain, then
    // scale it now before returning an instance to the caller.  This is done by
    // first reading the current size of the swapchain panel, then scaling
    if (SUCCEEDED(result))
    {
        if (mSwapChainSizeSpecified || mSwapChainScaleSpecified)
        {
            result = GetSwapChainPanelSize(mSwapChainPanel, mSwapChainPanelDispatcher,
                                           &currentPanelSize);

            // Scale the swapchain to fit inside the contents of the panel.
            if (SUCCEEDED(result))
            {
                result = scaleSwapChain(currentPanelSize, mClientRect);
            }
        }
    }

    return result;
}

HRESULT SwapChainPanelNativeWindow::scaleSwapChain(const SIZE &windowSize, const RECT &clientRect)
{
    Size renderScale = {(float)windowSize.cx / (float)clientRect.right,
                        (float)windowSize.cy / (float)clientRect.bottom};
    // Setup a scale matrix for the swap chain
    DXGI_MATRIX_3X2_F scaleMatrix = {};
    scaleMatrix._11 = renderScale.Width;
    scaleMatrix._22 = renderScale.Height;

    ComPtr<IDXGISwapChain2> swapChain2;
    HRESULT result = mSwapChain.As(&swapChain2);
    if (SUCCEEDED(result))
    {
        result = swapChain2->SetMatrixTransform(&scaleMatrix);
    }

    return result;
}

HRESULT GetSwapChainPanelSize(
    const ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> &swapChainPanel,
    const ComPtr<ICoreDispatcher> &dispatcher,
    SIZE *windowSize)
{
    ComPtr<IUIElement> uiElement;
    Size renderSize = {0, 0};
    HRESULT result = swapChainPanel.As(&uiElement);
    if (SUCCEEDED(result))
    {
        result = RunOnUIThread(
            [uiElement, &renderSize]
            {
                return uiElement->get_RenderSize(&renderSize);
            },
            dispatcher);
    }

    if (SUCCEEDED(result))
    {
        *windowSize = { lround(renderSize.Width), lround(renderSize.Height) };
    }

    return result;
}
}
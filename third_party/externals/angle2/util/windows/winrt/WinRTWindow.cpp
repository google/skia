//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WinRTWindow.cpp: Implementation of OSWindow for WinRT (Windows)

#include "windows/winrt/WinRTWindow.h"

#include <wrl.h>
#include <windows.applicationmodel.core.h>
#include <windows.ui.xaml.h>

#include "angle_windowsstore.h"
#include "common/debug.h"

using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI::Core;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

WinRTWindow::WinRTWindow() : mNativeWindow(nullptr)
{
}

WinRTWindow::~WinRTWindow()
{
    destroy();
}

bool WinRTWindow::initialize(const std::string &name, size_t width, size_t height)
{
    ComPtr<ICoreWindowStatic> coreWindowStatic;
    ComPtr<IActivationFactory> propertySetFactory;
    ComPtr<IPropertyValueStatics> propertyValueStatics;

    ComPtr<ICoreApplication> coreApplication;
    ComPtr<IPropertySet> coreApplicationProperties;
    ComPtr<IMap<HSTRING, IInspectable *>> coreApplicationPropertiesAsMap;

    ComPtr<IMap<HSTRING, IInspectable *>> nativeWindowAsMap;
    ComPtr<IInspectable> sizeValue;

    HRESULT result           = S_OK;
    boolean propertyReplaced = false;

    destroy();

    // Get all the relevant activation factories
    result = GetActivationFactory(
        HStringReference(RuntimeClass_Windows_UI_Core_CoreWindow).Get(), &coreWindowStatic);
    if (FAILED(result))
    {
        return false;
    }

    result = GetActivationFactory(
        HStringReference(RuntimeClass_Windows_Foundation_Collections_PropertySet).Get(),
        &propertySetFactory);
    if (FAILED(result))
    {
        return false;
    }

    result =
        GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get(),
                             &propertyValueStatics);
    if (FAILED(result))
    {
        return false;
    }

    result = GetActivationFactory(
        HStringReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(),
        &coreApplication);
    if (FAILED(result))
    {
        return false;
    }

    // Create a PropertySet to be used as the native window
    result = propertySetFactory->ActivateInstance(&mNativeWindow);
    if (FAILED(result))
    {
        return false;
    }

    // Get the PropertySet as a map, so we can Insert things into it later
    ComPtr<IInspectable> tempNativeWindow = mNativeWindow;
    result = tempNativeWindow.As(&nativeWindowAsMap);
    if (FAILED(result))
    {
        return false;
    }

    // Get the CoreApplication properties
    result = coreApplication->get_Properties(coreApplicationProperties.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    // Get the CoreApplication properties as a map
    result = coreApplicationProperties.As(&coreApplicationPropertiesAsMap);
    if (FAILED(result))
    {
        return false;
    }

    // See if the application properties contain an EGLNativeWindowTypeProperty
    boolean hasEGLNativeWindowTypeProperty;
    result = coreApplicationPropertiesAsMap->HasKey(
        HStringReference(EGLNativeWindowTypeProperty).Get(), &hasEGLNativeWindowTypeProperty);
    if (FAILED(result))
    {
        return false;
    }

    // If an EGLNativeWindowTypeProperty is inputted then use it
    if (hasEGLNativeWindowTypeProperty)
    {
        ComPtr<IInspectable> coreApplicationPropertyNativeWindow;

        result = coreApplicationPropertiesAsMap->Lookup(
            HStringReference(EGLNativeWindowTypeProperty).Get(),
            &coreApplicationPropertyNativeWindow);
        if (FAILED(result))
        {
            return false;
        }

        // See if the inputted window was a CoreWindow
        ComPtr<ICoreWindow> applicationPropertyCoreWindow;
        if (SUCCEEDED(coreApplicationPropertyNativeWindow.As(&applicationPropertyCoreWindow)))
        {
            // Store away the CoreWindow's dispatcher, to be used later to process messages
            result = applicationPropertyCoreWindow->get_Dispatcher(&mCoreDispatcher);
            if (FAILED(result))
            {
                return false;
            }
        }
        else
        {
            ComPtr<IPropertySet> propSet;

            // Disallow Property Sets here, since we want to wrap this window in
            // a property set with the size property below
            if (SUCCEEDED(coreApplicationPropertyNativeWindow.As(&propSet)))
            {
                return false;
            }
        }

        // Add the window to the map
        result =
            nativeWindowAsMap->Insert(HStringReference(EGLNativeWindowTypeProperty).Get(),
                                      coreApplicationPropertyNativeWindow.Get(), &propertyReplaced);
        if (FAILED(result))
        {
            return false;
        }
    }
    else
    {
        ComPtr<ICoreWindow> currentThreadCoreWindow;

        // Get the CoreWindow for the current thread
        result = coreWindowStatic->GetForCurrentThread(&currentThreadCoreWindow);
        if (FAILED(result))
        {
            return false;
        }

        // By default, just add this thread's CoreWindow to the PropertySet
        result = nativeWindowAsMap->Insert(HStringReference(EGLNativeWindowTypeProperty).Get(),
                                           currentThreadCoreWindow.Get(), &propertyReplaced);
        if (FAILED(result))
        {
            return false;
        }

        // Store away the CoreWindow's dispatcher, to be used later to process messages
        result = currentThreadCoreWindow->get_Dispatcher(&mCoreDispatcher);
        if (FAILED(result))
        {
            return false;
        }
    }

    // Create a Size to represent the Native Window's size
    Size renderSize;
    renderSize.Width  = static_cast<float>(width);
    renderSize.Height = static_cast<float>(height);
    result = propertyValueStatics->CreateSize(renderSize, sizeValue.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    // Add the Size to the PropertySet
    result = nativeWindowAsMap->Insert(HStringReference(EGLRenderSurfaceSizeProperty).Get(),
                                       sizeValue.Get(), &propertyReplaced);
    if (FAILED(result))
    {
        return false;
    }

    return true;
};

void WinRTWindow::destroy()
{
    SafeRelease(mNativeWindow);

    mCoreDispatcher.Reset();
}

EGLNativeWindowType WinRTWindow::getNativeWindow() const
{
    return mNativeWindow;
}

EGLNativeDisplayType WinRTWindow::getNativeDisplay() const
{
    UNIMPLEMENTED();
    return static_cast<EGLNativeDisplayType>(0);
}

void WinRTWindow::messageLoop()
{
    // If we have a CoreDispatcher then use it to process events
    if (mCoreDispatcher)
    {
        HRESULT result =
            mCoreDispatcher->ProcessEvents(CoreProcessEventsOption_ProcessAllIfPresent);
        UNUSED_ASSERTION_VARIABLE(result);
        ASSERT(SUCCEEDED(result));
    }
}

void WinRTWindow::setMousePosition(int /* x */, int /* y */)
{
    UNIMPLEMENTED();
}

bool WinRTWindow::setPosition(int /* x */, int /* y */)
{
    UNIMPLEMENTED();
    return false;
}

bool WinRTWindow::resize(int /* width */, int /* height */)
{
    UNIMPLEMENTED();
    return false;
}

void WinRTWindow::setVisible(bool isVisible)
{
    if (isVisible)
    {
        // Already visible by default
        return;
    }
    else
    {
        // Not implemented in WinRT
        UNIMPLEMENTED();
    }
}

void WinRTWindow::signalTestEvent()
{
    UNIMPLEMENTED();
}

OSWindow *CreateOSWindow()
{
    return new WinRTWindow();
}
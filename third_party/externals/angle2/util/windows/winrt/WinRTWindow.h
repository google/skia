//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WinRTWindow.h: Definition of the implementation of OSWindow for WinRT (Windows)

#ifndef UTIL_WINRT_WINDOW_H
#define UTIL_WINRT_WINDOW_H

#include <string>
#include <windows.h>
#include <windows.applicationmodel.core.h>
#include <wrl.h>

#include "OSWindow.h"

class WinRTWindow : public OSWindow
{
  public:
    WinRTWindow();
    ~WinRTWindow() override;

    bool initialize(const std::string &name, size_t width, size_t height) override;
    void destroy() override;

    EGLNativeWindowType getNativeWindow() const override;
    EGLNativeDisplayType getNativeDisplay() const override;

    void messageLoop() override;

    void setMousePosition(int x, int y) override;
    bool setPosition(int x, int y) override;
    bool resize(int width, int height) override;
    void setVisible(bool isVisible) override;

    void signalTestEvent() override;

  private:
    EGLNativeWindowType mNativeWindow;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Core::ICoreDispatcher> mCoreDispatcher;
};

#endif  // UTIL_WINRT_WINDOW_H

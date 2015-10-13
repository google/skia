//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Win32Window.h: Definition of the implementation of OSWindow for Win32 (Windows)

#ifndef UTIL_WIN32_WINDOW_H
#define UTIL_WIN32_WINDOW_H

#include <windows.h>
#include <string>

#include "OSWindow.h"
#include "Timer.h"

class Win32Window : public OSWindow
{
  public:
    Win32Window();
    ~Win32Window() override;

    bool initialize(const std::string &name, size_t width, size_t height) override;
    void destroy() override;

    bool takeScreenshot(uint8_t *pixelData) override;

    EGLNativeWindowType getNativeWindow() const override;
    EGLNativeDisplayType getNativeDisplay() const override;

    void messageLoop() override;

    void pushEvent(Event event) override;

    void setMousePosition(int x, int y) override;
    bool setPosition(int x, int y) override;
    bool resize(int width, int height) override;
    void setVisible(bool isVisible) override;

    void signalTestEvent() override;

  private:
    std::string mParentClassName;
    std::string mChildClassName;

    bool mIsVisible;
    Timer *mSetVisibleTimer;

    EGLNativeWindowType mNativeWindow;
    EGLNativeWindowType mParentWindow;
    EGLNativeDisplayType mNativeDisplay;
};

#endif  // UTIL_WIN32_WINDOW_H

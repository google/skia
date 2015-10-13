//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// X11Window.h: Definition of the implementation of OSWindow for X11

#ifndef UTIL_X11_WINDOW_H
#define UTIL_X11_WINDOW_H

#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include "OSWindow.h"

class X11Window : public OSWindow
{
  public:
    X11Window();
    ~X11Window();

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
    void processEvent(const XEvent &event);

    Atom WM_DELETE_WINDOW;
    Atom WM_PROTOCOLS;
    Atom TEST_EVENT;

    Display *mDisplay;
    Window mWindow;
};

#endif // UTIL_X11_WINDOW_H

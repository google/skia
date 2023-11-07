/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/ANGLEWindowContext.h"
#include "tools/window/win/WindowContextFactory_win.h"

using skwindow::DisplayParams;
using skwindow::internal::ANGLEWindowContext;

namespace {

class ANGLEWindowContext_win : public ANGLEWindowContext {
public:
    ANGLEWindowContext_win(HWND, const DisplayParams&);

protected:
    EGLDisplay onGetEGLDisplay(
            PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT) const override;
    NativeWindowType onGetNativeWindow() const override;
    SkISize onGetSize() const override;
    int onGetStencilBits() const override;

private:
    HWND fHWND;
    HDC fHDC;
};

ANGLEWindowContext_win::ANGLEWindowContext_win(HWND wnd, const DisplayParams& params)
        : ANGLEWindowContext(params), fHWND(wnd), fHDC(GetDC(fHWND)) {
    this->initializeContext();
}

EGLDisplay ANGLEWindowContext_win::onGetEGLDisplay(
        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT) const {
    // We currently only support D3D11 ANGLE.
    static constexpr EGLint kType = EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
    static constexpr EGLint attribs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, kType, EGL_NONE};
    return eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, fHDC, attribs);
}

NativeWindowType ANGLEWindowContext_win::onGetNativeWindow() const { return fHWND; }

int ANGLEWindowContext_win::onGetStencilBits() const {
    // use DescribePixelFormat to get the stencil depth.
    int pixelFormat = GetPixelFormat(fHDC);
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(fHDC, pixelFormat, sizeof(pfd), &pfd);
    return pfd.cStencilBits;
}

SkISize ANGLEWindowContext_win::onGetSize() const {
    RECT rect;
    GetClientRect(fHWND, &rect);
    return SkISize::Make(rect.right - rect.left, rect.bottom - rect.top);
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeANGLEForWin(HWND wnd, const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new ANGLEWindowContext_win(wnd, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

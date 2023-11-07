/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/ANGLEWindowContext.h"
#include "tools/window/mac/WindowContextFactory_mac.h"

using skwindow::DisplayParams;
using skwindow::MacWindowInfo;
using skwindow::internal::ANGLEWindowContext;

namespace {

class ANGLEWindowContext_mac : public ANGLEWindowContext {
public:
    ANGLEWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

protected:
    EGLDisplay onGetEGLDisplay(
            PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT) const override;
    NativeWindowType onGetNativeWindow() const override;
    SkISize onGetSize() const override;
    int onGetStencilBits() const override;

private:
    NSView* fMainView;
};

ANGLEWindowContext_mac::ANGLEWindowContext_mac(const MacWindowInfo& info,
                                               const DisplayParams& params)
        : ANGLEWindowContext(params), fMainView(info.fMainView) {
    this->initializeContext();
}

EGLDisplay ANGLEWindowContext_mac::onGetEGLDisplay(
        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT) const {
    static constexpr EGLint kType = EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE;
    static constexpr EGLint attribs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, kType, EGL_NONE};
    return eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void*>(EGL_DEFAULT_DISPLAY), attribs);
}

NativeWindowType ANGLEWindowContext_mac::onGetNativeWindow() const {
    [fMainView setWantsLayer:YES];
    return fMainView.layer;
}

int ANGLEWindowContext_mac::onGetStencilBits() const {
    GLint stencilBits;
    NSOpenGLPixelFormat* pixelFormat = skwindow::GetGLPixelFormat(fDisplayParams.fMSAASampleCount);
    [pixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
    return stencilBits;
}

SkISize ANGLEWindowContext_mac::onGetSize() const {
    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(fMainView);
    return SkISize::Make(fMainView.bounds.size.width * backingScaleFactor,
                         fMainView.bounds.size.height * backingScaleFactor);
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeANGLEForMac(const MacWindowInfo& info,
                                               const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new ANGLEWindowContext_mac(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

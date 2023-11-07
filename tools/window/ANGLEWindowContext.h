/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef ANGLEWindowContext_DEFINED
#define ANGLEWindowContext_DEFINED

#define EGL_EGL_PROTOTYPES 1

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "tools/window/GLWindowContext.h"

namespace skwindow::internal {

class ANGLEWindowContext : public GLWindowContext {
public:
    using GLWindowContext::GLWindowContext;
    ~ANGLEWindowContext() override;

protected:
    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

    virtual EGLDisplay onGetEGLDisplay(
            PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT) const = 0;
    virtual NativeWindowType onGetNativeWindow() const = 0;
    virtual SkISize onGetSize() const = 0;
    virtual int onGetStencilBits() const = 0;

private:
    EGLDisplay fDisplay = EGL_NO_DISPLAY;
    EGLContext fEGLContext = EGL_NO_CONTEXT;
    EGLSurface fEGLSurface = EGL_NO_SURFACE;
};

}  // namespace skwindow::internal

#endif

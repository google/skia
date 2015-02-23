
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/angle/SkANGLEGLContext.h"

#include <EGL/egl.h>

#define EGL_PLATFORM_ANGLE_ANGLE                0x3201
#define EGL_PLATFORM_ANGLE_TYPE_ANGLE           0x3202
#define EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE      0x3206
#define EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE     0x3207

void* SkANGLEGLContext::GetD3DEGLDisplay(void* nativeDisplay) {

    typedef EGLDisplay (*EGLGetPlatformDisplayEXT)(EGLenum platform,
                                                   void *native_display,
                                                   const EGLint *attrib_list);
    EGLGetPlatformDisplayEXT eglGetPlatformDisplayEXT;
    eglGetPlatformDisplayEXT =
        (EGLGetPlatformDisplayEXT) eglGetProcAddress("eglGetPlatformDisplayEXT");

    if (!eglGetPlatformDisplayEXT) {
        return eglGetDisplay(static_cast<EGLNativeDisplayType>(nativeDisplay));
    }

    // Try for an ANGLE D3D11 context, fall back to D3D9.
    EGLint attribs[2][3] = {
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE,
            EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_NONE
        },
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE,
            EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
            EGL_NONE
        }
    };

    EGLDisplay display = EGL_NO_DISPLAY;
    for (int i = 0; i < 2 && display == EGL_NO_DISPLAY; ++i) {
        display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                            nativeDisplay, attribs[i]);
    }
    return display;
}

SkANGLEGLContext::SkANGLEGLContext()
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE) {

    EGLint numConfigs;
    static const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    fDisplay = GetD3DEGLDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == fDisplay) {
        SkDebugf("Could not create EGL display!");
        return;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    eglInitialize(fDisplay, &majorVersion, &minorVersion);

    EGLConfig surfaceConfig;
    eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs);

    static const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    fContext = eglCreateContext(fDisplay, surfaceConfig, NULL, contextAttribs);


    static const EGLint surfaceAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    fSurface = eglCreatePbufferSurface(fDisplay, surfaceConfig, surfaceAttribs);

    eglMakeCurrent(fDisplay, fSurface, fSurface, fContext);

    fGL.reset(GrGLCreateANGLEInterface());
    if (NULL == fGL.get()) {
        SkDebugf("Could not create ANGLE GL interface!\n");
        this->destroyGLContext();
        return;
    }
    if (!fGL->validate()) {
        SkDebugf("Could not validate ANGLE GL interface!\n");
        this->destroyGLContext();
        return;
    }
}

SkANGLEGLContext::~SkANGLEGLContext() {
    this->destroyGLContext();
}

void SkANGLEGLContext::destroyGLContext() {
    fGL.reset(NULL);
    if (fDisplay) {
        eglMakeCurrent(fDisplay, 0, 0, 0);

        if (fContext) {
            eglDestroyContext(fDisplay, fContext);
            fContext = EGL_NO_CONTEXT;
        }

        if (fSurface) {
            eglDestroySurface(fDisplay, fSurface);
            fSurface = EGL_NO_SURFACE;
        }

        //TODO should we close the display?
        fDisplay = EGL_NO_DISPLAY;
    }
}

void SkANGLEGLContext::makeCurrent() const {
    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

void SkANGLEGLContext::swapBuffers() const {
    if (!eglSwapBuffers(fDisplay, fSurface)) {
        SkDebugf("Could not complete eglSwapBuffers.\n");
    }
}

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fiddle_main.h"

#include <EGL/egl.h>

static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE
};

static const int pbufferWidth = 9;
static const int pbufferHeight = 9;

static const EGLint pbufferAttribs[] = {
    EGL_WIDTH, pbufferWidth,
    EGL_HEIGHT, pbufferHeight,
    EGL_NONE,
};

// create_grcontext implementation for EGL.
sk_sp<GrContext> create_grcontext() {
    EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == eglDpy) {
        return nullptr;
    }

    EGLint major, minor;
    if (EGL_TRUE != eglInitialize(eglDpy, &major, &minor)) {
        return nullptr;
    }

    EGLint numConfigs;
    EGLConfig eglCfg;
    if (EGL_TRUE != eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs)) {
        return nullptr;
    }

    EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, pbufferAttribs);
    if (EGL_NO_SURFACE == eglSurf) {
        return nullptr;
    }

    if (EGL_TRUE != eglBindAPI(EGL_OPENGL_API)) {
        return nullptr;
    }

    EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, NULL);
    if (EGL_NO_CONTEXT == eglCtx) {
        return nullptr;
    }
    if (EGL_FALSE == eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx)) {
        return nullptr;
    }

    auto interface = GrGLCreateNativeInterface();
    if (!interface) {
        return nullptr;
    }
    eglTerminate(eglDpy);

    return sk_sp<GrContext>(GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)interface));
}

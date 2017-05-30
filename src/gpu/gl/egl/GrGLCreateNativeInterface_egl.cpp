/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

static GrGLFuncPtr egl_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    GrGLFuncPtr ptr = eglGetProcAddress(name);
    if (!ptr) {
        if (0 == strcmp("eglQueryString", name)) {
            return (GrGLFuncPtr)eglQueryString;
        } else if (0 == strcmp("eglGetCurrentDisplay", name)) {
            return (GrGLFuncPtr)eglGetCurrentDisplay;
        }
    }
    return ptr;
}

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

const GrGLInterface* GrGLCreateNativeInterface() {
    // It turns out eglGetProcAddress doesn't work unless there's an active
    // OpenGL context, so create a 'dummy' context for use just long enough
    // to make eglGetProcAdresss work when we pass it to
    // GrGlAssembleInterface().

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
    const GrGLInterface* ret = GrGLAssembleInterface(nullptr, egl_get_gl_proc);

    eglTerminate(eglDpy);

    return ret;
}

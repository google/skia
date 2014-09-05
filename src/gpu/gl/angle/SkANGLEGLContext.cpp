
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/SkANGLEGLContext.h"

SkANGLEGLContext::AutoContextRestore::AutoContextRestore() {
    fOldEGLContext = eglGetCurrentContext();
    fOldDisplay = eglGetCurrentDisplay();
    fOldSurface = eglGetCurrentSurface(EGL_DRAW);

}

SkANGLEGLContext::AutoContextRestore::~AutoContextRestore() {
    if (fOldDisplay) {
        eglMakeCurrent(fOldDisplay, fOldSurface, fOldSurface, fOldEGLContext);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkANGLEGLContext::SkANGLEGLContext()
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE) {
}

SkANGLEGLContext::~SkANGLEGLContext() {
    this->destroyGLContext();
}

void SkANGLEGLContext::destroyGLContext() {
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

const GrGLInterface* SkANGLEGLContext::createGLContext(GrGLStandard forcedGpuAPI) {
    if (kGL_GrGLStandard == forcedGpuAPI) {
        return NULL;
    }

    fDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    EGLint majorVersion;
    EGLint minorVersion;
    eglInitialize(fDisplay, &majorVersion, &minorVersion);

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

    const GrGLInterface* interface = GrGLCreateANGLEInterface();
    if (NULL == interface) {
        SkDebugf("Could not create ANGLE GL interface!\n");
        this->destroyGLContext();
        return NULL;
    }

    return interface;
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

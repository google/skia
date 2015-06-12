/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSWindow_Android.h"

#include <GLES/gl.h>

SkOSWindow::SkOSWindow(void* hwnd) {
    fWindow.fDisplay = EGL_NO_DISPLAY;
    fWindow.fContext = EGL_NO_CONTEXT;
    fWindow.fSurface = EGL_NO_SURFACE;
    fNativeWindow = (ANativeWindow*)hwnd;
    fDestroyRequested = false;
}

SkOSWindow::~SkOSWindow() {
    if (fWindow.fDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(fWindow.fDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (fWindow.fContext != EGL_NO_CONTEXT) {
            eglDestroyContext(fWindow.fDisplay, fWindow.fContext);
        }
        if (fWindow.fSurface != EGL_NO_SURFACE) {
            eglDestroySurface(fWindow.fDisplay, fWindow.fSurface);
        }
        eglTerminate(fWindow.fDisplay);
    }
    fWindow.fDisplay = EGL_NO_DISPLAY;
    fWindow.fContext = EGL_NO_CONTEXT;
    fWindow.fSurface = EGL_NO_SURFACE;
}

bool SkOSWindow::attach(SkBackEndTypes attachType,
                        int /*msaaSampleCount*/,
                        AttachmentInfo* info) {
    static const EGLint kEGLContextAttribsForOpenGL[] = {
        EGL_NONE
    };

    static const EGLint kEGLContextAttribsForOpenGLES[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    static const struct {
        const EGLint* fContextAttribs;
        EGLenum fAPI;
        EGLint  fRenderableTypeBit;
    } kAPIs[] = {
        {   // OpenGL
            kEGLContextAttribsForOpenGL,
            EGL_OPENGL_API,
            EGL_OPENGL_BIT,
        },
        {   // OpenGL ES. This seems to work for both ES2 and 3 (when available).
            kEGLContextAttribsForOpenGLES,
            EGL_OPENGL_ES_API,
            EGL_OPENGL_ES2_BIT,
        },
    };

    size_t apiLimit = SK_ARRAY_COUNT(kAPIs);

    for (size_t api = 0; api < apiLimit; ++api) {
        EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        EGLint majorVersion;
        EGLint minorVersion;
        eglInitialize(display, &majorVersion, &minorVersion);

#if 0
        SkDebugf("VENDOR: %s\n", eglQueryString(fDisplay, EGL_VENDOR));
        SkDebugf("APIS: %s\n", eglQueryString(fDisplay, EGL_CLIENT_APIS));
        SkDebugf("VERSION: %s\n", eglQueryString(fDisplay, EGL_VERSION));
        SkDebugf("EXTENSIONS %s\n", eglQueryString(fDisplay, EGL_EXTENSIONS));
#endif

        if (!eglBindAPI(kAPIs[api].fAPI)) {
            continue;
        }

        const EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, kAPIs[api].fRenderableTypeBit,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };

        EGLint format;
        EGLint numConfigs;
        EGLConfig config;
        EGLSurface surface;
        EGLContext context;

        /* Here, the application chooses the configuration it desires. In this
         * sample, we have a very simplified selection process, where we pick
         * the first EGLConfig that matches our criteria */
        eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);

        /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
         * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
         * As soon as we picked a EGLConfig, we can safely reconfigure the
         * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
        eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

        ANativeWindow_setBuffersGeometry(fNativeWindow, 0, 0, format);

        surface = eglCreateWindowSurface(display, config, fNativeWindow, NULL);
        context = eglCreateContext(display, config, NULL, kAPIs[api].fContextAttribs);
        if (EGL_NO_CONTEXT == context) {
            SkDebugf("eglCreateContext failed.  EGL Error: 0x%08x\n", eglGetError());
            continue;
        }

        if (!eglMakeCurrent(display, surface, surface, context)) {
            SkDebugf("eglMakeCurrent failed.  EGL Error: 0x%08x\n", eglGetError());
            continue;
        }

        fWindow.fDisplay = display;
        fWindow.fContext = context;
        fWindow.fSurface = surface;
    }

    if (fWindow.fDisplay && fWindow.fContext && fWindow.fSurface) {
        EGLint w, h;
        eglQuerySurface(fWindow.fDisplay, fWindow.fSurface, EGL_WIDTH, &w);
        eglQuerySurface(fWindow.fDisplay, fWindow.fSurface, EGL_HEIGHT, &h);

        glViewport(0, 0, w, h);
        glClearColor(0.0, 0, 0, 0.0);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // We retrieve the fullscreen width and height
        this->setSize((SkScalar)w, (SkScalar)h);
        return true;
    } else {
        return false;
    }
}

void SkOSWindow::detach() {
    fDestroyRequested = true;
}

void SkOSWindow::present() {
    if (fWindow.fDisplay != EGL_NO_DISPLAY && fWindow.fContext != EGL_NO_CONTEXT) {
        eglSwapBuffers(fWindow.fDisplay, fWindow.fSurface);
    }
}

void SkOSWindow::closeWindow() {
    fDestroyRequested = true;
}

void SkOSWindow::setVsync(bool vsync) {
    if (fWindow.fDisplay != EGL_NO_DISPLAY) {
        int swapInterval = vsync ? 1 : 0;
        eglSwapInterval(fWindow.fDisplay, swapInterval);
    }
}

void SkOSWindow::onSetTitle(const char title[]) {
}

void SkOSWindow::onHandleInval(const SkIRect& rect) {
}

void SkOSWindow::onPDFSaved(const char title[], const char desc[], const char path[]) {
}

///////////////////////////////////////////
/////////////// SkEvent impl //////////////
///////////////////////////////////////////

void SkEvent::SignalQueueTimer(SkMSec ms) {
}

void SkEvent::SignalNonEmptyQueue() {
}

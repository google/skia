
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNativeGLContext_DEFINED
#define SkNativeGLContext_DEFINED

#include "SkGLContextHelper.h"

#if defined(SK_BUILD_FOR_MAC)
    #include <OpenGL/OpenGL.h>
#elif defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_NACL)
    #include <GLES2/gl2.h>
    #include <EGL/egl.h>
#elif defined(SK_BUILD_FOR_UNIX)
    #include <X11/Xlib.h>
    #include <GL/glx.h>
#elif defined(SK_BUILD_FOR_WIN32)
    #include <windows.h>
    #include <GL/GL.h>
    #include "SkWGL.h"
#endif

class SkNativeGLContext : public SkGLContextHelper {
public:
    SkNativeGLContext();

    virtual ~SkNativeGLContext();

    virtual void makeCurrent() const SK_OVERRIDE;
    virtual void swapBuffers() const SK_OVERRIDE;

    class AutoContextRestore {
    public:
        AutoContextRestore();
        ~AutoContextRestore();

    private:
    #if defined(SK_BUILD_FOR_MAC)
        CGLContextObj fOldCGLContext;
    #elif defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_NACL)
        EGLContext fOldEGLContext;
        EGLDisplay fOldDisplay;
        EGLSurface fOldSurface;
    #elif defined(SK_BUILD_FOR_UNIX)
        GLXContext fOldGLXContext;
        Display* fOldDisplay;
        GLXDrawable fOldDrawable;
    #elif defined(SK_BUILD_FOR_WIN32)
        HDC fOldHDC;
        HGLRC fOldHGLRC;

    #elif defined(SK_BUILD_FOR_IOS)
        void* fEAGLContext;
    #endif
    };

protected:
    virtual const GrGLInterface* createGLContext(GrGLStandard forcedGpuAPI) SK_OVERRIDE;
    virtual void destroyGLContext() SK_OVERRIDE;

private:
#if defined(SK_BUILD_FOR_MAC)
    CGLContextObj fContext;
#elif defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_NACL)
    EGLContext fContext;
    EGLDisplay fDisplay;
    EGLSurface fSurface;
#elif defined(SK_BUILD_FOR_UNIX)
    GLXContext fContext;
    Display* fDisplay;
    Pixmap fPixmap;
    GLXPixmap fGlxPixmap;
#elif defined(SK_BUILD_FOR_WIN32)
    HWND fWindow;
    HDC fDeviceContext;
    HGLRC fGlRenderContext;
    static ATOM gWC;
    SkWGLPbufferContext* fPbufferContext;
#elif defined(SK_BUILD_FOR_IOS)
    void* fEAGLContext;
#endif
};

#endif

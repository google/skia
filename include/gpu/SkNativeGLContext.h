
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNativeGLContext_DEFINED
#define SkNativeGLContext_DEFINED

#include "SkGLContext.h"

#if defined(SK_BUILD_FOR_MAC)
    #include <AGL/agl.h>
#elif defined(SK_BUILD_FOR_UNIX)
    #include <X11/Xlib.h>
    #include <GL/glx.h>
#elif defined(SK_BUILD_FOR_WIN32)
    #include <Windows.h>
    #include <GL/GL.h>
#endif

class SkNativeGLContext : public SkGLContext {
public:
    SkNativeGLContext();

    virtual ~SkNativeGLContext();

    virtual void makeCurrent() const SK_OVERRIDE;

protected:
    virtual const GrGLInterface* createGLContext() SK_OVERRIDE;
    void destroyGLContext() SK_OVERRIDE;

private:
#if defined(SK_BUILD_FOR_MAC)
    AGLContext fContext;
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
#endif
};

#endif

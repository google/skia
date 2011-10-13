
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkGLContext_DEFINED
#define SkGLContext_DEFINED

#if defined(SK_MESA)
    #include "GL/osmesa.h"
#elif defined(SK_BUILD_FOR_MAC)
    #include <AGL/agl.h>
#elif defined(SK_BUILD_FOR_UNIX)
    #include <X11/Xlib.h>
    #include <GL/glx.h>
#elif defined(SK_BUILD_FOR_WIN32)
    #include <Windows.h>
    #include <GL/GL.h>
#else

#endif

/**
 *  Create an offscreen opengl context with an RGBA8 / 8bit stencil FBO.
 */
class SkGLContext {
public:
    SkGLContext();
    ~SkGLContext();

    bool init(const int width, const int height);

    int getFBOID() const { return fFBO; }

private:
    GLuint fFBO;
#if defined(SK_MESA)
    OSMesaContext context;
    GLfloat *image;
#elif defined(SK_BUILD_FOR_MAC)
    AGLContext context;
#elif defined(SK_BUILD_FOR_UNIX)
    GLXContext context;
    Display *display;
    Pixmap pixmap;
    GLXPixmap glxPixmap;
#elif defined(SK_BUILD_FOR_WIN32)
    HWND fWindow;
    HDC fDeviceContext;
    HGLRC fGlRenderContext;
#else

#endif
};

#endif

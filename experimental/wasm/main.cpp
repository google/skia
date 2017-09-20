/*
 * Copyright 2017 The Chromium Authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPaint.h"
#include "SkCanvas.h"
#include "SkSurface.h"
#include "gl/GrGLInterface.h"
#include "GrContext.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE
};

static EGLint attribList[] =
   {
       EGL_RED_SIZE,       5,
       EGL_GREEN_SIZE,     6,
       EGL_BLUE_SIZE,      5,
       EGL_ALPHA_SIZE,     8,
       EGL_DEPTH_SIZE,     8,
       EGL_STENCIL_SIZE,   EGL_DONT_CARE,
       EGL_SAMPLE_BUFFERS, 0,
       EGL_NONE
   };

// create_grcontext implementation for EGL.
sk_sp<GrContext> create_grcontext(std::ostringstream &driverinfo) {
    EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == eglDpy) {
        SkDebugf("alpha");
        return nullptr;
    }

    EGLint major, minor;
    if (EGL_TRUE != eglInitialize(eglDpy, &major, &minor)) {
        SkDebugf("beta");
        return nullptr;
    }

    EGLint numConfigs;
    EGLConfig eglCfg;
    if (EGL_TRUE != eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs)) {
        SkDebugf("gamma");
        return nullptr;
    }

    EGLSurface eglSurf = eglCreateWindowSurface(eglDpy, eglCfg, 0, attribList);
    if (EGL_NO_SURFACE == eglSurf) {
        SkDebugf("delta");
        return nullptr;
    }

    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

    EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, contextAttribs);
    if (EGL_NO_CONTEXT == eglCtx) {
        SkDebugf("theta");
        return nullptr;
    }
    if (EGL_FALSE == eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx)) {
        SkDebugf("iota");
        return nullptr;
    }

    driverinfo << "EGL " << major << "." << minor << "\n";
    GrGLGetStringProc getString = (GrGLGetStringProc )eglGetProcAddress("glGetString");
    driverinfo << "GL Versionr: " << getString(GL_VERSION) << "\n";
    driverinfo << "GL Vendor: " << getString(GL_VENDOR) << "\n";
    driverinfo << "GL Renderer: " << getString(GL_RENDERER) << "\n";
    driverinfo << "GL Extensions: " << getString(GL_EXTENSIONS) << "\n";

    auto interface = GrGLCreateNativeInterface();
    if (!interface) {
        SkDebugf("kappa");
        return nullptr;
    }
    eglTerminate(eglDpy);

    return sk_sp<GrContext>(GrContext::MakeGL(interface));
}


int main(int argc, char** argv) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);

    std::ostringstream gGLDriverInfo;
    auto grContext = create_grcontext(gGLDriverInfo);
    if (!grContext) {
        SkDebugf("Unable to get GrContext.\n", stderr);
    } else {
        SkImageInfo info = SkImageInfo::MakeN32(200, 200, kOpaque_SkAlphaType);
        auto surface = SkSurface::MakeRenderTarget(grContext.get(), SkBudgeted::kYes, info);
        if (!surface) {
            SkDebugf("Unable to get render surface.\n", stderr);
        } else {
            surface->getCanvas()->drawLine(20, 20, 100, 100, p);
            SkDebugf("I drew a line");
        }
    }
    SkDebugf("GLINFO %s", gGLDriverInfo.str().c_str());
}

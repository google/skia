
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkNativeGLContext.h"

SkNativeGLContext::AutoContextRestore::AutoContextRestore() {
    fOldAGLContext = aglGetCurrentContext();
}

SkNativeGLContext::AutoContextRestore::~AutoContextRestore() {
    aglSetCurrentContext(fOldAGLContext);
}

///////////////////////////////////////////////////////////////////////////////

SkNativeGLContext::SkNativeGLContext()
    : fContext(NULL) {
}

SkNativeGLContext::~SkNativeGLContext() {
    this->destroyGLContext();
}

void SkNativeGLContext::destroyGLContext() {
    if (fContext) {
        aglDestroyContext(fContext);
    }
}

const GrGLInterface* SkNativeGLContext::createGLContext() {
    GLint major, minor;
    // AGLContext ctx;

    aglGetVersion(&major, &minor);
    //SkDebugf("---- agl version %d %d\n", major, minor);

    const GLint pixelAttrs[] = {
        AGL_RGBA,
        AGL_ACCELERATED,
        AGL_NONE
    };
    AGLPixelFormat format = aglChoosePixelFormat(NULL, 0, pixelAttrs);
    if (NULL == format) {
        SkDebugf("Format could not be found.\n");
        this->destroyGLContext();
        return NULL;
    }
    fContext = aglCreateContext(format, NULL);
    if (NULL == fContext) {
        SkDebugf("Context could not be created.\n");
        this->destroyGLContext();
        return NULL;
    }
    aglDestroyPixelFormat(format);

    aglSetCurrentContext(fContext);

    const GrGLInterface* interface = GrGLCreateNativeInterface();
    if (NULL == interface) {
        SkDebugf("Context could not create GL interface.\n");
        this->destroyGLContext();
        return NULL;
    }

    return interface;
}

void SkNativeGLContext::makeCurrent() const {
    aglSetCurrentContext(fContext);
}

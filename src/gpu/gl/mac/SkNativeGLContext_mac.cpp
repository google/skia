
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkNativeGLContext.h"

SkNativeGLContext::AutoContextRestore::AutoContextRestore() {
    fOldCGLContext = CGLGetCurrentContext();
}

SkNativeGLContext::AutoContextRestore::~AutoContextRestore() {
    CGLSetCurrentContext(fOldCGLContext);
}

///////////////////////////////////////////////////////////////////////////////

SkNativeGLContext::SkNativeGLContext()
    : fContext(NULL) {
}

SkNativeGLContext::~SkNativeGLContext() {
    this->destroyGLContext();
}

void SkNativeGLContext::destroyGLContext() {
    if (NULL != fContext) {
        CGLReleaseContext(fContext);
    }
}

const GrGLInterface* SkNativeGLContext::createGLContext() {
    SkASSERT(NULL == fContext);

    CGLPixelFormatAttribute attributes[] = {
#if 0
        kCGLPFAOpenGLProfile, kCGLOGLPVersion_3_2_Core,
#endif
        (CGLPixelFormatAttribute)0
    };
    CGLPixelFormatObj pixFormat;
    GLint npix;

    CGLChoosePixelFormat(attributes, &pixFormat, &npix);

    if (NULL == pixFormat) {
        SkDebugf("CGLChoosePixelFormat failed.");
        return NULL;
    }

    CGLCreateContext(pixFormat, NULL, &fContext);
    CGLReleasePixelFormat(pixFormat);

    if (NULL == fContext) {
        SkDebugf("CGLCreateContext failed.");
        return NULL;
    }

    CGLSetCurrentContext(fContext);

    const GrGLInterface* interface = GrGLCreateNativeInterface();
    if (NULL == interface) {
        SkDebugf("Context could not create GL interface.\n");
        this->destroyGLContext();
        return NULL;
    }

    return interface;
}

void SkNativeGLContext::makeCurrent() const {
    CGLSetCurrentContext(fContext);
}

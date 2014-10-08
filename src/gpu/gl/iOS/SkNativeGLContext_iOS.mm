
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/SkNativeGLContext.h"
#import <OpenGLES/EAGL.h>

#define EAGLCTX ((EAGLContext*)(fEAGLContext))

SkNativeGLContext::AutoContextRestore::AutoContextRestore() {
    fEAGLContext = [EAGLContext currentContext];
    if (EAGLCTX) {
        [EAGLCTX retain];
    }
}

SkNativeGLContext::AutoContextRestore::~AutoContextRestore() {
    if (EAGLCTX) {
        [EAGLContext setCurrentContext:EAGLCTX];
        [EAGLCTX release];
    }
}

///////////////////////////////////////////////////////////////////////////////

SkNativeGLContext::SkNativeGLContext()
    : fEAGLContext(NULL) {
}

SkNativeGLContext::~SkNativeGLContext() {
    this->destroyGLContext();
}

void SkNativeGLContext::destroyGLContext() {
    if (fEAGLContext) {
        if ([EAGLContext currentContext] == EAGLCTX) {
            [EAGLContext setCurrentContext:nil];
        }
        [EAGLCTX release];
        fEAGLContext = NULL;
    }
}

const GrGLInterface* SkNativeGLContext::createGLContext(GrGLStandard forcedGpuAPI) {
    if (kGL_GrGLStandard == forcedGpuAPI) {
        return NULL;
    }

    fEAGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:EAGLCTX];
    
    const GrGLInterface* interface = GrGLCreateNativeInterface();
    if (!interface) {
        SkDebugf("Failed to create gl interface");
        this->destroyGLContext();
        return NULL;
    }
    return interface;
}

void SkNativeGLContext::makeCurrent() const {
    if (![EAGLContext setCurrentContext:EAGLCTX]) {
        SkDebugf("Could not set the context.\n");
    }
}

void SkNativeGLContext::swapBuffers() const { }

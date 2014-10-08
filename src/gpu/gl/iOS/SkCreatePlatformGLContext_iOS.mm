
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/SkGLContext.h"
#import <OpenGLES/EAGL.h>

#define EAGLCTX ((EAGLContext*)(fEAGLContext))

namespace {

class IOSNativeGLContext : public SkNativeGLContext {
public:
    IOSNativeGLContext();

    virtual ~IOSNativeGLContext();

    virtual void makeCurrent() const SK_OVERRIDE;
    virtual void swapBuffers() const SK_OVERRIDE;
protected:
    virtual const GrGLInterface* createGLContext(GrGLStandard forcedGpuAPI) SK_OVERRIDE;
    virtual void destroyGLContext() SK_OVERRIDE;

private:
    void* fEAGLContext;
};

IOSNativeGLContext::IOSNativeGLContext()
    : fEAGLContext(NULL) {
}

IOSNativeGLContext::~IOSNativeGLContext() {
    this->destroyGLContext();
}

void IOSNativeGLContext::destroyGLContext() {
    if (fEAGLContext) {
        if ([EAGLContext currentContext] == EAGLCTX) {
            [EAGLContext setCurrentContext:nil];
        }
        [EAGLCTX release];
        fEAGLContext = NULL;
    }
}

const GrGLInterface* IOSNativeGLContext::createGLContext(GrGLStandard forcedGpuAPI) {
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

void IOSNativeGLContext::makeCurrent() const {
    if (![EAGLContext setCurrentContext:EAGLCTX]) {
        SkDebugf("Could not set the context.\n");
    }
}

void IOSNativeGLContext::swapBuffers() const { }

} // anonymous namespace


SkNativeGLContext* SkCreatePlatformGLContext() {
    return SkNEW(IOSNativeGLContext);
}


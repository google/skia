
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

class IOSGLContext : public SkGLContext {
public:
    IOSGLContext();

    virtual ~IOSGLContext();

    virtual void makeCurrent() const SK_OVERRIDE;
    virtual void swapBuffers() const SK_OVERRIDE;
protected:
    virtual const GrGLInterface* createGLContext(GrGLStandard forcedGpuAPI) SK_OVERRIDE;
    virtual void destroyGLContext() SK_OVERRIDE;

private:
    void* fEAGLContext;
};

IOSGLContext::IOSGLContext()
    : fEAGLContext(NULL) {
}

IOSGLContext::~IOSGLContext() {
    this->destroyGLContext();
}

void IOSGLContext::destroyGLContext() {
    if (fEAGLContext) {
        if ([EAGLContext currentContext] == EAGLCTX) {
            [EAGLContext setCurrentContext:nil];
        }
        [EAGLCTX release];
        fEAGLContext = NULL;
    }
}

const GrGLInterface* IOSGLContext::createGLContext(GrGLStandard forcedGpuAPI) {
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

void IOSGLContext::makeCurrent() const {
    if (![EAGLContext setCurrentContext:EAGLCTX]) {
        SkDebugf("Could not set the context.\n");
    }
}

void IOSGLContext::swapBuffers() const { }

} // anonymous namespace


SkGLContext* SkCreatePlatformGLContext() {
    return SkNEW(IOSGLContext);
}


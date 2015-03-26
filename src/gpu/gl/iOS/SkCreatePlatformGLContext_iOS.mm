
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
    ~IOSGLContext() override;
    void makeCurrent() const override;
    void swapBuffers() const override;

private:
    void destroyGLContext();

    void* fEAGLContext;
};

IOSGLContext::IOSGLContext()
    : fEAGLContext(NULL) {

    fEAGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:EAGLCTX];

    fGL.reset(GrGLCreateNativeInterface());
    if (NULL == fGL.get()) {
        SkDebugf("Failed to create gl interface");
        this->destroyGLContext();
        return;
    }
    if (!fGL->validate()) {
        SkDebugf("Failed to validate gl interface");
        this->destroyGLContext();
        return;
    }
}

IOSGLContext::~IOSGLContext() {
    this->destroyGLContext();
}

void IOSGLContext::destroyGLContext() {
    fGL.reset(NULL);
    if (fEAGLContext) {
        if ([EAGLContext currentContext] == EAGLCTX) {
            [EAGLContext setCurrentContext:nil];
        }
        [EAGLCTX release];
        fEAGLContext = NULL;
    }
}


void IOSGLContext::makeCurrent() const {
    if (![EAGLContext setCurrentContext:EAGLCTX]) {
        SkDebugf("Could not set the context.\n");
    }
}

void IOSGLContext::swapBuffers() const { }

} // anonymous namespace

SkGLContext* SkCreatePlatformGLContext(GrGLStandard forcedGpuAPI) {
    if (kGL_GrGLStandard == forcedGpuAPI) {
        return NULL;
    }
    IOSGLContext* ctx = SkNEW(IOSGLContext);
    if (!ctx->isValid()) {
        SkDELETE(ctx);
        return NULL;
    }
    return ctx;
}



/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/SkGLContext.h"
#import <OpenGLES/EAGL.h>
#include <dlfcn.h>

#define EAGLCTX ((EAGLContext*)(fEAGLContext))

namespace {

class IOSGLContext : public SkGLContext {
public:
    IOSGLContext();
    ~IOSGLContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    void* fEAGLContext;
    void* fGLLibrary;
};

IOSGLContext::IOSGLContext()
    : fEAGLContext(NULL)
    , fGLLibrary(RTLD_DEFAULT) {

    fEAGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:EAGLCTX];

    SkAutoTUnref<const GrGLInterface> gl(GrGLCreateNativeInterface());
    if (NULL == gl.get()) {
        SkDebugf("Failed to create gl interface");
        this->destroyGLContext();
        return;
    }
    if (!gl->validate()) {
        SkDebugf("Failed to validate gl interface");
        this->destroyGLContext();
        return;
    }

    fGLLibrary = dlopen(
        "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib",
        RTLD_LAZY);

    this->init(gl.detach());
}

IOSGLContext::~IOSGLContext() {
    this->teardown();
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
    if (RTLD_DEFAULT != fGLLibrary) {
        dlclose(fGLLibrary);
    }
}


void IOSGLContext::onPlatformMakeCurrent() const {
    if (![EAGLContext setCurrentContext:EAGLCTX]) {
        SkDebugf("Could not set the context.\n");
    }
}

void IOSGLContext::onPlatformSwapBuffers() const { }

GrGLFuncPtr IOSGLContext::onPlatformGetProcAddress(const char* procName) const {
    return reinterpret_cast<GrGLFuncPtr>(dlsym(fGLLibrary, procName));
}

} // anonymous namespace

SkGLContext* SkCreatePlatformGLContext(GrGLStandard forcedGpuAPI) {
    if (kGL_GrGLStandard == forcedGpuAPI) {
        return NULL;
    }
    IOSGLContext* ctx = new IOSGLContext;
    if (!ctx->isValid()) {
        delete ctx;
        return NULL;
    }
    return ctx;
}

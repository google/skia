
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLTestContext.h"
#import <OpenGLES/EAGL.h>
#include <dlfcn.h>

#define EAGLCTX ((EAGLContext*)(fEAGLContext))

namespace {

class IOSGLTestContext : public sk_gpu_test::GLTestContext {
public:
    IOSGLTestContext();
    ~IOSGLTestContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    void* fEAGLContext;
    void* fGLLibrary;
};

IOSGLTestContext::IOSGLTestContext()
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

    this->init(gl.release());
}

IOSGLTestContext::~IOSGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void IOSGLTestContext::destroyGLContext() {
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


void IOSGLTestContext::onPlatformMakeCurrent() const {
    if (![EAGLContext setCurrentContext:EAGLCTX]) {
        SkDebugf("Could not set the context.\n");
    }
}

void IOSGLTestContext::onPlatformSwapBuffers() const { }

GrGLFuncPtr IOSGLTestContext::onPlatformGetProcAddress(const char* procName) const {
    return reinterpret_cast<GrGLFuncPtr>(dlsym(fGLLibrary, procName));
}

}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext *CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext *shareContext) {
    SkASSERT(!shareContext);
    if (shareContext) {
        return NULL;
    }
    if (kGL_GrGLStandard == forcedGpuAPI) {
        return NULL;
    }
    IOSGLTestContext *ctx = new IOSGLTestContext;
    if (!ctx->isValid()) {
        delete ctx;
        return NULL;
    }
    return ctx;
}
}

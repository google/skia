
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/gl/GLTestContext.h"
#import <OpenGLES/EAGL.h>
#include <dlfcn.h>

#include "include/ports/SkCFObject.h"

#define EAGLCTX ((EAGLContext*)(fEAGLContext))

namespace {

std::function<void()> context_restorer() {
    EAGLContext* context = [EAGLContext currentContext];
    return [context] { [EAGLContext setCurrentContext:context]; };
}

class IOSGLTestContext : public sk_gpu_test::GLTestContext {
public:
    IOSGLTestContext(IOSGLTestContext* shareContext);
    ~IOSGLTestContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeNotCurrent() const override;
    void onPlatformMakeCurrent() const override;
    std::function<void()> onPlatformGetAutoContextRestore() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    sk_cfp<EAGLContext*> fEAGLContext;
    void* fGLLibrary;
};

IOSGLTestContext::IOSGLTestContext(IOSGLTestContext* shareContext)
    : fGLLibrary(RTLD_DEFAULT) {

    if (shareContext) {
        EAGLContext* iosShareContext = shareContext->fEAGLContext.get();
        fEAGLContext.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
                                                 sharegroup:[iosShareContext sharegroup]]);
        if (!fEAGLContext) {
            fEAGLContext.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
                                                     sharegroup:[iosShareContext sharegroup]]);
        }
    } else {
        fEAGLContext.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]);
        if (!fEAGLContext) {
            fEAGLContext.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]);
        }
    }
    SkScopeExit restorer(context_restorer());
    [EAGLContext setCurrentContext:fEAGLContext.get()];

    sk_sp<const GrGLInterface> gl(GrGLCreateNativeInterface());
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

    this->init(std::move(gl));
}

IOSGLTestContext::~IOSGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void IOSGLTestContext::destroyGLContext() {
    if (fEAGLContext) {
        if ([EAGLContext currentContext] == fEAGLContext.get()) {
            // This will ensure that the context is immediately deleted.
            [EAGLContext setCurrentContext:nil];
        }
        fEAGLContext.reset();
    }
    if (nullptr != fGLLibrary) {
        dlclose(fGLLibrary);
    }
}

void IOSGLTestContext::onPlatformMakeNotCurrent() const {
    if (![EAGLContext setCurrentContext:nil]) {
        SkDebugf("Could not reset the context.\n");
    }
}

void IOSGLTestContext::onPlatformMakeCurrent() const {
    if (![EAGLContext setCurrentContext:fEAGLContext.get()]) {
        SkDebugf("Could not set the context.\n");
    }
}

std::function<void()> IOSGLTestContext::onPlatformGetAutoContextRestore() const {
    if ([EAGLContext currentContext] == fEAGLContext.get()) {
        return nullptr;
    }
    return context_restorer();
}

GrGLFuncPtr IOSGLTestContext::onPlatformGetProcAddress(const char* procName) const {
    void* handle = (nullptr == fGLLibrary) ? RTLD_DEFAULT : fGLLibrary;
    return reinterpret_cast<GrGLFuncPtr>(dlsym(handle, procName));
}

}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext *CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext *shareContext) {
    if (kGL_GrGLStandard == forcedGpuAPI) {
        return NULL;
    }
    IOSGLTestContext* iosShareContext = reinterpret_cast<IOSGLTestContext*>(shareContext);
    IOSGLTestContext *ctx = new IOSGLTestContext(iosShareContext);
    if (!ctx->isValid()) {
        delete ctx;
        return NULL;
    }
    return ctx;
}
}

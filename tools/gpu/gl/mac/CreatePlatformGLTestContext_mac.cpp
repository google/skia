
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"

#include "gl/GLTestContext.h"
#include "AvailabilityMacros.h"

#include <OpenGL/OpenGL.h>
#include <dlfcn.h>

namespace {
class MacGLTestContext : public sk_gpu_test::GLTestContext {
public:
    MacGLTestContext();
    ~MacGLTestContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    CGLContextObj fContext;
    void* fGLLibrary;
};

MacGLTestContext::MacGLTestContext()
    : fContext(nullptr)
    , fGLLibrary(RTLD_DEFAULT) {
    CGLPixelFormatAttribute attributes[] = {
#if MAC_OS_X_VERSION_10_7
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
#endif
        kCGLPFADoubleBuffer,
        (CGLPixelFormatAttribute)0
    };
    CGLPixelFormatObj pixFormat;
    GLint npix;

    CGLChoosePixelFormat(attributes, &pixFormat, &npix);

    if (nullptr == pixFormat) {
        SkDebugf("CGLChoosePixelFormat failed.");
        return;
    }

    CGLCreateContext(pixFormat, nullptr, &fContext);
    CGLReleasePixelFormat(pixFormat);

    if (nullptr == fContext) {
        SkDebugf("CGLCreateContext failed.");
        return;
    }

    CGLSetCurrentContext(fContext);

    SkAutoTUnref<const GrGLInterface> gl(GrGLCreateNativeInterface());
    if (nullptr == gl.get()) {
        SkDebugf("Context could not create GL interface.\n");
        this->destroyGLContext();
        return;
    }
    if (!gl->validate()) {
        SkDebugf("Context could not validate GL interface.\n");
        this->destroyGLContext();
        return;
    }

    fGLLibrary = dlopen(
        "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib",
        RTLD_LAZY);

    this->init(gl.release());
}

MacGLTestContext::~MacGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void MacGLTestContext::destroyGLContext() {
    if (fContext) {
        CGLReleaseContext(fContext);
        fContext = nullptr;
    }
    if (RTLD_DEFAULT != fGLLibrary) {
        dlclose(fGLLibrary);
    }
}

void MacGLTestContext::onPlatformMakeCurrent() const {
    CGLSetCurrentContext(fContext);
}

void MacGLTestContext::onPlatformSwapBuffers() const {
    CGLFlushDrawable(fContext);
}

GrGLFuncPtr MacGLTestContext::onPlatformGetProcAddress(const char* procName) const {
    return reinterpret_cast<GrGLFuncPtr>(dlsym(fGLLibrary, procName));
}

}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext* CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext* shareContext) {
    SkASSERT(!shareContext);
    if (shareContext) {
        return nullptr;
    }

    if (kGLES_GrGLStandard == forcedGpuAPI) {
        return nullptr;
    }
    MacGLTestContext* ctx = new MacGLTestContext;
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}
}  // namespace sk_gpu_test

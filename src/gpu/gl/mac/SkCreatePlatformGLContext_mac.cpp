
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"
#include "AvailabilityMacros.h"

#include <OpenGL/OpenGL.h>
#include <dlfcn.h>

namespace {
class MacGLContext : public SkGLContext {
public:
    MacGLContext();
    ~MacGLContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    CGLContextObj fContext;
    void* fGLLibrary;
};

MacGLContext::MacGLContext()
    : fContext(NULL)
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

    if (NULL == pixFormat) {
        SkDebugf("CGLChoosePixelFormat failed.");
        return;
    }

    CGLCreateContext(pixFormat, NULL, &fContext);
    CGLReleasePixelFormat(pixFormat);

    if (NULL == fContext) {
        SkDebugf("CGLCreateContext failed.");
        return;
    }

    CGLSetCurrentContext(fContext);

    SkAutoTUnref<const GrGLInterface> gl(GrGLCreateNativeInterface());
    if (NULL == gl.get()) {
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

    this->init(gl.detach());
}

MacGLContext::~MacGLContext() {
    this->teardown();
    this->destroyGLContext();
}

void MacGLContext::destroyGLContext() {
    if (fContext) {
        CGLReleaseContext(fContext);
        fContext = NULL;
    }
    if (RTLD_DEFAULT != fGLLibrary) {
        dlclose(fGLLibrary);
    }
}

void MacGLContext::onPlatformMakeCurrent() const {
    CGLSetCurrentContext(fContext);
}

void MacGLContext::onPlatformSwapBuffers() const {
    CGLFlushDrawable(fContext);
}

GrGLFuncPtr MacGLContext::onPlatformGetProcAddress(const char* procName) const {
    return reinterpret_cast<GrGLFuncPtr>(dlsym(fGLLibrary, procName));
}

} // anonymous namespace

SkGLContext* SkCreatePlatformGLContext(GrGLStandard forcedGpuAPI) {
    if (kGLES_GrGLStandard == forcedGpuAPI) {
        return NULL;
    }
    MacGLContext* ctx = SkNEW(MacGLContext);
    if (!ctx->isValid()) {
        SkDELETE(ctx);
        return NULL;
    }
    return ctx;
}

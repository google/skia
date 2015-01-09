
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"
#include "AvailabilityMacros.h"

#include <OpenGL/OpenGL.h>

namespace {
class MacGLContext : public SkGLContext {
public:
    MacGLContext();
    ~MacGLContext() SK_OVERRIDE;
    void makeCurrent() const SK_OVERRIDE;
    void swapBuffers() const SK_OVERRIDE;

private:
    void destroyGLContext();

    CGLContextObj fContext;
};

MacGLContext::MacGLContext()
    : fContext(NULL) {
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

    fGL.reset(GrGLCreateNativeInterface());
    if (NULL == fGL.get()) {
        SkDebugf("Context could not create GL interface.\n");
        this->destroyGLContext();
        return;
    }
    if (!fGL->validate()) {
        SkDebugf("Context could not validate GL interface.\n");
        this->destroyGLContext();
        return;
    }
}

MacGLContext::~MacGLContext() {
    this->destroyGLContext();
}

void MacGLContext::destroyGLContext() {
    fGL.reset(NULL);
    if (fContext) {
        CGLReleaseContext(fContext);
        fContext = NULL;
    }
}

void MacGLContext::makeCurrent() const {
    CGLSetCurrentContext(fContext);
}

void MacGLContext::swapBuffers() const {
    CGLFlushDrawable(fContext);
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

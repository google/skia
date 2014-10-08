
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

    virtual ~MacGLContext();

    virtual void makeCurrent() const SK_OVERRIDE;
    virtual void swapBuffers() const SK_OVERRIDE;
protected:
    virtual const GrGLInterface* createGLContext(GrGLStandard forcedGpuAPI) SK_OVERRIDE;
    virtual void destroyGLContext() SK_OVERRIDE;

private:
    CGLContextObj fContext;
};

MacGLContext::MacGLContext()
    : fContext(NULL) {
}

MacGLContext::~MacGLContext() {
    this->destroyGLContext();
}

void MacGLContext::destroyGLContext() {
    if (fContext) {
        CGLReleaseContext(fContext);
    }
}

const GrGLInterface* MacGLContext::createGLContext(GrGLStandard forcedGpuAPI) {
    SkASSERT(NULL == fContext);
    if (kGLES_GrGLStandard == forcedGpuAPI) {
        return NULL;
    }

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
        return NULL;
    }

    CGLCreateContext(pixFormat, NULL, &fContext);
    CGLReleasePixelFormat(pixFormat);

    if (NULL == fContext) {
        SkDebugf("CGLCreateContext failed.");
        return NULL;
    }

    CGLSetCurrentContext(fContext);

    const GrGLInterface* interface = GrGLCreateNativeInterface();
    if (NULL == interface) {
        SkDebugf("Context could not create GL interface.\n");
        this->destroyGLContext();
        return NULL;
    }

    return interface;
}

void MacGLContext::makeCurrent() const {
    CGLSetCurrentContext(fContext);
}

void MacGLContext::swapBuffers() const {
    CGLFlushDrawable(fContext);
}

} // anonymous namespace

SkGLContext* SkCreatePlatformGLContext() {
    return SkNEW(MacGLContext);
}

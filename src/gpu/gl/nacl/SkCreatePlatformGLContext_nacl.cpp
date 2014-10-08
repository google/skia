
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"

#include <GLES2/gl2.h>
#include <EGL/egl.h>

namespace {
class NACLGLContext : public SkGLContext  {
public:
    SkGLContextEGL();

    virtual ~NACLGLContext();

    virtual void makeCurrent() const SK_OVERRIDE;
    virtual void swapBuffers() const SK_OVERRIDE;
protected:
    virtual const GrGLInterface* createGLContext(GrGLStandard forcedGpuAPI) SK_OVERRIDE;
    virtual void destroyGLContext() SK_OVERRIDE;

private:
    EGLContext fContext;
    EGLDisplay fDisplay;
    EGLSurface fSurface;
};

NACLGLContext::NACLGLContext()
    : fContext(NULL)
    , fDisplay(NULL)
{
}

NACLGLContext::~NACLGLContext() {
    this->destroyGLContext();
}

void NACLGLContext::destroyGLContext() {
}

const GrGLInterface* NACLGLContext::createGLContext(GrGLStandard forcedGpuAPI) {
    return NULL;
}

void NACLGLContext::makeCurrent() const {
}

void NACLGLContext::swapBuffers() const {
}

} // anonymous namespace

NACLGLContext* SkCreatePlatformGLContext() {
    return SkNEW(NACLGLContext);
}


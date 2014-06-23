
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDebugGLContext_DEFINED
#define SkDebugGLContext_DEFINED

#include "SkGLContextHelper.h"

class SkDebugGLContext : public SkGLContextHelper {

public:
    SkDebugGLContext() {};

    virtual void makeCurrent() const SK_OVERRIDE {};
    virtual void swapBuffers() const SK_OVERRIDE {};

protected:
    virtual const GrGLInterface* createGLContext(GrGLStandard forcedGpuAPI) SK_OVERRIDE;

    virtual void destroyGLContext() SK_OVERRIDE {};
};

#endif

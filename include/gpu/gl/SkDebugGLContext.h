
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDebugGLContext_DEFINED
#define SkDebugGLContext_DEFINED

#include "SkGLContext.h"

class SkDebugGLContext : public SkGLContext {
public:
    virtual ~SkDebugGLContext() SK_OVERRIDE;
    virtual void makeCurrent() const SK_OVERRIDE {}
    virtual void swapBuffers() const SK_OVERRIDE {}

    static SkDebugGLContext* Create(GrGLStandard forcedGpuAPI) {
        if (kGLES_GrGLStandard == forcedGpuAPI) {
            return NULL;
        }
        return SkNEW(SkDebugGLContext);
    }
private:
    SkDebugGLContext();
};

#endif


/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDebugGLContext_DEFINED
#define SkDebugGLContext_DEFINED

#include "gl/SkGLContext.h"

class SkDebugGLContext : public SkGLContext {
public:
    ~SkDebugGLContext() override;
    void makeCurrent() const override {}
    void swapBuffers() const override {}

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

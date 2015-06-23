
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

    static SkDebugGLContext* Create(GrGLStandard forcedGpuAPI) {
        if (kGLES_GrGLStandard == forcedGpuAPI) {
            return NULL;
        }
        return SkNEW(SkDebugGLContext);
    }
private:
    void onPlatformMakeCurrent() const override {}
    void onPlatformSwapBuffers() const override {}
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override { return NULL; }

    SkDebugGLContext();
};

#endif

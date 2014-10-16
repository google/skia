
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkANGLEGLContext_DEFINED
#define SkANGLEGLContext_DEFINED

#if SK_ANGLE

#include "SkGLContext.h"

#include <GLES2/gl2.h>
#include <EGL/egl.h>

class SkANGLEGLContext : public SkGLContext {
public:
    virtual ~SkANGLEGLContext() SK_OVERRIDE;
    virtual void makeCurrent() const SK_OVERRIDE;
    virtual void swapBuffers() const SK_OVERRIDE;

    static SkANGLEGLContext* Create(GrGLStandard forcedGpuAPI) {
        if (kGL_GrGLStandard == forcedGpuAPI) {
            return NULL;
        }
        SkANGLEGLContext* ctx = SkNEW(SkANGLEGLContext);
        if (!ctx->isValid()) {
            SkDELETE(ctx);
            return NULL;
        }
        return ctx;
    }

private:
    SkANGLEGLContext();
    void destroyGLContext();

    EGLContext fContext;
    EGLDisplay fDisplay;
    EGLSurface fSurface;
};

#endif

#endif

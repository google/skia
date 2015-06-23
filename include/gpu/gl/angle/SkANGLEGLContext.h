
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkANGLEGLContext_DEFINED
#define SkANGLEGLContext_DEFINED

#if SK_ANGLE

#include "gl/SkGLContext.h"

class SkANGLEGLContext : public SkGLContext {
public:
    ~SkANGLEGLContext() override;

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

    // The param is an EGLNativeDisplayType and the return is an EGLDispay.
    static void* GetD3DEGLDisplay(void* nativeDisplay);

private:
    SkANGLEGLContext();
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char* name) const override;

    void* fContext;
    void* fDisplay;
    void* fSurface;
};

#endif

#endif

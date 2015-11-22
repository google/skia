
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

    static SkANGLEGLContext* Create(GrGLStandard forcedGpuAPI, bool useGLBackend) {
        if (kGL_GrGLStandard == forcedGpuAPI) {
            return NULL;
        }
        SkANGLEGLContext* ctx = new SkANGLEGLContext(useGLBackend);
        if (!ctx->isValid()) {
            delete ctx;
            return NULL;
        }
        return ctx;
    }
    GrEGLImage texture2DToEGLImage(GrGLuint texID) const override;
    void destroyEGLImage(GrEGLImage) const override;
    GrGLuint eglImageToExternalTexture(GrEGLImage) const override;
    SkGLContext* createNew() const override;

    // The param is an EGLNativeDisplayType and the return is an EGLDispay.
    static void* GetD3DEGLDisplay(void* nativeDisplay, bool useGLBackend);

private:
    SkANGLEGLContext(bool preferGLBackend);
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char* name) const override;

    void* fContext;
    void* fDisplay;
    void* fSurface;
    bool  fIsGLBackend;
};

#endif

#endif


/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SKCOMMANDBUFFERGLCONTEXT_DEFINED
#define SKCOMMANDBUFFERGLCONTEXT_DEFINED

#if SK_COMMAND_BUFFER

#include "gl/SkGLContext.h"

class SkCommandBufferGLContext : public SkGLContext {
public:
    ~SkCommandBufferGLContext() override;

    static SkCommandBufferGLContext* Create(GrGLStandard forcedGpuAPI) {
        if (kGL_GrGLStandard == forcedGpuAPI) {
            return nullptr;
        }
        SkCommandBufferGLContext* ctx = new SkCommandBufferGLContext;
        if (!ctx->isValid()) {
            delete ctx;
            return nullptr;
        }
        return ctx;
    }

    static SkCommandBufferGLContext* Create(void* nativeWindow, int msaaSampleCount) {
        SkCommandBufferGLContext* ctx = new SkCommandBufferGLContext(nativeWindow,
                                                                     msaaSampleCount);
        if (!ctx->isValid()) {
            delete ctx;
            return nullptr;
        }
        return ctx;
    }

    void presentCommandBuffer();

    bool makeCurrent();
    int getStencilBits();
    int getSampleCount();

private:
    SkCommandBufferGLContext();
    SkCommandBufferGLContext(void* nativeWindow, int msaaSampleCount);
    void initializeGLContext(void* nativeWindow, const int* configAttribs,
                             const int* surfaceAttribs);
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char* name) const override;

    void* fContext;
    void* fDisplay;
    void* fSurface;
    void* fConfig;
};

#endif // SK_COMMAND_BUFFER

#endif

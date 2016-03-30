
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLContext_command_buffer_DEFINED
#define GLContext_command_buffer_DEFINED

#include "gl/GLContext.h"

namespace sk_gpu_test {
class CommandBufferGLContext : public GLContext {
public:
    ~CommandBufferGLContext() override;

    static CommandBufferGLContext *Create() {
        CommandBufferGLContext *ctx = new CommandBufferGLContext;
        if (!ctx->isValid()) {
            delete ctx;
            return nullptr;
        }
        return ctx;
    }

    static CommandBufferGLContext *Create(void *nativeWindow, int msaaSampleCount) {
        CommandBufferGLContext *ctx = new CommandBufferGLContext(nativeWindow, msaaSampleCount);
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
    CommandBufferGLContext();

    CommandBufferGLContext(void *nativeWindow, int msaaSampleCount);

    void initializeGLContext(void *nativeWindow, const int *configAttribs,
                             const int *surfaceAttribs);

    void destroyGLContext();

    void onPlatformMakeCurrent() const override;

    void onPlatformSwapBuffers() const override;

    GrGLFuncPtr onPlatformGetProcAddress(const char *name) const override;

    void *fContext;
    void *fDisplay;
    void *fSurface;
    void *fConfig;
};
}   // namespace sk_gpu_test

#endif


/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLTestContext_command_buffer_DEFINED
#define GLTestContext_command_buffer_DEFINED

#include "gl/GLTestContext.h"

namespace sk_gpu_test {
class CommandBufferGLTestContext : public GLTestContext {
public:
    ~CommandBufferGLTestContext() override;

    static CommandBufferGLTestContext *Create() {
        CommandBufferGLTestContext *ctx = new CommandBufferGLTestContext;
        if (!ctx->isValid()) {
            delete ctx;
            return nullptr;
        }
        return ctx;
    }

    static CommandBufferGLTestContext *Create(void *nativeWindow, int msaaSampleCount) {
        CommandBufferGLTestContext *ctx = new CommandBufferGLTestContext(nativeWindow, msaaSampleCount);
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
    CommandBufferGLTestContext();

    CommandBufferGLTestContext(void *nativeWindow, int msaaSampleCount);

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

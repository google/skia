
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLTestContext_command_buffer_DEFINED
#define GLTestContext_command_buffer_DEFINED

#include "tools/gpu/gl/GLTestContext.h"

namespace sk_gpu_test {
class CommandBufferGLTestContext : public GLTestContext {
public:
    ~CommandBufferGLTestContext() override;

    static CommandBufferGLTestContext *Create(GLTestContext* shareContext) {
        CommandBufferGLTestContext* cbShareContext =
                reinterpret_cast<CommandBufferGLTestContext*>(shareContext);
        CommandBufferGLTestContext *ctx = new CommandBufferGLTestContext(cbShareContext);
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
    CommandBufferGLTestContext(CommandBufferGLTestContext* shareContext);

    void destroyGLContext();

    void onPlatformMakeCurrent() const override;

    std::function<void()> onPlatformGetAutoContextRestore() const override;

    void onPlatformSwapBuffers() const override;

    GrGLFuncPtr onPlatformGetProcAddress(const char *name) const override;

    void *fContext;
    void *fDisplay;
    void *fSurface;
    void *fConfig;
};
}   // namespace sk_gpu_test

#endif

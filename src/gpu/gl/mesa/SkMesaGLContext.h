
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMesaGLContext_DEFINED
#define SkMesaGLContext_DEFINED

#include "gl/SkGLContext.h"

#if SK_MESA

class SkMesaGLContext : public SkGLContext {
private:
    typedef intptr_t Context;

public:
    ~SkMesaGLContext() override;

    static SkMesaGLContext* Create() {
        SkMesaGLContext* ctx = new SkMesaGLContext;
        if (!ctx->isValid()) {
            delete ctx;
            return nullptr;
        }
        return ctx;
    }

private:
    SkMesaGLContext();
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    Context fContext;
    GrGLubyte *fImage;
};

#endif

#endif

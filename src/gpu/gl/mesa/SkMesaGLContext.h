
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
    void makeCurrent() const override;
    void swapBuffers() const override;

    static SkMesaGLContext* Create(GrGLStandard forcedGpuAPI) {
        if (kGLES_GrGLStandard == forcedGpuAPI) {
            return NULL;
        }
        SkMesaGLContext* ctx = SkNEW(SkMesaGLContext);
        if (!ctx->isValid()) {
            SkDELETE(ctx);
            return NULL;
        }
        return ctx;
    }

private:
    SkMesaGLContext();
    void destroyGLContext();

    Context fContext;
    GrGLubyte *fImage;
};

#endif

#endif

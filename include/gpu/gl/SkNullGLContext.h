
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNullGLContext_DEFINED
#define SkNullGLContext_DEFINED

#include "SkGLContext.h"

class SK_API SkNullGLContext : public SkGLContext {
public:
    virtual ~SkNullGLContext() SK_OVERRIDE;
    virtual void makeCurrent() const SK_OVERRIDE {};
    virtual void swapBuffers() const SK_OVERRIDE {};

    static SkNullGLContext* Create(GrGLStandard forcedGpuAPI) {
        if (kGLES_GrGLStandard == forcedGpuAPI) {
            return NULL;
        }
        SkNullGLContext* ctx = SkNEW(SkNullGLContext);
        if (!ctx->isValid()) {
            SkDELETE(ctx);
            return NULL;
        }
        return ctx;
    }

private:
    SkNullGLContext();
};

#endif

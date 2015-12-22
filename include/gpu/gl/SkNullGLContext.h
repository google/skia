
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNullGLContext_DEFINED
#define SkNullGLContext_DEFINED

#include "gl/SkGLContext.h"

class SK_API SkNullGLContext : public SkGLContext {
public:
    ~SkNullGLContext() override;

    static SkNullGLContext* Create();
    // FIXME: remove once Chromium has been updated.
    static SkNullGLContext* Create(GrGLStandard forcedAPI) {
        SkASSERT(forcedAPI == kNone_GrGLStandard);
        (void)forcedAPI;        return Create();
    }

    class ContextState;

private:
    SkNullGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override {}
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override { return NULL; }

    ContextState* fState;
};

#endif

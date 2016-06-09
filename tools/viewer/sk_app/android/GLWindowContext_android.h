
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLWindowContext_android_DEFINED
#define GLWindowContext_android_DEFINED

#include "../GLWindowContext.h"
#include "Window_android.h"

#include <EGL/egl.h>

struct ANativeWindow;

namespace sk_app {

class GLWindowContext_android : public GLWindowContext {
public:
    friend GLWindowContext* GLWindowContext::Create(void* platformData, const DisplayParams&);

    ~GLWindowContext_android() override;

    void onSwapBuffers() override;

    void onInitializeContext(void*, const DisplayParams&) override;
    void onDestroyContext() override;

private:
    GLWindowContext_android(void*, const DisplayParams&);

    EGLDisplay fDisplay;
    EGLContext fEGLContext;
    EGLSurface fSurface;

    // For setDisplayParams and resize which call onInitializeContext with null platformData
    ANativeWindow* fNativeWindow = nullptr;
};


}

#endif


/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLWindowContext_unix_DEFINED
#define GLWindowContext_unix_DEFINED

#include "../GLWindowContext.h"
#include "Window_unix.h"

namespace sk_app {

class GLWindowContext_unix : public GLWindowContext {
public:
    friend GLWindowContext* GLWindowContext::Create(void* platformData, const DisplayParams&);

    ~GLWindowContext_unix() override;

    void onSwapBuffers() override;

    void onInitializeContext(void*, const DisplayParams&) override;
    void onDestroyContext() override;

private:
    GLWindowContext_unix(void*, const DisplayParams&);

    Display*     fDisplay;
    XWindow      fWindow;
    XVisualInfo* fVisualInfo;
    GLXContext   fGLContext;
};


}

#endif

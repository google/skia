
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLWindowContext_mac_DEFINED
#define GLWindowContext_mac_DEFINED

#include "../GLWindowContext.h"
#include "Window_mac.h"

namespace sk_app {

class GLWindowContext_mac : public GLWindowContext {
public:
    friend GLWindowContext* GLWindowContext::Create(void* platformData, const DisplayParams&);

    ~GLWindowContext_mac() override;

    void onSwapBuffers() override;

    void onInitializeContext(void*, const DisplayParams&) override;
    void onDestroyContext() override;

private:
    GLWindowContext_mac(void*, const DisplayParams&);

#if 0
    // TODO: add Mac-specific GL display objects
    Display*     fDisplay;
    XWindow      fWindow;
    XVisualInfo* fVisualInfo;
    GLXContext   fGLContext;
#endif
};


}

#endif

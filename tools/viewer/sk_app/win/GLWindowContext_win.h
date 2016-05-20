
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLWindowContext_win_DEFINED
#define GLWindowContext_win_DEFINED

#include <windows.h>
#include "../GLWindowContext.h"

namespace sk_app {

class GLWindowContext_win : public GLWindowContext {
public:
    friend GLWindowContext* GLWindowContext::Create(void* platformData, const DisplayParams&);

    ~GLWindowContext_win() override;

    void onSwapBuffers() override;

    void onInitializeContext(void*, const DisplayParams&) override;
    void onDestroyContext() override;

private:
    GLWindowContext_win(void*, const DisplayParams&);

    HWND              fHWND;
    HGLRC             fHGLRC;
};


}

#endif

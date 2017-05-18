
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../GLWindowContext.h"
#include "WindowContextFactory_mac.h"

#include "SDL.h"

#include <OpenGL/gl.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::MacWindowInfo;
using sk_app::GLWindowContext;

namespace {

class GLWindowContext_mac : public GLWindowContext {
public:
    GLWindowContext_mac(const MacWindowInfo&, const DisplayParams&);
    
    ~GLWindowContext_mac() override;
    
    void onSwapBuffers() override;
    
    void onInitializeContext() override;
    void onDestroyContext() override;
    
private:
    SDL_Window*   fWindow;
    SDL_GLContext fGLContext;
    
    typedef GLWindowContext INHERITED;
};

GLWindowContext_mac::GLWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
    : INHERITED(params)
    , fWindow(info.fWindow)
    , fGLContext(nullptr) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_mac::~GLWindowContext_mac() {
    this->destroyContext();
}

void GLWindowContext_mac::onInitializeContext() {
    SkASSERT(fWindow);

    fGLContext = SDL_GL_CreateContext(fWindow);
    if (!fGLContext) {
        SkDebugf("%s\n", SDL_GetError());
        return;
    }

    if (0 == SDL_GL_MakeCurrent(fWindow, fGLContext)) {
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &fStencilBits);
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &fSampleCount);

        SDL_GetWindowSize(fWindow, &fWidth, &fHeight);
        glViewport(0, 0, fWidth, fHeight);
    } else {
        SkDebugf("MakeCurrent failed: %s\n", SDL_GetError());
    }
}

void GLWindowContext_mac::onDestroyContext() {
    if (!fWindow || !fGLContext) {
        return;
    }
    SDL_GL_DeleteContext(fGLContext);
    fGLContext = nullptr;
}


void GLWindowContext_mac::onSwapBuffers() {
    if (fWindow && fGLContext) {
        SDL_GL_SwapWindow(fWindow);
    }
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

WindowContext* NewGLForMac(const MacWindowInfo& info, const DisplayParams& params) {
    WindowContext* ctx = new GLWindowContext_mac(info, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app

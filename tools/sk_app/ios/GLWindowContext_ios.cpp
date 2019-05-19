
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <OpenGLES/ES2/gl.h>
#include "tools/sk_app/GLWindowContext.h"
#include "SDL.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "tools/sk_app/ios/WindowContextFactory_ios.h"

using sk_app::DisplayParams;
using sk_app::window_context_factory::IOSWindowInfo;
using sk_app::GLWindowContext;

namespace {

class GLWindowContext_ios : public GLWindowContext {
public:
    GLWindowContext_ios(const IOSWindowInfo&, const DisplayParams&);

    ~GLWindowContext_ios() override;

    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override {}

private:
    SDL_Window*   fWindow;
    SDL_GLContext fGLContext;

    typedef GLWindowContext INHERITED;
};

GLWindowContext_ios::GLWindowContext_ios(const IOSWindowInfo& info, const DisplayParams& params)
    : INHERITED(params)
    , fWindow(info.fWindow)
    , fGLContext(info.fGLContext) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_ios::~GLWindowContext_ios() {
    this->destroyContext();
}

sk_sp<const GrGLInterface> GLWindowContext_ios::onInitializeContext() {
    SkASSERT(fWindow);
    SkASSERT(fGLContext);

    if (0 == SDL_GL_MakeCurrent(fWindow, fGLContext)) {
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &fStencilBits);
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &fSampleCount);
        fSampleCount = SkTMax(fSampleCount, 1);

        SDL_GL_GetDrawableSize(fWindow, &fWidth, &fHeight);
        glViewport(0, 0, fWidth, fHeight);
    } else {
        SkDebugf("MakeCurrent failed: %s\n", SDL_GetError());
    }
    return GrGLMakeNativeInterface();
}

void GLWindowContext_ios::onSwapBuffers() {
    if (fWindow && fGLContext) {
        SDL_GL_SwapWindow(fWindow);
    }
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

WindowContext* NewGLForIOS(const IOSWindowInfo& info, const DisplayParams& params) {
    WindowContext* ctx = new GLWindowContext_ios(info, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app

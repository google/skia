
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"
#include "tools/sk_app/GLWindowContext.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"

#include <OpenGL/gl.h>
#include <Cocoa/Cocoa.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::MacWindowInfo;
using sk_app::GLWindowContext;

namespace {

class GLWindowContext_mac : public GLWindowContext {
public:
    GLWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

    ~GLWindowContext_mac() override;

    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

private:
    GLFWwindow* fWindow;

    typedef GLWindowContext INHERITED;
};

GLWindowContext_mac::GLWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
    : INHERITED(params)
    , fWindow(info.fWindow) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_mac::~GLWindowContext_mac() {
    this->destroyContext();
}

sk_sp<const GrGLInterface> GLWindowContext_mac::onInitializeContext() {
    SkASSERT(fWindow);

    fStencilBits = 8;
    fSampleCount = 1;

    glfwMakeContextCurrent(fWindow);

    glfwGetWindowSize(fWindow, &fWidth, &fHeight);
    glViewport(0, 0, fWidth, fHeight);

    return GrGLMakeNativeInterface();
}

void GLWindowContext_mac::onDestroyContext() {
}

void GLWindowContext_mac::onSwapBuffers() {
    glfwSwapBuffers(fWindow);
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

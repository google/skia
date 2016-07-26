
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../GLWindowContext.h"
#include "WindowContextFactory_mac.h"

//#include <GL/gl.h>

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
  
#if 0
    // TODO: add Mac-specific GL display objects
    Display*     fDisplay;
    XWindow      fWindow;
    XVisualInfo* fVisualInfo;
    GLXContext   fGLContext;
#endif
};

GLWindowContext_mac::GLWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
    : GLWindowContext(params)
#if 0
    // TODO: init Mac-specific OpenGL objects
    , fDisplay(nullptr)
    , fWindow(0)
    , fGLContext(0)
#endif
    {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_mac::~GLWindowContext_mac() {
    this->destroyContext();
}

void GLWindowContext_mac::onInitializeContext() {
#if 0
    // TODO: Init for Mac
    SkASSERT(fDisplay);

    fGLContext = glXCreateContext(fDisplay, fVisualInfo, nullptr, GL_TRUE);
    if (!fGLContext) {
        return;
    }

    if (glXMakeCurrent(fDisplay, fWindow, fGLContext)) {
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        int redBits, greenBits, blueBits;
        glXGetConfig(fDisplay, fVisualInfo, GLX_RED_SIZE, &redBits);
        glXGetConfig(fDisplay, fVisualInfo, GLX_GREEN_SIZE, &greenBits);
        glXGetConfig(fDisplay, fVisualInfo, GLX_BLUE_SIZE, &blueBits);
        fColorBits = redBits + greenBits + blueBits;
        glXGetConfig(fDisplay, fVisualInfo, GLX_STENCIL_SIZE, &fStencilBits);
        glXGetConfig(fDisplay, fVisualInfo, GLX_SAMPLES_ARB, &fSampleCount);

        XWindow root;
        int x, y;
        unsigned int border_width, depth;
        XGetGeometry(fDisplay, fWindow, &root, &x, &y,
                     (unsigned int*)&fWidth, (unsigned int*)&fHeight, &border_width, &depth); 
        glViewport(0, 0, fWidth, fHeight);
    }
#endif
}

void GLWindowContext_mac::onDestroyContext() {
#if 0
    // TODO: teardown for Mac
    if (!fDisplay || !fGLContext) {
        return;
    }
    glXMakeCurrent(fDisplay, None, nullptr);
    glXDestroyContext(fDisplay, fGLContext);
    fGLContext = nullptr;
#endif
}


void GLWindowContext_mac::onSwapBuffers() {
#if 0
    // TODO: swap for Mac
    if (fDisplay && fGLContext) {
        glXSwapBuffers(fDisplay, fWindow);
    }
#endif
}

}  // anonymous namespace


namespace sk_app {
namespace window_context_factory {

WindowContext* MakeGLForMac(const MacWindowInfo& info, const DisplayParams& params) {
    WindowContext* ctx = new GLWindowContext_mac(info, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app

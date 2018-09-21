/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <OpenGL/gl.h>
#include "../NXTGLWindowContext.h"
#include "WindowContextFactory_mac.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/OpenGLBackend.h"
#include "nxt/GrNXTBackendContext.h"

using sk_app::DisplayParams;
using sk_app::window_context_factory::MacWindowInfo;
using sk_app::NXTGLWindowContext;

namespace {

class NXTGLWindowContext_mac : public NXTGLWindowContext {
public:
    NXTGLWindowContext_mac(const MacWindowInfo&, const DisplayParams&);
    ~NXTGLWindowContext_mac() override;

    void onSwapBuffers() override;
    sk_sp<GrNXTBackendContext> onInitializeContext() override;
    void onDestroyContext() override;

private:
    void createGLInterface();

private:
    NXTGLWindowContext_mac(void*, const DisplayParams&);

    SDL_Window*                fWindow;
    SDL_GLContext              fGLContext;

    typedef NXTGLWindowContext INHERITED;
};

NXTGLWindowContext_mac::NXTGLWindowContext_mac(const MacWindowInfo& winInfo, const DisplayParams& params)
        : INHERITED(params)
        , fWindow(winInfo.fWindow) {
    int width, height;
    SDL_GetWindowSize(fWindow, &width, &height);
    this->initializeContext(width, height);
}

sk_sp<GrNXTBackendContext> NXTGLWindowContext_mac::onInitializeContext() {
    createGLInterface();
    SDL_GL_MakeCurrent(fWindow, fGLContext);
    dawnProcTable backendProcs = dawn_native::GetProcs();
    dawnDevice backendDevice = dawn_native::opengl::CreateDevice(reinterpret_cast<void*(*)(const char*)>(SDL_GL_GetProcAddress));

    dawnSetProcs(&backendProcs);
//    backendProcs.deviceSetErrorCallback(backendDevice, PrintDeviceError, 0);
    dawnQueue backendQueue = dawnDeviceCreateQueue(backendDevice);
    sk_sp<GrNXTBackendContext> ctx(new GrNXTBackendContext(backendDevice, backendQueue));
    return ctx;
}

void NXTGLWindowContext_mac::createGLInterface() {
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
        fSampleCount = SkTMax(fSampleCount, 1);

        glViewport(0, 0, width(), height());
    } else {
        SkDebugf("MakeCurrent failed: %s\n", SDL_GetError());
    }
}

NXTGLWindowContext_mac::~NXTGLWindowContext_mac() {
    this->destroyContext();
}

void NXTGLWindowContext_mac::onDestroyContext() {
    if (!fWindow || !fGLContext) {
        return;
    }
    SDL_GL_DeleteContext(fGLContext);
    fGLContext = nullptr;
}

void NXTGLWindowContext_mac::onSwapBuffers() {
    if (fWindow && fGLContext) {
        SDL_GL_SwapWindow(fWindow);
    }
}

}  // anonymous namespace

namespace sk_app {

namespace window_context_factory {

WindowContext* NewNXTGLForMac(const MacWindowInfo& winInfo, const DisplayParams& params) {
    WindowContext* ctx = new NXTGLWindowContext_mac(winInfo, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory

}  // namespace sk_app



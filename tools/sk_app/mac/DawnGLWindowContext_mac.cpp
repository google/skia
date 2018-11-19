/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <OpenGL/gl.h>
#include "../DawnGLWindowContext.h"
#include "WindowContextFactory_mac.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/OpenGLBackend.h"
#include "dawn/GrDawnBackendContext.h"

using sk_app::DisplayParams;
using sk_app::window_context_factory::MacWindowInfo;
using sk_app::DawnGLWindowContext;

namespace {

class DawnGLWindowContext_mac : public DawnGLWindowContext {
public:
    DawnGLWindowContext_mac(const MacWindowInfo&, const DisplayParams&);
    ~DawnGLWindowContext_mac() override;

    void onSwapBuffers() override;
    sk_sp<GrDawnBackendContext> onInitializeContext() override;
    void onDestroyContext() override;

private:
    void createGLInterface();

private:
    DawnGLWindowContext_mac(void*, const DisplayParams&);

    SDL_Window*                fWindow;
    SDL_GLContext              fGLContext;

    typedef DawnGLWindowContext INHERITED;
};

DawnGLWindowContext_mac::DawnGLWindowContext_mac(const MacWindowInfo& winInfo, const DisplayParams& params)
        : INHERITED(params)
        , fWindow(winInfo.fWindow) {
    int width, height;
    SDL_GetWindowSize(fWindow, &width, &height);
    this->initializeContext(width, height);
}

sk_sp<GrDawnBackendContext> DawnGLWindowContext_mac::onInitializeContext() {
    createGLInterface();
    SDL_GL_MakeCurrent(fWindow, fGLContext);
    dawnProcTable backendProcs = dawn_native::GetProcs();
    dawnDevice backendDevice = dawn_native::opengl::CreateDevice(reinterpret_cast<void*(*)(const char*)>(SDL_GL_GetProcAddress));

    dawnSetProcs(&backendProcs);
//    backendProcs.deviceSetErrorCallback(backendDevice, PrintDeviceError, 0);
    dawnQueue backendQueue = dawnDeviceCreateQueue(backendDevice);
    sk_sp<GrDawnBackendContext> ctx(new GrDawnBackendContext(backendDevice, backendQueue));
    return ctx;
}

void DawnGLWindowContext_mac::createGLInterface() {
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

DawnGLWindowContext_mac::~DawnGLWindowContext_mac() {
    this->destroyContext();
}

void DawnGLWindowContext_mac::onDestroyContext() {
    if (!fWindow || !fGLContext) {
        return;
    }
    SDL_GL_DeleteContext(fGLContext);
    fGLContext = nullptr;
}

void DawnGLWindowContext_mac::onSwapBuffers() {
    if (fWindow && fGLContext) {
        SDL_GL_SwapWindow(fWindow);
    }
}

}  // anonymous namespace

namespace sk_app {

namespace window_context_factory {

WindowContext* NewDawnGLForMac(const MacWindowInfo& winInfo, const DisplayParams& params) {
    WindowContext* ctx = new DawnGLWindowContext_mac(winInfo, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory

}  // namespace sk_app



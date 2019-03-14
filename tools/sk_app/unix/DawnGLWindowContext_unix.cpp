/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../DawnGLWindowContext.h"
#include "WindowContextFactory_unix.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/OpenGLBackend.h"

using sk_app::window_context_factory::XlibWindowInfo;
using sk_app::DisplayParams;
using sk_app::DawnGLWindowContext;

namespace {

class DawnGLWindowContext_xlib : public DawnGLWindowContext {
public:
    DawnGLWindowContext_xlib(const XlibWindowInfo&, const DisplayParams&);
    ~DawnGLWindowContext_xlib() override;

    void onSwapBuffers() override;
    dawn::Device onInitializeContext() override;
    void onDestroyContext() override;
    void discoverAdapters() override;

private:
    void createGLInterface();

private:
    DawnGLWindowContext_xlib(void*, const DisplayParams&);

    Display*     fDisplay;
    XWindow      fWindow;
    GLXFBConfig* fFBConfig;
    XVisualInfo* fVisualInfo;
    GLXContext   fGLContext;

    typedef DawnGLWindowContext INHERITED;
};

DawnGLWindowContext_xlib::DawnGLWindowContext_xlib(const XlibWindowInfo& winInfo, const DisplayParams& params)
        : INHERITED(params)
        , fDisplay(winInfo.fDisplay)
        , fWindow(winInfo.fWindow)
        , fFBConfig(winInfo.fFBConfig)
        , fVisualInfo(winInfo.fVisualInfo)
        , fGLContext() {
    XWindow root;
    int x, y;
    unsigned int border_width, depth;
    unsigned int width, height;
    XGetGeometry(fDisplay, fWindow, &root, &x, &y, &width, &height, &border_width, &depth);
    this->initializeContext(width, height);
}

dawn::Device DawnGLWindowContext_xlib::onInitializeContext() {
    createGLInterface();
    glXMakeCurrent(fDisplay, fWindow, fGLContext);
    return this->createDevice(dawn_native::BackendType::OpenGL);
}

using CreateContextAttribsFn = GLXContext(Display*, GLXFBConfig, GLXContext, Bool, const int*);

void DawnGLWindowContext_xlib::createGLInterface() {
    SkASSERT(fDisplay);
    SkASSERT(!fGLContext);
    // We attempt to use glXCreateContextAttribsARB as RenderDoc requires that the context be
    // created with this rather than glXCreateContext.
    CreateContextAttribsFn* createContextAttribs = (CreateContextAttribsFn*)glXGetProcAddressARB(
            (const GLubyte*)"glXCreateContextAttribsARB");
    if (createContextAttribs && fFBConfig) {
        // Specifying 3.2 allows an arbitrarily high context version (so long as no 3.2 features
        // have been removed).
        for (int minor = 2; minor >= 0 && !fGLContext; --minor) {
            // Ganesh prefers a compatibility profile for possible NVPR support. However, RenderDoc
            // requires a core profile. Edit this code to use RenderDoc.
            for (int profile : {GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                                GLX_CONTEXT_CORE_PROFILE_BIT_ARB}) {
                int attribs[] = {
                        GLX_CONTEXT_MAJOR_VERSION_ARB, 3, GLX_CONTEXT_MINOR_VERSION_ARB, minor,
                        GLX_CONTEXT_PROFILE_MASK_ARB, profile,
                        0
                };
                fGLContext = createContextAttribs(fDisplay, *fFBConfig, nullptr, True, attribs);
                if (fGLContext) {
                    break;
                }
            }
        }
    }
    if (!fGLContext) {
        fGLContext = glXCreateContext(fDisplay, fVisualInfo, nullptr, GL_TRUE);
    }
    if (!fGLContext) {
        return;
    }

    if (!glXMakeCurrent(fDisplay, fWindow, fGLContext)) {
        return;
    }
    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glXGetConfig(fDisplay, fVisualInfo, GLX_STENCIL_SIZE, &fStencilBits);
    glXGetConfig(fDisplay, fVisualInfo, GLX_SAMPLES_ARB, &fSampleCount);
    fSampleCount = SkTMax(fSampleCount, 1);

    glViewport(0, 0, width(), height());
}

DawnGLWindowContext_xlib::~DawnGLWindowContext_xlib() {
    this->destroyContext();
}

void DawnGLWindowContext_xlib::onDestroyContext() {
    if (!fDisplay || !fGLContext) {
        return;
    }
    glXMakeCurrent(fDisplay, None, nullptr);
    glXDestroyContext(fDisplay, fGLContext);
    fGLContext = nullptr;
}

void DawnGLWindowContext_xlib::onSwapBuffers() {
    if (fDisplay && fGLContext) {
        glXSwapBuffers(fDisplay, fWindow);
    }
}

void DawnGLWindowContext_xlib::discoverAdapters() {
    dawn_native::opengl::AdapterDiscoveryOptions adapterOptions;
    adapterOptions.getProc = reinterpret_cast<void* (*)(const char*)>(glXGetProcAddress);
    fInstance->DiscoverAdapters(&adapterOptions);
}

}  // anonymous namespace

namespace sk_app {

namespace window_context_factory {

WindowContext* NewDawnGLForXlib(const XlibWindowInfo& winInfo, const DisplayParams& params) {
    WindowContext* ctx = new DawnGLWindowContext_xlib(winInfo, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory

}  // namespace sk_app

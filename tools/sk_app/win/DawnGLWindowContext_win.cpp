/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <Windows.h>
#include <GL/gl.h>
#include "../DawnGLWindowContext.h"
#include "WindowContextFactory_win.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/OpenGLBackend.h"
#include "win/SkWGL.h"

using sk_app::DisplayParams;
using sk_app::DawnGLWindowContext;

namespace {

class ProcGetter {
public:
    typedef void(*Proc)();

    ProcGetter()
      : fModule(LoadLibraryA("opengl32.dll")) {
        SkASSERT(!fInstance);
        fInstance = this;
    }

    ~ProcGetter() {
        if (fModule) {
            FreeLibrary(fModule);
        }
        fInstance = nullptr;
    }

    static void* getProcAddress(const char* name) {
        return fInstance->getProc(name);
    }

private:
    Proc getProc(const char* name) {
        PROC proc;
        if (proc = GetProcAddress(fModule, name)) {
            return (Proc) proc;
        }
        if (proc = wglGetProcAddress(name)) {
            return (Proc) proc;
        }
        return nullptr;
    }

    HMODULE fModule;
    static ProcGetter* fInstance;
};

class DawnGLWindowContext_win : public DawnGLWindowContext {
public:
    DawnGLWindowContext_win(HWND, const DisplayParams&);
    ~DawnGLWindowContext_win() override;

    void onSwapBuffers() override;
    dawn::Device onInitializeContext() override;
    void onDestroyContext() override;
    void DawnGLWindowContext_win::discoverAdapters() override;

private:
    void createGLInterface();

private:
    DawnGLWindowContext_win(void*, const DisplayParams&);

    HWND              fHWND;
    HGLRC             fHGLRC;

    typedef DawnGLWindowContext INHERITED;
};

DawnGLWindowContext_win::DawnGLWindowContext_win(HWND hwnd, const DisplayParams& params)
        : INHERITED(params)
        , fHWND(hwnd)
        , fHGLRC(nullptr) {

    RECT rect;
    GetClientRect(hwnd, &rect);
    this->initializeContext(rect.right - rect.left, rect.bottom - rect.top);
}

dawn::Device DawnGLWindowContext_win::onInitializeContext() {
    HDC dc = GetDC(fHWND);

    fHGLRC = SkCreateWGLContext(dc, fDisplayParams.fMSAASampleCount, false /* deepColor */,
                                kGLPreferCompatibilityProfile_SkWGLContextRequest);
    if (NULL == fHGLRC) {
        return nullptr;
    }

    if (!wglMakeCurrent(dc, fHGLRC)) {
        return nullptr;
    }

    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // use DescribePixelFormat to get the stencil and color bit depth.
    int pixelFormat = GetPixelFormat(dc);
    PIXELFORMATDESCRIPTOR pfd;

    DescribePixelFormat(dc, pixelFormat, sizeof(pfd), &pfd);
    fStencilBits = pfd.cStencilBits;

    // Get sample count if the MSAA WGL extension is present
    SkWGLExtensions extensions;
    if (extensions.hasExtension(dc, "WGL_ARB_multisample")) {
        static const int kSampleCountAttr = SK_WGL_SAMPLES;
        extensions.getPixelFormatAttribiv(dc,
                                          pixelFormat,
                                          0,
                                          1,
                                          &kSampleCountAttr,
                                          &fSampleCount);
        fSampleCount = SkTMax(fSampleCount, 1);
    } else {
        fSampleCount = 1;
    }

    glViewport(0, 0, width(), height());

    return this->createDevice(dawn_native::BackendType::OpenGL);
}

ProcGetter* ProcGetter::fInstance;

DawnGLWindowContext_win::~DawnGLWindowContext_win() {
    this->destroyContext();
}

void DawnGLWindowContext_win::onDestroyContext() {
    if (!fHWND || !fHGLRC) {
        return;
    }
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(fHGLRC);
    fHWND = nullptr;
    fHGLRC = nullptr;
}

void DawnGLWindowContext_win::onSwapBuffers() {
    wglSwapLayerBuffers(GetDC(fHWND), WGL_SWAP_MAIN_PLANE);
}

void DawnGLWindowContext_win::discoverAdapters() {
    ProcGetter getter;
    dawn_native::opengl::AdapterDiscoveryOptions adapterOptions;
    adapterOptions.getProc = ProcGetter::getProcAddress;
    fInstance->DiscoverAdapters(&adapterOptions);
}

}  // anonymous namespace

namespace sk_app {

namespace window_context_factory {

WindowContext* NewDawnGLForWin(HWND hwnd, const DisplayParams& params) {
    WindowContext* ctx = new DawnGLWindowContext_win(hwnd, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory

}  // namespace sk_app

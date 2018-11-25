/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLTestContext_angle.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"

#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "../ports/SkOSLibrary.h"

#include <EGL/egl.h>

#define EGL_PLATFORM_ANGLE_ANGLE                0x3202
#define EGL_PLATFORM_ANGLE_TYPE_ANGLE           0x3203
#define EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE      0x3207
#define EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE     0x3208
#define EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE    0x320D

using sk_gpu_test::ANGLEBackend;
using sk_gpu_test::ANGLEContextVersion;

namespace {
struct Libs {
    void* fGLLib;
    void* fEGLLib;
};

std::function<void()> context_restorer() {
    auto display = eglGetCurrentDisplay();
    auto dsurface = eglGetCurrentSurface(EGL_DRAW);
    auto rsurface = eglGetCurrentSurface(EGL_READ);
    auto context = eglGetCurrentContext();
    return [display, dsurface, rsurface, context] {
        eglMakeCurrent(display, dsurface, rsurface, context);
    };
}

static GrGLFuncPtr angle_get_gl_proc(void* ctx, const char name[]) {
    const Libs* libs = reinterpret_cast<const Libs*>(ctx);
    GrGLFuncPtr proc = (GrGLFuncPtr) GetProcedureAddress(libs->fGLLib, name);
    if (proc) {
        return proc;
    }
    proc = (GrGLFuncPtr) GetProcedureAddress(libs->fEGLLib, name);
    if (proc) {
        return proc;
    }
    return eglGetProcAddress(name);
}

void* get_angle_egl_display(void* nativeDisplay, ANGLEBackend type) {
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
    eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    // We expect ANGLE to support this extension
    if (!eglGetPlatformDisplayEXT) {
        return EGL_NO_DISPLAY;
    }

    EGLint typeNum = 0;
    switch (type) {
        case ANGLEBackend::kD3D9:
            typeNum = EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE;
            break;
        case ANGLEBackend::kD3D11:
            typeNum = EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
            break;
        case ANGLEBackend::kOpenGL:
            typeNum = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
            break;
    }
    const EGLint attribs[] = { EGL_PLATFORM_ANGLE_TYPE_ANGLE, typeNum, EGL_NONE };
    return eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, nativeDisplay, attribs);
}

class ANGLEGLContext : public sk_gpu_test::GLTestContext {
public:
    ANGLEGLContext(ANGLEBackend, ANGLEContextVersion, ANGLEGLContext* shareContext, void* display);
    ~ANGLEGLContext() override;

    GrEGLImage texture2DToEGLImage(GrGLuint texID) const override;
    void destroyEGLImage(GrEGLImage) const override;
    GrGLuint eglImageToExternalTexture(GrEGLImage) const override;
    std::unique_ptr<sk_gpu_test::GLTestContext> makeNew() const override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    std::function<void()> onPlatformGetAutoContextRestore() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char* name) const override;

    void*                       fContext;
    void*                       fDisplay;
    void*                       fSurface;
    ANGLEBackend                fType;
    ANGLEContextVersion         fVersion;

#ifdef SK_BUILD_FOR_WIN
    HWND                        fWindow;
    HDC                         fDeviceContext;
    static ATOM                 gWC;
#endif
};

#ifdef SK_BUILD_FOR_WIN
ATOM ANGLEGLContext::gWC = 0;
#endif

ANGLEGLContext::ANGLEGLContext(ANGLEBackend type, ANGLEContextVersion version,
                               ANGLEGLContext* shareContext, void* display)
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(display)
    , fSurface(EGL_NO_SURFACE)
    , fType(type)
    , fVersion(version) {
#ifdef SK_BUILD_FOR_WIN
    fWindow = nullptr;
    fDeviceContext = nullptr;

    if (EGL_NO_DISPLAY == fDisplay) {
        HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);

        if (!gWC) {
            WNDCLASS wc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hbrBackground = nullptr;
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
            wc.hInstance = hInstance;
            wc.lpfnWndProc = (WNDPROC) DefWindowProc;
            wc.lpszClassName = TEXT("ANGLE-win");
            wc.lpszMenuName = nullptr;
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

            gWC = RegisterClass(&wc);
            if (!gWC) {
                SkDebugf("Could not register window class.\n");
                return;
            }
        }
        if (!(fWindow = CreateWindow(TEXT("ANGLE-win"),
                                        TEXT("The Invisible Man"),
                                        WS_OVERLAPPEDWINDOW,
                                        0, 0, 1, 1,
                                        nullptr, nullptr,
                                        hInstance, nullptr))) {
            SkDebugf("Could not create window.\n");
            return;
        }

        if (!(fDeviceContext = GetDC(fWindow))) {
            SkDebugf("Could not get device context.\n");
            this->destroyGLContext();
            return;
        }

        fDisplay = get_angle_egl_display(fDeviceContext, type);
    }
#else
    SkASSERT(EGL_NO_DISPLAY == fDisplay);
    fDisplay = get_angle_egl_display(EGL_DEFAULT_DISPLAY, type);
#endif
    if (EGL_NO_DISPLAY == fDisplay) {
        SkDebugf("Could not create EGL display!");
        return;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    if (!eglInitialize(fDisplay, &majorVersion, &minorVersion)) {
        SkDebugf("Could not initialize display!");
        this->destroyGLContext();
        return;
    }

    EGLint numConfigs;
    static const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLConfig surfaceConfig;
    if (!eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs)) {
        SkDebugf("Could not create choose config!");
        this->destroyGLContext();
        return;
    }

    int versionNum = ANGLEContextVersion::kES2 == version ? 2 : 3;
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, versionNum,
        EGL_NONE
    };
    EGLContext eglShareContext = shareContext ? shareContext->fContext : nullptr;
    fContext = eglCreateContext(fDisplay, surfaceConfig, eglShareContext, contextAttribs);
    if (EGL_NO_CONTEXT == fContext) {
        SkDebugf("Could not create context!");
        this->destroyGLContext();
        return;
    }

    static const EGLint surfaceAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    fSurface = eglCreatePbufferSurface(fDisplay, surfaceConfig, surfaceAttribs);

    SkScopeExit restorer(context_restorer());
    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.");
        this->destroyGLContext();
        return;
    }

    sk_sp<const GrGLInterface> gl = sk_gpu_test::CreateANGLEGLInterface();
    if (nullptr == gl.get()) {
        SkDebugf("Could not create ANGLE GL interface!\n");
        this->destroyGLContext();
        return;
    }
    if (!gl->validate()) {
        SkDebugf("Could not validate ANGLE GL interface!\n");
        this->destroyGLContext();
        return;
    }

#ifdef SK_DEBUG
    // Verify that the interface we requested was actually returned to us
    const GrGLubyte* rendererUByte;
    GR_GL_CALL_RET(gl.get(), rendererUByte, GetString(GR_GL_RENDERER));
    const char* renderer = reinterpret_cast<const char*>(rendererUByte);
    switch (type) {
    case ANGLEBackend::kD3D9:
        SkASSERT(strstr(renderer, "Direct3D9"));
        break;
    case ANGLEBackend::kD3D11:
        SkASSERT(strstr(renderer, "Direct3D11"));
        break;
    case ANGLEBackend::kOpenGL:
        SkASSERT(strstr(renderer, "OpenGL"));
        break;
    }
#endif

    this->init(std::move(gl));
}

ANGLEGLContext::~ANGLEGLContext() {
    this->teardown();
    this->destroyGLContext();
}

GrEGLImage ANGLEGLContext::texture2DToEGLImage(GrGLuint texID) const {
    if (!this->gl()->hasExtension("EGL_KHR_gl_texture_2D_image")) {
        return GR_EGL_NO_IMAGE;
    }
    GrEGLImage img;
    GrEGLint attribs[] = { GR_EGL_GL_TEXTURE_LEVEL, 0,
                           GR_EGL_IMAGE_PRESERVED, GR_EGL_TRUE,
                           GR_EGL_NONE };
    // 64 bit cast is to shut Visual C++ up about casting 32 bit value to a pointer.
    GrEGLClientBuffer clientBuffer = reinterpret_cast<GrEGLClientBuffer>((uint64_t)texID);
    GR_GL_CALL_RET(this->gl(), img,
                   EGLCreateImage(fDisplay, fContext, GR_EGL_GL_TEXTURE_2D, clientBuffer,
                                  attribs));
    return img;
}

void ANGLEGLContext::destroyEGLImage(GrEGLImage image) const {
    GR_GL_CALL(this->gl(), EGLDestroyImage(fDisplay, image));
}

GrGLuint ANGLEGLContext::eglImageToExternalTexture(GrEGLImage image) const {
    GrGLClearErr(this->gl());
    if (!this->gl()->hasExtension("GL_OES_EGL_image_external")) {
        return 0;
    }
    typedef GrGLvoid (EGLAPIENTRY *EGLImageTargetTexture2DProc)(GrGLenum, GrGLeglImage);
    EGLImageTargetTexture2DProc glEGLImageTargetTexture2D =
        (EGLImageTargetTexture2DProc)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!glEGLImageTargetTexture2D) {
        return 0;
    }
    GrGLuint texID;
    GR_GL_CALL(this->gl(), GenTextures(1, &texID));
    if (!texID) {
        return 0;
    }
    GR_GL_CALL(this->gl(), BindTexture(GR_GL_TEXTURE_EXTERNAL, texID));
    if (GR_GL_GET_ERROR(this->gl()) != GR_GL_NO_ERROR) {
        GR_GL_CALL(this->gl(), DeleteTextures(1, &texID));
        return 0;
    }
    glEGLImageTargetTexture2D(GR_GL_TEXTURE_EXTERNAL, image);
    if (GR_GL_GET_ERROR(this->gl()) != GR_GL_NO_ERROR) {
        GR_GL_CALL(this->gl(), DeleteTextures(1, &texID));
        return 0;
    }
    return texID;
}

std::unique_ptr<sk_gpu_test::GLTestContext> ANGLEGLContext::makeNew() const {
    // For EGLImage sharing between contexts to work in ANGLE the two contexts
    // need to share the same display
    std::unique_ptr<sk_gpu_test::GLTestContext> ctx =
        sk_gpu_test::MakeANGLETestContext(fType, fVersion, nullptr, fDisplay);
    if (ctx) {
        ctx->makeCurrent();
    }
    return ctx;
}

void ANGLEGLContext::destroyGLContext() {
    if (EGL_NO_DISPLAY != fDisplay) {
        if (eglGetCurrentContext() == fContext) {
            // This will ensure that the context is immediately deleted.
            eglMakeCurrent(fDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        }

        if (EGL_NO_CONTEXT != fContext) {
            eglDestroyContext(fDisplay, fContext);
            fContext = EGL_NO_CONTEXT;
        }

        if (EGL_NO_SURFACE != fSurface) {
            eglDestroySurface(fDisplay, fSurface);
            fSurface = EGL_NO_SURFACE;
        }

        eglTerminate(fDisplay);
        fDisplay = EGL_NO_DISPLAY;
    }

#ifdef SK_BUILD_FOR_WIN
    if (fWindow) {
        if (fDeviceContext) {
            ReleaseDC(fWindow, fDeviceContext);
            fDeviceContext = 0;
        }

        DestroyWindow(fWindow);
        fWindow = 0;
    }
#endif
}

void ANGLEGLContext::onPlatformMakeCurrent() const {
    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context 0x%x.\n", eglGetError());
    }
}

std::function<void()> ANGLEGLContext::onPlatformGetAutoContextRestore() const {
    if (eglGetCurrentContext() == fContext) {
        return nullptr;
    }
    return context_restorer();
}

void ANGLEGLContext::onPlatformSwapBuffers() const {
    if (!eglSwapBuffers(fDisplay, fSurface)) {
        SkDebugf("Could not complete eglSwapBuffers.\n");
    }
}

GrGLFuncPtr ANGLEGLContext::onPlatformGetProcAddress(const char* name) const {
    return eglGetProcAddress(name);
}
}  // anonymous namespace

namespace sk_gpu_test {
sk_sp<const GrGLInterface> CreateANGLEGLInterface() {
    static Libs gLibs = { nullptr, nullptr };

    if (nullptr == gLibs.fGLLib) {
        // We load the ANGLE library and never let it go
#if defined _WIN32
        gLibs.fGLLib = DynamicLoadLibrary("libGLESv2.dll");
        gLibs.fEGLLib = DynamicLoadLibrary("libEGL.dll");
#elif defined SK_BUILD_FOR_MAC
        gLibs.fGLLib = DynamicLoadLibrary("libGLESv2.dylib");
        gLibs.fEGLLib = DynamicLoadLibrary("libEGL.dylib");
#else
        gLibs.fGLLib = DynamicLoadLibrary("libGLESv2.so");
        gLibs.fEGLLib = DynamicLoadLibrary("libEGL.so");
#endif
    }

    if (nullptr == gLibs.fGLLib || nullptr == gLibs.fEGLLib) {
        // We can't setup the interface correctly w/o the so
        return nullptr;
    }

    return GrGLMakeAssembledGLESInterface(&gLibs, angle_get_gl_proc);
}

std::unique_ptr<GLTestContext> MakeANGLETestContext(ANGLEBackend type, ANGLEContextVersion version,
                                                    GLTestContext* shareContext, void* display){
    ANGLEGLContext* angleShareContext = reinterpret_cast<ANGLEGLContext*>(shareContext);
    std::unique_ptr<GLTestContext> ctx(new ANGLEGLContext(type, version,
                                                          angleShareContext, display));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}
}  // namespace sk_gpu_test

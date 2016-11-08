
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLTestContext_angle.h"

#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"

#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "../ports/SkOSLibrary.h"

/**
 * EGL.h stuff. We don't want to worry about whether we're pulling in ANGLE's functions or some
 * other system EGL implementation. So we just define all the EGL types we need and dynamically
 * load all the functions we need.
 */

#if defined(SK_BUILD_FOR_WIN)
    #include <Windows.h>
    using EGLNativeDisplayType  = HDC;
    using EGLNativePixmapType   = HBITMAP;
    using EGLNativeWindowType   = HWND;
#elif defined(SK_BUILD_FOR_UNIX)
    #include <X11/Xlib.h>
    using EGLNativeDisplayType  = Display*;
    using EGLNativePixmapType   = Pixmap;
    using EGLNativeWindowType   = Window;
#else
    using EGLNativeDisplayType  = void*;
    using EGLNativePixmapType   = void*;
    using EGLNativeWindowType   = void*;
#endif

using EGLint = int32_t;
using EGLFn = void(void);
using EGLDisplay = void*;
using EGLSurface = void*;
using EGLContext = void*;
using EGLConfig = void*;
using EGLenum = unsigned int;
using EGLBoolean = unsigned int;

using EGLGetProcAddressFn = EGLFn*(const char *procname);
using EGLInitializeFn = EGLBoolean(EGLDisplay display, EGLint * major, EGLint * minor);
using EGLChooseConfigFn = EGLBoolean(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
using EGLCreateContextFn = EGLContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
using EGLCreatePbufferSurfaceFn = EGLSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
using EGLMakeCurrentFn = EGLBoolean(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
using EGLDestroyContextFn = EGLBoolean(EGLDisplay dpy, EGLContext ctx);
using EGLDestroySurfaceFn = EGLBoolean(EGLDisplay dpy, EGLSurface surface);
using EGLSwapBuffersFn = EGLBoolean(EGLDisplay dpy, EGLSurface surface);

#define EGL_PLATFORM_ANGLE_ANGLE                0x3202
#define EGL_PLATFORM_ANGLE_TYPE_ANGLE           0x3203
#define EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE      0x3207
#define EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE     0x3208
#define EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE    0x320D
#define EGL_NO_DISPLAY                          ((EGLDisplay)0)
#define EGL_NO_CONTEXT                          ((EGLContext)0)
#define EGL_NO_SURFACE                          ((EGLSurface)0)
#define EGL_DEFAULT_DISPLAY                     ((EGLNativeDisplayType)0)
#define EGL_NONE                                0x3038
#define EGL_SURFACE_TYPE                        0x3033
#define EGL_PBUFFER_BIT                         0x0001
#define EGL_RENDERABLE_TYPE                     0x3040
#define EGL_OPENGL_ES2_BIT                      0x0004
#define EGL_RED_SIZE                            0x3024
#define EGL_GREEN_SIZE                          0x3023
#define EGL_BLUE_SIZE                           0x3022
#define EGL_ALPHA_SIZE                          0x3021
#define EGL_CONTEXT_CLIENT_VERSION              0x3098
#define EGL_WIDTH                               0x3057
#define EGL_HEIGHT                              0x3056

using sk_gpu_test::ANGLEBackend;
using sk_gpu_test::ANGLEContextVersion;

namespace {

class ANGLE {
public:
    bool load() {
        // We load the ANGLE library and never let it go
#if defined(SK_BUILD_FOR_WIN)
        fGLLib = DynamicLoadLibrary("libGLESv2.dll");
        fEGLLib = DynamicLoadLibrary("libEGL.dll");
#elif defined(SK_BUILD_FOR_MAC)
        fGLLib = DynamicLoadLibrary("libGLESv2.dylib");
        fEGLLib = DynamicLoadLibrary("libEGL.dylib");
#else
        fGLLib = DynamicLoadLibrary("libGLESv2.so");
        fEGLLib = DynamicLoadLibrary("libEGL.so");
#endif
        if (!fGLLib || !fEGLLib) {
            return false;
        }
#define GET_PROC(NAME) f ## NAME = (EGL ## NAME ##Fn*) GetProcedureAddress(fEGLLib, "egl" #NAME); \
                       if (!f ## NAME) { return false; }
        GET_PROC(GetProcAddress);
        GET_PROC(Initialize);
        GET_PROC(ChooseConfig);
        GET_PROC(CreateContext)
        GET_PROC(CreatePbufferSurface);
        GET_PROC(MakeCurrent);
        GET_PROC(DestroyContext);
        GET_PROC(DestroySurface);
        GET_PROC(SwapBuffers);
#undef GET_PROC
        return true;
    }

    EGLBoolean initialize(EGLDisplay display, EGLint * major, EGLint * minor) const {
        return fInitialize(display, major, minor);
    }

    EGLFn* getProcAddress(const char* procname) const {
        void* lib = (0 == strncmp("gl", procname, 2)) ? fGLLib : fEGLLib;
        EGLFn* proc = (EGLFn*)GetProcedureAddress(lib, procname);
        if (!proc) {
            proc = fGetProcAddress(procname);
        }
        return proc;
    }

    EGLBoolean chooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs,
                            EGLint config_size, EGLint *num_config) const {
        return fChooseConfig(dpy, attrib_list, configs, config_size, num_config);
    }

    EGLContext createContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context,
                             const EGLint *attrib_list) const {
        return fCreateContext(dpy, config, share_context, attrib_list);
    }

    EGLSurface createPbufferSurface(EGLDisplay dpy, EGLConfig config,
                                    const EGLint *attrib_list) const {
        return fCreatePbufferSurface(dpy, config, attrib_list);
    }

    EGLBoolean makeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) const {
        return fMakeCurrent(dpy, draw, read, ctx);
    }

    EGLBoolean destroyContext(EGLDisplay dpy, EGLContext ctx) const {
        return fDestroyContext(dpy, ctx);
    }

    EGLBoolean destroySurface(EGLDisplay dpy, EGLSurface surface) const {
        return fDestroySurface(dpy, surface);
    }

    EGLBoolean swapBuffers(EGLDisplay display, EGLSurface surface) const {
        return fSwapBuffers(display, surface);
    }

private:
    EGLGetProcAddressFn*        fGetProcAddress;
    EGLInitializeFn*            fInitialize;
    EGLChooseConfigFn*          fChooseConfig;
    EGLCreateContextFn*         fCreateContext;
    EGLCreatePbufferSurfaceFn*  fCreatePbufferSurface;
    EGLMakeCurrentFn*           fMakeCurrent;
    EGLDestroyContextFn*        fDestroyContext;
    EGLDestroySurfaceFn*        fDestroySurface;
    EGLSwapBuffersFn*           fSwapBuffers;

    void*                       fGLLib;
    void*                       fEGLLib;
};

static GrGLFuncPtr angle_get_gl_proc(void* ctx, const char name[]) {
    const ANGLE* angle = reinterpret_cast<const ANGLE*>(ctx);
    return angle->getProcAddress(name);
}

void* get_angle_egl_display(void* nativeDisplay, ANGLEBackend type, const ANGLE& angle) {
    using EGLGetPlatformDisplayEXTFn = EGLDisplay (EGLenum platform, void *native_display,
                                                   const EGLint *attrib_list);
    EGLGetPlatformDisplayEXTFn* eglGetPlatformDisplayEXT =
            (EGLGetPlatformDisplayEXTFn*)angle.getProcAddress("eglGetPlatformDisplayEXT");

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
    ANGLEGLContext(ANGLEBackend, ANGLEContextVersion);
    ~ANGLEGLContext() override;

    GrEGLImage texture2DToEGLImage(GrGLuint texID) const override;
    void destroyEGLImage(GrEGLImage) const override;
    GrGLuint eglImageToExternalTexture(GrEGLImage) const override;
    std::unique_ptr<sk_gpu_test::GLTestContext> makeNew() const override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char* name) const override;

    ANGLE                       fANGLE;
    void*                       fContext;
    void*                       fDisplay;
    void*                       fSurface;
    ANGLEBackend                fType;
    ANGLEContextVersion         fVersion;
};

ANGLEGLContext::ANGLEGLContext(ANGLEBackend type, ANGLEContextVersion version)
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE)
    , fType(type)
    , fVersion(version) {

    if (!fANGLE.load()) {
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

    fDisplay = get_angle_egl_display(EGL_DEFAULT_DISPLAY, type, fANGLE);
    if (EGL_NO_DISPLAY == fDisplay) {
        SkDebugf("Could not create EGL display!");
        return;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    fANGLE.initialize(fDisplay, &majorVersion, &minorVersion);

    EGLConfig surfaceConfig;
    fANGLE.chooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs);

    int versionNum = ANGLEContextVersion::kES2 == version ? 2 : 3;
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, versionNum,
        EGL_NONE
    };
    fContext = fANGLE.createContext(fDisplay, surfaceConfig, nullptr, contextAttribs);


    static const EGLint surfaceAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    fSurface = fANGLE.createPbufferSurface(fDisplay, surfaceConfig, surfaceAttribs);

    fANGLE.makeCurrent(fDisplay, fSurface, fSurface, fContext);

    sk_sp<const GrGLInterface> gl(GrGLAssembleGLESInterface((void*)&fANGLE, angle_get_gl_proc));
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

    this->init(gl.release());
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
    using EGLImageTargetTexture2DFn = GrGLvoid(GrGLenum, GrGLeglImage);
    EGLImageTargetTexture2DFn* glEGLImageTargetTexture2D =
        (EGLImageTargetTexture2DFn*)fANGLE.getProcAddress("glEGLImageTargetTexture2DOES");
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
    std::unique_ptr<sk_gpu_test::GLTestContext> ctx =
        sk_gpu_test::MakeANGLETestContext(fType, fVersion);
    if (ctx) {
        ctx->makeCurrent();
    }
    return ctx;
}

void ANGLEGLContext::destroyGLContext() {
    if (fDisplay) {
        fANGLE.makeCurrent(fDisplay, 0, 0, 0);

        if (fContext) {
            fANGLE.destroyContext(fDisplay, fContext);
            fContext = EGL_NO_CONTEXT;
        }

        if (fSurface) {
            fANGLE.destroySurface(fDisplay, fSurface);
            fSurface = EGL_NO_SURFACE;
        }

        //TODO should we close the display?
        fDisplay = EGL_NO_DISPLAY;
    }
}

void ANGLEGLContext::onPlatformMakeCurrent() const {
    if (!fANGLE.makeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

void ANGLEGLContext::onPlatformSwapBuffers() const {
    if (!fANGLE.swapBuffers(fDisplay, fSurface)) {
        SkDebugf("Could not complete eglSwapBuffers.\n");
    }
}

GrGLFuncPtr ANGLEGLContext::onPlatformGetProcAddress(const char* name) const {
    return fANGLE.getProcAddress(name);
}

}  // anonymous namespace

namespace sk_gpu_test {
const GrGLInterface* CreateANGLEGLInterface() {
    ANGLE angle;
    if (!angle.load()) {
        return nullptr;
    }
    return GrGLAssembleGLESInterface((void*)&angle, angle_get_gl_proc);
}

std::unique_ptr<GLTestContext> MakeANGLETestContext(ANGLEBackend type, ANGLEContextVersion version){
    std::unique_ptr<GLTestContext> ctx(new ANGLEGLContext(type, version));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}
}  // namespace sk_gpu_test

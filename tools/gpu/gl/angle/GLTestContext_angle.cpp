
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
    ANGLEGLContext(ANGLEBackend, ANGLEContextVersion, ANGLEGLContext* shareContext);
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

    void*                       fContext;
    void*                       fDisplay;
    void*                       fSurface;
    ANGLEBackend                fType;
    ANGLEContextVersion         fVersion;
};

ANGLEGLContext::ANGLEGLContext(ANGLEBackend type, ANGLEContextVersion version,
                               ANGLEGLContext* shareContext)
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE)
    , fType(type)
    , fVersion(version) {

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

    fDisplay = get_angle_egl_display(EGL_DEFAULT_DISPLAY, type);
    if (EGL_NO_DISPLAY == fDisplay) {
        SkDebugf("Could not create EGL display!");
        return;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    eglInitialize(fDisplay, &majorVersion, &minorVersion);

    EGLConfig surfaceConfig;
    eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs);

    int versionNum = ANGLEContextVersion::kES2 == version ? 2 : 3;
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, versionNum,
        EGL_NONE
    };
    EGLContext eglShareContext = shareContext ? shareContext->fContext : nullptr;
    fContext = eglCreateContext(fDisplay, surfaceConfig, eglShareContext, contextAttribs);


    static const EGLint surfaceAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    fSurface = eglCreatePbufferSurface(fDisplay, surfaceConfig, surfaceAttribs);

    eglMakeCurrent(fDisplay, fSurface, fSurface, fContext);

    sk_sp<const GrGLInterface> gl(sk_gpu_test::CreateANGLEGLInterface());
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
    std::unique_ptr<sk_gpu_test::GLTestContext> ctx =
        sk_gpu_test::MakeANGLETestContext(fType, fVersion);
    if (ctx) {
        ctx->makeCurrent();
    }
    return ctx;
}

void ANGLEGLContext::destroyGLContext() {
    if (fDisplay) {
        eglMakeCurrent(fDisplay, 0, 0, 0);

        if (fContext) {
            eglDestroyContext(fDisplay, fContext);
            fContext = EGL_NO_CONTEXT;
        }

        if (fSurface) {
            eglDestroySurface(fDisplay, fSurface);
            fSurface = EGL_NO_SURFACE;
        }

        //TODO should we close the display?
        fDisplay = EGL_NO_DISPLAY;
    }
}

void ANGLEGLContext::onPlatformMakeCurrent() const {
    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
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
const GrGLInterface* CreateANGLEGLInterface() {
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

    return GrGLAssembleGLESInterface(&gLibs, angle_get_gl_proc);
}

std::unique_ptr<GLTestContext> MakeANGLETestContext(ANGLEBackend type, ANGLEContextVersion version,
                                                    GLTestContext* shareContext){
    ANGLEGLContext* angleShareContext = reinterpret_cast<ANGLEGLContext*>(shareContext);
    std::unique_ptr<GLTestContext> ctx(new ANGLEGLContext(type, version, angleShareContext));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}
}  // namespace sk_gpu_test

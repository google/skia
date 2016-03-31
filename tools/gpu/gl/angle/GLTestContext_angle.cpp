
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

void* get_angle_egl_display(void* nativeDisplay, bool useGLBackend) {
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
    eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    // We expect ANGLE to support this extension
    if (!eglGetPlatformDisplayEXT) {
        return EGL_NO_DISPLAY;
    }

    EGLDisplay display = EGL_NO_DISPLAY;
    if (useGLBackend) {
        EGLint attribs[3] = {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE,
            EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE,
            EGL_NONE
        };
        display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, nativeDisplay, attribs);
    } else {
        // Try for an ANGLE D3D11 context, fall back to D3D9.
        EGLint attribs[3][3] = {
            {
                EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
                EGL_NONE
            },
            {
                EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
                EGL_NONE
            },
        };
        for (int i = 0; i < 3 && display == EGL_NO_DISPLAY; ++i) {
            display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,nativeDisplay, attribs[i]);
        }
    }
    return display;
}

class ANGLEGLContext : public sk_gpu_test::GLTestContext {
public:
    ANGLEGLContext(bool preferGLBackend);
    ~ANGLEGLContext() override;

    GrEGLImage texture2DToEGLImage(GrGLuint texID) const override;
    void destroyEGLImage(GrEGLImage) const override;
    GrGLuint eglImageToExternalTexture(GrEGLImage) const override;
    sk_gpu_test::GLTestContext* createNew() const override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char* name) const override;

    void* fContext;
    void* fDisplay;
    void* fSurface;
    bool  fIsGLBackend;
};

ANGLEGLContext::ANGLEGLContext(bool useGLBackend)
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE) {

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

    fIsGLBackend = useGLBackend;
    fDisplay = get_angle_egl_display(EGL_DEFAULT_DISPLAY, useGLBackend);
    if (EGL_NO_DISPLAY == fDisplay) {
        SkDebugf("Could not create EGL display!");
        return;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    eglInitialize(fDisplay, &majorVersion, &minorVersion);

    EGLConfig surfaceConfig;
    eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs);

    static const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    fContext = eglCreateContext(fDisplay, surfaceConfig, nullptr, contextAttribs);


    static const EGLint surfaceAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    fSurface = eglCreatePbufferSurface(fDisplay, surfaceConfig, surfaceAttribs);

    eglMakeCurrent(fDisplay, fSurface, fSurface, fContext);

    SkAutoTUnref<const GrGLInterface> gl(sk_gpu_test::CreateANGLEGLInterface());
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
    typedef GrGLvoid (*EGLImageTargetTexture2DProc)(GrGLenum, GrGLeglImage);
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

sk_gpu_test::GLTestContext* ANGLEGLContext::createNew() const {
#ifdef SK_BUILD_FOR_WIN
    sk_gpu_test::GLTestContext* ctx = fIsGLBackend
        ? sk_gpu_test::CreateANGLEOpenGLGLTestContext()
        : sk_gpu_test::CreateANGLEDirect3DGLTestContext();
#else
    sk_gpu_test::GLTestContext* ctx = sk_gpu_test::CreateANGLEOpenGLGLTestContext();
#endif
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

#ifdef SK_BUILD_FOR_WIN
GLTestContext* CreateANGLEDirect3DGLTestContext() {
        ANGLEGLContext* ctx = new ANGLEGLContext(false);
        if (!ctx->isValid()) {
            delete ctx;
            return NULL;
        }
        return ctx;
    }
#endif

GLTestContext* CreateANGLEOpenGLGLTestContext() {
    ANGLEGLContext* ctx = new ANGLEGLContext(true);
    if (!ctx->isValid()) {
        delete ctx;
        return NULL;
    }
    return ctx;
}
}  // namespace sk_gpu_test

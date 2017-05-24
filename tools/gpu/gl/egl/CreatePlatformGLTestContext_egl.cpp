
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/GLTestContext.h"

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"

namespace {

// TODO: Share this class with ANGLE if/when it gets support for EGL_KHR_fence_sync.
class EGLFenceSync : public sk_gpu_test::FenceSync {
public:
    static std::unique_ptr<EGLFenceSync> MakeIfSupported(EGLDisplay);

    sk_gpu_test::PlatformFence SK_WARN_UNUSED_RESULT insertFence() const override;
    bool waitFence(sk_gpu_test::PlatformFence fence) const override;
    void deleteFence(sk_gpu_test::PlatformFence fence) const override;

private:
    EGLFenceSync(EGLDisplay display);

    PFNEGLCREATESYNCKHRPROC       fEGLCreateSyncKHR;
    PFNEGLCLIENTWAITSYNCKHRPROC   fEGLClientWaitSyncKHR;
    PFNEGLDESTROYSYNCKHRPROC      fEGLDestroySyncKHR;

    EGLDisplay                    fDisplay;

    typedef sk_gpu_test::FenceSync INHERITED;
};

class EGLGLTestContext : public sk_gpu_test::GLTestContext {
public:
    EGLGLTestContext(GrGLStandard forcedGpuAPI, EGLGLTestContext* shareContext);
    ~EGLGLTestContext() override;

    GrEGLImage texture2DToEGLImage(GrGLuint texID) const override;
    void destroyEGLImage(GrEGLImage) const override;
    GrGLuint eglImageToExternalTexture(GrEGLImage) const override;
    std::unique_ptr<sk_gpu_test::GLTestContext> makeNew() const override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    EGLContext fContext;
    EGLDisplay fDisplay;
    EGLSurface fSurface;
};

EGLGLTestContext::EGLGLTestContext(GrGLStandard forcedGpuAPI, EGLGLTestContext* shareContext)
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE) {

    EGLContext eglShareContext = shareContext ? shareContext->fContext : nullptr;

    static const EGLint kEGLContextAttribsForOpenGL[] = {
        EGL_NONE
    };

    static const EGLint kEGLContextAttribsForOpenGLES[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    static const struct {
        const EGLint* fContextAttribs;
        EGLenum fAPI;
        EGLint  fRenderableTypeBit;
        GrGLStandard fStandard;
    } kAPIs[] = {
        {   // OpenGL
            kEGLContextAttribsForOpenGL,
            EGL_OPENGL_API,
            EGL_OPENGL_BIT,
            kGL_GrGLStandard
        },
        {   // OpenGL ES. This seems to work for both ES2 and 3 (when available).
            kEGLContextAttribsForOpenGLES,
            EGL_OPENGL_ES_API,
            EGL_OPENGL_ES2_BIT,
            kGLES_GrGLStandard
        },
    };

    size_t apiLimit = SK_ARRAY_COUNT(kAPIs);
    size_t api = 0;
    if (forcedGpuAPI == kGL_GrGLStandard) {
        apiLimit = 1;
    } else if (forcedGpuAPI == kGLES_GrGLStandard) {
        api = 1;
    }
    SkASSERT(forcedGpuAPI == kNone_GrGLStandard || kAPIs[api].fStandard == forcedGpuAPI);

    sk_sp<const GrGLInterface> gl;

    for (; nullptr == gl.get() && api < apiLimit; ++api) {
        fDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        EGLint majorVersion;
        EGLint minorVersion;
        eglInitialize(fDisplay, &majorVersion, &minorVersion);

#if 0
        SkDebugf("VENDOR: %s\n", eglQueryString(fDisplay, EGL_VENDOR));
        SkDebugf("APIS: %s\n", eglQueryString(fDisplay, EGL_CLIENT_APIS));
        SkDebugf("VERSION: %s\n", eglQueryString(fDisplay, EGL_VERSION));
        SkDebugf("EXTENSIONS %s\n", eglQueryString(fDisplay, EGL_EXTENSIONS));
#endif

        if (!eglBindAPI(kAPIs[api].fAPI)) {
            continue;
        }

        EGLint numConfigs = 0;
        const EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, kAPIs[api].fRenderableTypeBit,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };

        EGLConfig surfaceConfig;
        if (!eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs)) {
            SkDebugf("eglChooseConfig failed. EGL Error: 0x%08x\n", eglGetError());
            continue;
        }

        if (0 == numConfigs) {
            SkDebugf("No suitable EGL config found.\n");
            continue;
        }

        fContext = eglCreateContext(fDisplay, surfaceConfig, eglShareContext,
                                    kAPIs[api].fContextAttribs);
        if (EGL_NO_CONTEXT == fContext) {
            SkDebugf("eglCreateContext failed.  EGL Error: 0x%08x\n", eglGetError());
            continue;
        }

        static const EGLint kSurfaceAttribs[] = {
            EGL_WIDTH, 1,
            EGL_HEIGHT, 1,
            EGL_NONE
        };

        fSurface = eglCreatePbufferSurface(fDisplay, surfaceConfig, kSurfaceAttribs);
        if (EGL_NO_SURFACE == fSurface) {
            SkDebugf("eglCreatePbufferSurface failed. EGL Error: 0x%08x\n", eglGetError());
            this->destroyGLContext();
            continue;
        }

        if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
            SkDebugf("eglMakeCurrent failed.  EGL Error: 0x%08x\n", eglGetError());
            this->destroyGLContext();
            continue;
        }

        gl.reset(GrGLCreateNativeInterface());
        if (nullptr == gl.get()) {
            SkDebugf("Failed to create gl interface.\n");
            this->destroyGLContext();
            continue;
        }

        if (!gl->validate()) {
            SkDebugf("Failed to validate gl interface.\n");
            this->destroyGLContext();
            continue;
        }

        this->init(gl.release(), EGLFenceSync::MakeIfSupported(fDisplay));
        break;
    }
}

EGLGLTestContext::~EGLGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void EGLGLTestContext::destroyGLContext() {
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

GrEGLImage EGLGLTestContext::texture2DToEGLImage(GrGLuint texID) const {
    if (!this->gl()->hasExtension("EGL_KHR_gl_texture_2D_image")) {
        return GR_EGL_NO_IMAGE;
    }
    GrEGLImage img;
    GrEGLint attribs[] = { GR_EGL_GL_TEXTURE_LEVEL, 0, GR_EGL_NONE };
    GrEGLClientBuffer clientBuffer = reinterpret_cast<GrEGLClientBuffer>(texID);
    GR_GL_CALL_RET(this->gl(), img,
                   EGLCreateImage(fDisplay, fContext, GR_EGL_GL_TEXTURE_2D, clientBuffer, attribs));
    return img;
}

void EGLGLTestContext::destroyEGLImage(GrEGLImage image) const {
    GR_GL_CALL(this->gl(), EGLDestroyImage(fDisplay, image));
}

GrGLuint EGLGLTestContext::eglImageToExternalTexture(GrEGLImage image) const {
    GrGLClearErr(this->gl());
    if (!this->gl()->hasExtension("GL_OES_EGL_image_external")) {
        return 0;
    }
#ifndef EGL_NO_IMAGE_EXTERNAL
    typedef GrGLvoid (*EGLImageTargetTexture2DProc)(GrGLenum, GrGLeglImage);

    EGLImageTargetTexture2DProc glEGLImageTargetTexture2D =
        (EGLImageTargetTexture2DProc) eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!glEGLImageTargetTexture2D) {
        return 0;
    }
    GrGLuint texID;
    glGenTextures(1, &texID);
    if (!texID) {
        return 0;
    }
    glBindTexture(GR_GL_TEXTURE_EXTERNAL, texID);
    if (glGetError() != GR_GL_NO_ERROR) {
        glDeleteTextures(1, &texID);
        return 0;
    }
    glEGLImageTargetTexture2D(GR_GL_TEXTURE_EXTERNAL, image);
    if (glGetError() != GR_GL_NO_ERROR) {
        glDeleteTextures(1, &texID);
        return 0;
    }
    return texID;
#else
    return 0;
#endif //EGL_NO_IMAGE_EXTERNAL
}

std::unique_ptr<sk_gpu_test::GLTestContext> EGLGLTestContext::makeNew() const {
    std::unique_ptr<sk_gpu_test::GLTestContext> ctx(new EGLGLTestContext(this->gl()->fStandard,
                                                                         nullptr));
    if (ctx) {
        ctx->makeCurrent();
    }
    return ctx;
}

void EGLGLTestContext::onPlatformMakeCurrent() const {
    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

void EGLGLTestContext::onPlatformSwapBuffers() const {
    if (!eglSwapBuffers(fDisplay, fSurface)) {
        SkDebugf("Could not complete eglSwapBuffers.\n");
    }
}

GrGLFuncPtr EGLGLTestContext::onPlatformGetProcAddress(const char* procName) const {
    return eglGetProcAddress(procName);
}

static bool supports_egl_extension(EGLDisplay display, const char* extension) {
    size_t extensionLength = strlen(extension);
    const char* extensionsStr = eglQueryString(display, EGL_EXTENSIONS);
    while (const char* match = strstr(extensionsStr, extension)) {
        // Ensure the string we found is its own extension, not a substring of a larger extension
        // (e.g. GL_ARB_occlusion_query / GL_ARB_occlusion_query2).
        if ((match == extensionsStr || match[-1] == ' ') &&
            (match[extensionLength] == ' ' || match[extensionLength] == '\0')) {
            return true;
        }
        extensionsStr = match + extensionLength;
    }
    return false;
}

std::unique_ptr<EGLFenceSync> EGLFenceSync::MakeIfSupported(EGLDisplay display) {
    if (!display || !supports_egl_extension(display, "EGL_KHR_fence_sync")) {
        return nullptr;
    }
    return std::unique_ptr<EGLFenceSync>(new EGLFenceSync(display));
}

EGLFenceSync::EGLFenceSync(EGLDisplay display)
    : fDisplay(display) {
    fEGLCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) eglGetProcAddress("eglCreateSyncKHR");
    fEGLClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC) eglGetProcAddress("eglClientWaitSyncKHR");
    fEGLDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) eglGetProcAddress("eglDestroySyncKHR");
    SkASSERT(fEGLCreateSyncKHR && fEGLClientWaitSyncKHR && fEGLDestroySyncKHR);
}

sk_gpu_test::PlatformFence EGLFenceSync::insertFence() const {
    EGLSyncKHR eglsync = fEGLCreateSyncKHR(fDisplay, EGL_SYNC_FENCE_KHR, nullptr);
    return reinterpret_cast<sk_gpu_test::PlatformFence>(eglsync);
}

bool EGLFenceSync::waitFence(sk_gpu_test::PlatformFence platformFence) const {
    EGLSyncKHR eglsync = reinterpret_cast<EGLSyncKHR>(platformFence);
    return EGL_CONDITION_SATISFIED_KHR ==
            fEGLClientWaitSyncKHR(fDisplay,
                                  eglsync,
                                  EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                  EGL_FOREVER_KHR);
}

void EGLFenceSync::deleteFence(sk_gpu_test::PlatformFence platformFence) const {
    EGLSyncKHR eglsync = reinterpret_cast<EGLSyncKHR>(platformFence);
    fEGLDestroySyncKHR(fDisplay, eglsync);
}

GR_STATIC_ASSERT(sizeof(EGLSyncKHR) <= sizeof(sk_gpu_test::PlatformFence));

}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext *CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext *shareContext) {
    EGLGLTestContext* eglShareContext = reinterpret_cast<EGLGLTestContext*>(shareContext);
    EGLGLTestContext *ctx = new EGLGLTestContext(forcedGpuAPI, eglShareContext);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}
}  // namespace sk_gpu_test

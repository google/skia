
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/gpu/gl/GLTestContext.h"

#define GL_GLEXT_PROTOTYPES

#include <GLES2/gl2.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLUtil.h"

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

std::function<void()> context_restorer() {
    auto display = eglGetCurrentDisplay();
    auto dsurface = eglGetCurrentSurface(EGL_DRAW);
    auto rsurface = eglGetCurrentSurface(EGL_READ);
    auto context = eglGetCurrentContext();
    return [display, dsurface, rsurface, context] {
        eglMakeCurrent(display, dsurface, rsurface, context);
    };
}

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
    std::function<void()> onPlatformGetAutoContextRestore() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    PFNEGLCREATEIMAGEKHRPROC fEglCreateImageProc = nullptr;
    PFNEGLDESTROYIMAGEKHRPROC fEglDestroyImageProc = nullptr;

    EGLContext fContext;
    EGLDisplay fDisplay;
    EGLSurface fSurface;
};

static EGLContext create_gles_egl_context(EGLDisplay display,
                                          EGLConfig surfaceConfig,
                                          EGLContext eglShareContext,
                                          EGLint eglContextClientVersion) {
    const EGLint contextAttribsForOpenGLES[] = {
        EGL_CONTEXT_CLIENT_VERSION,
        eglContextClientVersion,
        EGL_NONE
    };
    return eglCreateContext(display, surfaceConfig, eglShareContext, contextAttribsForOpenGLES);
}
static EGLContext create_gl_egl_context(EGLDisplay display,
                                        EGLConfig surfaceConfig,
                                        EGLContext eglShareContext) {
    const EGLint contextAttribsForOpenGL[] = {
        EGL_NONE
    };
    return eglCreateContext(display, surfaceConfig, eglShareContext, contextAttribsForOpenGL);
}

EGLGLTestContext::EGLGLTestContext(GrGLStandard forcedGpuAPI, EGLGLTestContext* shareContext)
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE) {

    EGLContext eglShareContext = shareContext ? shareContext->fContext : nullptr;

    static const GrGLStandard kStandards[] = {
        kGL_GrGLStandard,
        kGLES_GrGLStandard,
    };

    size_t apiLimit = SK_ARRAY_COUNT(kStandards);
    size_t api = 0;
    if (forcedGpuAPI == kGL_GrGLStandard) {
        apiLimit = 1;
    } else if (forcedGpuAPI == kGLES_GrGLStandard) {
        api = 1;
    }
    SkASSERT(forcedGpuAPI == kNone_GrGLStandard || kStandards[api] == forcedGpuAPI);

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
        bool gles = kGLES_GrGLStandard == kStandards[api];

        if (!eglBindAPI(gles ? EGL_OPENGL_ES_API : EGL_OPENGL_API)) {
            continue;
        }

        EGLint numConfigs = 0;
        const EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, gles ? EGL_OPENGL_ES2_BIT : EGL_OPENGL_BIT,
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

        if (gles) {
#ifdef GR_EGL_TRY_GLES3_THEN_GLES2
            // Some older devices (Nexus7/Tegra3) crash when you try this.  So it is (for now)
            // hidden behind this flag.
            fContext = create_gles_egl_context(fDisplay, surfaceConfig, eglShareContext, 3);
            if (EGL_NO_CONTEXT == fContext) {
                fContext = create_gles_egl_context(fDisplay, surfaceConfig, eglShareContext, 2);
            }
#else
            fContext = create_gles_egl_context(fDisplay, surfaceConfig, eglShareContext, 2);
#endif
        } else {
            fContext = create_gl_egl_context(fDisplay, surfaceConfig, eglShareContext);
        }
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

        SkScopeExit restorer(context_restorer());
        if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
            SkDebugf("eglMakeCurrent failed.  EGL Error: 0x%08x\n", eglGetError());
            this->destroyGLContext();
            continue;
        }

        gl = GrGLMakeNativeInterface();
        if (!gl) {
            SkDebugf("Failed to create gl interface.\n");
            this->destroyGLContext();
            continue;
        }

        if (!gl->validate()) {
            SkDebugf("Failed to validate gl interface.\n");
            this->destroyGLContext();
            continue;
        }
        const char* extensions = eglQueryString(fDisplay, EGL_EXTENSIONS);
        if (strstr(extensions, "EGL_KHR_image")) {
            fEglCreateImageProc = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
            fEglDestroyImageProc =
                    (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
        }

        this->init(std::move(gl), EGLFenceSync::MakeIfSupported(fDisplay));
        break;
    }
}

EGLGLTestContext::~EGLGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void EGLGLTestContext::destroyGLContext() {
    if (fDisplay) {
        if (fContext) {
            if (eglGetCurrentContext() == fContext) {
                // This will ensure that the context is immediately deleted.
                eglMakeCurrent(fDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            }
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
    if (!this->gl()->hasExtension("EGL_KHR_gl_texture_2D_image") || !fEglCreateImageProc) {
        return GR_EGL_NO_IMAGE;
    }
    EGLint attribs[] = { GR_EGL_GL_TEXTURE_LEVEL, 0, GR_EGL_NONE };
    GrEGLClientBuffer clientBuffer = reinterpret_cast<GrEGLClientBuffer>(texID);
    return fEglCreateImageProc(fDisplay, fContext, GR_EGL_GL_TEXTURE_2D, clientBuffer, attribs);
}

void EGLGLTestContext::destroyEGLImage(GrEGLImage image) const {
    fEglDestroyImageProc(fDisplay, image);
}

GrGLuint EGLGLTestContext::eglImageToExternalTexture(GrEGLImage image) const {
    GrGLClearErr(this->gl());
    if (!this->gl()->hasExtension("GL_OES_EGL_image_external")) {
        return 0;
    }
    typedef GrGLvoid (*EGLImageTargetTexture2DProc)(GrGLenum, GrGLeglImage);

    EGLImageTargetTexture2DProc glEGLImageTargetTexture2D =
        (EGLImageTargetTexture2DProc) eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!glEGLImageTargetTexture2D) {
        return 0;
    }
    GrGLuint texID;
    GR_GL_CALL(this->gl(), GenTextures(1, &texID));
    if (!texID) {
        return 0;
    }
    GR_GL_CALL_NOERRCHECK(this->gl(), BindTexture(GR_GL_TEXTURE_EXTERNAL, texID));
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

std::function<void()> EGLGLTestContext::onPlatformGetAutoContextRestore() const {
    if (eglGetCurrentContext() == fContext) {
        return nullptr;
    }
    return context_restorer();
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

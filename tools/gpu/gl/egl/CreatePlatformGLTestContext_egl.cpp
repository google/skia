/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/egl/GrGLMakeEGLInterface.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "tools/gpu/gl/GLTestContext.h"

#include <vector>

#define EGL_PROTECTED_CONTENT_EXT 0x32C0
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

extern bool gCreateProtectedContext;

namespace {

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

    void onPlatformMakeNotCurrent() const override;
    void onPlatformMakeCurrent() const override;
    std::function<void()> onPlatformGetAutoContextRestore() const override;
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
                                          EGLint eglContextClientVersion,
                                          bool createProtected) {

    std::vector<EGLint> contextAttribs = {
            EGL_CONTEXT_CLIENT_VERSION, eglContextClientVersion,
    };

    if (createProtected) {
        contextAttribs.push_back(EGL_PROTECTED_CONTENT_EXT);
        contextAttribs.push_back(EGL_TRUE);
    }

    contextAttribs.push_back(EGL_NONE);

    return eglCreateContext(display, surfaceConfig, eglShareContext, contextAttribs.data());
}

static EGLContext create_gl_egl_context(EGLDisplay display,
                                        EGLConfig surfaceConfig,
                                        EGLContext eglShareContext,
                                        bool createProtected) {

    std::vector<EGLint> contextAttribs;

    if (createProtected) {
        contextAttribs.push_back(EGL_PROTECTED_CONTENT_EXT);
        contextAttribs.push_back(EGL_TRUE);
    }

    contextAttribs.push_back(EGL_NONE);

    return eglCreateContext(display, surfaceConfig, eglShareContext, contextAttribs.data());
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

    size_t apiLimit = std::size(kStandards);
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

        const char* extensions = eglQueryString(fDisplay, EGL_EXTENSIONS);

#if 0
        SkDebugf("VENDOR: %s\n", eglQueryString(fDisplay, EGL_VENDOR));
        SkDebugf("APIS: %s\n", eglQueryString(fDisplay, EGL_CLIENT_APIS));
        SkDebugf("VERSION: %s\n", eglQueryString(fDisplay, EGL_VERSION));
        SkDebugf("EXTENSIONS %s\n", extensions);
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

        bool createProtected = gCreateProtectedContext;
        if (createProtected && !strstr(extensions, "EGL_EXT_protected_content")) {
            SkDebugf("Missing EGL_EXT_protected_content support!\n");
            createProtected = false;
        }

        if (gles) {
#if defined(GR_EGL_TRY_GLES3_THEN_GLES2)
            // Some older devices (Nexus7/Tegra3) crash when you try this.  So it is (for now)
            // hidden behind this flag.
            fContext = create_gles_egl_context(fDisplay, surfaceConfig, eglShareContext, 3,
                                               createProtected);
            if (EGL_NO_CONTEXT == fContext) {
                fContext = create_gles_egl_context(fDisplay, surfaceConfig, eglShareContext, 2,
                                                   createProtected);
            }
#else
            fContext = create_gles_egl_context(fDisplay, surfaceConfig, eglShareContext, 2,
                                               createProtected);
#endif
        } else {
            fContext = create_gl_egl_context(fDisplay, surfaceConfig, eglShareContext,
                                             createProtected);
        }
        if (EGL_NO_CONTEXT == fContext) {
            SkDebugf("eglCreateContext failed.  EGL Error: 0x%08x\n", eglGetError());
            continue;
        }

        static const EGLint kSurfaceAttribs[] = {
            EGL_WIDTH, 1,
            EGL_HEIGHT, 1,
            createProtected ? EGL_PROTECTED_CONTENT_EXT : EGL_NONE,
            createProtected ? EGL_TRUE : EGL_NONE,
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

#if defined(SK_GL)
        gl = GrGLInterfaces::MakeEGL();
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
        if (strstr(extensions, "EGL_KHR_image")) {
            fEglCreateImageProc = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
            fEglDestroyImageProc =
                    (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
        }

        this->init(std::move(gl));
#else
        // Allow the GLTestContext creation to succeed without a GrGLInterface to support
        // GrContextFactory's persistent GL context workaround for Vulkan. We won't need the
        // GrGLInterface since we're not running the GL backend.
        this->init(nullptr);
#endif
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
#if defined(SK_GL)
    if (!this->gl()->hasExtension("EGL_KHR_gl_texture_2D_image") || !fEglCreateImageProc) {
        return GR_EGL_NO_IMAGE;
    }
    EGLint attribs[] = { GR_EGL_GL_TEXTURE_LEVEL, 0, GR_EGL_NONE };
    GrEGLClientBuffer clientBuffer = reinterpret_cast<GrEGLClientBuffer>(texID);
    return fEglCreateImageProc(fDisplay, fContext, GR_EGL_GL_TEXTURE_2D, clientBuffer, attribs);
#else
    (void)fEglCreateImageProc;
    return nullptr;
#endif
}

void EGLGLTestContext::destroyEGLImage(GrEGLImage image) const {
    fEglDestroyImageProc(fDisplay, image);
}

GrGLuint EGLGLTestContext::eglImageToExternalTexture(GrEGLImage image) const {
#if defined(SK_GL)
    while (this->gl()->fFunctions.fGetError() != GR_GL_NO_ERROR) {}
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
    if (this->gl()->fFunctions.fGetError() != GR_GL_NO_ERROR) {
        GR_GL_CALL(this->gl(), DeleteTextures(1, &texID));
        return 0;
    }
    glEGLImageTargetTexture2D(GR_GL_TEXTURE_EXTERNAL, image);
    if (this->gl()->fFunctions.fGetError() != GR_GL_NO_ERROR) {
        GR_GL_CALL(this->gl(), DeleteTextures(1, &texID));
        return 0;
    }
    return texID;
#else
    return 0;
#endif
}

std::unique_ptr<sk_gpu_test::GLTestContext> EGLGLTestContext::makeNew() const {
    std::unique_ptr<sk_gpu_test::GLTestContext> ctx(new EGLGLTestContext(this->gl()->fStandard,
                                                                         nullptr));
    if (ctx) {
        ctx->makeCurrent();
    }
    return ctx;
}

void EGLGLTestContext::onPlatformMakeNotCurrent() const {
    if (!eglMakeCurrent(fDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT )) {
        SkDebugf("Could not reset the context.\n");
    }
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

GrGLFuncPtr EGLGLTestContext::onPlatformGetProcAddress(const char* procName) const {
    return eglGetProcAddress(procName);
}

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

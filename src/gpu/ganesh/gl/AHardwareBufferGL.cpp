/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/GrAHardwareBufferUtils.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#include <android/hardware_buffer.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#define PROT_CONTENT_EXT_STR "EGL_EXT_protected_content"
#define EGL_PROTECTED_CONTENT_EXT 0x32C0

namespace GrAHardwareBufferUtils {

GrBackendFormat GetGLBackendFormat(GrDirectContext* dContext,
                                   uint32_t bufferFormat, bool requireKnownFormat) {
    GrBackendApi backend = dContext->backend();
    if (backend != GrBackendApi::kOpenGL) {
        return GrBackendFormat();
    }
    switch (bufferFormat) {
        //TODO: find out if we can detect, which graphic buffers support GR_GL_TEXTURE_2D
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            return GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
            return GrBackendFormats::MakeGL(GR_GL_RGBA16F, GR_GL_TEXTURE_EXTERNAL);
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
            return GrBackendFormats::MakeGL(GR_GL_RGB565, GR_GL_TEXTURE_EXTERNAL);
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
            return GrBackendFormats::MakeGL(GR_GL_RGB10_A2, GR_GL_TEXTURE_EXTERNAL);
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
            return GrBackendFormats::MakeGL(GR_GL_RGB8, GR_GL_TEXTURE_EXTERNAL);
#if __ANDROID_API__ >= 33
        case AHARDWAREBUFFER_FORMAT_R8_UNORM:
            return GrBackendFormats::MakeGL(GR_GL_R8, GR_GL_TEXTURE_EXTERNAL);
#endif
        default:
            if (requireKnownFormat) {
                return GrBackendFormat();
            } else {
                return GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
            }
    }
    SkUNREACHABLE;
}

class GLTextureHelper {
public:
    GLTextureHelper(GrGLuint texID, EGLImageKHR image, EGLDisplay display, GrGLuint texTarget)
        : fTexID(texID)
        , fImage(image)
        , fDisplay(display)
        , fTexTarget(texTarget) { }
    ~GLTextureHelper() {
        glDeleteTextures(1, &fTexID);
        // eglDestroyImageKHR will remove a ref from the AHardwareBuffer
        eglDestroyImageKHR(fDisplay, fImage);
    }
    void rebind(GrDirectContext*);

private:
    GrGLuint    fTexID;
    EGLImageKHR fImage;
    EGLDisplay  fDisplay;
    GrGLuint    fTexTarget;
};

void GLTextureHelper::rebind(GrDirectContext* dContext) {
    glBindTexture(fTexTarget, fTexID);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glBindTexture(%#x, %d) failed (%#x)", (int) fTexTarget,
            (int) fTexID, (int) status);
        return;
    }
    glEGLImageTargetTexture2DOES(fTexTarget, fImage);
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glEGLImageTargetTexture2DOES failed (%#x)", (int) status);
        return;
    }
    dContext->resetContext(kTextureBinding_GrGLBackendState);
}

void delete_gl_texture(void* context) {
    GLTextureHelper* cleanupHelper = static_cast<GLTextureHelper*>(context);
    delete cleanupHelper;
}

void update_gl_texture(void* context, GrDirectContext* dContext) {
    GLTextureHelper* cleanupHelper = static_cast<GLTextureHelper*>(context);
    cleanupHelper->rebind(dContext);
}

static GrBackendTexture make_gl_backend_texture(
        GrDirectContext* dContext,
        AHardwareBuffer* hardwareBuffer,
        int width, int height,
        DeleteImageProc* deleteProc,
        UpdateImageProc* updateProc,
        TexImageCtx* imageCtx,
        bool isProtectedContent,
        const GrBackendFormat& backendFormat,
        bool isRenderable) {
    while (GL_NO_ERROR != glGetError()) {} //clear GL errors

    EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(hardwareBuffer);
    EGLint attribs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                         isProtectedContent ? EGL_PROTECTED_CONTENT_EXT : EGL_NONE,
                         isProtectedContent ? EGL_TRUE : EGL_NONE,
                         EGL_NONE };
    EGLDisplay display = eglGetCurrentDisplay();
    // eglCreateImageKHR will add a ref to the AHardwareBuffer
    EGLImageKHR image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                          clientBuffer, attribs);
    if (EGL_NO_IMAGE_KHR == image) {
        SkDebugf("Could not create EGL image, err = (%#x)", (int) eglGetError() );
        return GrBackendTexture();
    }

    GrGLuint texID;
    glGenTextures(1, &texID);
    if (!texID) {
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }

    GrGLuint target = isRenderable ? GR_GL_TEXTURE_2D : GR_GL_TEXTURE_EXTERNAL;

    glBindTexture(target, texID);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glBindTexture failed (%#x)", (int) status);
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    glEGLImageTargetTexture2DOES(target, image);
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glEGLImageTargetTexture2DOES failed (%#x)", (int) status);
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    dContext->resetContext(kTextureBinding_GrGLBackendState);

    GrGLTextureInfo textureInfo;
    textureInfo.fID = texID;
    SkASSERT(backendFormat.isValid());
    textureInfo.fTarget = target;
    textureInfo.fFormat = GrBackendFormats::AsGLFormatEnum(backendFormat);
    textureInfo.fProtected = skgpu::Protected(isProtectedContent);

    *deleteProc = delete_gl_texture;
    *updateProc = update_gl_texture;
    *imageCtx = new GLTextureHelper(texID, image, display, target);

    return GrBackendTextures::MakeGL(width, height, skgpu::Mipmapped::kNo, textureInfo);
}

static bool can_import_protected_content_eglimpl() {
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    const char* exts = eglQueryString(dpy, EGL_EXTENSIONS);
    size_t cropExtLen = strlen(PROT_CONTENT_EXT_STR);
    size_t extsLen = strlen(exts);
    bool equal = !strcmp(PROT_CONTENT_EXT_STR, exts);
    bool atStart = !strncmp(PROT_CONTENT_EXT_STR " ", exts, cropExtLen+1);
    bool atEnd = (cropExtLen+1) < extsLen
                  && !strcmp(" " PROT_CONTENT_EXT_STR,
                  exts + extsLen - (cropExtLen+1));
    bool inMiddle = strstr(exts, " " PROT_CONTENT_EXT_STR " ");
    return equal || atStart || atEnd || inMiddle;
}

static bool can_import_protected_content(GrDirectContext* dContext) {
    SkASSERT(GrBackendApi::kOpenGL == dContext->backend());
    // Only compute whether the extension is present once the first time this
    // function is called.
    static bool hasIt = can_import_protected_content_eglimpl();
    return hasIt;
}

GrBackendTexture MakeGLBackendTexture(GrDirectContext* dContext,
                                      AHardwareBuffer* hardwareBuffer,
                                      int width, int height,
                                      DeleteImageProc* deleteProc,
                                      UpdateImageProc* updateProc,
                                      TexImageCtx* imageCtx,
                                      bool isProtectedContent,
                                      const GrBackendFormat& backendFormat,
                                      bool isRenderable) {
    SkASSERT(dContext);
    if (!dContext || dContext->abandoned()) {
        return GrBackendTexture();
    }

    if (GrBackendApi::kOpenGL != dContext->backend()) {
        return GrBackendTexture();
    }

    if (isProtectedContent && !can_import_protected_content(dContext)) {
        return GrBackendTexture();
    }

    return make_gl_backend_texture(dContext, hardwareBuffer, width, height, deleteProc,
                                   updateProc, imageCtx, isProtectedContent, backendFormat,
                                   isRenderable);
}

}  // namespace GrAHardwareBufferUtils

#endif

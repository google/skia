/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"


#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include "GrAHardwareBufferImageGenerator.h"

#include <android/hardware_buffer.h>

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "SkMessageBus.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLTypes.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#define PROT_CONTENT_EXT_STR "EGL_EXT_protected_content"
#define EGL_PROTECTED_CONTENT_EXT 0x32C0

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

static bool can_import_protected_content(GrContext* context) {
    if (kOpenGL_GrBackend == context->contextPriv().getBackend()) {
        // Only compute whether the extension is present once the first time this
        // function is called.
        static bool hasIt = can_import_protected_content_eglimpl();
        return hasIt;
    }
    return false;
}

std::unique_ptr<SkImageGenerator> GrAHardwareBufferImageGenerator::Make(
        AHardwareBuffer* graphicBuffer, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(graphicBuffer, &bufferDesc);
    SkColorType colorType;
    switch (bufferDesc.format) {
    case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
        colorType = kRGBA_8888_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
        colorType = kRGBA_F16_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
        colorType = kRGB_565_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
    case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
        colorType = kRGB_888x_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
        colorType = kRGBA_1010102_SkColorType;
        break;
    default:
        // Given that we only use this texture as a source, colorType will not impact how Skia uses
        // the texture.  The only potential affect this is anticipated to have is that for some
        // format types if we are not bound as an OES texture we may get invalid results for SKP
        // capture if we read back the texture.
        colorType = kRGBA_8888_SkColorType;
        break;
    }
    SkImageInfo info = SkImageInfo::Make(bufferDesc.width, bufferDesc.height, colorType,
                                         alphaType, std::move(colorSpace));
    bool createProtectedImage = 0 != (bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);
    return std::unique_ptr<SkImageGenerator>(new GrAHardwareBufferImageGenerator(info, graphicBuffer,
            alphaType, createProtectedImage, bufferDesc.format));
}

GrAHardwareBufferImageGenerator::GrAHardwareBufferImageGenerator(const SkImageInfo& info,
        AHardwareBuffer* hardwareBuffer, SkAlphaType alphaType, bool isProtectedContent,
        uint32_t bufferFormat)
    : INHERITED(info)
    , fHardwareBuffer(hardwareBuffer)
    , fBufferFormat(bufferFormat)
    , fIsProtectedContent(isProtectedContent) {
    AHardwareBuffer_acquire(fHardwareBuffer);
}

GrAHardwareBufferImageGenerator::~GrAHardwareBufferImageGenerator() {
    AHardwareBuffer_release(fHardwareBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class GLCleanupHelper {
public:
    GLCleanupHelper(GrGLuint texID, EGLImageKHR image, EGLDisplay display)
        : fTexID(texID)
        , fImage(image)
        , fDisplay(display) { }
    ~GLCleanupHelper() {
        glDeleteTextures(1, &fTexID);
        // eglDestroyImageKHR will remove a ref from the AHardwareBuffer
        eglDestroyImageKHR(fDisplay, fImage);
    }
private:
    GrGLuint    fTexID;
    EGLImageKHR fImage;
    EGLDisplay  fDisplay;
};


void GrAHardwareBufferImageGenerator::DeleteGLTexture(void* context) {
    GLCleanupHelper* cleanupHelper = static_cast<GLCleanupHelper*>(context);
    delete cleanupHelper;
}

static GrBackendTexture make_gl_backend_texture(
        GrContext* context, AHardwareBuffer* hardwareBuffer,
        int width, int height, GrPixelConfig config,
        GrAHardwareBufferImageGenerator::DeleteImageProc* deleteProc,
        GrAHardwareBufferImageGenerator::DeleteImageCtx* deleteCtx,
        bool isProtectedContent,
        const GrBackendFormat& backendFormat) {
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
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texID);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glBindTexture failed (%#x)", (int) status);
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glEGLImageTargetTexture2DOES failed (%#x)", (int) status);
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    context->resetContext(kTextureBinding_GrGLBackendState);

    GrGLTextureInfo textureInfo;
    textureInfo.fID = texID;
    SkASSERT(backendFormat.isValid());
    textureInfo.fTarget = *backendFormat.getGLTarget();
    textureInfo.fFormat = *backendFormat.getGLFormat();

    *deleteProc = GrAHardwareBufferImageGenerator::DeleteGLTexture;
    *deleteCtx = new GLCleanupHelper(texID, image, display);

    return GrBackendTexture(width, height, GrMipMapped::kNo, textureInfo);
}

static GrBackendTexture make_backend_texture(
        GrContext* context, AHardwareBuffer* hardwareBuffer,
        int width, int height, GrPixelConfig config,
        GrAHardwareBufferImageGenerator::DeleteImageProc* deleteProc,
        GrAHardwareBufferImageGenerator::DeleteImageCtx* deleteCtx,
        bool isProtectedContent,
        const GrBackendFormat& backendFormat) {
    if (context->abandoned() || kOpenGL_GrBackend != context->contextPriv().getBackend()) {
        // Check if GrContext is not abandoned and the backend is GL.
        return GrBackendTexture();
    }
    bool createProtectedImage = isProtectedContent && can_import_protected_content(context);
    return make_gl_backend_texture(context, hardwareBuffer, width, height, config, deleteProc,
                                   deleteCtx, createProtectedImage, backendFormat);
}

GrBackendFormat get_backend_format(GrBackend backend, uint32_t bufferFormat) {
    if (backend == kOpenGL_GrBackend) {
        switch (bufferFormat) {
            //TODO: find out if we can detect, which graphic buffers support GR_GL_TEXTURE_2D
            case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
                return GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
            case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
                return GrBackendFormat::MakeGL(GR_GL_RGBA16F, GR_GL_TEXTURE_EXTERNAL);
            case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
                return GrBackendFormat::MakeGL(GR_GL_RGB565, GR_GL_TEXTURE_EXTERNAL);
            case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
                return GrBackendFormat::MakeGL(GR_GL_RGB10_A2, GR_GL_TEXTURE_EXTERNAL);
            case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
                return GrBackendFormat::MakeGL(GR_GL_RGB8, GR_GL_TEXTURE_EXTERNAL);
            default:
                return GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
        }
    }
    return GrBackendFormat();
}

sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::makeProxy(GrContext* context) {
    if (context->abandoned() || kOpenGL_GrBackend != context->contextPriv().getBackend()) {
        // Check if GrContext is not abandoned and the backend is GL.
        return nullptr;
    }

    GrPixelConfig pixelConfig;
    GrBackendFormat backendFormat = get_backend_format(context->contextPriv().getBackend(),
                                                       fBufferFormat);
    if (!context->contextPriv().caps()->getConfigFromBackendFormat(
            backendFormat, this->getInfo().colorType(), &pixelConfig)) {
        return nullptr;
    }

    int width = this->getInfo().width();
    int height = this->getInfo().height();

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = pixelConfig;

    GrTextureType textureType = GrTextureType::k2D;
    if (context->contextPriv().getBackend() == kOpenGL_GrBackend) {
        textureType = GrTextureType::kExternal;
    }

    auto proxyProvider = context->contextPriv().proxyProvider();

    AHardwareBuffer* hardwareBuffer = fHardwareBuffer;
    AHardwareBuffer_acquire(hardwareBuffer);

    const bool isProtectedContent = fIsProtectedContent;

    sk_sp<GrTextureProxy> texProxy = proxyProvider->createLazyProxy(
            [context, hardwareBuffer, width, height, pixelConfig, isProtectedContent, backendFormat]
            (GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    AHardwareBuffer_release(hardwareBuffer);
                    return sk_sp<GrTexture>();
                }

                DeleteImageProc deleteImageProc = nullptr;
                DeleteImageCtx deleteImageCtx = nullptr;

                GrBackendTexture backendTex = make_backend_texture(context, hardwareBuffer,
                                                                   width, height, pixelConfig,
                                                                   &deleteImageProc,
                                                                   &deleteImageCtx,
                                                                   isProtectedContent,
                                                                   backendFormat);
                if (!backendTex.isValid()) {
                    return sk_sp<GrTexture>();
                }
                SkASSERT(deleteImageProc && deleteImageCtx);

                backendTex.fConfig = pixelConfig;
                sk_sp<GrTexture> tex = resourceProvider->wrapBackendTexture(backendTex);
                if (!tex) {
                    deleteImageProc(deleteImageCtx);
                    return sk_sp<GrTexture>();
                }

                sk_sp<GrReleaseProcHelper> releaseProcHelper(
                        new GrReleaseProcHelper(deleteImageProc, deleteImageCtx));
                tex->setRelease(releaseProcHelper);

                return tex;
            },
            desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, textureType, SkBackingFit::kExact,
            SkBudgeted::kNo);

    if (!texProxy) {
        AHardwareBuffer_release(hardwareBuffer);
    }
    return texProxy;
}

sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::onGenerateTexture(
        GrContext* context, const SkImageInfo& info, const SkIPoint& origin, bool willNeedMipMaps) {
    sk_sp<GrTextureProxy> texProxy = this->makeProxy(context);
    if (!texProxy) {
        return nullptr;
    }

    if (0 == origin.fX && 0 == origin.fY &&
        info.width() == this->getInfo().width() && info.height() == this->getInfo().height()) {
        // If the caller wants the full texture we're done. The caller will handle making a copy for
        // mip maps if that is required.
        return texProxy;
    }
    // Otherwise, make a copy for the requested subset.
    SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());

    GrMipMapped mipMapped = willNeedMipMaps ? GrMipMapped::kYes : GrMipMapped::kNo;

    return GrSurfaceProxy::Copy(context, texProxy.get(), mipMapped, subset, SkBudgeted::kYes);
}

bool GrAHardwareBufferImageGenerator::onIsValid(GrContext* context) const {
    if (nullptr == context) {
        return false; //CPU backend is not supported, because hardware buffer can be swizzled
    }
    // TODO: add Vulkan support
    return kOpenGL_GrBackend == context->contextPriv().getBackend();
}

#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK

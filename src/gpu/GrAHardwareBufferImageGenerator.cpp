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
    default:
        return nullptr;
    }
    SkImageInfo info = SkImageInfo::Make(bufferDesc.width, bufferDesc.height, colorType,
                                         alphaType, std::move(colorSpace));
    bool createProtectedImage = 0 != (bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);
    return std::unique_ptr<SkImageGenerator>(new GrAHardwareBufferImageGenerator(info, graphicBuffer,
            alphaType, createProtectedImage));
}

GrAHardwareBufferImageGenerator::GrAHardwareBufferImageGenerator(const SkImageInfo& info,
        AHardwareBuffer* hardwareBuffer, SkAlphaType alphaType, bool isProtectedContent)
    : INHERITED(info)
    , fHardwareBuffer(hardwareBuffer)
    , fIsProtectedContent(isProtectedContent) {
    AHardwareBuffer_acquire(fHardwareBuffer);
}

void GrAHardwareBufferImageGenerator::releaseTextureRef() {
    // We must release our ref on the proxy before we send the free message to the actual texture so
    // that we make sure the last ref (if it is owned by this class) is released on the owning
    // context.
    fCachedProxy.reset();
    if (fOwnedTexture) {
        SkASSERT(fOwningContextID != SK_InvalidGenID);
        // Notify the original cache that it can free the last ref, so it happens on the correct
        // thread.
        GrGpuResourceFreedMessage msg { fOwnedTexture, fOwningContextID };
        SkMessageBus<GrGpuResourceFreedMessage>::Post(msg);
        fOwnedTexture = nullptr;
        fOwningContextID = SK_InvalidGenID;
    }
}

GrAHardwareBufferImageGenerator::~GrAHardwareBufferImageGenerator() {
    this->releaseTextureRef();
    AHardwareBuffer_release(fHardwareBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::onGenerateTexture(
        GrContext* context, const SkImageInfo& info, const SkIPoint& origin, bool willNeedMipMaps) {
    this->makeProxy(context);
    if (!fCachedProxy) {
        return nullptr;
    }

    bool makingASubset = true;
    if (0 == origin.fX && 0 == origin.fY &&
            info.width() == this->getInfo().width() && info.height() == this->getInfo().height()) {
        makingASubset = false;
        if (!willNeedMipMaps || GrMipMapped::kYes == fCachedProxy->mipMapped()) {
            // If the caller wants the full texture and we have the correct mip support, we're done
            return fCachedProxy;
        }
    }
    // Otherwise, make a copy for the requested subset or for mip maps.
    SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());

    GrMipMapped mipMapped = willNeedMipMaps ? GrMipMapped::kYes : GrMipMapped::kNo;

    sk_sp<GrTextureProxy> texProxy = GrSurfaceProxy::Copy(context, fCachedProxy.get(), mipMapped,
                                                          subset, SkBudgeted::kYes);
    if (!makingASubset && texProxy) {
        // We are in this case if we wanted the full texture, but we will be mip mapping the
        // texture. Therefore we want to update the cached texture so that we point to the
        // mipped version instead of the old one.
        SkASSERT(willNeedMipMaps);
        SkASSERT(GrMipMapped::kYes == texProxy->mipMapped());

        // The only way we should get into here is if we just made a new texture in makeProxy or
        // we found a cached texture in the same context. Thus the current and cached contexts
        // should match.
        SkASSERT(context->uniqueID() == fOwningContextID);

        // Since we no longer will be caching the and reusing the texture that actually wraps the
        // hardware buffer, we can release our refs on it.
        this->releaseTextureRef();

        fCachedProxy = texProxy;
    }
    return texProxy;
}

class BufferCleanupHelper {
public:
    BufferCleanupHelper(EGLImageKHR image, EGLDisplay display)
        : fImage(image)
        , fDisplay(display) { }
    ~BufferCleanupHelper() {
        // eglDestroyImageKHR will remove a ref from the AHardwareBuffer
        eglDestroyImageKHR(fDisplay, fImage);
    }
private:
    EGLImageKHR fImage;
    EGLDisplay fDisplay;
};


void GrAHardwareBufferImageGenerator::DeleteEGLImage(void* context) {
    BufferCleanupHelper* cleanupHelper = static_cast<BufferCleanupHelper*>(context);
    delete cleanupHelper;
}

static GrBackendTexture make_gl_backend_texture(
        GrContext* context, AHardwareBuffer* hardwareBuffer,
        int width, int height, GrPixelConfig config,
        GrAHardwareBufferImageGenerator::DeleteImageProc* deleteProc,
        GrAHardwareBufferImageGenerator::DeleteImageCtx* deleteCtx,
        bool isProtectedContent) {
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
    textureInfo.fTarget = GL_TEXTURE_EXTERNAL_OES;
    textureInfo.fID = texID;
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
            textureInfo.fFormat = GR_GL_RGBA8;
            break;
        case kRGBA_half_GrPixelConfig:
            textureInfo.fFormat = GR_GL_RGBA16F;
            break;
        case kRGB_565_GrPixelConfig:
            textureInfo.fFormat = GR_GL_RGB565;
            break;
        default:
            SkASSERT(false);
    }

    *deleteProc = GrAHardwareBufferImageGenerator::DeleteEGLImage;
    *deleteCtx = new BufferCleanupHelper(image, display);

    return GrBackendTexture(width, height, GrMipMapped::kNo, textureInfo);
}

static GrBackendTexture make_backend_texture(
        GrContext* context, AHardwareBuffer* hardwareBuffer,
        int width, int height, GrPixelConfig config,
        GrAHardwareBufferImageGenerator::DeleteImageProc* deleteProc,
        GrAHardwareBufferImageGenerator::DeleteImageCtx* deleteCtx,
        bool isProtectedContent) {
    if (context->abandoned() || kOpenGL_GrBackend != context->contextPriv().getBackend()) {
        // Check if GrContext is not abandoned and the backend is GL.
        return GrBackendTexture();
    }
    bool createProtectedImage = isProtectedContent && can_import_protected_content(context);
    return make_gl_backend_texture(context, hardwareBuffer, width, height, config, deleteProc,
                                   deleteCtx, createProtectedImage);
}

static void free_backend_texture(GrBackendTexture* backendTexture) {
    SkASSERT(backendTexture && backendTexture->isValid());

    switch (backendTexture->backend()) {
        case kOpenGL_GrBackend: {
            GrGLTextureInfo texInfo;
            SkAssertResult(backendTexture->getGLTextureInfo(&texInfo));
            glDeleteTextures(1, &texInfo.fID);
            return;
        }
        case kVulkan_GrBackend: // fall through
        default:
            return;
    }
}

void GrAHardwareBufferImageGenerator::makeProxy(GrContext* context) {
    if (context->abandoned() || kOpenGL_GrBackend != context->contextPriv().getBackend()) {
        // Check if GrContext is not abandoned and the backend is GL.
        return;
    }

    if (SK_InvalidGenID != fOwningContextID) {
        SkASSERT(fCachedProxy);
        if (context->uniqueID() != fOwningContextID) {
            this->releaseTextureRef();
        } else {
            return;
        }
    }
    SkASSERT(!fCachedProxy);

    fOwningContextID = context->uniqueID();

    GrPixelConfig pixelConfig;
    switch (this->getInfo().colorType()) {
        case kRGBA_8888_SkColorType:
            pixelConfig = kRGBA_8888_GrPixelConfig;
            break;
        case kRGBA_F16_SkColorType:
            pixelConfig = kRGBA_half_GrPixelConfig;
            break;
        case kRGB_565_SkColorType:
            pixelConfig = kRGB_565_GrPixelConfig;
            break;
        default:
            return;
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

    GrTexture** ownedTexturePtr = &fOwnedTexture;
    const bool isProtectedContent = fIsProtectedContent;

    fCachedProxy = proxyProvider->createLazyProxy(
            [context, hardwareBuffer, width, height, pixelConfig, ownedTexturePtr,
             isProtectedContent]
            (GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    AHardwareBuffer_release(hardwareBuffer);
                    return sk_sp<GrTexture>();
                }

                SkASSERT(!*ownedTexturePtr);

                DeleteImageProc deleteImageProc = nullptr;
                DeleteImageCtx deleteImageCtx = nullptr;

                GrBackendTexture backendTex = make_backend_texture(context, hardwareBuffer,
                                                                   width, height, pixelConfig,
                                                                   &deleteImageProc,
                                                                   &deleteImageCtx,
                                                                   isProtectedContent);
                if (!backendTex.isValid()) {
                    return sk_sp<GrTexture>();
                }
                SkASSERT(deleteImageProc && deleteImageCtx);

                backendTex.fConfig = pixelConfig;
                sk_sp<GrTexture> tex = resourceProvider->wrapBackendTexture(
                        backendTex, kAdopt_GrWrapOwnership);
                if (!tex) {
                    free_backend_texture(&backendTex);
                    deleteImageProc(deleteImageCtx);
                    return sk_sp<GrTexture>();
                }

                sk_sp<GrReleaseProcHelper> releaseProcHelper(
                        new GrReleaseProcHelper(deleteImageProc, deleteImageCtx));
                tex->setRelease(releaseProcHelper);

                *ownedTexturePtr = tex.get();

                // Attach our texture to this context's resource cache. This ensures that deletion
                // will happen in the correct thread/context. This adds the only ref to the texture
                // that will persist from this point. To trigger GrTexture deletion a message is
                // sent by generator dtor or by makeProxy when it is invoked with a different
                // context.
                context->contextPriv().getResourceCache()->insertCrossContextGpuResource(tex.get());
                return tex;
            },
            desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, textureType, SkBackingFit::kExact,
            SkBudgeted::kNo);


    if (!fCachedProxy) {
        AHardwareBuffer_release(hardwareBuffer);
        return;
    }
}

bool GrAHardwareBufferImageGenerator::onIsValid(GrContext* context) const {
    if (nullptr == context) {
        return false; //CPU backend is not supported, because hardware buffer can be swizzled
    }
    // TODO: add Vulkan support
    return kOpenGL_GrBackend == context->contextPriv().getBackend();
}

#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK

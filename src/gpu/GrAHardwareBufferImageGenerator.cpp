/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"

//TODO: This define is temporary and we will compile with NDK after
//TODO: Skia bug 6672 is resolved.
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include "GrAHardwareBufferImageGenerator.h"

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrTexture.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

class BufferCleanupHelper {
public:
    BufferCleanupHelper(EGLImageKHR image, EGLDisplay display)
        : fImage(image)
        , fDisplay(display) { }
    ~BufferCleanupHelper() {
        eglDestroyImageKHR(fDisplay, fImage);
    }
private:
    EGLImageKHR fImage;
    EGLDisplay fDisplay;
};

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
    return std::unique_ptr<SkImageGenerator>(new GrAHardwareBufferImageGenerator(info, graphicBuffer,
            alphaType));
}

GrAHardwareBufferImageGenerator::GrAHardwareBufferImageGenerator(const SkImageInfo& info,
        AHardwareBuffer* graphicBuffer, SkAlphaType alphaType)
    : INHERITED(info)
    , fGraphicBuffer(graphicBuffer)
    , fAlphaType(alphaType) {
    AHardwareBuffer_acquire(fGraphicBuffer);
}

GrAHardwareBufferImageGenerator::~GrAHardwareBufferImageGenerator() {
    AHardwareBuffer_release(fGraphicBuffer);
}

void GrAHardwareBufferImageGenerator::deleteImageTexture(void* context) {
     BufferCleanupHelper* cleanupHelper = static_cast<BufferCleanupHelper*>(context);
     delete cleanupHelper;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU


sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::onGenerateTexture(
        GrContext* context, const SkImageInfo& info, const SkIPoint& origin) {
    // TODO: return a cached GrTextureProxy if invoked with the same context
    // TODO: if we cache GrTextureProxy, then deleteImageTexture may be invoked on the wrong thread

    if (!context->getGpu() || kOpenGL_GrBackend != context->contextPriv().getBackend()) {
        // Check if GrContext is not abandoned and the backend is GL.
        return nullptr;
    }

    while (GL_NO_ERROR != glGetError()) {} //clear GL errors

    EGLClientBuffer  clientBuffer = eglGetNativeClientBufferANDROID(fGraphicBuffer);
    EGLint attribs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                         EGL_NONE };
    EGLDisplay display = eglGetCurrentDisplay();
    EGLImageKHR image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                          clientBuffer, attribs);
    if (EGL_NO_IMAGE_KHR == image) {
        SkDebugf("Could not create EGL image, err = (%#x)", (int) eglGetError() );
        return nullptr;
    }
    GrGLuint texID;
    glGenTextures(1, &texID);
    if (!texID) {
        eglDestroyImageKHR(display, image);
        return nullptr;
    }
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texID);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glBindTexture failed (%#x)", (int) status);
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return nullptr;
    }
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glEGLImageTargetTexture2DOES failed (%#x)", (int) status);
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return nullptr;
    }
    context->resetContext(kTextureBinding_GrGLBackendState);

    GrGLTextureInfo textureInfo;
    textureInfo.fTarget = GL_TEXTURE_EXTERNAL_OES;
    textureInfo.fID = texID;

    GrPixelConfig pixelConfig;
    switch (getInfo().colorType()) {
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
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return nullptr;
    }

    GrBackendTexture backendTex(getInfo().width(), getInfo().height(), pixelConfig, textureInfo);
    if (backendTex.width() <= 0 || backendTex.height() <= 0) {
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return nullptr;
    }
    GrBackendTextureFlags flags = kNone_GrBackendTextureFlag;
    sk_sp<GrTexture> tex = context->resourceProvider()->wrapBackendTexture(backendTex,
                                                                       kTopLeft_GrSurfaceOrigin,
                                                                       flags,
                                                                       0,
                                                                       kAdopt_GrWrapOwnership);
    if (!tex) {
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return nullptr;
    }
    tex->setRelease(deleteImageTexture, new BufferCleanupHelper(image, display));
    sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(tex)));

    if (0 == origin.fX && 0 == origin.fY &&
            info.width() == backendTex.width() && info.height() == backendTex.height()) {
        // If the caller wants the entire texture, we're done
        return proxy;
    } else {
        // Otherwise, make a copy of the requested subset.
        GrSurfaceDesc desc;
        desc.fConfig = proxy->config();
        desc.fWidth = info.width();
        desc.fHeight = info.height();
        desc.fOrigin = proxy->origin();
        desc.fIsMipMapped = proxy->isMipMapped();

        sk_sp<GrSurfaceContext> sContext(context->contextPriv().makeDeferredSurfaceContext(
            desc, SkBackingFit::kExact, SkBudgeted::kYes));
        if (!sContext) {
            return nullptr;
        }

        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());
        if (!sContext->copy(proxy.get(), subset, SkIPoint::Make(0, 0))) {
            return nullptr;
        }

        return sContext->asTextureProxyRef();
    }
}
#endif

bool GrAHardwareBufferImageGenerator::onIsValid(GrContext* context) const {
    if (nullptr == context) {
        return false; //CPU backend is not supported, because hardware buffer can be swizzled
    }
    // TODO: add Vulkan support
    return kOpenGL_GrBackend == context->contextPriv().getBackend();
}

#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK

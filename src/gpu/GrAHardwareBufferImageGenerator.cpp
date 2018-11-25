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
    , fGraphicBuffer(graphicBuffer) {
    AHardwareBuffer_acquire(fGraphicBuffer);
}

GrAHardwareBufferImageGenerator::~GrAHardwareBufferImageGenerator() {
    AHardwareBuffer_release(fGraphicBuffer);
    this->clear();
}

void GrAHardwareBufferImageGenerator::clear() {
    if (fOriginalTexture) {
        // Notify the original cache that it can free the last ref, so it happens on the correct
        // thread.
        GrGpuResourceFreedMessage msg { fOriginalTexture, fOwningContextID };
        SkMessageBus<GrGpuResourceFreedMessage>::Post(msg);
        fOriginalTexture = nullptr;
    }
}

void GrAHardwareBufferImageGenerator::deleteImageTexture(void* context) {
    BufferCleanupHelper* cleanupHelper = static_cast<BufferCleanupHelper*>(context);
    delete cleanupHelper;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::onGenerateTexture(
        GrContext* context, const SkImageInfo& info, const SkIPoint& origin,
        SkTransferFunctionBehavior, bool willNeedMipMaps) {
    auto proxy = this->makeProxy(context);
    if (!proxy) {
        return nullptr;
    }

    bool makingASubset = true;
    if (0 == origin.fX && 0 == origin.fY &&
            info.width() == getInfo().width() && info.height() == getInfo().height()) {
        makingASubset = false;
        if (!willNeedMipMaps || GrMipMapped::kYes == proxy->mipMapped()) {
            // If the caller wants the full texture and we have the correct mip support, we're done
            return proxy;
        }
    }
    // Otherwise, make a copy for the requested subset or for mip maps.
    SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());

    GrMipMapped mipMapped = willNeedMipMaps ? GrMipMapped::kYes : GrMipMapped::kNo;

    sk_sp<GrTextureProxy> texProxy = GrSurfaceProxy::Copy(context, proxy.get(), mipMapped,
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

        // Clear out the old cached texture.
        this->clear();

        // We need to get the actual GrTexture so force instantiation of the GrTextureProxy
        texProxy->instantiate(context->contextPriv().resourceProvider());
        GrTexture* texture = texProxy->priv().peekTexture();
        SkASSERT(texture);
        fOriginalTexture = texture;
    }
    return texProxy;
}
#endif

sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::makeProxy(GrContext* context) {
    if (context->abandoned() || kOpenGL_GrBackend != context->contextPriv().getBackend()) {
        // Check if GrContext is not abandoned and the backend is GL.
        return nullptr;
    }

    auto proxyProvider = context->contextPriv().proxyProvider();

    // return a cached GrTexture if invoked with the same context
    if (fOriginalTexture && fOwningContextID == context->uniqueID()) {
        return proxyProvider->createWrapped(sk_ref_sp(fOriginalTexture),
                                            kTopLeft_GrSurfaceOrigin);
    }

    while (GL_NO_ERROR != glGetError()) {} //clear GL errors

    EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(fGraphicBuffer);
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
    sk_sp<GrTexture> tex = context->contextPriv().resourceProvider()->wrapBackendTexture(
                                                        backendTex, kAdopt_GrWrapOwnership);
    if (!tex) {
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return nullptr;
    }
    sk_sp<GrReleaseProcHelper> releaseHelper(
            new GrReleaseProcHelper(deleteImageTexture, new BufferCleanupHelper(image, display)));

    tex->setRelease(std::move(releaseHelper));

    // We fail this assert, if the context has changed. This will be fully handled after
    // skbug.com/6812 is ready.
    SkASSERT(!fOriginalTexture);

    this->clear();
    fOriginalTexture = tex.get();
    fOwningContextID = context->uniqueID();
    // Attach our texture to this context's resource cache. This ensures that deletion will happen
    // in the correct thread/context. This adds the only ref to the texture that will persist from
    // this point. To trigger GrTexture deletion a message is sent by generator dtor or by
    // makeProxy when it is invoked with a different context.
    //TODO: GrResourceCache should delete GrTexture, when GrContext is deleted. Currently
    //TODO: SkMessageBus ignores messages for deleted contexts and GrTexture will leak.
    context->contextPriv().getResourceCache()->insertCrossContextGpuResource(fOriginalTexture);
    return proxyProvider->createWrapped(std::move(tex), kTopLeft_GrSurfaceOrigin);
}

bool GrAHardwareBufferImageGenerator::onIsValid(GrContext* context) const {
    if (nullptr == context) {
        return false; //CPU backend is not supported, because hardware buffer can be swizzled
    }
    // TODO: add Vulkan support
    return kOpenGL_GrBackend == context->contextPriv().getBackend();
}

#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK
